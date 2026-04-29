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

    s32 load_ret = ex_module_handle(&ex, "libScePad.sprx");
    void *pad_init = ex_sym_module(&ex, "libScePad.sprx", "scePadInit");
    void *pad_geth = ex_sym_module(&ex, "libScePad.sprx", "scePadGetHandle");
    void *pad_light = ex_sym_module(&ex, "libScePad.sprx", "scePadSetLightBar");
    void *pad_light_reset = ex_sym_module(&ex, "libScePad.sprx", "scePadResetLightBar");

    s32 init_ret = pad_init ? (s32)NC(ex.G, pad_init, 0, 0, 0, 0, 0, 0) : -1;
    s32 pad_h = pad_geth ? (s32)NC(ex.G, pad_geth, (u64)ex.user_id, 0, 0, 0, 0, 0) : -1;

    u8 *work = (u8 *)NC(ex.G, ex.mmap_fn, 0, 0x1000, 3, 0x1002, (u64)-1, 0);
    if ((s64)work == -1) work = 0;

    s32 r1 = -1;
    s32 r2 = -1;
    s32 r3 = -1;
    s32 rr = -1;

    if (pad_light && pad_h >= 0 && work) {
        r1 = ps5_sdk_pad_set_lightbar(ex.G, pad_light, pad_h, work, 0xFF, 0x20, 0x20);
        if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 500000, 0, 0, 0, 0, 0);
        r2 = ps5_sdk_pad_set_lightbar(ex.G, pad_light, pad_h, work, 0x20, 0xFF, 0x40);
        if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 500000, 0, 0, 0, 0, 0);
        r3 = ps5_sdk_pad_set_lightbar(ex.G, pad_light, pad_h, work, 0x30, 0x60, 0xFF);
        if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 700000, 0, 0, 0, 0, 0);
    }

    if (pad_light_reset && pad_h >= 0) {
        rr = ps5_sdk_pad_reset_lightbar(ex.G, pad_light_reset, pad_h);
    }

    ext->dbg[0] = (u64)(s64)pad_h;
    ext->dbg[1] = (u64)(s64)r1;
    ext->dbg[2] = (u64)(s64)r2;
    ext->dbg[3] = (u64)(s64)r3;
    ext->dbg[4] = (u64)(s64)rr;
    ext->status = 0;

    u8 *buf = ex_alloc_dialog(&ex);
    if (buf) {
        ex_dialog_begin(&ex);
        ret = ex_open_text(
            &ex,
            buf,
            "Pad RGB cycle\nload=%x init=%x h=%x\nr=%x g=%x b=%x\nreset=%x",
            (u32)load_ret,
            (u32)init_ret,
            (u32)pad_h,
            (u32)r1,
            (u32)r2,
            (u32)r3,
            (u32)rr
        );
        if (ret == 0) ex_wait_user_close(&ex);
        ex_close_dialog(&ex);
        ex_free_dialog(&ex, buf);
    }

    if (work && ex.munmap_fn) NC(ex.G, ex.munmap_fn, (u64)work, 0x1000, 0, 0, 0, 0);
    ext->step = 99;
}
