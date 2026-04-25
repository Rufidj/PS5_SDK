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

    s32 audio_mod = ps5_sdk_load_sprx(ex.G, ex.load_mod, "libSceAudioOut.sprx");
    void *init_fn = SYM(ex.G, ex.D, AUDIOOUT_HANDLE, AUDIOOUT_sceAudioOutInit);
    void *open_fn = SYM(ex.G, ex.D, AUDIOOUT_HANDLE, AUDIOOUT_sceAudioOutOpen);
    void *output_fn = SYM(ex.G, ex.D, AUDIOOUT_HANDLE, AUDIOOUT_sceAudioOutOutput);
    void *close_fn = SYM(ex.G, ex.D, AUDIOOUT_HANDLE, AUDIOOUT_sceAudioOutClose);
    void *volume_fn = SYM(ex.G, ex.D, AUDIOOUT_HANDLE, AUDIOOUT_sceAudioOutSetVolume);
    void *state_fn = SYM(ex.G, ex.D, AUDIOOUT_HANDLE, AUDIOOUT_sceAudioOutGetSystemState);

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
        "AudioOut resolve\nmod=%x init=%p\nopen=%p out=%p\nclose=%p vol=%p state=%p",
        (u32)audio_mod,
        init_fn,
        open_fn,
        output_fn,
        close_fn,
        volume_fn,
        state_fn
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
