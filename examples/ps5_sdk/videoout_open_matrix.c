#include "example_common.h"

struct vo_try {
    s32 user_type;
    s32 index;
    s32 ret;
};

__attribute__((section(".text._start")))
void _start(u64 eboot_base, u64 dlsym_addr, struct ext_args *ext) {
    static struct vo_try tries[] = {
        { 0xFF, 0, 0x7fffffff },
        { 0,    0, 0x7fffffff },
        { 1,    0, 0x7fffffff },
        { 2,    0, 0x7fffffff },
        { 0xFF, 1, 0x7fffffff },
        { 0,    1, 0x7fffffff },
    };

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

    for (u32 i = 0; i < (u32)(sizeof(tries) / sizeof(tries[0])); i++) {
        if (!vid_open) {
            tries[i].ret = -1;
            continue;
        }
        tries[i].ret = (s32)NC(ex.G, vid_open, (u64)tries[i].user_type, (u64)tries[i].index, 0, 0, 0, 0);
        if (tries[i].ret >= 0 && vid_close) {
            NC(ex.G, vid_close, (u64)tries[i].ret, 0, 0, 0, 0, 0);
        }
        if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 50000, 0, 0, 0, 0, 0);
    }

    ext->dbg[0] = (u64)(u32)load_ret;
    for (u32 i = 0; i < (u32)(sizeof(tries) / sizeof(tries[0])) && i < 6; i++) {
        ext->dbg[1 + i] = ((u64)(u32)tries[i].user_type << 32) | (u32)tries[i].ret;
    }
    ext->status = 0;

    u8 *buf = ex_alloc_dialog(&ex);
    if (!buf) {
        ext->status = -3;
        return;
    }

    ex_dialog_begin(&ex);
    ret = ex_open_text(
        &ex,
        buf,
        "VO matrix\nff/0=%x 0/0=%x\n1/0=%x 2/0=%x\nff/1=%x 0/1=%x",
        (u32)tries[0].ret,
        (u32)tries[1].ret,
        (u32)tries[2].ret,
        (u32)tries[3].ret,
        (u32)tries[4].ret,
        (u32)tries[5].ret
    );
    if (ret == 0) ex_wait_user_close(&ex);
    ex_close_dialog(&ex);
    ex_free_dialog(&ex, buf);
    ext->step = 99;
}
