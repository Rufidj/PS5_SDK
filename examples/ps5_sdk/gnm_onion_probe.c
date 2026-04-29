#include "example_common.h"

/*
 * gnm_onion_probe — diagnostico de tipo de memoria para command buffers.
 *
 * Hipotesis: los command buffers deben estar en WB_ONION (type=1),
 * no en WC_GARLIC (type=3).  El GPU Command Processor no puede leer
 * instrucciones PM4 desde memoria write-combining.
 *
 * Este payload:
 *   1. Aloca CB en ONION  (alloc type=1, WB write-back)
 *   2. Aloca FB en GARLIC (alloc type=3, WC write-combining)  — igual que siempre
 *   3. Escribe en el CB:
 *        IT_WRITE_DATA (DST_SEL=2=TC_L2) → fence=0xBEEF0001 a ONION
 *        IT_DMA_DATA   (DMA fill RED)    → CPU VA del framebuffer
 *        IT_WRITE_DATA (DST_SEL=2=TC_L2) → fence=0xBEEF0002 a ONION
 *        IT_NOP
 *   4. CPU spin-wait hasta fence==0xBEEF0002 (timeout 2s)
 *   5. Flip + hold 3s
 *   6. Dialog con resultados
 *
 * IT_WRITE_DATA (opcode 0x37):
 *   header  = (3<<30)|(3<<16)|(0x37<<8) = 0xC0033700
 *   CONTROL = DST_SEL=2 @ [20:16] = 0x00020000  (TC_L2 = RAM via L2 cache)
 *   dst_lo, dst_hi, value
 *
 * IT_DMA_DATA (opcode 0x50):
 *   header  = 0xC0055000  (count-1=5)
 *   ATTR0   = 0xC0000000  (CP_SYNC | SRC=DATA | DST=L2)
 *   fill, src_hi, dst_lo, dst_hi, bytecount
 */

enum {
    FW = 1920, FH = 1080,
    FB_SZ  = FW * FH * 4,
    FB_AL  = (FB_SZ + 0x1FFFFF) & ~0x1FFFFF,
    /* Command buffer: 4KB ONION */
    CB_SZ  = 0x1000,
    /* Framebuffers: 2x FB_AL GARLIC */
    F0_OFF = 0,
    F1_OFF = FB_AL,
    FB_TOT_AL = FB_AL * 2,
    /* Fence lives at end of CB region */
    FENCE_OFF = CB_SZ - 16,
};

