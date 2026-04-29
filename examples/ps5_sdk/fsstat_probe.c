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

    void *getfsstat_ptr = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, "getfsstat");
    s32 fs_count = -1;
    if (getfsstat_ptr) {
        fs_count = (s32)NC(ex.G, getfsstat_ptr, 0, 0, 0, 0, 0, 0);
    }

    ext->dbg[0] = (u64)getfsstat_ptr;
    ext->dbg[1] = (u64)(u32)fs_count;
    ext->status = 0;

    u8 *buf = ex_alloc_dialog(&ex);
    if (!buf) {
        ext->status = -3;
        return;
    }

    ex_dialog_begin(&ex);
    ret = ex_open_text(
        &ex,
        buf,
        "FS stat probe\ngetfsstat=%p\ncount=%d",
        getfsstat_ptr,
        (int)fs_count
    );
    if (ret == 0) ex_wait_user_close(&ex);
    ex_close_dialog(&ex);
    ex_free_dialog(&ex, buf);
    ext->step = 99;
}
