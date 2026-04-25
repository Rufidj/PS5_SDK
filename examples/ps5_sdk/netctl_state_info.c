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

    s32 netctl_mod = (s32)NC(ex.G, ex.load_mod, (u64)"libSceNetCtl.sprx", 0, 0, 0, 0, 0);
    void *get_state_fn = SYM(ex.G, ex.D, NETCTL_HANDLE, "sceNetCtlGetState");

    s32 state = -1;
    s32 state_ret = ps5_sdk_netctl_get_state(ex.G, get_state_fn, &state);

    ext->dbg[0] = (u64)(s64)netctl_mod;
    ext->dbg[1] = (u64)(s64)state_ret;
    ext->dbg[2] = (u64)(s64)state;
    ext->status = 0;

    u8 *buf = ex_alloc_dialog(&ex);
    if (!buf) {
        ext->status = -3;
        return;
    }

    ext->step = 2;
    ex_dialog_begin(&ex);
    if (state_ret == 0) {
        ret = ex_open_text(
            &ex,
            buf,
            "NetCtl state\nmod=%x ret=%x\nstate=%d %s\nNetCtl OK",
            (u32)netctl_mod,
            (u32)state_ret,
            (int)state,
            ps5_sdk_netctl_state_name(state)
        );
    } else {
        ret = ex_open_text(
            &ex,
            buf,
            "NetCtl state\nmod=%x ret=%x\nfn=%p\nNo state",
            (u32)netctl_mod,
            (u32)state_ret,
            get_state_fn
        );
    }
    if (ret == 0) ex_wait_user_close(&ex);
    ex_close_dialog(&ex);
    ex_free_dialog(&ex, buf);
    ext->step = 99;
}
