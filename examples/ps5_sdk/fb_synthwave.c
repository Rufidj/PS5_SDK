#include "example_common.h"

enum {
    GW = 1920, GH = 1080,
    FB_SZ  = GW * GH * 4,
    FB_AL  = (FB_SZ + 0x1FFFFF) & ~0x1FFFFF,
    FB_TOT = FB_AL * 2
};

static void draw_line(u32 *fb, s32 x0, s32 y0, s32 x1, s32 y1, u32 color) {
    s32 dx = x1 - x0;
    s32 dy = y1 - y0;
    s32 stepx = 1, stepy = 1;
    
    if (dx < 0) { dx = -dx; stepx = -1; }
    if (dy < 0) { dy = -dy; stepy = -1; }
    
    s32 err = dx - dy;
    
    while (1) {
        if (x0 >= 0 && x0 < GW && y0 >= 0 && y0 < GH) {
            fb[y0 * GW + x0] = color;
        }
        if (x0 == x1 && y0 == y1) break;
        s32 e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += stepx; }
        if (e2 < dx) { err += dx; y0 += stepy; }
    }
}

static void draw_sun(u32 *fb, s32 cx, s32 cy, s32 r) {
    s32 r2 = r * r;
    for (s32 y = -r; y <= r; y++) {
        if (y > 0) {
            // Cutout lines for synthwave aesthetic
            s32 y_norm = y * 100 / r; // 0 to 100
            s32 gap = 5 + (y_norm / 10);
            if ((y % 16) < gap) continue;
        }
        for (s32 x = -r; x <= r; x++) {
            if (x * x + y * y <= r2) {
                if (cx + x >= 0 && cx + x < GW && cy + y >= 0 && cy + y < GH) {
                    s32 t = ((y + r) * 255) / (2 * r);
                    // Gradient: Yellow (0xFFD700) to Pink (0xFF2A6D)
                    u32 r_col = 0xFF;
                    u32 g_col = 0xD7 - ((0xD7 - 0x2A) * t) / 255;
                    u32 b_col = 0x00 + ((0x6D - 0x00) * t) / 255;
                    fb[(cy + y) * GW + (cx + x)] = (r_col << 16) | (g_col << 8) | b_col;
                }
            }
        }
    }
}

static void draw_mountains(u32 *fb) {
    s32 y_horiz = GH / 2;
    s32 p[13] = {0, 150, 300, 450, 600, 750, 900, 1100, 1300, 1500, 1650, 1800, 1920};
    s32 h[13] = {0, -80, -30, -150, -50, -200, -100, -250, -90, -160, -40, -120, 0};
    
    for (int i = 0; i < 12; i++) {
        draw_line(fb, p[i], y_horiz + h[i], p[i+1], y_horiz + h[i+1], 0x0005D9E8);
        // Fill mountain with black/dark blue below the line? Too slow to do polygon fill.
        // We'll leave it as wireframe!
    }
}

