#include "example_common.h"

enum {
    PD_W = 1920, PD_H = 1080,
    PD_FB_SIZE    = PD_W * PD_H * 4,
    PD_FB_ALIGNED = (PD_FB_SIZE + 0x1FFFFF) & ~0x1FFFFF,
    PD_FB_TOTAL   = PD_FB_ALIGNED * 2,
    PD_SQ         = 80,
    PD_SPEED      = 8,
    PD_MAX_FRAMES = 900,   /* 15 s max si no se pulsa CROSS */
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
    u32 n = (u32)(PD_W * PD_H);
    for (u32 i = 0; i < n; i++) fb[i] = color;
}

static void draw_rect(u32 *fb, s32 x, s32 y, s32 w, s32 h, u32 color) {
    if (!fb) return;
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > PD_W) w = PD_W - x;
    if (y + h > PD_H) h = PD_H - y;
    if (w <= 0 || h <= 0) return;
    for (s32 yy = 0; yy < h; yy++) {
        u32 *row = fb + (u32)(y + yy) * PD_W + (u32)x;
        for (s32 xx = 0; xx < w; xx++) row[xx] = color;
    }
}

/* Face button → color in 0x00RRGGBB */
static u32 face_color(u32 b) {
    if (b & PS5SDK_PAD_BTN_CROSS)    return 0x002060ff; /* azul    */
    if (b & PS5SDK_PAD_BTN_CIRCLE)   return 0x00ff2020; /* rojo    */
    if (b & PS5SDK_PAD_BTN_TRIANGLE) return 0x0020ff40; /* verde   */
    if (b & PS5SDK_PAD_BTN_SQUARE)   return 0x00d020d0; /* magenta */
    if (b & PS5SDK_PAD_BTN_L1)       return 0x00ffffff; /* blanco  */
    if (b & PS5SDK_PAD_BTN_R1)       return 0x00ffaa00; /* dorado  */
    return 0;
}

