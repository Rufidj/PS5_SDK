#include "example_common.h"

static void fill_square_beep(s16 *samples, int frames, int phase) {
    for (int i = 0; i < frames; i++) {
        int t = (i + phase) % 48;
        s16 v = (t < 24) ? 2200 : -2200;
        samples[i * 2 + 0] = v;
        samples[i * 2 + 1] = v;
    }
}

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
    void *init_fn = SYM(ex.G, ex.D, AUDIOOUT_HANDLE, AUDIOOUT_sceAudioOutInit);
    void *open_fn = SYM(ex.G, ex.D, AUDIOOUT_HANDLE, AUDIOOUT_sceAudioOutOpen);
    void *output_fn = SYM(ex.G, ex.D, AUDIOOUT_HANDLE, AUDIOOUT_sceAudioOutOutput);
    void *close_fn = SYM(ex.G, ex.D, AUDIOOUT_HANDLE, AUDIOOUT_sceAudioOutClose);

    s32 init_ret = init_fn ? (s32)NC(ex.G, init_fn, 0, 0, 0, 0, 0, 0) : -1;
    if (close_fn) {
        for (int h = 0; h < 8; h++) {
            NC(ex.G, close_fn, (u64)h, 0, 0, 0, 0, 0);
        }
    }

    s32 audio_h = open_fn
        ? (s32)NC(ex.G, open_fn, 0xFF, 0, 0, SAMPLES_PER_BUF, SAMPLE_RATE, AUDIO_S16_STEREO)
        : -1;

    s16 *samples = 0;
    if (audio_h >= 0 && output_fn) {
        samples = (s16 *)NC(ex.G, ex.mmap_fn, 0, 0x1000, 3, 0x1002, (u64)-1, 0);
        if ((s64)samples == -1) samples = 0;
    }

    s32 last_out = -1;
    if (samples) {
        for (int n = 0; n < 80; n++) {
            fill_square_beep(samples, SAMPLES_PER_BUF, n * SAMPLES_PER_BUF);
            last_out = (s32)NC(ex.G, output_fn, (u64)audio_h, (u64)samples, 0, 0, 0, 0);
        }
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
