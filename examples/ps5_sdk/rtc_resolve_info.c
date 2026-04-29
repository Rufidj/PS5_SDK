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
    void *get_clock = ex_sym_module(&ex, "libSceRtc.sprx", "sceRtcGetCurrentClock");
    void *get_clock_local = ex_sym_module(&ex, "libSceRtc.sprx", "sceRtcGetCurrentClockLocalTime");
    void *tick_to_time = ex_sym_module(&ex, "libSceRtc.sprx", "sceRtcSetTick");
    void *format_rfc = ex_sym_module(&ex, "libSceRtc.sprx", "sceRtcFormatRFC3339");
    void *get_day = ex_sym_module(&ex, "libSceRtc.sprx", "sceRtcGetDayOfWeek");

    u32 resolved = 0;
    if (get_tick) resolved++;
    if (get_clock) resolved++;
    if (get_clock_local) resolved++;
    if (tick_to_time) resolved++;
    if (format_rfc) resolved++;
    if (get_day) resolved++;

    ext->dbg[0] = (u64)(s64)rtc_mod;
    ext->dbg[1] = (u64)resolved;
    ext->dbg[2] = (u64)get_tick;
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
        "RTC resolve\nrtc=%x resolved=%d/6\ntick=%p local=%p\nfmt=%p day=%p\nresolve-only OK",
        (u32)rtc_mod,
        (int)resolved,
        get_tick,
        get_clock_local,
        format_rfc,
        get_day
    );
    if (ret == 0) ex_wait_user_close(&ex);
    ex_close_dialog(&ex);
    ex_free_dialog(&ex, buf);
    ext->step = 99;
}
