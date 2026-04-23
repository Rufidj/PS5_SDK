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

    s32 load_ret = (s32)NC(ex.G, ex.load_mod, (u64)"libSceVideoOut.sprx", 0, 0, 0, 0, 0);
    void *vid_open = SYM(ex.G, ex.D, VIDEOOUT_HANDLE, "sceVideoOutOpen");
    void *vid_close = SYM(ex.G, ex.D, VIDEOOUT_HANDLE, "sceVideoOutClose");

    s32 tried_type = 0xFF;
    s32 video_h = try_video_open(ex.G, vid_open, tried_type);
    if (video_h < 0) {
        s32 tries[3] = { 0, 1, 2 };
        for (int i = 0; i < 3; i++) {
            tried_type = tries[i];
            video_h = try_video_open(ex.G, vid_open, tried_type);
            if (video_h >= 0) break;
            if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 50000, 0, 0, 0, 0, 0);
        }
    }

    s32 close_ret = 0x7fffffff;
    if (vid_close && video_h >= 0) {
        if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 100000, 0, 0, 0, 0, 0);
        close_ret = (s32)NC(ex.G, vid_close, (u64)video_h, 0, 0, 0, 0, 0);
    }

    ext->dbg[0] = (u64)(s64)video_h;
    ext->dbg[1] = (u64)(s64)close_ret;
    ext->dbg[2] = (u64)(s64)tried_type;
    ext->status = 0;

    u8 *buf = ex_alloc_dialog(&ex);
    if (!buf) {
        ext->status = -3;
        return;
    }

    ext->step = 2;
    ex_dialog_begin(&ex);
    if (video_h >= 0) {
        ret = ex_open_text(
            &ex,
            buf,
            "VideoOut open probe\nload=%x type=%x\nhandle=%x close=%x\nOpen/Close OK",
            (u32)load_ret,
            (u32)tried_type,
            (u32)video_h,
            (u32)close_ret
        );
    } else if ((u32)video_h == 0x80290009) {
        ret = ex_open_text(
            &ex,
            buf,
            "VideoOut open probe\nload=%x open=%x\nVideoOut ocupado o no permitido.\nSin buffers, sin cambios.",
            (u32)load_ret,
            (u32)video_h
        );
    } else {
        ret = ex_open_text(
            &ex,
            buf,
            "VideoOut open probe\nload=%x open=%x\nNo handle opened",
            (u32)load_ret,
            (u32)video_h
        );
    }
    if (ret == 0) ex_wait_user_close(&ex);
    ex_close_dialog(&ex);
    ex_free_dialog(&ex, buf);
    ext->step = 99;
}