/* PM4 constants */
#define PM4_NOP      0xC0001000u
#define PM4_WD_HDR   0xC0033700u   /* IT_WRITE_DATA, body=4  */
#define WD_TC_L2     0x00020000u   /* DST_SEL=2 (TC_L2, RAM) */
#define PM4_DMA_HDR  0xC0055000u   /* IT_DMA_DATA,  body=6   */
#define DMA_FILL     0xC0000000u   /* CP_SYNC|SRC=DATA|DST=L2 */

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

    s32  sub_ret   = 0x7F;
    s32  flip_ret  = 0x7F;
    u32  fence_val = 0;
    u64  cb_phys   = 0, fb_phys = 0;
    void *cb_mem   = 0, *fb_mem = 0;

    if (!fn_submit || video_h < 0 || !alloc_dm || !map_dm || !vid_reg || !vid_flip)
        goto show;

    /* ── 1. Allocate CB in ONION (type=1, write-back coherent) ───────── */
    if ((s32)NC(ex.G, alloc_dm, 0, 0x100000000ULL, CB_SZ, 0x1000, 1, (u64)&cb_phys))
        goto show;
    NC(ex.G, map_dm, (u64)&cb_mem, CB_SZ, 0x33, 0, cb_phys, 0x1000);
    if (!cb_mem) goto show;

    /* ── 2. Allocate FBs in GARLIC (type=3, write-combining) ─────────── */
    if ((s32)NC(ex.G, alloc_dm, 0, 0x300000000ULL, FB_TOT_AL, 0x200000, 3, (u64)&fb_phys))
        goto show;
    NC(ex.G, map_dm, (u64)&fb_mem, FB_TOT_AL, 0x33, 0, fb_phys, 0x200000);
    if (!fb_mem) goto show;

    {
        u8 *cb  = (u8 *)cb_mem;
        u8 *fb0 = (u8 *)fb_mem + F0_OFF;
        u8 *fb1 = (u8 *)fb_mem + F1_OFF;

        void *fbs[2] = { fb0, fb1 };
        u8 attr[64]; ex_zero(attr, sizeof(attr));
        *(u32 *)(attr +  0) = 0x80000000;
        *(u32 *)(attr +  4) = 1;
        *(u32 *)(attr + 12) = FW;
        *(u32 *)(attr + 16) = FH;
        *(u32 *)(attr + 20) = FW;
        if ((s32)NC(ex.G, vid_reg, (u64)video_h, 0, (u64)fbs, 2, (u64)attr, 0))
            goto show;
        if (vid_rate) NC(ex.G, vid_rate, (u64)video_h, 0, 0, 0, 0, 0);

        /* ── 3. Build PM4 in ONION CB ─────────────────────────────────── */
        volatile u32 *fence = (volatile u32 *)(cb + FENCE_OFF);
        *fence = 0u;                  /* clear fence from CPU              */

        u64 fence_va = (u64)(cb + FENCE_OFF);
        u64 fb0_va   = (u64)fb0;
        u32 fbsz     = FW * FH * 4u;

        u32 *pm4 = (u32 *)cb;
        u32  n   = 0;

        /* WRITE_DATA: fence=0xBEEF0001 (confirms CP reads ONION CB)     */
        pm4[n++] = PM4_WD_HDR;
        pm4[n++] = WD_TC_L2;
        pm4[n++] = (u32)(fence_va & 0xFFFFFFFFu);
        pm4[n++] = (u32)(fence_va >> 32);
        pm4[n++] = 0xBEEF0001u;

        /* DMA_DATA: fill RED into framebuffer via CPU VA                 */
        pm4[n++] = PM4_DMA_HDR;
        pm4[n++] = DMA_FILL;
        pm4[n++] = 0x00FF0000u;        /* RED (0x00RRGGBB)               */
        pm4[n++] = 0u;
        pm4[n++] = (u32)(fb0_va & 0xFFFFFFFFu);
        pm4[n++] = (u32)(fb0_va >> 32);
        pm4[n++] = fbsz;

        /* WRITE_DATA: fence=0xBEEF0002 (confirms DMA finished)          */
        pm4[n++] = PM4_WD_HDR;
        pm4[n++] = WD_TC_L2;
        pm4[n++] = (u32)(fence_va & 0xFFFFFFFFu);
        pm4[n++] = (u32)(fence_va >> 32);
        pm4[n++] = 0xBEEF0002u;

        pm4[n++] = PM4_NOP;

        u32  cb_bytes = n * 4u;
        void *cb_ptr  = (void *)cb;

        /* ── 4. Submit ───────────────────────────────────────────────── */
        sub_ret = (s32)NC(ex.G, fn_submit, 1,
                           (u64)&cb_ptr, (u64)&cb_bytes, 0, 0, 0);

        /* ── 5. Spin-wait for fence 0xBEEF0002 (timeout 2s) ─────────── */
        for (u32 i = 0; i < 2000u && *fence != 0xBEEF0002u; i++) {
            if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 1000, 0, 0, 0, 0, 0);
        }
        fence_val = *fence;

        /* ── 6. Flip + hold ──────────────────────────────────────────── */
        flip_ret = (s32)NC(ex.G, vid_flip, (u64)video_h, 0, 1, 0, 0, 0);
        if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 3000000, 0, 0, 0, 0, 0);
    }

show:
    if (vid_close && video_h >= 0) NC(ex.G, vid_close, (u64)video_h, 0, 0, 0, 0, 0);

    ext->dbg[0] = (u64)(u32)sub_ret;
    ext->dbg[1] = fence_val;
    ext->dbg[2] = cb_phys;
    ext->dbg[3] = fb_phys;
    ext->status = 0;

    u8 *buf = ex_alloc_dialog(&ex);
    if (!buf) { ext->status = -3; return; }
    ex_dialog_begin(&ex);
    ret = ex_open_text(&ex, buf,
        "gnm_onion_probe\n"
        "sub=%x  flip=%x\n"
        "fence=%x\n"
        "BEEF0001=WD ok\n"
        "BEEF0002=DMA+WD ok\n"
        "RED screen=GPU fill ok\n"
        "cb_phys=%lx\n"
        "fb_phys=%lx",
        (u32)sub_ret, (u32)flip_ret,
        fence_val, cb_phys, fb_phys);
    if (ret == 0) ex_wait_user_close(&ex);
    ex_close_dialog(&ex);
    ex_free_dialog(&ex, buf);
    ext->step = 99;
}
