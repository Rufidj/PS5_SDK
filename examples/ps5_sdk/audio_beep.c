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

    NC(ex.G, ex.load_mod, (u64)"libSceAudioOut.sprx", 0, 0, 0, 0, 0);
    void *init_fn = SYM(ex.G, ex.D, AUDIOOUT_HANDLE, "sceAudioOutInit");
    void *open_fn = SYM(ex.G, ex.D, AUDIOOUT_HANDLE, "sceAudioOutOpen");
    void *output_fn = SYM(ex.G, ex.D, AUDIOOUT_HANDLE, "sceAudioOutOutput");
    void *close_fn = SYM(ex.G, ex.D, AUDIOOUT_HANDLE, "sceAudioOutClose");

    s32 init_ret = init_fn ? (s32)NC(ex.G, init_fn, 0, 0, 0, 0, 0, 0) : -1;
    ps5_sdk_audio_close_known_handles(ex.G, close_fn, 8);

    s32 audio_h = ps5_sdk_audio_open_default(ex.G, open_fn);

    s16 *samples = 0;
    if (audio_h >= 0 && output_fn) {
        samples = (s16 *)NC(ex.G, ex.mmap_fn, 0, 0x1000, 3, 0x1002, (u64)-1, 0);
        if ((s64)samples == -1) samples = 0;
    }

    s32 last_out = -1;
    if (samples) {
        last_out = ps5_sdk_audio_play_square_tone(ex.G, output_fn, audio_h, samples, 80, 48, 2200);
    }

    s32 close_ret = 0;
    if (close_fn && audio_h >= 0) {
        close_ret = (s32)NC(ex.G, close_fn, (u64)audio_h, 0, 0, 0, 0, 0);
    }
    if (samples && ex.munmap_fn) {
        NC(ex.G, ex.munmap_fn, (u64)samples, 0x1000, 0, 0, 0, 0);
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
        "Audio beep\ninit=%x handle=%x\nlastOut=%x close=%x",
        (u32)init_ret,
        (u32)audio_h,
        (u32)last_out,
        (u32)close_ret
    );
    ext->dbg[0] = (u64)(s64)audio_h;
    ext->dbg[1] = (u64)(s64)last_out;
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
