#include "example_common.h"

/*
 * gnm_wd_only — test minimo absoluto de ejecucion GPU.
 *
 * Sin VideoOut, sin DMA_DATA, sin framebuffer.
 * Solo: CB ONION + un IT_WRITE_DATA + esperar fence.
 *
 * Correcciones respecto a gnm_onion_probe:
 *   - CB ONION ahora en [0, 0x300000000] (el rango [0, 0x100000000]
 *     esta reservado por el sistema en PS5 y falla la alloc).
 *   - Fence en el mismo bloque ONION.
 *   - Sin vid_reg ni VideoOut (eliminamos variables de confusion).
 *
 * Si fence=0x12345678 → el GPU ejecuta nuestros PM4. Ir a paso siguiente.
 * Si fence=0          → el GPU no procesa nuestras submissions en este modo.
 *
 * Ademas probamos un segundo intento con IT_WRITE_DATA DST_SEL=0x1 (SYNC)
 * por si DST_SEL=2 no es el correcto para RDNA2.
 */

enum {
    CB_SZ    = 0x2000,           /* 8 KB ONION, dos CBs */
    CB_A_OFF = 0,                /* CB intento A en offset 0    */
    CB_B_OFF = 0x1000,           /* CB intento B en offset 4KB  */
    FA_OFF   = CB_SZ - 32,       /* fence A                     */
    FB_OFF   = CB_SZ - 16,       /* fence B                     */
};

#define PM4_NOP     0xC0001000u
#define PM4_WD_HDR  0xC0033700u   /* IT_WRITE_DATA, count-1=3   */
#define WD_DST_SEL2 0x00020000u   /* DST_SEL=2 (TC_L2, RAM)     */
#define WD_DST_SEL1 0x00010000u   /* DST_SEL=1 (MEMORY_SYNC)    */

__attribute__((section(".text._start")))
void _start(u64 eboot_base, u64 dlsym_addr, struct ext_args *ext) {
    if (!ext) return;
    ext->step = 1;

    struct ps5_example ex;
    s32 ret = ex_init(&ex, eboot_base, dlsym_addr, ext);
    if (ret != 0) { ext->status = ret; return; }

    ps5_sdk_load_sprx(ex.G, ex.load_mod, "libSceGnmDriver.sprx");
    void *fn_submit = SYM(ex.G, ex.D, GNMDRIVER_HANDLE, "sceGnmSubmitCommandBuffers");

    void *alloc_dm = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, KERNEL_sceKernelAllocateDirectMemory);
    void *map_dm   = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, KERNEL_sceKernelMapDirectMemory);

    u64  cb_phys = 0;
    void *cb_mem = 0;
    s32  alloc_ret = 0x7F;
    s32  sub_a = 0x7F, sub_b = 0x7F;
    u32  fence_a = 0, fence_b = 0;

    if (!fn_submit || !alloc_dm || !map_dm) goto show;

    /* Alloc 8 KB ONION en [0, 12 GB] — rango amplio, evita fallo */
    alloc_ret = (s32)NC(ex.G, alloc_dm,
                         0, 0x300000000ULL, CB_SZ, 0x1000, 1, (u64)&cb_phys);
    if (alloc_ret != 0) goto show;

    NC(ex.G, map_dm, (u64)&cb_mem, CB_SZ, 0x33, 0, cb_phys, 0x1000);
    if (!cb_mem) goto show;

    {
        volatile u32 *fa = (volatile u32 *)((u8 *)cb_mem + FA_OFF);
        volatile u32 *fb = (volatile u32 *)((u8 *)cb_mem + FB_OFF);
        *fa = 0u; *fb = 0u;

        u64 fa_va = (u64)(fa);
        u64 fb_va = (u64)(fb);

        /* ── A: DST_SEL=2 (TC_L2) ───────────────────────────────────── */
        u32 *pm4a = (u32 *)((u8 *)cb_mem + CB_A_OFF);
        u32 na = 0;
        pm4a[na++] = PM4_WD_HDR;
        pm4a[na++] = WD_DST_SEL2;
        pm4a[na++] = (u32)(fa_va & 0xFFFFFFFFu);
        pm4a[na++] = (u32)(fa_va >> 32);
        pm4a[na++] = 0x12345678u;
        pm4a[na++] = PM4_NOP;
        u32  sza  = na * 4u;
        void *pa  = (void *)pm4a;
        sub_a = (s32)NC(ex.G, fn_submit, 1, (u64)&pa, (u64)&sza, 0, 0, 0);

        for (u32 i = 0; i < 2000u && *fa != 0x12345678u; i++)
            if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 1000, 0, 0, 0, 0, 0);
        fence_a = *fa;

        /* ── B: DST_SEL=1 (MEMORY_SYNC) ─────────────────────────────── */
        u32 *pm4b = (u32 *)((u8 *)cb_mem + CB_B_OFF);
        u32 nb = 0;
        pm4b[nb++] = PM4_WD_HDR;
        pm4b[nb++] = WD_DST_SEL1;
        pm4b[nb++] = (u32)(fb_va & 0xFFFFFFFFu);
        pm4b[nb++] = (u32)(fb_va >> 32);
        pm4b[nb++] = 0xABCDEF01u;
        pm4b[nb++] = PM4_NOP;
        u32  szb  = nb * 4u;
        void *pb  = (void *)pm4b;
        sub_b = (s32)NC(ex.G, fn_submit, 1, (u64)&pb, (u64)&szb, 0, 0, 0);

        for (u32 i = 0; i < 2000u && *fb != 0xABCDEF01u; i++)
            if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 1000, 0, 0, 0, 0, 0);
        fence_b = *fb;
    }

show:
    ext->dbg[0] = cb_phys;
    ext->dbg[1] = (u64)cb_mem;
    ext->dbg[2] = ((u64)(u32)sub_a << 32) | (u32)sub_b;
    ext->dbg[3] = ((u64)fence_a << 32)    | fence_b;
    ext->status = 0;

    u8 *buf = ex_alloc_dialog(&ex);
    if (!buf) { ext->status = -3; return; }
    ex_dialog_begin(&ex);
    ret = ex_open_text(&ex, buf,
        "gnm_wd_only\n"
        "alloc=%x\n"
        "cb=%p phys=%lx\n"
        "sub_A(sel2)=%x sub_B(sel1)=%x\n"
        "fence_A=%x\n"
        "fence_B=%x\n"
        "12345678=GPU ejecuta A\n"
        "ABCDEF01=GPU ejecuta B",
        (u32)alloc_ret,
        cb_mem, cb_phys,
        (u32)sub_a, (u32)sub_b,
        fence_a, fence_b);
    if (ret == 0) ex_wait_user_close(&ex);
    ex_close_dialog(&ex);
    ex_free_dialog(&ex, buf);
    ext->step = 99;
}
