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

    s32 audio_mod = ex_module_handle(&ex, "libSceAudioOut.sprx");
    void *init_fn = ex_sym_module(&ex, "libSceAudioOut.sprx", AUDIOOUT_sceAudioOutInit);
    void *open_fn = ex_sym_module(&ex, "libSceAudioOut.sprx", AUDIOOUT_sceAudioOutOpen);
    void *close_fn = ex_sym_module(&ex, "libSceAudioOut.sprx", AUDIOOUT_sceAudioOutClose);

    s32 init_ret = init_fn ? (s32)NC(ex.G, init_fn, 0, 0, 0, 0, 0, 0) : -1;

    if (close_fn) {
        for (int h = 0; h < 8; h++) {
            NC(ex.G, close_fn, (u64)h, 0, 0, 0, 0, 0);
        }
    }

    s32 audio_h = ps5_sdk_audio_open_default(ex.G, open_fn);
    s32 close_ret = 0;
    if (close_fn && audio_h >= 0) {
        close_ret = (s32)NC(ex.G, close_fn, (u64)audio_h, 0, 0, 0, 0, 0);
    }

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
        "AudioOut open probe\nmod=%x init=%x\nhandle/ret=%x close=%x",
        (u32)audio_mod,
        (u32)init_ret,
        (u32)audio_h,
        (u32)close_ret
    );
    ext->dbg[0] = (u64)(s64)audio_h;
    ext->dbg[1] = (u64)(s64)init_ret;
    ext->dbg[2] = (u64)(s64)close_ret;
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
