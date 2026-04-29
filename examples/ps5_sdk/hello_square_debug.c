#include "example_common.h"

enum {
    DBG_SCR_W = 1920,
    DBG_SCR_H = 1080,
    DBG_FB_SIZE = DBG_SCR_W * DBG_SCR_H * 4,
    DBG_FB_ALIGNED = (DBG_FB_SIZE + 0x1FFFFF) & ~0x1FFFFF,
    DBG_FB_TOTAL = DBG_FB_ALIGNED * 2,
};

static void fill_fb(u32 *fb, u32 color) {
    for (u32 i = 0; i < (u32)(DBG_SCR_W * DBG_SCR_H); i++) fb[i] = color;
}

static void draw_rect(u32 *fb, s32 x, s32 y, s32 w, s32 h, u32 color) {
    if (!fb) return;
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > DBG_SCR_W) w = DBG_SCR_W - x;
    if (y + h > DBG_SCR_H) h = DBG_SCR_H - y;
    if (w <= 0 || h <= 0) return;
    for (s32 yy = 0; yy < h; yy++) {
        u32 *row = fb + (u32)(y + yy) * DBG_SCR_W + (u32)x;
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

    s32 video = -1;
    s32 reg_ret = 0x7fffffff;
    s32 flip_ret = 0x7fffffff;
    u64 phys = 0;
    void *vmem = 0;

    if (alloc_dm && map_dm && vid_open && vid_reg && vid_flip) {
        video = (s32)NC(ex.G, vid_open, 0xFF, 0, 0, 0, 0, 0);
        if (video >= 0) {
            NC(ex.G, alloc_dm, 0, 0x300000000ULL, DBG_FB_TOTAL, 0x200000, 3, (u64)&phys);
            NC(ex.G, map_dm, (u64)&vmem, DBG_FB_TOTAL, 0x33, 0, phys, 0x200000);
            if (vmem) {
                u8 attr[64];
                ex_zero(attr, sizeof(attr));
                *(u32 *)(attr + 0) = 0x80000000;
                *(u32 *)(attr + 4) = 1;
                *(u32 *)(attr + 12) = DBG_SCR_W;
                *(u32 *)(attr + 16) = DBG_SCR_H;
                *(u32 *)(attr + 20) = DBG_SCR_W;

                void *fbs[2];
                fbs[0] = vmem;
                fbs[1] = (u8 *)vmem + DBG_FB_ALIGNED;

                reg_ret = (s32)NC(ex.G, vid_reg, (u64)video, 0, (u64)fbs, 2, (u64)attr, 0);
                if (reg_ret == 0) {
                    if (vid_rate) NC(ex.G, vid_rate, (u64)video, 0, 0, 0, 0, 0);
                    fill_fb((u32 *)fbs[0], 0x00102040);
                    draw_rect((u32 *)fbs[0], (DBG_SCR_W - 240) / 2, (DBG_SCR_H - 240) / 2, 240, 240, 0x00f04020);
                    flip_ret = (s32)NC(ex.G, vid_flip, (u64)video, 0, 1, 0, 0, 0);
                    if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 500000, 0, 0, 0, 0, 0);
                }
            }
        }
    }

    ext->dbg[0] = (u64)(u32)vid_mod;
    ext->dbg[1] = (u64)(u32)video;
    ext->dbg[2] = (u64)phys;
    ext->dbg[3] = (u64)vmem;
    ext->dbg[4] = (u64)(u32)reg_ret;
    ext->dbg[5] = (u64)(u32)flip_ret;
    ext->status = 0;

    if (vid_close && video >= 0) NC(ex.G, vid_close, (u64)video, 0, 0, 0, 0, 0);

    u8 *buf = ex_alloc_dialog(&ex);
    if (!buf) {
        ext->status = -3;
        return;
    }

    ex_dialog_begin(&ex);
    ret = ex_open_text(
        &ex,
        buf,
        "Hello square dbg\nvid=%x reg=%x\nflip=%x\nphys=%lx\nvmem=%p",
        (u32)video,
        (u32)reg_ret,
        (u32)flip_ret,
        phys,
        vmem
    );
    if (ret == 0) ex_wait_user_close(&ex);
    ex_close_dialog(&ex);
    ex_free_dialog(&ex, buf);
    ext->step = 99;
}