__attribute__((section(".text._start")))
void _start(u64 eboot_base, u64 dlsym_addr, struct ext_args *ext) {
    if (!ext) return;
    ext->step = 1;

    struct ps5_example ex;
    s32 ret = ex_init(&ex, eboot_base, dlsym_addr, ext);
    if (ret != 0) { ext->status = ret; return; }

    ps5_sdk_load_sprx(ex.G, ex.load_mod, "libScePad.sprx");
    void *pad_init = SYM(ex.G, ex.D, PAD_HANDLE, "scePadInit");
    void *pad_geth = SYM(ex.G, ex.D, PAD_HANDLE, "scePadGetHandle");
    void *pad_read = SYM(ex.G, ex.D, PAD_HANDLE, "scePadRead");
    if (pad_init) NC(ex.G, pad_init, 0, 0, 0, 0, 0, 0);
    s32 pad_h = pad_geth ? (s32)NC(ex.G, pad_geth, (u64)ex.user_id, 0, 0, 0, 0, 0) : -1;

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
    u8 *pad_buf = 0;

    if (pad_h >= 0 && ex.mmap_fn) {
        pad_buf = (u8 *)NC(ex.G, ex.mmap_fn, 0, 0x1000, 3, 0x1002, (u64)-1, 0);
        if ((s64)pad_buf == -1) pad_buf = 0;
    }

    if (video_h >= 0 && alloc_dm && map_dm && vid_reg && vid_flip) {
        NC(ex.G, alloc_dm, 0, 0x300000000ULL, PD_FB_TOTAL, 0x200000, 3, (u64)&phys);
        NC(ex.G, map_dm, (u64)&vmem, PD_FB_TOTAL, 0x33, 0, phys, 0x200000);
        if (vmem) {
            u8 attr[64];
            void *fbs[2];
            ex_zero(attr, sizeof(attr));
            *(u32 *)(attr +  0) = 0x80000000;
            *(u32 *)(attr +  4) = 1;
            *(u32 *)(attr + 12) = PD_W;
            *(u32 *)(attr + 16) = PD_H;
            *(u32 *)(attr + 20) = PD_W;
            fbs[0] = vmem;
            fbs[1] = (u8 *)vmem + PD_FB_ALIGNED;

            s32 reg_ret = (s32)NC(ex.G, vid_reg, (u64)video_h, 0, (u64)fbs, 2, (u64)attr, 0);
            if (reg_ret == 0) {
                if (vid_rate) NC(ex.G, vid_rate, (u64)video_h, 0, 0, 0, 0, 0);

                s32 cx = (PD_W - PD_SQ) / 2;
                s32 cy = (PD_H - PD_SQ) / 2;
                u32 cur_color = 0x00f06020;
                s32 done = 0;

                for (s32 f = 0; f < PD_MAX_FRAMES && !done; f++) {
                    u32 raw = 0, buttons = 0;
                    if (pad_read && pad_h >= 0 && pad_buf)
                        ps5_sdk_pad_read_buttons(ex.G, pad_read, pad_h, pad_buf, &raw, &buttons);

                    if (buttons & PS5SDK_PAD_BTN_UP)    cy -= PD_SPEED;
                    if (buttons & PS5SDK_PAD_BTN_DOWN)  cy += PD_SPEED;
                    if (buttons & PS5SDK_PAD_BTN_LEFT)  cx -= PD_SPEED;
                    if (buttons & PS5SDK_PAD_BTN_RIGHT) cx += PD_SPEED;
                    if (cx < 0)            cx = 0;
                    if (cy < 0)            cy = 0;
                    if (cx + PD_SQ > PD_W) cx = PD_W - PD_SQ;
                    if (cy + PD_SQ > PD_H) cy = PD_H - PD_SQ;

                    u32 fc = face_color(buttons);
                    if (fc) cur_color = fc;

                    if (buttons & PS5SDK_PAD_BTN_CROSS) done = 1;

                    s32 idx = f & 1;
                    u32 *fb = (u32 *)fbs[idx];

                    fill_fb(fb, 0x00050510);

                    /* light grid for depth reference */
                    for (s32 gx = 120; gx < PD_W; gx += 120)
                        draw_rect(fb, gx - 1, 0, 2, PD_H, 0x0010102a);
                    for (s32 gy = 120; gy < PD_H; gy += 120)
                        draw_rect(fb, 0, gy - 1, PD_W, 2, 0x0010102a);

                    draw_rect(fb, cx + 8, cy + 8, PD_SQ, PD_SQ, 0x00020205); /* shadow */
                    draw_rect(fb, cx, cy, PD_SQ, PD_SQ, cur_color);
                    draw_rect(fb, cx + PD_SQ/4, cy + PD_SQ/4, PD_SQ/2, PD_SQ/2, 0x00ffffff);

                    NC(ex.G, vid_flip, (u64)video_h, (u64)idx, 1, 0, 0, 0);
                    if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 16000, 0, 0, 0, 0, 0);
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

    if (pad_buf && ex.munmap_fn) NC(ex.G, ex.munmap_fn, (u64)pad_buf, 0x1000, 0, 0, 0, 0);
    if (vid_close && video_h >= 0) NC(ex.G, vid_close, (u64)video_h, 0, 0, 0, 0, 0);
    if (vmem && ex.munmap_fn) NC(ex.G, ex.munmap_fn, (u64)vmem, PD_FB_TOTAL, 0, 0, 0, 0);
    if (rel_dm && phys) NC(ex.G, rel_dm, phys, PD_FB_TOTAL, 0, 0, 0, 0);

    u8 *buf = ex_alloc_dialog(&ex);
    if (!buf) { ext->status = -3; return; }
    ex_dialog_begin(&ex);
    ret = ex_open_text(&ex, buf,
        "fb_pad_draw\npad_h=%x frames=%d\nvmem=%p\nphys=%lx",
        (u32)pad_h, frames_done, vmem, phys);
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
