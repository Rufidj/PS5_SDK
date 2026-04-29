#include "example_common.h"

enum {
    AN_W = 1920, AN_H = 1080,
    AN_FB_SIZE    = AN_W * AN_H * 4,
    AN_FB_ALIGNED = (AN_FB_SIZE + 0x1FFFFF) & ~0x1FFFFF,
    AN_FB_TOTAL   = AN_FB_ALIGNED * 2,
    AN_FRAMES     = 360,
    AN_SQ         = 160,
};

static s32 takeover_open(void *G, void *vid_open) {
    s32 types[4] = { 0xFF, 0, 1, 2 };
    for (int i = 0; i < 4; i++) {
        s32 h = (s32)NC(G, vid_open, (u64)types[i], 0, 0, 0, 0, 0);
        if (h >= 0) return h;
    }
    return -1;
}

static void fill_fb(u32 *fb, u32 color) {
    u32 n = (u32)(AN_W * AN_H);
    for (u32 i = 0; i < n; i++) fb[i] = color;
}

static void draw_rect(u32 *fb, s32 x, s32 y, s32 w, s32 h, u32 color) {
    if (!fb) return;
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > AN_W) w = AN_W - x;
    if (y + h > AN_H) h = AN_H - y;
    if (w <= 0 || h <= 0) return;
    for (s32 yy = 0; yy < h; yy++) {
        u32 *row = fb + (u32)(y + yy) * AN_W + (u32)x;
        for (s32 xx = 0; xx < w; xx++) row[xx] = color;
    }
}

