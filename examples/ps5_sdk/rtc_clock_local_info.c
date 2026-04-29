#include "example_common.h"

struct rtc_clock_data {
    u16 year;
    u16 month;
    u16 day;
    u16 hour;
    u16 minute;
    u16 second;
    u32 microsecond;
};

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
    void *get_clock_local = ex_sym_module(&ex, "libSceRtc.sprx", "sceRtcGetCurrentClockLocalTime");

    struct rtc_clock_data clk;
    clk.year = clk.month = clk.day = 0;
    clk.hour = clk.minute = clk.second = 0;
    clk.microsecond = 0;

    s32 clk_ret = get_clock_local ? (s32)NC(ex.G, get_clock_local, (u64)&clk, 0, 0, 0, 0, 0) : -1;

    ext->dbg[0] = (u64)(s64)rtc_mod;
    ext->dbg[1] = (u64)(s64)clk_ret;
    ext->dbg[2] = (u64)clk.year;
    ext->status = 0;

    u8 *buf = ex_alloc_dialog(&ex);
    if (!buf) {
        ext->status = -3;
        return;
    }

    ext->step = 2;
    ex_dialog_begin(&ex);
    if (clk_ret == 0 && clk.year >= 2000 && clk.year < 3000 &&
        clk.month >= 1 && clk.month <= 12 && clk.day >= 1 && clk.day <= 31) {
        ret = ex_open_text(
            &ex,
            buf,
            "RTC local clock\n%04d-%02d-%02d\n%02d:%02d:%02d\nusec=%u",
            (int)clk.year,
            (int)clk.month,
            (int)clk.day,
            (int)clk.hour,
            (int)clk.minute,
            (int)clk.second,
            (u32)clk.microsecond
        );
    } else {
        ret = ex_open_text(
            &ex,
            buf,
            "RTC local clock\nrtc=%x ret=%x\nY=%d M=%d D=%d\nh=%d m=%d s=%d",
            (u32)rtc_mod,
            (u32)clk_ret,
            (int)clk.year,
            (int)clk.month,
            (int)clk.day,
            (int)clk.hour,
            (int)clk.minute,
            (int)clk.second
        );
    }
    if (ret == 0) ex_wait_user_close(&ex);
    ex_close_dialog(&ex);
    ex_free_dialog(&ex, buf);
    ext->step = 99;
}
