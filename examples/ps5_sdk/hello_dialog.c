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
    ret = ex_open_text(&ex, buf, "Hola mundo desde ps5_sdk.h\nuser=%d", ex.user_id);
    ext->dbg[0] = (u64)(s64)ret;
    if (ret == 0) {
        ext->status = 0;
        ex_wait_user_close(&ex);
    } else {
        ext->status = -4;
    }
    ex_close_dialog(&ex);

    ex_free_dialog(&ex, buf);
    ext->step = 99;
    ex_finish(&ex, ext, ext->status, 0);
}
