#include "example_common.h"

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
    s32 load_ret = ex_module_handle(&ex, "libSceVideoOut.sprx");
    void *vid_open = ex_sym_module(&ex, "libSceVideoOut.sprx", "sceVideoOutOpen");
    void *vid_close = ex_sym_module(&ex, "libSceVideoOut.sprx", "sceVideoOutClose");

    s32 emu_vid = *(s32 *)(eboot_base + EBOOT_VIDOUT);
    u64 gs_thread = *(u64 *)(eboot_base + EBOOT_GS_THREAD);

    s32 cancel_ret = 0x7fffffff;
    s32 close_ret = 0x7fffffff;
    s32 tried_type = 0xFF;
    s32 video_h = -1;

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

    ext->dbg[0] = (u64)(u32)emu_vid;
    ext->dbg[1] = gs_thread;
    ext->dbg[2] = (u64)(u32)cancel_ret;
    ext->dbg[3] = (u64)(u32)close_ret;
    ext->dbg[4] = (u64)(u32)tried_type;
    ext->dbg[5] = (u64)(u32)video_h;
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
        "VO takeover probe\nload=%x raw=%x\ngs=%lx cancel=%x\nclose=%x type=%x\nopen=%x",
        (u32)load_ret,
        (u32)emu_vid,
        gs_thread,
        (u32)cancel_ret,
        (u32)close_ret,
        (u32)tried_type,
        (u32)video_h
    );
    if (ret == 0) ex_wait_user_close(&ex);
    ex_close_dialog(&ex);
    ex_free_dialog(&ex, buf);
    ext->step = 99;
}
