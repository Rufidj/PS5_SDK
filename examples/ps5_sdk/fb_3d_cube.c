#include "example_common.h"

enum {
    GW = 1920, GH = 1080,
    FB_SZ  = GW * GH * 4,
    FB_AL  = (FB_SZ + 0x1FFFFF) & ~0x1FFFFF,
    FB_TOT = FB_AL * 2,
    CUBE_FRAMES = 300
};

static const s16 sin_tab[360] = {
    0, 4, 8, 13, 17, 22, 26, 31, 35, 40, 44, 48, 53, 57, 61, 66, 70, 74, 79, 83, 87, 91, 95, 100, 104, 108, 112, 116, 120, 124, 127, 131, 135, 139, 143, 146, 150, 154, 157, 161, 164, 167, 171, 174, 177, 181, 184, 187, 190, 193, 196, 198, 201, 204, 207, 209, 212, 214, 217, 219, 221, 223, 226, 228, 230, 232, 233, 235, 237, 238, 240, 242, 243, 244, 246, 247, 248, 249, 250, 251, 252, 252, 253, 254, 254, 255, 255, 255, 255, 255, 256, 255, 255, 255, 255, 255, 254, 254, 253, 252, 252, 251, 250, 249, 248, 247, 246, 244, 243, 242, 240, 238, 237, 235, 233, 232, 230, 228, 226, 223, 221, 219, 217, 214, 212, 209, 207, 204, 201, 198, 196, 193, 190, 187, 184, 181, 177, 174, 171, 167, 164, 161, 157, 154, 150, 146, 143, 139, 135, 131, 127, 124, 120, 116, 112, 108, 104, 100, 95, 91, 87, 83, 79, 74, 70, 66, 61, 57, 53, 48, 44, 40, 35, 31, 26, 22, 17, 13, 8, 4, 0, -4, -8, -13, -17, -22, -26, -31, -35, -40, -44, -48, -53, -57, -61, -66, -70, -74, -79, -83, -87, -91, -95, -100, -104, -108, -112, -116, -120, -124, -128, -131, -135, -139, -143, -146, -150, -154, -157, -161, -164, -167, -171, -174, -177, -181, -184, -187, -190, -193, -196, -198, -201, -204, -207, -209, -212, -214, -217, -219, -221, -223, -226, -228, -230, -232, -233, -235, -237, -238, -240, -242, -243, -244, -246, -247, -248, -249, -250, -251, -252, -252, -253, -254, -254, -255, -255, -255, -255, -255, -256, -255, -255, -255, -255, -255, -254, -254, -253, -252, -252, -251, -250, -249, -248, -247, -246, -244, -243, -242, -240, -238, -237, -235, -233, -232, -230, -228, -226, -223, -221, -219, -217, -214, -212, -209, -207, -204, -201, -198, -196, -193, -190, -187, -184, -181, -177, -174, -171, -167, -164, -161, -157, -154, -150, -146, -143, -139, -135, -131, -128, -124, -120, -116, -112, -108, -104, -100, -95, -91, -87, -83, -79, -74, -70, -66, -61, -57, -53, -48, -44, -40, -35, -31, -26, -22, -17, -13, -8, -4
};

