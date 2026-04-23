#include "example_common.h"

enum {
    HELLO_SCR_W = 1920,
    HELLO_SCR_H = 1080,
    HELLO_FB_SIZE = HELLO_SCR_W * HELLO_SCR_H * 4,
    HELLO_FB_ALIGNED = (HELLO_FB_SIZE + 0x1FFFFF) & ~0x1FFFFF,
    HELLO_FB_TOTAL = HELLO_FB_ALIGNED * 2,
};

static void fill_fb(u32 *fb, u32 color) {
    for (u32 i = 0; i < (u32)(HELLO_SCR_W * HELLO_SCR_H); i++) fb[i] = color;
}

static void draw_rect(u32 *fb, s32 x, s32 y, s32 w, s32 h, u32 color) {
    if (!fb) return;
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > HELLO_SCR_W) w = HELLO_SCR_W - x;
    if (y + h > HELLO_SCR_H) h = HELLO_SCR_H - y;
    if (w <= 0 || h <= 0) return;

    for (s32 yy = 0; yy < h; yy++) {
        u32 *row = fb + (u32)(y + yy) * HELLO_SCR_W + (u32)x;
        for (s32 xx = 0; xx < w; xx++) row[xx] = color;
    }
}

__attribute__((section(".text._start")))
void _start(u64 eboot_base, u64 dlsym_addr, struct ext_args *ext) {
    if (!ext) return;
    ext->step = 1;

    struct ps5_example ex;
    s32 ret = ex_init(&ex, eboot_base, dlsym_addr, ext);
    if (ret != 0) {
        ext->status = ret;
        return;
    }

    void *alloc_dm = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, KERNEL_sceKernelAllocateDirectMemory);
    void *map_dm = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, KERNEL_sceKernelMapDirectMemory);
    s32 vid_mod = (s32)NC(ex.G, ex.load_mod, (u64)"libSceVideoOut.sprx", 0, 0, 0, 0, 0);
    void *vid_open = SYM(ex.G, ex.D, VIDEOOUT_HANDLE, "sceVideoOutOpen");
    void *vid_close = SYM(ex.G, ex.D, VIDEOOUT_HANDLE, "sceVideoOutClose");
    void *vid_reg = SYM(ex.G, ex.D, VIDEOOUT_HANDLE, "sceVideoOutRegisterBuffers");
    void *vid_flip = SYM(ex.G, ex.D, VIDEOOUT_HANDLE, "sceVideoOutSubmitFlip");
    void *vid_rate = SYM(ex.G, ex.D, VIDEOOUT_HANDLE, "sceVideoOutSetFlipRate");

    ext->dbg[0] = (u64)(u32)vid_mod;
    ext->dbg[1] = (u64)vid_open;
    ext->dbg[2] = (u64)vid_reg;

    if (!alloc_dm || !map_dm || !vid_open || !vid_reg || !vid_flip) {
        ext->status = -10;
        return;
    }

    s32 video = (s32)NC(ex.G, vid_open, 0xFF, 0, 0, 0, 0, 0);
    if (video < 0) {
        ext->dbg[3] = (u64)(u32)video;
        ext->status = -11;
        return;
    }

    u64 phys = 0;
    NC(ex.G, alloc_dm, 0, 0x300000000ULL, HELLO_FB_TOTAL, 0x200000, 3, (u64)&phys);
    void *vmem = 0;
    NC(ex.G, map_dm, (u64)&vmem, HELLO_FB_TOTAL, 0x33, 0, phys, 0x200000);
    if (!vmem) {
        ext->status = -12;
        return;
    }

    u8 attr[64];
    ex_zero(attr, sizeof(attr));
    *(u32 *)(attr + 0) = 0x80000000;
    *(u32 *)(attr + 4) = 1;
    *(u32 *)(attr + 12) = HELLO_SCR_W;
    *(u32 *)(attr + 16) = HELLO_SCR_H;
    *(u32 *)(attr + 20) = HELLO_SCR_W;

    void *fbs[2];
    fbs[0] = vmem;
    fbs[1] = (u8 *)vmem + HELLO_FB_ALIGNED;

    ret = (s32)NC(ex.G, vid_reg, (u64)video, 0, (u64)fbs, 2, (u64)attr, 0);
    if (ret != 0) {
        ext->dbg[4] = (u64)(u32)ret;
        ext->status = -13;
        return;
    }

    if (vid_rate) NC(ex.G, vid_rate, (u64)video, 0, 0, 0, 0, 0);

    u32 *fb0 = (u32 *)fbs[0];
    fill_fb(fb0, 0x00201830);
    draw_rect(fb0, (HELLO_SCR_W - 200) / 2, (HELLO_SCR_H - 200) / 2, 200, 200, 0x00f06020);
    draw_rect(fb0, (HELLO_SCR_W - 140) / 2, (HELLO_SCR_H - 140) / 2, 140, 140, 0x00fff4d0);

    ret = (s32)NC(ex.G, vid_flip, (u64)video, 0, 1, 0, 0, 0);
    ext->dbg[5] = (u64)(u32)ret;
    ext->dbg[6] = (u64)(u32)video;
    ext->status = 0;
    ext->step = 2;

    if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 3000000, 0, 0, 0, 0, 0);

    if (vid_close) NC(ex.G, vid_close, (u64)video, 0, 0, 0, 0, 0);
    ext->step = 99;
}
