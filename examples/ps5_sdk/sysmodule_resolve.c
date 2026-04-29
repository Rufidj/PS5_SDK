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

    void *load_fn = SYM(ex.G, ex.D, SYSMODULE_HANDLE, SYSMODULE_sceSysmoduleLoadModule);
    void *unload_fn = SYM(ex.G, ex.D, SYSMODULE_HANDLE, SYSMODULE_sceSysmoduleUnloadModule);
    void *is_loaded_fn = SYM(ex.G, ex.D, SYSMODULE_HANDLE, SYSMODULE_sceSysmoduleIsLoaded);
    void *get_handle_fn = SYM(ex.G, ex.D, SYSMODULE_HANDLE, SYSMODULE_sceSysmoduleGetModuleHandleInternal);

    s32 state_pad = ps5_sdk_sysmodule_is_loaded(ex.G, is_loaded_fn, 0x00980000u);

    u8 *buf = ex_alloc_dialog(&ex);
    if (!buf) {
        ext->status = -3;
        return;
    }

    ext->step = 2;
    ex_dialog_begin(&ex);
    ret = ex_open_text(
        &ex,
        buf,
        "Sysmodule resolve\nload=%p unload=%p\nisLoaded=%p getHandle=%p\npadState=%x",
        load_fn,
        unload_fn,
        is_loaded_fn,
        get_handle_fn,
        (u32)state_pad
    );
    ext->dbg[0] = (u64)(s64)ret;
    if (ret == 0) {
        ext->status = 0;
        ex_wait_user_close(&ex);
        ex_close_dialog(&ex);
    } else {
        ext->status = -4;
    }

    ex_free_dialog(&ex, buf);
    ext->step = 99;
}