static const s16 cos_tab[360] = {
    256, 255, 255, 255, 255, 255, 254, 254, 253, 252, 252, 251, 250, 249, 248, 247, 246, 244, 243, 242, 240, 238, 237, 235, 233, 232, 230, 228, 226, 223, 221, 219, 217, 214, 212, 209, 207, 204, 201, 198, 196, 193, 190, 187, 184, 181, 177, 174, 171, 167, 164, 161, 157, 154, 150, 146, 143, 139, 135, 131, 128, 124, 120, 116, 112, 108, 104, 100, 95, 91, 87, 83, 79, 74, 70, 66, 61, 57, 53, 48, 44, 40, 35, 31, 26, 22, 17, 13, 8, 4, 0, -4, -8, -13, -17, -22, -26, -31, -35, -40, -44, -48, -53, -57, -61, -66, -70, -74, -79, -83, -87, -91, -95, -100, -104, -108, -112, -116, -120, -124, -127, -131, -135, -139, -143, -146, -150, -154, -157, -161, -164, -167, -171, -174, -177, -181, -184, -187, -190, -193, -196, -198, -201, -204, -207, -209, -212, -214, -217, -219, -221, -223, -226, -228, -230, -232, -233, -235, -237, -238, -240, -242, -243, -244, -246, -247, -248, -249, -250, -251, -252, -252, -253, -254, -254, -255, -255, -255, -255, -255, -256, -255, -255, -255, -255, -255, -254, -254, -253, -252, -252, -251, -250, -249, -248, -247, -246, -244, -243, -242, -240, -238, -237, -235, -233, -232, -230, -228, -226, -223, -221, -219, -217, -214, -212, -209, -207, -204, -201, -198, -196, -193, -190, -187, -184, -181, -177, -174, -171, -167, -164, -161, -157, -154, -150, -146, -143, -139, -135, -131, -128, -124, -120, -116, -112, -108, -104, -100, -95, -91, -87, -83, -79, -74, -70, -66, -61, -57, -53, -48, -44, -40, -35, -31, -26, -22, -17, -13, -8, -4, 0, 4, 8, 13, 17, 22, 26, 31, 35, 40, 44, 48, 53, 57, 61, 66, 70, 74, 79, 83, 87, 91, 95, 100, 104, 108, 112, 116, 120, 124, 128, 131, 135, 139, 143, 146, 150, 154, 157, 161, 164, 167, 171, 174, 177, 181, 184, 187, 190, 193, 196, 198, 201, 204, 207, 209, 212, 214, 217, 219, 221, 223, 226, 228, 230, 232, 233, 235, 237, 238, 240, 242, 243, 244, 246, 247, 248, 249, 250, 251, 252, 252, 253, 254, 254, 255, 255, 255, 255, 255
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

struct vec3 { s32 x, y, z; };

static const struct vec3 cube_vertices[8] = {
    {-100, -100, -100},
    { 100, -100, -100},
    { 100,  100, -100},
    {-100,  100, -100},
    {-100, -100,  100},
    { 100, -100,  100},
    { 100,  100,  100},
    {-100,  100,  100}
};

static const s32 cube_edges[12][2] = {
    {0,1}, {1,2}, {2,3}, {3,0},
    {4,5}, {5,6}, {6,7}, {7,4},
    {0,4}, {1,5}, {2,6}, {3,7}
};

__attribute__((section(".text._start")))
void _start(u64 eboot_base, u64 dlsym_addr, struct ext_args *ext) {
    if (!ext) return;
    ext->step = 1;

    struct ps5_example ex;
    if (ex_init(&ex, eboot_base, dlsym_addr, ext) != 0) { ext->status = -1; return; }

    /* VideoOut */
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
        /* Fallback takeover only if normal open fails. */
        if (cancel_fn && gs_thread) NC(ex.G, cancel_fn, gs_thread, 0, 0, 0, 0, 0);
        if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 300000, 0, 0, 0, 0, 0);
        if (vid_close && emu_vid >= 0) NC(ex.G, vid_close, (u64)emu_vid, 0, 0, 0, 0, 0);
        if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 100000, 0, 0, 0, 0, 0);
        video_h = vid_open ? ps5sdk_vo_open(ex.G, vid_open) : -1;
    }
    u64 phys = 0; void *vmem = 0;
    s32 frames_done = 0;

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
                u32 frame = 0;
                s32 rot_x = 0, rot_y = 0, rot_z = 0;
                struct vec3 proj[8];
                if (vid_rate) NC(ex.G, vid_rate, (u64)video_h, 0, 0, 0, 0, 0);
                
                for (s32 f = 0; f < CUBE_FRAMES; f++) {
                    s32 idx = (s32)(frame & 1u);
                    u32 *fb = (u32 *)fbs[idx];
                    
                    ps5sdk_fb_fill(fb, GW, GH, 0x00101010);
                    ps5sdk_fb_str(fb, GW, GH, 20, 20, "3D CUBE - REALTIME SOFTWARE RENDERER", 0x0005D9E8, 0, 3, 1);
                    ps5sdk_fb_str(fb, GW, GH, 20, 60, "OPTIONS to Exit", 0x00c0c0c0, 0, 2, 1);
                    
                    /* Rotate vertices */
                    s32 cx = cos_tab[rot_x], sx = sin_tab[rot_x];
                    s32 cy = cos_tab[rot_y], sy = sin_tab[rot_y];
                    s32 cz = cos_tab[rot_z], sz = sin_tab[rot_z];
                    
                    for (int i = 0; i < 8; i++) {
                        s32 x = cube_vertices[i].x;
                        s32 y = cube_vertices[i].y;
                        s32 z = cube_vertices[i].z;
                        
                        /* Rot Z */
                        s32 x1 = (x * cz - y * sz) >> 8;
                        s32 y1 = (y * cz + x * sz) >> 8;
                        s32 z1 = z;
                        
                        /* Rot X */
                        s32 y2 = (y1 * cx - z1 * sx) >> 8;
                        s32 z2 = (z1 * cx + y1 * sx) >> 8;
                        s32 x2 = x1;
                        
                        /* Rot Y */
                        s32 x3 = (x2 * cy + z2 * sy) >> 8;
                        s32 z3 = (z2 * cy - x2 * sy) >> 8;
                        s32 y3 = y2;
                        
                        /* Projection */
                        s32 dist = 400;
                        s32 z_offset = z3 + dist;
                        if (z_offset == 0) z_offset = 1;
                        
                        s32 fov = 1000;
                        proj[i].x = GW/2 + (x3 * fov) / z_offset;
                        proj[i].y = GH/2 + (y3 * fov) / z_offset;
                    }
                    
                    /* Draw edges */
                    u32 color = 0x00FF2A6D; /* Neon Pink */
                    for (int i = 0; i < 12; i++) {
                        s32 p1 = cube_edges[i][0];
                        s32 p2 = cube_edges[i][1];
                        draw_line(fb, proj[p1].x, proj[p1].y, proj[p2].x, proj[p2].y, color);
                    }
                    
                    /* Draw vertices as little rects */
                    for (int i = 0; i < 8; i++) {
                        ps5sdk_fb_rect(fb, GW, GH, proj[i].x - 4, proj[i].y - 4, 8, 8, 0x0005D9E8);
                    }
                    
                    rot_x = (rot_x + 1) % 360;
                    rot_y = (rot_y + 2) % 360;
                    rot_z = (rot_z + 1) % 360;

                    NC(ex.G, vid_flip, (u64)video_h, (u64)idx, 1, 0, 0, 0);
                    if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 16000, 0, 0, 0, 0, 0);
                    frame++;
                    frames_done++;
                }

                ps5sdk_fb_fill((u32*)fbs[0], GW, GH, 0);
                ps5sdk_fb_fill((u32*)fbs[1], GW, GH, 0);
                NC(ex.G, vid_flip, (u64)video_h, 0, 1, 0, 0, 0);
                if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 32000, 0, 0, 0, 0, 0);
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
    s32 ret = ex_open_text(
        &ex, buf, "fb_3d_cube\nframes=%d\nvideo=%x\nvmem=%p",
        frames_done, (u32)video_h, vmem
    );
    if (ret == 0) ex_wait_user_close(&ex);
    ex_close_dialog(&ex);
    ex_free_dialog(&ex, buf);

    ext->step = 99;
    ex_exit_clean(&ex, 0);
}