/* Smooth HSV hue cycle using 0x00RRGGBB encoding (BGRA8 framebuffer) */
static u32 hue_color(u32 f) {
    u32 t   = (f % 60u) * 255u / 60u;
    u32 inv = 255u - t;
    switch ((f / 60u) % 6u) {
        case 0: return (255u << 16) | (t   << 8);          /* red   → yellow */
        case 1: return (inv  << 16) | (255u << 8);          /* yellow→ green  */
        case 2: return              (255u << 8) | t;         /* green → cyan   */
        case 3: return              (inv  << 8) | 255u;      /* cyan  → blue   */
        case 4: return (t    << 16)             | 255u;      /* blue  → magenta*/
        default: return (255u << 16)            | inv;       /* magenta→ red   */
    }
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
    void *rel_dm    = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, KERNEL_sceKernelReleaseDirectMemory);
    ps5_sdk_load_sprx(ex.G, ex.load_mod, "libSceVideoOut.sprx");
    void *vid_open  = SYM(ex.G, ex.D, VIDEOOUT_HANDLE, "sceVideoOutOpen");
    void *vid_close = SYM(ex.G, ex.D, VIDEOOUT_HANDLE, "sceVideoOutClose");
    void *vid_reg   = SYM(ex.G, ex.D, VIDEOOUT_HANDLE, "sceVideoOutRegisterBuffers");
    void *vid_flip  = SYM(ex.G, ex.D, VIDEOOUT_HANDLE, "sceVideoOutSubmitFlip");
    void *vid_rate  = SYM(ex.G, ex.D, VIDEOOUT_HANDLE, "sceVideoOutSetFlipRate");

    s32 emu_vid   = *(s32 *)(eboot_base + EBOOT_VIDOUT);
    u64 gs_thread = *(u64 *)(eboot_base + EBOOT_GS_THREAD);

    s32 video_h    = vid_open ? takeover_open(ex.G, vid_open) : -1;
    if (video_h < 0) {
        if (cancel_fn && gs_thread) NC(ex.G, cancel_fn, gs_thread, 0, 0, 0, 0, 0);
        if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 300000, 0, 0, 0, 0, 0);
        if (vid_close && emu_vid >= 0) NC(ex.G, vid_close, (u64)emu_vid, 0, 0, 0, 0, 0);
        if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 100000, 0, 0, 0, 0, 0);
        video_h = vid_open ? takeover_open(ex.G, vid_open) : -1;
    }
    s32 frames_done = 0;
    u64 phys = 0;
    void *vmem = 0;

    if (video_h >= 0 && alloc_dm && map_dm && vid_reg && vid_flip) {
        NC(ex.G, alloc_dm, 0, 0x300000000ULL, AN_FB_TOTAL, 0x200000, 3, (u64)&phys);
        NC(ex.G, map_dm, (u64)&vmem, AN_FB_TOTAL, 0x33, 0, phys, 0x200000);
        if (vmem) {
            u8 attr[64];
            void *fbs[2];
            ex_zero(attr, sizeof(attr));
            *(u32 *)(attr +  0) = 0x80000000;
            *(u32 *)(attr +  4) = 1;
            *(u32 *)(attr + 12) = AN_W;
            *(u32 *)(attr + 16) = AN_H;
            *(u32 *)(attr + 20) = AN_W;
            fbs[0] = vmem;
            fbs[1] = (u8 *)vmem + AN_FB_ALIGNED;

            s32 reg_ret = (s32)NC(ex.G, vid_reg, (u64)video_h, 0, (u64)fbs, 2, (u64)attr, 0);
            if (reg_ret == 0) {
                if (vid_rate) NC(ex.G, vid_rate, (u64)video_h, 0, 0, 0, 0, 0);

                s32 bx = (AN_W - AN_SQ) / 2;
                s32 by = (AN_H - AN_SQ) / 2;
                s32 vx = 5, vy = 3;

                for (s32 f = 0; f < AN_FRAMES; f++) {
                    s32 idx    = f & 1;
                    u32 *fb    = (u32 *)fbs[idx];
                    u32 sq_col = hue_color((u32)f);

                    fill_fb(fb, 0x00080820);
                    draw_rect(fb, bx + 8, by + 8, AN_SQ, AN_SQ, 0x00020208); /* shadow */
                    draw_rect(fb, bx, by, AN_SQ, AN_SQ, sq_col);
                    draw_rect(fb, bx + AN_SQ/4, by + AN_SQ/4, AN_SQ/2, AN_SQ/2, 0x00ffffff);

                    NC(ex.G, vid_flip, (u64)video_h, (u64)idx, 1, 0, 0, 0);
                    if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 16000, 0, 0, 0, 0, 0);

                    bx += vx; by += vy;
                    if (bx < 0)             { bx = 0;             vx = -vx; }
                    if (by < 0)             { by = 0;             vy = -vy; }
                    if (bx + AN_SQ > AN_W)  { bx = AN_W - AN_SQ; vx = -vx; }
                    if (by + AN_SQ > AN_H)  { by = AN_H - AN_SQ; vy = -vy; }
                    frames_done++;
                }
            }
        }
    }

    ext->dbg[0] = (u64)(u32)emu_vid;
    ext->dbg[1] = gs_thread;
    ext->dbg[4] = phys;
    ext->dbg[5] = (u64)vmem;
    ext->dbg[6] = (u64)(u32)frames_done;
    ext->status = 0;

    if (vid_close && video_h >= 0) NC(ex.G, vid_close, (u64)video_h, 0, 0, 0, 0, 0);
    if (vmem && ex.munmap_fn) NC(ex.G, ex.munmap_fn, (u64)vmem, AN_FB_TOTAL, 0, 0, 0, 0);
    if (rel_dm && phys) NC(ex.G, rel_dm, phys, AN_FB_TOTAL, 0, 0, 0, 0);

    u8 *buf = ex_alloc_dialog(&ex);
    if (!buf) { ext->status = -3; return; }
    ex_dialog_begin(&ex);
    ret = ex_open_text(&ex, buf,
        "fb_animation\nframes=%d\nvmem=%p\nphys=%lx",
        frames_done, vmem, phys);
    if (ret == 0) {
        ext->status = 0;
        ex_wait_user_close(&ex);
    } else {
        ext->status = -4;
    }
    ex_close_dialog(&ex);
    ex_free_dialog(&ex, buf);
    ext->step = 99;
    ex_finish(&ex, ext, ext->status, 0);
}