__attribute__((section(".text._start")))
void _start(u64 eboot_base, u64 dlsym_addr, struct ext_args *ext) {
    if (!ext) return;
    ext->step = 1;

    struct ps5_example ex;
    if (ex_init(&ex, eboot_base, dlsym_addr, ext) != 0) { ext->status = -1; return; }

    /* Pad */
    ps5_sdk_load_sprx(ex.G, ex.load_mod, "libScePad.sprx");
    void *pad_geth = SYM(ex.G, ex.D, PAD_HANDLE, "scePadGetHandle");
    void *pad_read = SYM(ex.G, ex.D, PAD_HANDLE, "scePadRead");
    s32 pad_h = pad_geth ? (s32)NC(ex.G, pad_geth, (u64)ex.user_id, 0, 0, 0, 0, 0) : -1;

    /* VideoOut */
    void *alloc_dm  = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, KERNEL_sceKernelAllocateDirectMemory);
    void *map_dm    = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, KERNEL_sceKernelMapDirectMemory);
    ps5_sdk_load_sprx(ex.G, ex.load_mod, "libSceVideoOut.sprx");
    void *vid_open  = SYM(ex.G, ex.D, VIDEOOUT_HANDLE, "sceVideoOutOpen");
    void *vid_close = SYM(ex.G, ex.D, VIDEOOUT_HANDLE, "sceVideoOutClose");
    void *vid_reg   = SYM(ex.G, ex.D, VIDEOOUT_HANDLE, "sceVideoOutRegisterBuffers");
    void *vid_flip  = SYM(ex.G, ex.D, VIDEOOUT_HANDLE, "sceVideoOutSubmitFlip");
    void *cancel_fn = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, KERNEL_scePthreadCancel);

    s32 emu_vid   = *(s32 *)(eboot_base + EBOOT_VIDOUT);
    u64 gs_thread = *(u64 *)(eboot_base + EBOOT_GS_THREAD);

    if (cancel_fn && gs_thread) NC(ex.G, cancel_fn, gs_thread, 0, 0, 0, 0, 0);
    if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 300000, 0, 0, 0, 0, 0);
    if (vid_close && emu_vid >= 0) NC(ex.G, vid_close, (u64)emu_vid, 0, 0, 0, 0, 0);
    if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 100000, 0, 0, 0, 0, 0);

    s32 video_h = vid_open ? ps5sdk_vo_open(ex.G, vid_open) : -1;
    u64 phys = 0; void *vmem = 0;
    u8 *pad_buf = 0;

    if (pad_h >= 0 && ex.mmap_fn) {
        pad_buf = (u8 *)NC(ex.G, ex.mmap_fn, 0, 0x1000, 3, 0x1002, (u64)-1, 0);
        if ((s64)pad_buf == -1) pad_buf = 0;
    }

    if (video_h >= 0 && alloc_dm && map_dm && vid_reg && vid_flip) {
        NC(ex.G, alloc_dm, 0, 0x300000000ULL, FB_TOT, 0x200000, 3, (u64)&phys);
        NC(ex.G, map_dm, (u64)&vmem, FB_TOT, 0x33, 0, phys, 0x200000);
        if (vmem) {
            u8 attr[64]; void *fbs[2];
            ex_zero(attr, sizeof(attr));
            *(u32 *)(attr +  0) = 0x80000000; *(u32 *)(attr +  4) = 1;
            *(u32 *)(attr + 12) = GW; *(u32 *)(attr + 16) = GH; *(u32 *)(attr + 20) = GW;
            fbs[0] = vmem; fbs[1] = (u8 *)vmem + FB_AL;

            if ((s32)NC(ex.G, vid_reg, (u64)video_h, 0, (u64)fbs, 2, (u64)attr, 0) == 0) {
                u32 frame = 0, done = 0;
                
                while (!done) {
                    if (pad_read && pad_h >= 0 && pad_buf) {
                        u32 raw = 0, btns = 0;
                        ps5_sdk_pad_read_buttons(ex.G, pad_read, pad_h, pad_buf, &raw, &btns);
                        if (btns & PS5SDK_PAD_BTN_OPTIONS) done = 1;
                    }

                    s32 idx = (s32)(frame & 1u);
                    u32 *fb = (u32 *)fbs[idx];
                    
                    /* Background */
                    ps5sdk_fb_fill(fb, GW, GH, 0x000B0E14);
                    
                    /* Sun */
                    draw_sun(fb, GW / 2, GH / 2 - 50, 200);
                    
                    /* Clear bottom half for the grid (masking the sun) */
                    ps5sdk_fb_rect(fb, GW, GH, 0, GH / 2, GW, GH / 2, 0x000B0E14);
                    
                    /* Draw Mountains */
                    draw_mountains(fb);
                    
                    /* Horizon glow line */
                    ps5sdk_fb_rect(fb, GW, GH, 0, GH / 2, GW, 3, 0x00FF2A6D);
                    
                    /* Synthwave Grid */
                    s32 fov = 600;
                    s32 cam_y = 60;
                    
                    /* Vertical lines converging */
                    for (s32 x = -3000; x <= 3000; x += 150) {
                        s32 z_start = 10;
                        s32 z_end = 2000;
                        s32 px1 = GW / 2 + (x * fov) / z_start;
                        s32 py1 = GH / 2 + (cam_y * fov) / z_start;
                        s32 px2 = GW / 2 + (x * fov) / z_end;
                        s32 py2 = GH / 2 + (cam_y * fov) / z_end;
                        draw_line(fb, px1, py1, px2, py2, 0x00FF2A6D); // Pink lines
                    }
                    
                    /* Horizontal scrolling lines */
                    s32 speed = 10;
                    s32 scroll = (frame * speed) % 150;
                    
                    for (s32 z = 10; z < 2000; z += 150) {
                        s32 real_z = z - scroll;
                        if (real_z <= 5) continue;
                        s32 py = GH / 2 + (cam_y * fov) / real_z;
                        if (py >= GH / 2 && py < GH) {
                            s32 thickness = 80 / real_z;
                            if (thickness < 1) thickness = 1;
                            if (thickness > 4) thickness = 4;
                            ps5sdk_fb_rect(fb, GW, GH, 0, py, GW, thickness, 0x0005D9E8); // Cyan horizontal
                        }
                    }

                    /* HUD */
                    ps5sdk_fb_str(fb, GW, GH, 20, 20, "SYNTHWAVE GRID - SOFTWARE RENDERER", 0x00FFFFFF, 0, 3, 1);
                    ps5sdk_fb_str(fb, GW, GH, 20, 60, "OPTIONS to Exit", 0x00A0A0A0, 0, 2, 1);

                    NC(ex.G, vid_flip, (u64)video_h, (u64)idx, 1, 0, 0, 0);
                    if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 16000, 0, 0, 0, 0, 0);
                    frame++;
                }

                /* Clear screen before exit */
                ps5sdk_fb_fill((u32*)fbs[0], GW, GH, 0);
                ps5sdk_fb_fill((u32*)fbs[1], GW, GH, 0);
                NC(ex.G, vid_flip, (u64)video_h, 0, 1, 0, 0, 0);
                if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 32000, 0, 0, 0, 0, 0);
            }
        }
    }

    if (pad_buf && ex.munmap_fn) NC(ex.G, ex.munmap_fn, (u64)pad_buf, 0x1000, 0, 0, 0, 0);
    
    if (vid_close && video_h >= 0) NC(ex.G, vid_close, (u64)video_h, 0, 0, 0, 0, 0);

    ext->step = 99;
}
