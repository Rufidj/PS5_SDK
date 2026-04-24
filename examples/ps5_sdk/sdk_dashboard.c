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

static const char *net_state_name(s32 st) {
    if (st == 0) return "OFFLINE";
    if (st == 1) return "CONNECT";
    if (st == 2) return "IP WAIT";
    if (st == 3) return "ONLINE";
    return "UNKNOWN";
}

static void safe_copy_name(char *dst, const char *src, u32 max) {
    u32 i = 0;
    if (!dst || max == 0) return;
    if (src) {
        while (i + 1 < max && src[i]) {
            char c = src[i];
            dst[i] = (c >= 32 && c <= 126) ? c : '?';
            i++;
        }
    }
    dst[i] = 0;
}

static s64 get_sys_param_int(void *G, void *fn, s32 id, s32 *out) {
    if (!fn || !out) return -1;
    *out = 0;
    return (s64)NC(G, fn, (u64)id, (u64)out, 0, 0, 0, 0);
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

    NC(ex.G, ex.load_mod, (u64)"libSceUserService.sprx", 0, 0, 0, 0, 0);
    NC(ex.G, ex.load_mod, (u64)"libSceRtc.sprx", 0, 0, 0, 0, 0);
    NC(ex.G, ex.load_mod, (u64)"libSceNetCtl.sprx", 0, 0, 0, 0, 0);
    NC(ex.G, ex.load_mod, (u64)"libSceSystemService.sprx", 0, 0, 0, 0, 0);
    NC(ex.G, ex.load_mod, (u64)"libSceGnmDriver.sprx", 0, 0, 0, 0, 0);

    void *get_login_list = SYM(ex.G, ex.D, USERSERVICE_HANDLE, "sceUserServiceGetLoginUserIdList");
    void *get_user_name = SYM(ex.G, ex.D, USERSERVICE_HANDLE, "sceUserServiceGetUserName");
    void *get_clock_local = SYM(ex.G, ex.D, RTC_HANDLE, "sceRtcGetCurrentClockLocalTime");
    void *get_state_fn = SYM(ex.G, ex.D, NETCTL_HANDLE, "sceNetCtlGetState");
    void *param_get_int = SYM(ex.G, ex.D, SYSTEMSERVICE_HANDLE, "sceSystemServiceParamGetInt");
    void *get_clock = SYM(ex.G, ex.D, GNMDRIVER_HANDLE, "sceGnmGetGpuCoreClockFrequency");
    void *is_pa_enabled = SYM(ex.G, ex.D, GNMDRIVER_HANDLE, "sceGnmIsUserPaEnabled");

    u8 *work = (u8 *)NC(ex.G, ex.mmap_fn, 0, 0x1000, 3, 0x1002, (u64)-1, 0);
    if ((s64)work == -1) work = 0;

    char user_name[32];
    user_name[0] = 0;
    s32 ids[4];
    for (int i = 0; i < 4; i++) ids[i] = -1;

    if (work && get_login_list) {
        for (int i = 0; i < 16; i++) ((s32 *)work)[i] = -1;
        if ((s32)NC(ex.G, get_login_list, (u64)work, 0, 0, 0, 0, 0) == 0) {
            for (int i = 0; i < 4; i++) ids[i] = ((s32 *)work)[i];
        }
    }
    if (work && get_user_name) {
        s32 target_id = (ids[0] != -1) ? ids[0] : ex.user_id;
        ex_zero(work + 0x100, 0x100);
        if ((s32)NC(ex.G, get_user_name, (u64)target_id, (u64)(work + 0x100), 0x100, 0, 0, 0) == 0) {
            safe_copy_name(user_name, (char *)(work + 0x100), sizeof(user_name));
        }
    }

    struct rtc_clock_data clk;
    clk.year = clk.month = clk.day = 0;
    clk.hour = clk.minute = clk.second = 0;
    clk.microsecond = 0;
    s32 clk_ret = get_clock_local ? (s32)NC(ex.G, get_clock_local, (u64)&clk, 0, 0, 0, 0, 0) : -1;

    s32 net_state = -1;
    s32 net_ret = get_state_fn ? (s32)NC(ex.G, get_state_fn, (u64)&net_state, 0, 0, 0, 0, 0) : -1;

    s32 lang = 0;
    s32 time_zone = 0;
    s64 r_lang = get_sys_param_int(ex.G, param_get_int, 1, &lang);
    s64 r_zone = get_sys_param_int(ex.G, param_get_int, 4, &time_zone);

    u64 clock_hz = get_clock ? (u64)NC(ex.G, get_clock, 0, 0, 0, 0, 0, 0) : 0;
    s32 pa_enabled = is_pa_enabled ? (s32)NC(ex.G, is_pa_enabled, 0, 0, 0, 0, 0, 0) : -1;
    u32 clock_mhz = (u32)(clock_hz / 1000000ULL);

    if (!user_name[0]) {
        ps5_sdk_snprintf(user_name, sizeof(user_name), "user#%d", (int)ex.user_id);
    }

    ext->dbg[0] = (u64)net_state;
    ext->dbg[1] = (u64)clock_mhz;
    ext->dbg[2] = (u64)clk.year;
    ext->status = 0;

    u8 *buf = ex_alloc_dialog(&ex);
    if (!buf) {
        if (work && ex.munmap_fn) NC(ex.G, ex.munmap_fn, (u64)work, 0x1000, 0, 0, 0, 0);
        ext->status = -3;
        return;
    }

    ext->step = 2;
    ex_dialog_begin(&ex);
    ret = ex_open_text(
        &ex,
        buf,
        "SDK dashboard\n%s\n%04d-%02d-%02d %02d:%02d\nNet:%s GPU:%uMHz\nLang:%d TZ:%d PA:%d",
        user_name,
        (clk_ret == 0) ? (int)clk.year : 0,
        (clk_ret == 0) ? (int)clk.month : 0,
        (clk_ret == 0) ? (int)clk.day : 0,
        (clk_ret == 0) ? (int)clk.hour : 0,
        (clk_ret == 0) ? (int)clk.minute : 0,
        (net_ret == 0) ? net_state_name(net_state) : "N/A",
        (u32)clock_mhz,
        (r_lang == 0) ? (int)lang : -1,
        (r_zone == 0) ? (int)time_zone : -1,
        (int)pa_enabled
    );
    if (ret == 0) ex_wait_user_close(&ex);
    ex_close_dialog(&ex);

    ex_free_dialog(&ex, buf);
    if (work && ex.munmap_fn) NC(ex.G, ex.munmap_fn, (u64)work, 0x1000, 0, 0, 0, 0);
    ext->step = 99;
}
