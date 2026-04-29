#include "example_common.h"

struct mod_probe {
    const char *name;
    void *fn;
};

__attribute__((section(".text._start")))
void _start(u64 eboot_base, u64 dlsym_addr, struct ext_args *ext) {
    static struct mod_probe mods[] = {
        { "Pad", 0 },
        { "Audio", 0 },
        { "User", 0 },
        { "Rtc", 0 },
        { "NetCtl", 0 },
        { "System", 0 },
    };

    if (!ext) return;
    ext->step = 1;

    struct ps5_example ex;
    s32 ret = ex_init(&ex, eboot_base, dlsym_addr, ext);
    if (ret != 0) {
        ext->status = ret;
        return;
    }

    mods[0].fn = ex_sym_module(&ex, "libScePad.sprx", "scePadRead");
    mods[1].fn = ex_sym_module(&ex, "libSceAudioOut.sprx", "sceAudioOutOpen");
    mods[2].fn = ex_sym_module(&ex, "libSceUserService.sprx", "sceUserServiceGetUserName");
    mods[3].fn = ex_sym_module(&ex, "libSceRtc.sprx", "sceRtcGetCurrentTick");
    mods[4].fn = ex_sym_module(&ex, "libSceNetCtl.sprx", "sceNetCtlGetState");
    mods[5].fn = ex_sym_module(&ex, "libSceSystemService.sprx", "sceSystemServiceParamGetInt");

    ext->dbg[0] = (u64)mods[0].fn;
    ext->dbg[1] = (u64)mods[1].fn;
    ext->dbg[2] = (u64)mods[2].fn;
    ext->dbg[3] = (u64)mods[3].fn;
    ext->dbg[4] = (u64)mods[4].fn;
    ext->dbg[5] = (u64)mods[5].fn;
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
        "SDK quick surface\nPad=%p Audio=%p User=%p\nRtc=%p NetCtl=%p Sys=%p\n\nResolve-only OK",
        mods[0].fn,
        mods[1].fn,
        mods[2].fn,
        mods[3].fn,
        mods[4].fn,
        mods[5].fn
    );
    if (ret == 0) ex_wait_user_close(&ex);
    ex_close_dialog(&ex);
    ex_free_dialog(&ex, buf);
    ext->step = 99;
}
