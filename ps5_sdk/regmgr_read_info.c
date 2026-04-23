#include "example_common.h"

static s64 reg_get_int(void *G, void *fn, const char *key, s32 *out) {
    if (!fn || !key || !out) return -1;
    *out = 0x7fffffff;
    return (s64)NC(G, fn, (u64)key, (u64)out, 0, 0, 0, 0);
}

static s64 reg_get_str(void *G, void *fn, const char *key, char *out, u64 size) {
    if (!fn || !key || !out || size == 0) return -1;
    for (u64 i = 0; i < size; i++) out[i] = 0;
    return (s64)NC(G, fn, (u64)key, (u64)out, size - 1, 0, 0, 0);
}

static void sanitize_ascii(char *s) {
    if (!s) return;
    for (int i = 0; i < 96 && s[i]; i++) {
        if (s[i] < 32 || s[i] > 126) s[i] = '?';
    }
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

    NC(ex.G, ex.load_mod, (u64)"libSceRegMgr.sprx", 0, 0, 0, 0, 0);
    void *get_int = SYM(ex.G, ex.D, REGMGR_HANDLE, "sceRegMgrGetInt");
    void *get_str = SYM(ex.G, ex.D, REGMGR_HANDLE, "sceRegMgrGetStr");

    s32 lang = -1;
    s32 np = -1;
    s32 devenv0 = -1;
    s32 security0 = -1;
    s64 r_lang = reg_get_int(ex.G, get_int, "/SYSTEM/LANGUAGE/0", &lang);
    s64 r_np = reg_get_int(ex.G, get_int, "/CONFIG/NP/0", &np);
    s64 r_devenv0 = reg_get_int(ex.G, get_int, "/CONFIG/DEVENV/0", &devenv0);
    s64 r_security0 = reg_get_int(ex.G, get_int, "/CONFIG/SECURITY/0", &security0);

    u8 *work = (u8 *)NC(ex.G, ex.mmap_fn, 0, 0x1000, 3, 0x1002, (u64)-1, 0);
    if ((s64)work == -1) work = 0;

    char *system0 = work ? (char *)work : 0;
    char *swver = work ? (char *)(work + 0x100) : 0;
    s64 r_system0 = work ? reg_get_str(ex.G, get_str, "/CONFIG/SYSTEM/0", system0, 96) : -1;
    s64 r_swver = work ? reg_get_str(ex.G, get_str, "/SYSTEM/SWVERSION/0", swver, 96) : -1;
    sanitize_ascii(system0);
    sanitize_ascii(swver);

    ext->dbg[0] = (u64)(s64)lang;
    ext->dbg[1] = (u64)(s64)np;
    ext->dbg[2] = (u64)(s64)devenv0;
    ext->status = 0;

    u8 *buf = ex_alloc_dialog(&ex);
    if (!buf) {
        if (work && ex.munmap_fn) NC(ex.G, ex.munmap_fn, (u64)work, 0x1000, 0, 0, 0, 0);
        ext->status = -3;
        return;
    }

    ext->step = 2;
    ex_dialog_begin(&ex);
    if ((u32)r_lang == 0x80020001 && (u32)r_np == 0x80020001 &&
        (u32)r_devenv0 == 0x80020001 && (u32)r_security0 == 0x80020001) {
        ret = ex_open_text(
            &ex,
            buf,
            "RegMgr read-only\nLectura bloqueada por permisos.\nNo se escribio nada.\nRegMgr protegido OK"
        );
    } else if (r_lang == 0 && r_np == 0 && r_devenv0 == 0 && r_security0 == 0) {
        ret = ex_open_text(
            &ex,
            buf,
            "RegMgr read-only\nlang=%d np=%d dev=%d sec=%d\nsys=%s\nsw=%s",
            (int)lang,
            (int)np,
            (int)devenv0,
            (int)security0,
            (r_system0 == 0 && system0 && system0[0]) ? system0 : "n/a",
            (r_swver == 0 && swver && swver[0]) ? swver : "n/a"
        );
    } else {
        ret = ex_open_text(
            &ex,
            buf,
            "RegMgr read-only\nret=%x %x %x %x\nvals=%d %d %d %d",
            (u32)r_lang,
            (u32)r_np,
            (u32)r_devenv0,
            (u32)r_security0,
            (int)lang,
            (int)np,
            (int)devenv0,
            (int)security0
        );
    }
    if (ret == 0) ex_wait_user_close(&ex);
    ex_close_dialog(&ex);
    ex_free_dialog(&ex, buf);
    if (work && ex.munmap_fn) NC(ex.G, ex.munmap_fn, (u64)work, 0x1000, 0, 0, 0, 0);
    ext->step = 99;
}
