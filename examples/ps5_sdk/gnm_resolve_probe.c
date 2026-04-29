#include "example_common.h"

/*
 * gnm_resolve_probe — primera sonda de la GPU real.
 *
 * Fase 1: resolve por nombre ~15 funciones clave de GnmDriver.
 * Fase 2: si sceGnmSubmitCommandBuffers resuelve, intenta enviar
 *         un command buffer con un único paquete PM4 NOP (completamente
 *         inofensivo: la GPU lo ignora).
 *
 * PM4 NOP (RDNA2 / GCN compatible):
 *   0xC0001000 = type3 | count=0 | opcode=IT_NOP(0x10) | shader_type=0
 *
 * Firma de sceGnmSubmitCommandBuffers (misma que PS4):
 *   int submit(u32 count,
 *              const void *const *dcbGpuAddrs,
 *              const u32 *dcbSizes,
 *              const void *const *ccbGpuAddrs,  // NULL = sin CCB
 *              const u32 *ccbSizes);
 */

/* Helper: 1 si fn != NULL */
static u32 R(void *fn) { return fn ? 1u : 0u; }

__attribute__((section(".text._start")))
void _start(u64 eboot_base, u64 dlsym_addr, struct ext_args *ext) {
    if (!ext) return;
    ext->step = 1;

    struct ps5_example ex;
    s32 ret = ex_init(&ex, eboot_base, dlsym_addr, ext);
    if (ret != 0) { ext->status = ret; return; }

    ps5_sdk_load_sprx(ex.G, ex.load_mod, "libSceGnmDriver.sprx");

    /* ── Submit / flip ─────────────────────────────────────────────────── */
    void *fn_submit   = SYM(ex.G, ex.D, GNMDRIVER_HANDLE, "sceGnmSubmitCommandBuffers");
    void *fn_flip_sub = SYM(ex.G, ex.D, GNMDRIVER_HANDLE, "sceGnmSubmitAndFlipCommandBuffers");
    void *fn_sub_wkld = SYM(ex.G, ex.D, GNMDRIVER_HANDLE, "sceGnmSubmitCommandBuffersForWorkload");
    void *fn_done     = SYM(ex.G, ex.D, GNMDRIVER_HANDLE, "sceGnmSubmitDone");
    void *fn_req_flip = SYM(ex.G, ex.D, GNMDRIVER_HANDLE, "sceGnmRequestFlipAndSubmitDone");

    /* ── Control / sync ────────────────────────────────────────────────── */
    void *fn_flush    = SYM(ex.G, ex.D, GNMDRIVER_HANDLE, "sceGnmFlushGarlic");
    void *fn_wait_rdr = SYM(ex.G, ex.D, GNMDRIVER_HANDLE, "sceGnmWaitUntilSafeForRendering");
    void *fn_beg_wkld = SYM(ex.G, ex.D, GNMDRIVER_HANDLE, "sceGnmBeginWorkload");
    void *fn_end_wkld = SYM(ex.G, ex.D, GNMDRIVER_HANDLE, "sceGnmEndWorkload");
    void *fn_clock    = SYM(ex.G, ex.D, GNMDRIVER_HANDLE, "sceGnmGetGpuCoreClockFrequency");
    void *fn_debug    = SYM(ex.G, ex.D, GNMDRIVER_HANDLE, "sceGnmDebugHardwareStatus");

    /* ── Shader update ─────────────────────────────────────────────────── */
    void *fn_cs  = SYM(ex.G, ex.D, GNMDRIVER_HANDLE, "sceGnmUpdateCsShader");
    void *fn_vs  = SYM(ex.G, ex.D, GNMDRIVER_HANDLE, "sceGnmUpdateVsShader");
    void *fn_ps  = SYM(ex.G, ex.D, GNMDRIVER_HANDLE, "sceGnmUpdatePsShader");
    void *fn_vgt = SYM(ex.G, ex.D, GNMDRIVER_HANDLE, "sceGnmSetVgtControl");

    /* ── GPU clock (sanity check) ──────────────────────────────────────── */
    u64 clock_hz = fn_clock
        ? (u64)NC(ex.G, fn_clock, 0, 0, 0, 0, 0, 0)
        : 0ULL;

    /* ── NOP submit attempt ────────────────────────────────────────────── */
    s32  nop_ret  = 0x7FFFFFFF; /* sentinel = not attempted */
    u64  nop_phys = 0;
    void *nop_vmem = 0;

    void *alloc_dm = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, KERNEL_sceKernelAllocateDirectMemory);
    void *map_dm   = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, KERNEL_sceKernelMapDirectMemory);

    if (fn_submit && alloc_dm && map_dm) {
        /* Allocate 64 KB of CPU+GPU visible DirectMemory (prot=0x33) */
        s32 ar = (s32)NC(ex.G, alloc_dm,
                          0, 0x100000000ULL, 0x10000, 0x1000, 3, (u64)&nop_phys);
        if (ar == 0) {
            NC(ex.G, map_dm,
               (u64)&nop_vmem, 0x10000, 0x33, 0, nop_phys, 0x1000);
        }
        if (nop_vmem) {
            /* PM4 command buffer: single IT_NOP header (0 body dwords).
             * 0xC0001000 = (3<<30)|(0<<16)|(0x10<<8)|0  */
            u32 *cb = (u32 *)nop_vmem;
            cb[0] = 0xC0001000u;                /* IT_NOP                  */

            u32  cb_size = 4;                   /* 1 dword = 4 bytes       */
            void *cb_ptr = nop_vmem;

            /* sceGnmSubmitCommandBuffers(1, &cb_ptr, &cb_size, NULL, NULL) */
            nop_ret = (s32)NC(ex.G, fn_submit,
                              1,
                              (u64)&cb_ptr,
                              (u64)&cb_size,
                              0, 0, 0);
        }
    }

    /* store raw pointers for inspection */
    ext->dbg[0] = (u64)fn_submit;
    ext->dbg[1] = (u64)fn_flip_sub;
    ext->dbg[2] = (u64)fn_cs;
    ext->dbg[3] = clock_hz;
    ext->dbg[4] = nop_phys;
    ext->dbg[5] = (u64)(u32)nop_ret;
    ext->status = 0;

    /* ── Dialog ────────────────────────────────────────────────────────── */
    u8 *buf = ex_alloc_dialog(&ex);
    if (!buf) { ext->status = -3; return; }
    ex_dialog_begin(&ex);
    ret = ex_open_text(&ex, buf,
        /* submit / flip */
        "GnmDriver resolve probe\n"
        "submit=%u flip_sub=%u done=%u\n"
        "sub_wkld=%u req_flip=%u\n"
        /* control */
        "flush=%u wait_rdr=%u\n"
        "beg_wkld=%u end_wkld=%u\n"
        "clock=%u debug=%u\n"
        /* shaders */
        "cs=%u vs=%u ps=%u vgt=%u\n"
        /* results */
        "gpu_mhz=%u\n"
        "nop_ret=%x",
        R(fn_submit),   R(fn_flip_sub), R(fn_done),
        R(fn_sub_wkld), R(fn_req_flip),
        R(fn_flush),    R(fn_wait_rdr),
        R(fn_beg_wkld), R(fn_end_wkld),
        R(fn_clock),    R(fn_debug),
        R(fn_cs),       R(fn_vs), R(fn_ps), R(fn_vgt),
        (u32)(clock_hz / 1000000u),
        (u32)nop_ret
    );
    if (ret == 0) ex_wait_user_close(&ex);
    ex_close_dialog(&ex);
    ex_free_dialog(&ex, buf);
    ext->step = 99;
}
