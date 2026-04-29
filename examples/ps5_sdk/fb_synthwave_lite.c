#include "example_common.h"

enum {
    GW = 1920, GH = 1080,
    FB_SZ  = GW * GH * 4,
    FB_AL  = (FB_SZ + 0x1FFFFF) & ~0x1FFFFF,
    FB_TOT = FB_AL * 2,
    LITE_FRAMES = 240
};

static void draw_line(u32 *fb, s32 x0, s32 y0, s32 x1, s32 y1, u32 color) {
    s32 dx = x1 - x0;
    s32 dy = y1 - y0;
    s32 stepx = 1, stepy = 1;
    if (dx < 0) { dx = -dx; stepx = -1; }
    if (dy < 0) { dy = -dy; stepy = -1; }
    s32 err = dx - dy;
    while (1) {
        if (x0 >= 0 && x0 < GW && y0 >= 0 && y0 < GH) fb[y0 * GW + x0] = color;
        if (x0 == x1 && y0 == y1) break;
        s32 e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += stepx; }
        if (e2 < dx) { err += dx; y0 += stepy; }
    }
}

static void draw_sun_lite(u32 *fb, s32 cx, s32 cy, s32 r) {
    s32 r2 = r * r;
    for (s32 y = -r; y <= r; y++) {
        for (s32 x = -r; x <= r; x++) {
            if (x * x + y * y <= r2) {
                s32 px = cx + x, py = cy + y;
                if (px >= 0 && px < GW && py >= 0 && py < GH) {
                    fb[py * GW + px] = 0x00FF4A8A;
                }
            }
        }
    }
}

