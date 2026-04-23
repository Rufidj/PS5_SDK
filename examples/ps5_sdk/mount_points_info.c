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

static void run_probe_dialog(struct ps5_example *ex, u8 *buf, void *kopen, void *kclose, const struct path_probe *p, u64 *dbg_slot) {
    s32 fd = -1;
    s32 close_ret = -1;

    show_text_and_wait(ex, buf, "Mount probe\nVamos a probar:\n%s", p->path);

    if (kopen) {
        fd = (s32)NC(ex->G, kopen, (u64)p->path, 0, 0, 0, 0, 0);
        if (fd >= 0 && kclose) {
            close_ret = (s32)NC(ex->G, kclose, (u64)fd, 0, 0, 0, 0, 0);
        }
    }

    if (dbg_slot) {
        *dbg_slot = ((u64)(u32)fd << 32) | (u32)close_ret;
    }

    show_text_and_wait(
        ex,
        buf,
        "Resultado\n%s\nopen=%x\nclose=%x",
        p->label,
        (u32)fd,
        (u32)close_ret
    );
}

__attribute__((section(".text._start")))
void _start(u64 eboot_base, u64 dlsym_addr, struct ext_args *ext) {
    static const struct path_probe probe = {
        "usb0", "/mnt/usb0/"
    };
    void *kopen;
    void *kclose;

    if (!ext) return;
    ext->step = 1;

    struct ps5_example ex;
    s32 ret = ex_init(&ex, eboot_base, dlsym_addr, ext);
    if (ret != 0) {
        ext->status = ret;
        return;
    }

    kopen = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, KERNEL_sceKernelOpen);
    kclose = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, KERNEL_sceKernelClose);
    ext->dbg[0] = (u64)kopen;
    ext->dbg[1] = (u64)kclose;

    u8 *buf = ex_alloc_dialog(&ex);
    if (!buf) {
        ext->status = -3;
        return;
    }

    ext->step = 10;
    run_probe_dialog(&ex, buf, kopen, kclose, &probe, &ext->dbg[2]);

    ex_free_dialog(&ex, buf);
    ext->status = 0;
    ext->step = 99;
}
