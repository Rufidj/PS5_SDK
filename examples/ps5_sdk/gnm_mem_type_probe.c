#include "example_common.h"

/*
 * gnm_mem_type_probe — dos diagnosticos en uno:
 *
 * 1) Busca el tipo de memoria coherente valido en PS5:
 *    type=1 devuelve EINVAL (no existe en PS5).
 *    Probamos type=0 y type=2.
 *
 * 2) Prueba sceGnmBeginWorkload + submit + sceGnmEndWorkload:
 *    En PS5 fw13 el driver puede requerir este contexto para
 *    despachar comandos al GPU. Sin el, submit=0 pero GPU ignora el CB.
 *
 * Resultados:
 *   alloc0 / alloc2 = 0  → tipo valido, usar para CB
 *   fence=12345678       → GPU ejecuta con WorkLoad context
 *   fence=0              → GPU sigue sin ejecutar, problema mas profundo
 */

enum {
    CB_SZ    = 0x2000,
    FENCE_OFF = CB_SZ - 16,
};

#define PM4_NOP     0xC0001000u
#define PM4_WD_HDR  0xC0033700u
#define WD_DST_SEL2 0x00020000u

__attribute__((section(".text._start")))
void _start(u64 eboot_base, u64 dlsym_addr, struct ext_args *ext) {
    if (!ext) return;
    ext->step = 1;

    struct ps5_example ex;
    s32 ret = ex_init(&ex, eboot_base, dlsym_addr, ext);
    if (ret != 0) { ext->status = ret; return; }

    ps5_sdk_load_sprx(ex.G, ex.load_mod, "libSceGnmDriver.sprx");
    void *fn_submit  = SYM(ex.G, ex.D, GNMDRIVER_HANDLE, "sceGnmSubmitCommandBuffers");
    void *fn_beg_wkl = SYM(ex.G, ex.D, GNMDRIVER_HANDLE, "sceGnmBeginWorkload");
    void *fn_end_wkl = SYM(ex.G, ex.D, GNMDRIVER_HANDLE, "sceGnmEndWorkload");

    void *alloc_dm = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, KERNEL_sceKernelAllocateDirectMemory);
    void *map_dm   = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, KERNEL_sceKernelMapDirectMemory);

    /* ── 1. Probe memory types ────────────────────────────────────────── */
    u64  phys0 = 0, phys2 = 0, phys3 = 0;
    void *mem0 = 0, *mem2 = 0, *mem3 = 0;
    s32  r0 = 0x7F, r2 = 0x7F, r3 = 0x7F;

    if (!alloc_dm || !map_dm) goto show;

    r0 = (s32)NC(ex.G, alloc_dm, 0, 0x300000000ULL, CB_SZ, 0x1000, 0, (u64)&phys0);
    if (r0 == 0) NC(ex.G, map_dm, (u64)&mem0, CB_SZ, 0x33, 0, phys0, 0x1000);

    r2 = (s32)NC(ex.G, alloc_dm, 0, 0x300000000ULL, CB_SZ, 0x1000, 2, (u64)&phys2);
    if (r2 == 0) NC(ex.G, map_dm, (u64)&mem2, CB_SZ, 0x33, 0, phys2, 0x1000);

    r3 = (s32)NC(ex.G, alloc_dm, 0, 0x300000000ULL, CB_SZ, 0x1000, 3, (u64)&phys3);
    if (r3 == 0) NC(ex.G, map_dm, (u64)&mem3, CB_SZ, 0x33, 0, phys3, 0x1000);

    /* ── 2. GPU submit con el primer tipo valido + WorkLoad context ───── */
    s32  sub_ret   = 0x7F;
    s32  beg_ret   = 0x7F;
    s32  end_ret   = 0x7F;
    u32  fence_val = 0;
    void *cb_used  = 0;

    /* Elegir el mejor CB: preferimos type=0 (mas coherente), luego 2, 3 */
    if      (r0 == 0 && mem0) cb_used = mem0;
    else if (r2 == 0 && mem2) cb_used = mem2;
    else if (r3 == 0 && mem3) cb_used = mem3;

    if (!fn_submit || !cb_used) goto show;

    {
        volatile u32 *fence = (volatile u32 *)((u8 *)cb_used + FENCE_OFF);
        *fence = 0u;
        u64 fence_va = (u64)(fence);

        u32 *pm4 = (u32 *)cb_used;
        u32  n   = 0;
        pm4[n++] = PM4_WD_HDR;
        pm4[n++] = WD_DST_SEL2;
        pm4[n++] = (u32)(fence_va & 0xFFFFFFFFu);
        pm4[n++] = (u32)(fence_va >> 32);
        pm4[n++] = 0x12345678u;
        pm4[n++] = PM4_NOP;
        u32  cb_sz = n * 4u;
        void *cb_ptr = (void *)pm4;

        /* Intentar BeginWorkload si resuelve */
        u8 wkl_handle[64];
        ex_zero(wkl_handle, sizeof(wkl_handle));
        if (fn_beg_wkl)
            beg_ret = (s32)NC(ex.G, fn_beg_wkl,
                               (u64)wkl_handle, 0, 0, 0, 0, 0);

        sub_ret = (s32)NC(ex.G, fn_submit, 1,
                           (u64)&cb_ptr, (u64)&cb_sz, 0, 0, 0);

        if (fn_end_wkl)
            end_ret = (s32)NC(ex.G, fn_end_wkl,
                               (u64)wkl_handle, 0, 0, 0, 0, 0);

        for (u32 i = 0; i < 2000u && *fence != 0x12345678u; i++)
            if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 1000, 0, 0, 0, 0, 0);
        fence_val = *fence;
    }

show:
    ext->status = 0;

    u8 *buf = ex_alloc_dialog(&ex);
    if (!buf) { ext->status = -3; return; }
    ex_dialog_begin(&ex);
    ret = ex_open_text(&ex, buf,
        "gnm_mem_type_probe\n"
        "type0=%x type2=%x type3=%x\n"
        "beg=%x sub=%x end=%x\n"
        "fence=%x\n"
        "12345678=GPU ejecuta\n"
        "beg/end=7f si no resuelven",
        (u32)r0, (u32)r2, (u32)r3,
        (u32)beg_ret, (u32)sub_ret, (u32)end_ret,
        fence_val);
    if (ret == 0) ex_wait_user_close(&ex);
    ex_close_dialog(&ex);
    ex_free_dialog(&ex, buf);
    ext->step = 99;
}
