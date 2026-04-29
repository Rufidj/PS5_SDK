#include "example_common.h"

/*
 * gnm_dma_fill3 — mismo DMA_DATA que v1 (probado, funciona),
 * pero sleep de 2s antes del flip para eliminar el timing como causa.
 *
 * Ademas intenta los dos addresses juntos en el mismo payload:
 *   A) CPU virtual address  → fill RED   (0x00FF0000)
 *   B) Physical address     → fill GREEN (0x0000FF00)
 *
 * Sin WRITE_DATA (el control word era invalido en RDNA2 y
 * el driver rechazaba el command buffer completo).
 *
 * Resultado esperado:
 *   Pantalla entera ROJA  → CPU VA correcto, v1 era timing issue
 *   Banda ROJA + resto negro → CPU VA parcialmente mapeado
 *   Pantalla entera VERDE → phys addr es el GPU addr correcto
 *   Todo negro → ambas direcciones incorrectas
 */

enum {
    FW = 1920, FH = 1080,
    FB_SZ  = FW * FH * 4,
    FB_AL  = (FB_SZ + 0x1FFFFF) & ~0x1FFFFF,
    CB_SZ  = 0x1000,
    F0_OFF = CB_SZ,
    F1_OFF = CB_SZ + FB_AL,
    DM_TOT = CB_SZ + FB_AL * 2,
    DM_AL  = (DM_TOT + 0x1FFFFF) & ~0x1FFFFF,
};

#define PM4_DMA_HDR  0xC0055000u   /* IT_DMA_DATA,  count-1=5  */
#define PM4_NOP      0xC0001000u
#define DMA_FILL     0xC0000000u   /* CP_SYNC | SRC=DATA | DST=L2 */

static s32 takeover_open(void *G, void *vid_open) {
    s32 types[4] = { 0xFF, 0, 1, 2 };
    for (int i = 0; i < 4; i++) {
        s32 h = (s32)NC(G, vid_open, (u64)types[i], 0, 0, 0, 0, 0);
        if (h >= 0) return h;
    }
    return -1;
}

static s32 dma_submit(void *G, void *fn_submit,
                      u32 *cb, u64 dst, u32 color, u32 bytes) {
    u32 n = 0;
    cb[n++] = PM4_DMA_HDR;
    cb[n++] = DMA_FILL;
    cb[n++] = color;
    cb[n++] = 0u;
    cb[n++] = (u32)(dst & 0xFFFFFFFFu);
    cb[n++] = (u32)(dst >> 32);
    cb[n++] = bytes;
    cb[n++] = PM4_NOP;
    u32  sz  = n * 4u;
    void *ptr = (void *)cb;
    return (s32)NC(G, fn_submit, 1, (u64)&ptr, (u64)&sz, 0, 0, 0);
}

