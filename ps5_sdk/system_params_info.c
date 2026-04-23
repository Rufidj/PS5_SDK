#include "example_common.h"

static s64 get_sys_param_int(void *G, void *fn, s32 id, s32 *out) {
    if (!fn || !out) return -1;
    *out = 0x7fffffff;
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

    NC(ex.G, ex.load_mod, (u64)"libSceSystemService.sprx", 0, 0, 0, 0, 0);
    void *param_get_int = SYM(ex.G, ex.D, SYSTEMSERVICE_HANDLE, "sceSystemServiceParamGetInt");

    s32 lang = 0;
    s32 date_fmt = 0;
    s32 time_fmt = 0;
    s32 time_zone = 0;
    s32 summer = 0;
    s32 parental = 0;

    s64 r_lang = get_sys_param_int(ex.G, param_get_int, 1, &lang);
    s64 r_date = get_sys_param_int(ex.G, param_get_int, 2, &date_fmt);
    s64 r_time = get_sys_param_int(ex.G, param_get_int, 3, &time_fmt);
    s64 r_zone = get_sys_param_int(ex.G, param_get_int, 4, &time_zone);
    s64 r_summer = get_sys_param_int(ex.G, param_get_int, 5, &summer);
    s64 r_parental = get_sys_param_int(ex.G, param_get_int, 7, &parental);

    ext->dbg[0] = (u64)(s64)lang;
    ext->dbg[1] = (u64)(s64)date_fmt;
    ext->dbg[2] = (u64)(s64)time_zone;
    ext->status = 0;

    u8 *buf = ex_alloc_dialog(&ex);
    if (!buf) {
        ext->status = -3;
        return;
    }

    ext->step = 2;
    ex_dialog_begin(&ex);
    if (r_lang == 0 && r_date == 0 && r_time == 0 && r_zone == 0 && r_summer == 0 && r_parental == 0) {
        ret = ex_open_text(
            &ex,
            buf,
            "SystemService params\nlang=%d date=%d time=%d\nzone=%d summer=%d parental=%d\nSystemService OK",
            (int)lang,
            (int)date_fmt,
            (int)time_fmt,
            (int)time_zone,
            (int)summer,
            (int)parental
        );
    } else {
        ret = ex_open_text(
            &ex,
            buf,
            "SystemService params\nret: %x %x %x %x\nvals: %d %d %d %d",
            (u32)r_lang,
            (u32)r_date,
            (u32)r_time,
            (u32)r_zone,
            (int)lang,
            (int)date_fmt,
            (int)time_fmt,
            (int)time_zone
        );
    }
    if (ret == 0) ex_wait_user_close(&ex);
    ex_close_dialog(&ex);
    ex_free_dialog(&ex, buf);
    ext->step = 99;
}
