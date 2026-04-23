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

    u8 *buf = ex_alloc_dialog(&ex);
    if (!buf) {
        ext->status = -3;
        return;
    }

    ext->step = 2;
    ex_dialog_begin(&ex);
    ret = ex_open_progress(&ex, buf, "Progreso SDK inicializando...");
    ext->dbg[0] = (u64)(s64)ret;
    if (ret != 0) {
        ext->status = -4;
        ex_free_dialog(&ex, buf);
        return;
    }

    char *msg = (char *)(buf + 0xE0);
    for (int pct = 0; pct <= 100; pct += 5) {
        if (ex.dlg_update) NC(ex.G, ex.dlg_update, 0, 0, 0, 0, 0, 0);
        if (ex.dlg_set_val) NC(ex.G, ex.dlg_set_val, 0, (u64)pct, 0, 0, 0, 0);
        if (ex.dlg_set_msg) {
            ps5_sdk_snprintf(msg, 160, "Progreso SDK %d%%", pct);
            NC(ex.G, ex.dlg_set_msg, 0, (u64)msg, 0, 0, 0, 0);
        }
        if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 100000, 0, 0, 0, 0, 0);
    }

    if (ex.dlg_set_msg) {
        ps5_sdk_snprintf(msg, 160, "Completado. Cerrando...");
        NC(ex.G, ex.dlg_set_msg, 0, (u64)msg, 0, 0, 0, 0);
    }
    if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 1000000, 0, 0, 0, 0, 0);

    ext->status = 0;
    ex_close_dialog(&ex);
    ex_free_dialog(&ex, buf);
    ext->step = 99;
}