__attribute__((section(".text._start")))
void _start(u64 eboot_base, u64 dlsym_addr, struct ext_args *ext) {
    if (!ext) return;
    ext->step = 1;

    struct ps5_example ex;
    s32 ret = ex_init(&ex, eboot_base, dlsym_addr, ext);
    if (ret != 0) { ext->status = ret; return; }

    ps5_sdk_load_sprx(ex.G, ex.load_mod, "libSceGnmDriver.sprx");
    void *fn_submit = SYM(ex.G, ex.D, GNMDRIVER_HANDLE, "sceGnmSubmitCommandBuffers");

    void *cancel_fn = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, KERNEL_scePthreadCancel);
    void *alloc_dm  = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, KERNEL_sceKernelAllocateDirectMemory);
    void *map_dm    = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, KERNEL_sceKernelMapDirectMemory);
    ps5_sdk_load_sprx(ex.G, ex.load_mod, "libSceVideoOut.sprx");
    void *vid_open  = SYM(ex.G, ex.D, VIDEOOUT_HANDLE, "sceVideoOutOpen");
    void *vid_close = SYM(ex.G, ex.D, VIDEOOUT_HANDLE, "sceVideoOutClose");
    void *vid_reg   = SYM(ex.G, ex.D, VIDEOOUT_HANDLE, "sceVideoOutRegisterBuffers");
    void *vid_flip  = SYM(ex.G, ex.D, VIDEOOUT_HANDLE, "sceVideoOutSubmitFlip");
    void *vid_rate  = SYM(ex.G, ex.D, VIDEOOUT_HANDLE, "sceVideoOutSetFlipRate");

    s32 emu_vid   = *(s32 *)(eboot_base + EBOOT_VIDOUT);
    u64 gs_thread = *(u64 *)(eboot_base + EBOOT_GS_THREAD);

    if (cancel_fn && gs_thread) NC(ex.G, cancel_fn, gs_thread, 0, 0, 0, 0, 0);
    if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 300000, 0, 0, 0, 0, 0);
    if (vid_close && emu_vid >= 0) NC(ex.G, vid_close, (u64)emu_vid, 0, 0, 0, 0, 0);
    if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 100000, 0, 0, 0, 0, 0);

    s32 video_h = vid_open ? takeover_open(ex.G, vid_open) : -1;

    s32 sub_a = 0x7F, sub_b = 0x7F, flip_a = 0x7F, flip_b = 0x7F;
    u64 phys = 0; void *mem = 0;

    if (!fn_submit || video_h < 0 || !alloc_dm || !map_dm || !vid_reg || !vid_flip)
        goto show;

    if ((s32)NC(ex.G, alloc_dm, 0, 0x300000000ULL, DM_AL, 0x200000, 3, (u64)&phys))
        goto show;
    NC(ex.G, map_dm, (u64)&mem, DM_AL, 0x33, 0, phys, 0x200000);
    if (!mem) goto show;

    {
        u8 *base = (u8 *)mem;
        void *fbs[2] = { base + F0_OFF, base + F1_OFF };
        u8 attr[64]; ex_zero(attr, sizeof(attr));
        *(u32 *)(attr +  0) = 0x80000000;
        *(u32 *)(attr +  4) = 1;
        *(u32 *)(attr + 12) = FW;
        *(u32 *)(attr + 16) = FH;
        *(u32 *)(attr + 20) = FW;
        if ((s32)NC(ex.G, vid_reg, (u64)video_h, 0, (u64)fbs, 2, (u64)attr, 0))
            goto show;
        if (vid_rate) NC(ex.G, vid_rate, (u64)video_h, 0, 0, 0, 0, 0);

        u32 *cb  = (u32 *)base;
        u32  fbsz = FW * FH * 4u;

        /* ── A: CPU virtual address ─────────────────────────────────── */
        sub_a = dma_submit(ex.G, fn_submit, cb, (u64)fbs[0],
                           0x00FF0000u /* RED */, fbsz);
        if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 2000000, 0, 0, 0, 0, 0);
        flip_a = (s32)NC(ex.G, vid_flip, (u64)video_h, 0, 1, 0, 0, 0);
        if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 2000000, 0, 0, 0, 0, 0);

        /* ── B: physical address ────────────────────────────────────── */
        sub_b = dma_submit(ex.G, fn_submit, cb,
                           phys + (u64)F0_OFF,
                           0x0000FF00u /* GREEN */, fbsz);
        if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 2000000, 0, 0, 0, 0, 0);
        flip_b = (s32)NC(ex.G, vid_flip, (u64)video_h, 0, 1, 0, 0, 0);
        if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 2000000, 0, 0, 0, 0, 0);
    }

show:
    if (vid_close && video_h >= 0) NC(ex.G, vid_close, (u64)video_h, 0, 0, 0, 0, 0);
    ext->dbg[0] = phys;
    ext->dbg[1] = (u64)mem;
    ext->status = 0;

    u8 *buf = ex_alloc_dialog(&ex);
    if (!buf) { ext->status = -3; return; }
    ex_dialog_begin(&ex);
    ret = ex_open_text(&ex, buf,
        "gnm_dma_fill3\n"
        "sub_A(cpuva)=%x  sub_B(phys)=%x\n"
        "flip_A=%x  flip_B=%x\n"
        "phys=%lx",
        sub_a, sub_b, flip_a, flip_b, phys);
    if (ret == 0) ex_wait_user_close(&ex);
    ex_close_dialog(&ex);
    ex_free_dialog(&ex, buf);
    ext->step = 99;
}
