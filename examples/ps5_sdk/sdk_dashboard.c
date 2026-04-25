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
        ps5_sdk_user_get_login_ids(ex.G, get_login_list, ids, 4, work, 0x1000);
    }
    if (work && get_user_name) {
        s32 target_id = (ids[0] != -1) ? ids[0] : ex.user_id;
        ps5_sdk_user_get_name(ex.G, get_user_name, target_id, user_name, sizeof(user_name), work + 0x100, 0x100);
    }

    struct ps5_sdk_rtc_clock clk;
    s32 clk_ret = ps5_sdk_rtc_get_local_clock(ex.G, get_clock_local, &clk);

    s32 net_state = -1;
    s32 net_ret = ps5_sdk_netctl_get_state(ex.G, get_state_fn, &net_state);

    s32 lang = 0;
    s32 time_zone = 0;
    s64 r_lang = ps5_sdk_system_get_int(ex.G, param_get_int, 1, &lang);
    s64 r_zone = ps5_sdk_system_get_int(ex.G, param_get_int, 4, &time_zone);

    u32 clock_mhz = ps5_sdk_gpu_get_core_clock_mhz(ex.G, get_clock);
    s32 pa_enabled = ps5_sdk_gpu_is_user_pa_enabled(ex.G, is_pa_enabled);

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
        (net_ret == 0) ? ps5_sdk_netctl_state_short_name(net_state) : "N/A",
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
