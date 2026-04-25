#include "example_common.h"

/* Bouncing square + on-screen HUD rendered with ps5sdk_fb_* helpers.
 * Demonstrates the standalone font renderer without MsgDialog dependency. */

enum {
    FT_W = PS5SDK_SCR_W, FT_H = PS5SDK_SCR_H,
    FT_FB_SIZE    = FT_W * FT_H * 4,
    FT_FB_ALIGNED = (FT_FB_SIZE + 0x1FFFFF) & ~0x1FFFFF,
    FT_FB_TOTAL   = FT_FB_ALIGNED * 2,
    FT_FRAMES     = 360,
    FT_SQ         = 140,
    FT_SCALE      = 3,       /* font scale: 3 → 24×24 px per char */
};

static u32 hue_color(u32 f) {
    u32 t   = (f % 60u) * 255u / 60u;
    u32 inv = 255u - t;
    switch ((f / 60u) % 6u) {
        case 0: return (255u << 16) | (t   << 8);
        case 1: return (inv  << 16) | (255u << 8);
        case 2: return              (255u << 8) | t;
        case 3: return              (inv  << 8) | 255u;
        case 4: return (t    << 16)             | 255u;
        default: return (255u << 16)            | inv;
    }
}

/* Draw a labelled HUD row: "label: value" at (x,y) */
static void hud_kv(u32 *fb, s32 x, s32 y, const char *label, u32 val) {
    s32 cx = ps5sdk_fb_str(fb, FT_W, FT_H, x, y, label,
                            0x00d0d0d0, 0, FT_SCALE, 1);
    ps5sdk_fb_dec(fb, FT_W, FT_H, cx, y, val,
                  0x00ffff40, 0, FT_SCALE, 1);
}

__attribute__((section(".text._start")))
void _start(u64 eboot_base, u64 dlsym_addr, struct ext_args *ext) {
    if (!ext) return;
    ext->step = 1;

    struct ps5_example ex;
    s32 ret = ex_init(&ex, eboot_base, dlsym_addr, ext);
    if (ret != 0) { ext->status = ret; return; }

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

    s32 video_h    = vid_open ? ps5sdk_vo_open(ex.G, vid_open) : -1;
    s32 frames_done = 0;
    u64 phys = 0;
    void *vmem = 0;

    if (video_h >= 0 && alloc_dm && map_dm && vid_reg && vid_flip) {
        NC(ex.G, alloc_dm, 0, 0x300000000ULL, FT_FB_TOTAL, 0x200000, 3, (u64)&phys);
        NC(ex.G, map_dm, (u64)&vmem, FT_FB_TOTAL, 0x33, 0, phys, 0x200000);
        if (vmem) {
            u8 attr[64];
            void *fbs[2];
            ex_zero(attr, sizeof(attr));
            *(u32 *)(attr +  0) = 0x80000000;
            *(u32 *)(attr +  4) = 1;
            *(u32 *)(attr + 12) = FT_W;
            *(u32 *)(attr + 16) = FT_H;
            *(u32 *)(attr + 20) = FT_W;
            fbs[0] = vmem;
            fbs[1] = (u8 *)vmem + FT_FB_ALIGNED;

            s32 reg_ret = (s32)NC(ex.G, vid_reg, (u64)video_h, 0, (u64)fbs, 2, (u64)attr, 0);
            if (reg_ret == 0) {
                if (vid_rate) NC(ex.G, vid_rate, (u64)video_h, 0, 0, 0, 0, 0);

                s32 bx = (FT_W - FT_SQ) / 2;
                s32 by = (FT_H - FT_SQ) / 2;
                s32 vx = 5, vy = 3;

                for (s32 f = 0; f < FT_FRAMES; f++) {
                    s32 idx    = f & 1;
                    u32 *fb    = (u32 *)fbs[idx];
                    u32 sq_col = hue_color((u32)f);

                    /* background */
                    ps5sdk_fb_fill(fb, FT_W, FT_H, 0x00080820);

                    /* bouncing square */
                    ps5sdk_fb_rect(fb, FT_W, FT_H, bx + 8, by + 8, FT_SQ, FT_SQ, 0x00020208);
                    ps5sdk_fb_rect(fb, FT_W, FT_H, bx, by, FT_SQ, FT_SQ, sq_col);
                    ps5sdk_fb_rect(fb, FT_W, FT_H,
                                   bx + FT_SQ/4, by + FT_SQ/4,
                                   FT_SQ/2, FT_SQ/2, 0x00ffffff);

                    /* HUD — top-left corner */
                    s32 hx = 32, hy = 32, ls = (s32)(8u * FT_SCALE) + 4;
                    ps5sdk_fb_str(fb, FT_W, FT_H, hx, hy,
                                  "PS5 SDK fb_text", 0x00ffffff, 0, FT_SCALE, 1);
                    hud_kv(fb, hx, hy + ls,     "frame: ", (u32)f);
                    hud_kv(fb, hx, hy + ls * 2, "x:     ", (u32)bx);
                    hud_kv(fb, hx, hy + ls * 3, "y:     ", (u32)by);

                    /* color swatch next to HUD */
                    ps5sdk_fb_rect(fb, FT_W, FT_H,
                                   hx + (s32)(16 * 8u * FT_SCALE), hy + ls,
                                   (s32)(3u * 8u * FT_SCALE), (s32)(3u * 8u * FT_SCALE),
                                   sq_col);

                    NC(ex.G, vid_flip, (u64)video_h, (u64)idx, 1, 0, 0, 0);
                    if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 16000, 0, 0, 0, 0, 0);

                    bx += vx; by += vy;
                    if (bx < 0)             { bx = 0;             vx = -vx; }
                    if (by < 0)             { by = 0;             vy = -vy; }
                    if (bx + FT_SQ > FT_W)  { bx = FT_W - FT_SQ; vx = -vx; }
                    if (by + FT_SQ > FT_H)  { by = FT_H - FT_SQ; vy = -vy; }
                    frames_done++;
                }
            }
        }
    }

    ext->dbg[4] = phys;
    ext->dbg[5] = (u64)vmem;
    ext->dbg[6] = (u64)(u32)frames_done;
    ext->status = 0;

    if (vid_close && video_h >= 0) NC(ex.G, vid_close, (u64)video_h, 0, 0, 0, 0, 0);

    u8 *buf = ex_alloc_dialog(&ex);
    if (!buf) { ext->status = -3; return; }
    ex_dialog_begin(&ex);
    ret = ex_open_text(&ex, buf,
        "fb_text\nframes=%d\nvmem=%p\nphys=%lx",
        frames_done, vmem, phys);
    if (ret == 0) ex_wait_user_close(&ex);
    ex_close_dialog(&ex);
    ex_free_dialog(&ex, buf);
    ext->step = 99;
}
