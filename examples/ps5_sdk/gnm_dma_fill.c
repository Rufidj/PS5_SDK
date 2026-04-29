#include "example_common.h"

/*
 * gnm_dma_fill — primer render GPU real.
 *
 * Usa un paquete PM4 IT_DMA_DATA para que el DMA engine de la GPU
 * llene el framebuffer con un color solido sin ningun shader.
 *
 * PM4 IT_DMA_DATA (opcode 0x50, GCN4 / RDNA / RDNA2):
 *   HEADER  = (type3<<30) | ((count-1)<<16) | (opcode<<8)
 *           = (3<<30) | (5<<16) | (0x50<<8) = 0xC0055000
 *   ATTR0   = CP_SYNC(1<<31) | SRC_SEL=DATA(2<<29) | DST_SEL=L2(0<<20)
 *           = 0xC0000000
 *   FILL_LO = color value (32-bit, repetido en cada pixel)
 *   FILL_HI = 0 (solo 32-bit fill)
 *   DST_LO  = GPU virtual address low 32 bits
 *   DST_HI  = GPU virtual address high 32 bits
 *   BYTECOUNT = width * height * 4
 *
 * El command buffer se coloca en la misma region DirectMemory que los
 * framebuffers (offset 0, antes de ellos).
 *
 * Secuencia:
 *   1. VideoOut takeover (igual que siempre)
 *   2. AllocDM: 4KB CB + 2x FB_ALIGNED framebuffers
 *   3. RegisterBuffers
 *   4. Escribir PM4 en CB
 *   5. sceGnmSubmitCommandBuffers
 *   6. usleep(80ms) — esperar a que la GPU termine
 *   7. sceVideoOutSubmitFlip
 *   8. Mantener 3 s y mostrar dialog con resultados
 */

enum {
    FW = 1920, FH = 1080,
    FB_SZ  = FW * FH * 4,
    FB_AL  = (FB_SZ + 0x1FFFFF) & ~0x1FFFFF,
    CB_OFF = 0,               /* command buffer at offset 0   */
    CB_SZ  = 0x1000,          /* 4 KB, ample for a few PM4    */
    F0_OFF = CB_SZ,           /* framebuffer 0 after CB       */
    F1_OFF = CB_SZ + FB_AL,   /* framebuffer 1                */
    DM_TOT = CB_SZ + FB_AL * 2,
    DM_AL  = (DM_TOT + 0x1FFFFF) & ~0x1FFFFF,
};

/* PM4 packet constants (RDNA2 / GCN compatible) */
#define PM4_NOP          0xC0001000u   /* IT_NOP,       count-1=0  */
#define PM4_DMA_HDR      0xC0055000u   /* IT_DMA_DATA,  count-1=5  */
#define DMA_ATTR0_FILL   0xC0000000u   /* CP_SYNC|SRC=DATA|DST=L2  */

static s32 takeover_open(void *G, void *vid_open) {
    s32 types[4] = { 0xFF, 0, 1, 2 };
    for (int i = 0; i < 4; i++) {
        s32 h = (s32)NC(G, vid_open, (u64)types[i], 0, 0, 0, 0, 0);
        if (h >= 0) return h;
    }
    return -1;
}

