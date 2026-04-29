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

    s32 rtc_mod = ex_module_handle(&ex, "libSceRtc.sprx");
    void *get_tick = ex_sym_module(&ex, "libSceRtc.sprx", "sceRtcGetCurrentTick");

    u64 tick = 0;
    s32 tick_ret = get_tick ? (s32)NC(ex.G, get_tick, (u64)&tick, 0, 0, 0, 0, 0) : -1;

    ext->dbg[0] = (u64)(s64)rtc_mod;
    ext->dbg[1] = (u64)(s64)tick_ret;
    ext->dbg[2] = tick;
    ext->status = 0;

    u8 *buf = ex_alloc_dialog(&ex);
    if (!buf) {
        ext->status = -3;
        return;
    }

    ext->step = 2;
    ex_dialog_begin(&ex);
    if (tick_ret == 0) {
        ret = ex_open_text(
            &ex,
            buf,
            "RTC tick\nrtc=%x ret=%x\ntick=%llx\nsceRtcGetCurrentTick OK",
            (u32)rtc_mod,
            (u32)tick_ret,
            tick
        );
    } else {
        ret = ex_open_text(
            &ex,
            buf,
            "RTC tick\nrtc=%x ret=%x\nfn=%p\nNo tick",
            (u32)rtc_mod,
            (u32)tick_ret,
            get_tick
        );
    }
    if (ret == 0) ex_wait_user_close(&ex);
    ex_close_dialog(&ex);
    ex_free_dialog(&ex, buf);
    ext->step = 99;
}
