#include "example_common.h"

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
    void *vid_reg = SYM(ex.G, ex.D, VIDEOOUT_HANDLE, "sceVideoOutRegisterBuffers");
    void *vid_flip = SYM(ex.G, ex.D, VIDEOOUT_HANDLE, "sceVideoOutSubmitFlip");
    void *vid_rate = SYM(ex.G, ex.D, VIDEOOUT_HANDLE, "sceVideoOutSetFlipRate");
    void *vid_evt = SYM(ex.G, ex.D, VIDEOOUT_HANDLE, "sceVideoOutAddFlipEvent");

    u32 resolved = 0;
    if (vid_open) resolved++;
    if (vid_close) resolved++;
    if (vid_reg) resolved++;
    if (vid_flip) resolved++;
    if (vid_rate) resolved++;
    if (vid_evt) resolved++;

    ext->dbg[0] = (u64)(s64)load_ret;
    ext->dbg[1] = (u64)resolved;
    ext->dbg[2] = (u64)vid_open;
    ext->status = 0;

    u8 *buf = ex_alloc_dialog(&ex);
    if (!buf) {
        ext->status = -3;
        return;
    }

    ext->step = 2;
    ex_dialog_begin(&ex);
    ret = ex_open_text(
        &ex,
        buf,
        "VideoOut info\nload=%x resolved=%d/6\nopen=%p close=%p\nreg=%p flip=%p\nresolve-only OK",
        (u32)load_ret,
        (int)resolved,
        vid_open,
        vid_close,
        vid_reg,
        vid_flip
    );
    if (ret == 0) ex_wait_user_close(&ex);
    ex_close_dialog(&ex);
    ex_free_dialog(&ex, buf);
    ext->step = 99;
}
