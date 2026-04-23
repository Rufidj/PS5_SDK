#include "example_common.h"

struct path_probe {
    const char *label;
    const char *path;
};

static void show_text_and_wait(struct ps5_example *ex, u8 *buf, const char *fmt, ...) {
    __builtin_va_list ap;
    char *msg = (char *)(buf + 0xE0);
    u8 *dp = buf;
    u8 *pp = buf + 0x88;

    ex_zero(buf, 0x400);
    __builtin_va_start(ap, fmt);
    ps5_sdk_vsnprintf(msg, 160, fmt, ap);
    __builtin_va_end(ap);
    ex_setup_user_msg(dp, pp, msg, ex->user_id);
    ex_dialog_begin(ex);
    if ((s32)NC(ex->G, ex->dlg_open, (u64)dp, 0, 0, 0, 0, 0) == 0) {
        ex_wait_user_close(ex);
    }
    ex_close_dialog(ex);
}

static void run_probe_dialog(struct ps5_example *ex, u8 *buf, void *statfs_fn, const struct path_probe *p, u64 *dbg_slot) {
    s32 ret = -1;
    u8 *st = 0;

    show_text_and_wait(ex, buf, "statfs probe\nVamos a probar:\n%s", p->path);

    if (ex->mmap_fn) {
        st = (u8 *)NC(ex->G, ex->mmap_fn, 0, 0x400, 3, 0x1002, (u64)-1, 0);
        if ((s64)st == -1) st = 0;
    }
    if (st) ex_zero(st, 0x400);
    if (statfs_fn && st) {
        ret = (s32)NC(ex->G, statfs_fn, (u64)p->path, (u64)st, 0, 0, 0, 0);
    }

    if (dbg_slot) {
        *dbg_slot = ((u64)(u32)ret << 32) | (u32)(st ? 1 : 0);
    }

    show_text_and_wait(
        ex,
        buf,
        "Resultado\n%s\nret=%x\nbuf=%s",
        p->label,
        (u32)ret,
        st ? "OK" : "NULL"
    );

    if (st && ex->munmap_fn) {
        NC(ex->G, ex->munmap_fn, (u64)st, 0x400, 0, 0, 0, 0);
    }
}

__attribute__((section(".text._start")))
void _start(u64 eboot_base, u64 dlsym_addr, struct ext_args *ext) {
    static const struct path_probe probe = {
        "app0", "/app0/"
    };
    void *statfs_fn;

    if (!ext) return;
    ext->step = 1;

    struct ps5_example ex;
    s32 ret = ex_init(&ex, eboot_base, dlsym_addr, ext);
    if (ret != 0) {
        ext->status = ret;
        return;
    }

    statfs_fn = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, "statfs");
    ext->dbg[0] = (u64)statfs_fn;

    u8 *buf = ex_alloc_dialog(&ex);
    if (!buf) {
        ext->status = -3;
        return;
    }

    ext->step = 10;
    run_probe_dialog(&ex, buf, statfs_fn, &probe, &ext->dbg[1]);

    ex_free_dialog(&ex, buf);
    ext->status = 0;
    ext->step = 99;
}
