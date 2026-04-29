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

    s32 load_ret = (s32)NC(ex.G, ex.load_mod, (u64)"libScePad.sprx", 0, 0, 0, 0, 0);
    void *pad_init = SYM(ex.G, ex.D, PAD_HANDLE, "scePadInit");
    void *pad_geth = SYM(ex.G, ex.D, PAD_HANDLE, "scePadGetHandle");
    void *pad_read = SYM(ex.G, ex.D, PAD_HANDLE, "scePadRead");
    void *pad_fx = SYM(ex.G, ex.D, PAD_HANDLE, "scePadSetTriggerEffect");
    void *pad_light = SYM(ex.G, ex.D, PAD_HANDLE, "scePadSetLightBar");
    void *pad_light_reset = SYM(ex.G, ex.D, PAD_HANDLE, "scePadResetLightBar");
    void *pad_motion = SYM(ex.G, ex.D, PAD_HANDLE, "scePadSetMotionSensorState");
    void *pad_info = SYM(ex.G, ex.D, PAD_HANDLE, "scePadGetControllerInformation");
    void *pad_open_ext = SYM(ex.G, ex.D, PAD_HANDLE, "scePadOpenExt");
    void *pad_open_ext2 = SYM(ex.G, ex.D, PAD_HANDLE, "scePadOpenExt2");

    s32 init_ret = pad_init ? (s32)NC(ex.G, pad_init, 0, 0, 0, 0, 0, 0) : -1;
    s32 pad_h = pad_geth ? (s32)NC(ex.G, pad_geth, (u64)ex.user_id, 0, 0, 0, 0, 0) : -1;

    ext->dbg[0] = (u64)(s64)pad_h;
    ext->dbg[1] = (u64)(s64)init_ret;
    ext->dbg[2] = (u64)(pad_fx != 0);
    ext->dbg[3] = (u64)(pad_light != 0);
    ext->dbg[4] = (u64)(pad_motion != 0);
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
        "Pad FX probe\nload=%x init=%x h=%x read=%p\nfx=%p light=%p reset=%p\nmotion=%p info=%p\nopenExt=%p openExt2=%p",
        (u32)load_ret,
        (u32)init_ret,
        (u32)pad_h,
        pad_read,
        pad_fx,
        pad_light,
        pad_light_reset,
        pad_motion,
        pad_info,
        pad_open_ext,
        pad_open_ext2
    );
    if (ret == 0) ex_wait_user_close(&ex);
    ex_close_dialog(&ex);
    ex_free_dialog(&ex, buf);
    ext->step = 99;
}
