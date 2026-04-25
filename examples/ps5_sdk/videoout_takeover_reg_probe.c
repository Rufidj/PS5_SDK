#include "example_common.h"

enum {
    TAKE_W = 1920,
    TAKE_H = 1080,
    TAKE_FB_SIZE = TAKE_W * TAKE_H * 4,
    TAKE_FB_ALIGNED = (TAKE_FB_SIZE + 0x1FFFFF) & ~0x1FFFFF,
    TAKE_FB_TOTAL = TAKE_FB_ALIGNED * 2,
};

static s32 try_video_open(void *G, void *open_fn, s32 user_type) {
    if (!open_fn) return -1;
    return (s32)NC(G, open_fn, (u64)user_type, 0, 0, 0, 0, 0);
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

    void *cancel_fn = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, KERNEL_scePthreadCancel);
    void *alloc_dm = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, KERNEL_sceKernelAllocateDirectMemory);
    void *map_dm = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, KERNEL_sceKernelMapDirectMemory);
    s32 load_ret = ps5_sdk_load_sprx(ex.G, ex.load_mod, "libSceVideoOut.sprx");
    void *vid_open = SYM(ex.G, ex.D, VIDEOOUT_HANDLE, "sceVideoOutOpen");
    void *vid_close = SYM(ex.G, ex.D, VIDEOOUT_HANDLE, "sceVideoOutClose");
    void *vid_reg = SYM(ex.G, ex.D, VIDEOOUT_HANDLE, "sceVideoOutRegisterBuffers");

    s32 emu_vid = *(s32 *)(eboot_base + EBOOT_VIDOUT);
    u64 gs_thread = *(u64 *)(eboot_base + EBOOT_GS_THREAD);

    s32 cancel_ret = 0x7fffffff;
    s32 close_ret = 0x7fffffff;
    s32 tried_type = 0xFF;
    s32 video_h = -1;
    s32 reg_ret = 0x7fffffff;
    u64 phys = 0;
    void *vmem = 0;

    if (cancel_fn && gs_thread) {
        cancel_ret = (s32)NC(ex.G, cancel_fn, gs_thread, 0, 0, 0, 0, 0);
        if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 300000, 0, 0, 0, 0, 0);
    }

    if (vid_close && emu_vid >= 0) {
        close_ret = (s32)NC(ex.G, vid_close, (u64)emu_vid, 0, 0, 0, 0, 0);
        if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 100000, 0, 0, 0, 0, 0);
    }

    video_h = try_video_open(ex.G, vid_open, tried_type);
    if (video_h < 0) {
        s32 tries[3] = { 0, 1, 2 };
        for (int i = 0; i < 3; i++) {
            tried_type = tries[i];
            video_h = try_video_open(ex.G, vid_open, tried_type);
            if (video_h >= 0) break;
            if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 50000, 0, 0, 0, 0, 0);
        }
    }

    if (video_h >= 0 && alloc_dm && map_dm && vid_reg) {
        NC(ex.G, alloc_dm, 0, 0x300000000ULL, TAKE_FB_TOTAL, 0x200000, 3, (u64)&phys);
        NC(ex.G, map_dm, (u64)&vmem, TAKE_FB_TOTAL, 0x33, 0, phys, 0x200000);
        if (vmem) {
            u8 attr[64];
            void *fbs[2];

            ex_zero(attr, sizeof(attr));
            *(u32 *)(attr + 0) = 0x80000000;
            *(u32 *)(attr + 4) = 1;
            *(u32 *)(attr + 12) = TAKE_W;
            *(u32 *)(attr + 16) = TAKE_H;
            *(u32 *)(attr + 20) = TAKE_W;

            fbs[0] = vmem;
            fbs[1] = (u8 *)vmem + TAKE_FB_ALIGNED;

            reg_ret = (s32)NC(ex.G, vid_reg, (u64)video_h, 0, (u64)fbs, 2, (u64)attr, 0);
        }
    }

    ext->dbg[0] = (u64)(u32)emu_vid;
    ext->dbg[1] = gs_thread;
    ext->dbg[2] = ((u64)(u32)cancel_ret << 32) | (u32)close_ret;
    ext->dbg[3] = ((u64)(u32)tried_type << 32) | (u32)video_h;
    ext->dbg[4] = phys;
    ext->dbg[5] = (u64)vmem;
    ext->dbg[6] = (u64)(u32)reg_ret;
    ext->dbg[7] = (u64)(u32)load_ret;
    ext->status = 0;

    if (vid_close && video_h >= 0) {
        NC(ex.G, vid_close, (u64)video_h, 0, 0, 0, 0, 0);
    }

    u8 *buf = ex_alloc_dialog(&ex);
    if (!buf) {
        ext->status = -3;
        return;
    }

    ex_dialog_begin(&ex);
    ret = ex_open_text(
        &ex,
        buf,
        "VO takeover reg\nload=%x raw=%x\ngs=%lx cancel=%x\nclose=%x type=%x\nopen=%x reg=%x\nphys=%lx\nvmem=%p",
        (u32)load_ret,
        (u32)emu_vid,
        gs_thread,
        (u32)cancel_ret,
        (u32)close_ret,
        (u32)tried_type,
        (u32)video_h,
        (u32)reg_ret,
        phys,
        vmem
    );
    if (ret == 0) ex_wait_user_close(&ex);
    ex_close_dialog(&ex);
    ex_free_dialog(&ex, buf);
    ext->step = 99;
}