__attribute__((section(".text._start")))
void _start(u64 eboot_base, u64 dlsym_addr, struct ext_args *ext) {
    if (!ext) return;
    ext->step = 1;

    struct ps5_example ex;
    s32 ret = ex_init(&ex, eboot_base, dlsym_addr, ext);
    if (ret != 0) { ext->status = ret; return; }

    /* ── GnmDriver ────────────────────────────────────────────────────── */
    ps5_sdk_load_sprx(ex.G, ex.load_mod, "libSceGnmDriver.sprx");
    void *fn_submit = SYM(ex.G, ex.D, GNMDRIVER_HANDLE, "sceGnmSubmitCommandBuffers");
    void *fn_flush  = SYM(ex.G, ex.D, GNMDRIVER_HANDLE, "sceGnmFlushGarlic");

    /* ── VideoOut symbols ─────────────────────────────────────────────── */
    void *cancel_fn = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, KERNEL_scePthreadCancel);
    void *alloc_dm  = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, KERNEL_sceKernelAllocateDirectMemory);
    void *map_dm    = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, KERNEL_sceKernelMapDirectMemory);
    ps5_sdk_load_sprx(ex.G, ex.load_mod, "libSceVideoOut.sprx");
    void *vid_open  = SYM(ex.G, ex.D, VIDEOOUT_HANDLE, "sceVideoOutOpen");
    void *vid_close = SYM(ex.G, ex.D, VIDEOOUT_HANDLE, "sceVideoOutClose");
    void *vid_reg   = SYM(ex.G, ex.D, VIDEOOUT_HANDLE, "sceVideoOutRegisterBuffers");
    void *vid_flip  = SYM(ex.G, ex.D, VIDEOOUT_HANDLE, "sceVideoOutSubmitFlip");
    void *vid_rate  = SYM(ex.G, ex.D, VIDEOOUT_HANDLE, "sceVideoOutSetFlipRate");

    /* ── VideoOut takeover ────────────────────────────────────────────── */
    s32 emu_vid   = *(s32 *)(eboot_base + EBOOT_VIDOUT);
    u64 gs_thread = *(u64 *)(eboot_base + EBOOT_GS_THREAD);

    if (cancel_fn && gs_thread) NC(ex.G, cancel_fn, gs_thread, 0, 0, 0, 0, 0);
    if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 300000, 0, 0, 0, 0, 0);
    if (vid_close && emu_vid >= 0) NC(ex.G, vid_close, (u64)emu_vid, 0, 0, 0, 0, 0);
    if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 100000, 0, 0, 0, 0, 0);

    s32 video_h = vid_open ? takeover_open(ex.G, vid_open) : -1;

    u64  phys = 0;
    void *mem  = 0;
    s32  submit_ret = 0x7FFFFFFF;
    s32  flip_ret   = 0x7FFFFFFF;

    if (!fn_submit || video_h < 0 || !alloc_dm || !map_dm || !vid_reg || !vid_flip)
        goto show_result;

    /* ── Allocate one DirectMemory region (CB + 2 FBs) ───────────────── */
    {
        s32 ar = (s32)NC(ex.G, alloc_dm,
                          0, 0x300000000ULL, DM_AL, 0x200000, 3, (u64)&phys);
        if (ar != 0) goto show_result;
        NC(ex.G, map_dm, (u64)&mem, DM_AL, 0x33, 0, phys, 0x200000);
        if (!mem) goto show_result;
    }

    {
        u8 *base = (u8 *)mem;

        /* ── VideoOut RegisterBuffers ─────────────────────────────────── */
        void *fbs[2] = { base + F0_OFF, base + F1_OFF };
        u8    attr[64];
        ex_zero(attr, sizeof(attr));
        *(u32 *)(attr +  0) = 0x80000000;
        *(u32 *)(attr +  4) = 1;
        *(u32 *)(attr + 12) = FW;
        *(u32 *)(attr + 16) = FH;
        *(u32 *)(attr + 20) = FW;

        s32 rr = (s32)NC(ex.G, vid_reg, (u64)video_h, 0, (u64)fbs, 2, (u64)attr, 0);
        if (rr != 0) goto show_result;
        if (vid_rate) NC(ex.G, vid_rate, (u64)video_h, 0, 0, 0, 0, 0);

        /* ── Build PM4 command buffer ─────────────────────────────────── */
        /*
         * Fill framebuffer[0] with a bright GPU-rendered red
         * (0x00FF0000 = R=0xFF, G=0, B=0 in our BGRA8 layout = RED on screen)
         * so it's unmistakably different from any CPU-rendered frame.
         */
        u64  fb0_va   = (u64)fbs[0];   /* GPU virtual == CPU virtual      */
        u32  fill_val = 0x00FF0000u;   /* RED                             */
        u32  bytecount = FW * FH * 4u;

        u32 *cb = (u32 *)base;         /* command buffer at offset 0      */
        cb[0] = PM4_DMA_HDR;           /* IT_DMA_DATA, 6 body dwords      */
        cb[1] = DMA_ATTR0_FILL;        /* CP_SYNC | SRC=DATA | DST=L2     */
        cb[2] = fill_val;              /* fill pattern                    */
        cb[3] = 0u;                    /* src hi (ignored in DATA mode)   */
        cb[4] = (u32)(fb0_va & 0xFFFFFFFFu);   /* dst lo                 */
        cb[5] = (u32)(fb0_va >> 32);            /* dst hi                 */
        cb[6] = bytecount;             /* bytes to fill                   */
        cb[7] = PM4_NOP;               /* trailing NOP                    */

        u32  cb_bytes = 8u * 4u;       /* 8 dwords = 32 bytes             */
        void *cb_ptr  = (void *)cb;

        /* ── sceGnmSubmitCommandBuffers ──────────────────────────────── */
        submit_ret = (s32)NC(ex.G, fn_submit,
                              1,
                              (u64)&cb_ptr,
                              (u64)&cb_bytes,
                              0, 0, 0);

        /* flush Garlic cache if available */
        if (fn_flush) NC(ex.G, fn_flush, 0, 0, 0, 0, 0, 0);

        /* wait for GPU DMA to complete (~2 frames at 60 fps is plenty)  */
        if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 80000, 0, 0, 0, 0, 0);

        /* ── Flip to show the GPU-rendered frame ─────────────────────── */
        flip_ret = (s32)NC(ex.G, vid_flip, (u64)video_h, 0, 1, 0, 0, 0);

        /* hold for ~3 seconds so the result is visible */
        if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 3000000, 0, 0, 0, 0, 0);
    }

show_result:
    if (vid_close && video_h >= 0) NC(ex.G, vid_close, (u64)video_h, 0, 0, 0, 0, 0);

    ext->dbg[0] = (u64)(u32)submit_ret;
    ext->dbg[1] = (u64)(u32)flip_ret;
    ext->dbg[2] = phys;
    ext->dbg[3] = (u64)mem;
    ext->status = 0;

    u8 *buf = ex_alloc_dialog(&ex);
    if (!buf) { ext->status = -3; return; }
    ex_dialog_begin(&ex);
    ret = ex_open_text(&ex, buf,
        "gnm_dma_fill\n"
        "submit=%x  flip=%x\n"
        "phys=%lx\n"
        "mem=%p\n"
        "submit=0 -> GPU DMA ok\n"
        "Red screen = GPU render ok!",
        (u32)submit_ret, (u32)flip_ret,
        phys, mem);
    if (ret == 0) ex_wait_user_close(&ex);
    ex_close_dialog(&ex);
    ex_free_dialog(&ex, buf);
    ext->step = 99;
}