__attribute__((section(".text._start")))
void _start(u64 eboot_base, u64 dlsym_addr, struct ext_args *ext) {
    if (!ext) return;
    ext->step = 1;

    struct ps5_example ex;
    if (ex_init(&ex, eboot_base, dlsym_addr, ext) != 0) { ext->status = -1; return; }

    void *alloc_dm  = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, KERNEL_sceKernelAllocateDirectMemory);
    void *map_dm    = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, KERNEL_sceKernelMapDirectMemory);
    void *rel_dm    = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, KERNEL_sceKernelReleaseDirectMemory);
    ps5_sdk_load_sprx(ex.G, ex.load_mod, "libSceVideoOut.sprx");
    void *vid_open  = SYM(ex.G, ex.D, VIDEOOUT_HANDLE, "sceVideoOutOpen");
    void *vid_close = SYM(ex.G, ex.D, VIDEOOUT_HANDLE, "sceVideoOutClose");
    void *vid_reg   = SYM(ex.G, ex.D, VIDEOOUT_HANDLE, "sceVideoOutRegisterBuffers");
    void *vid_flip  = SYM(ex.G, ex.D, VIDEOOUT_HANDLE, "sceVideoOutSubmitFlip");
    void *vid_rate  = SYM(ex.G, ex.D, VIDEOOUT_HANDLE, "sceVideoOutSetFlipRate");
    void *cancel_fn = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, KERNEL_scePthreadCancel);

    s32 emu_vid   = *(s32 *)(eboot_base + EBOOT_VIDOUT);
    u64 gs_thread = *(u64 *)(eboot_base + EBOOT_GS_THREAD);
    s32 video_h = vid_open ? ps5sdk_vo_open(ex.G, vid_open) : -1;
    if (video_h < 0) {
        if (cancel_fn && gs_thread) NC(ex.G, cancel_fn, gs_thread, 0, 0, 0, 0, 0);
        if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 300000, 0, 0, 0, 0, 0);
        if (vid_close && emu_vid >= 0) NC(ex.G, vid_close, (u64)emu_vid, 0, 0, 0, 0, 0);
        if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 100000, 0, 0, 0, 0, 0);
        video_h = vid_open ? ps5sdk_vo_open(ex.G, vid_open) : -1;
    }
    u64 phys = 0;
    void *vmem = 0;
    s32 frames_done = 0;

    if (video_h >= 0 && alloc_dm && map_dm && vid_reg && vid_flip) {
        NC(ex.G, alloc_dm, 0, 0x300000000ULL, FB_TOT, 0x200000, 3, (u64)&phys);
        NC(ex.G, map_dm, (u64)&vmem, FB_TOT, 0x33, 0, phys, 0x200000);
        if (vmem) {
            u8 attr[64];
            void *fbs[2];
            ex_zero(attr, sizeof(attr));
            *(u32 *)(attr + 0) = 0x80000000;
            *(u32 *)(attr + 4) = 1;
            *(u32 *)(attr + 12) = GW;
            *(u32 *)(attr + 16) = GH;
            *(u32 *)(attr + 20) = GW;
            fbs[0] = vmem;
            fbs[1] = (u8 *)vmem + FB_AL;

            if ((s32)NC(ex.G, vid_reg, (u64)video_h, 0, (u64)fbs, 2, (u64)attr, 0) == 0) {
                if (vid_rate) NC(ex.G, vid_rate, (u64)video_h, 0, 0, 0, 0, 0);
                u32 frame = 0;
                for (s32 f = 0; f < LITE_FRAMES; f++) {
                    s32 idx = (s32)(frame & 1u);
                    u32 *fb = (u32 *)fbs[idx];

                    ps5sdk_fb_fill(fb, GW, GH, 0x000B0E14);
                    draw_sun_lite(fb, GW / 2, GH / 2 - 50, 150);
                    ps5sdk_fb_rect(fb, GW, GH, 0, GH / 2, GW, GH / 2, 0x000B0E14);
                    ps5sdk_fb_rect(fb, GW, GH, 0, GH / 2, GW, 2, 0x00FF2A6D);

                    s32 fov = 600;
                    s32 cam_y = 60;
                    s32 scroll = (frame * 8) % 240;

                    for (s32 x = -1800; x <= 1800; x += 300) {
                        s32 z1 = 20, z2 = 1600;
                        s32 px1 = GW / 2 + (x * fov) / z1;
                        s32 py1 = GH / 2 + (cam_y * fov) / z1;
                        s32 px2 = GW / 2 + (x * fov) / z2;
                        s32 py2 = GH / 2 + (cam_y * fov) / z2;
                        draw_line(fb, px1, py1, px2, py2, 0x00FF2A6D);
                    }

                    for (s32 z = 20; z < 1600; z += 120) {
                        s32 real_z = z - scroll;
                        if (real_z <= 8) continue;
                        s32 py = GH / 2 + (cam_y * fov) / real_z;
                        if (py >= GH / 2 && py < GH) {
                            ps5sdk_fb_rect(fb, GW, GH, 0, py, GW, 1, 0x0005D9E8);
                        }
                    }

                    ps5sdk_fb_str(fb, GW, GH, 20, 20, "SYNTHWAVE LITE", 0x00FFFFFF, 0, 3, 1);
                    ps5sdk_fb_str(fb, GW, GH, 20, 60, "AUTO DEMO", 0x00A0A0A0, 0, 2, 1);

                    NC(ex.G, vid_flip, (u64)video_h, (u64)idx, 1, 0, 0, 0);
                    if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 16000, 0, 0, 0, 0, 0);
                    frame++;
                    frames_done++;
                }
            }
        }
    }

    if (vid_close && video_h >= 0) NC(ex.G, vid_close, (u64)video_h, 0, 0, 0, 0, 0);
    if (vmem && ex.munmap_fn) NC(ex.G, ex.munmap_fn, (u64)vmem, FB_TOT, 0, 0, 0, 0);
    if (rel_dm && phys) NC(ex.G, rel_dm, phys, FB_TOT, 0, 0, 0, 0);
    ext->dbg[0] = (u64)(u32)video_h;
    ext->dbg[1] = (u64)(u32)frames_done;
    ext->dbg[4] = phys;
    ext->dbg[5] = (u64)vmem;
    ext->status = 0;

    u8 *buf = ex_alloc_dialog(&ex);
    if (!buf) { ext->status = -3; return; }
    ex_dialog_begin(&ex);
    s32 ret = ex_open_text(&ex, buf, "fb_synthwave_lite\nframes=%d\nvideo=%x\nvmem=%p", frames_done, (u32)video_h, vmem);
    if (ret == 0) ex_wait_user_close(&ex);
    ex_close_dialog(&ex);
    ex_free_dialog(&ex, buf);
    ext->step = 99;
}
