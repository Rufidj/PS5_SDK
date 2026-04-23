#include "example_common.h"

static u32 pad_buttons_from_raw(u32 raw) {
    return raw & 0x001FFFFF;
}

static u32 first_button(u32 b) {
    if (b & 0x00004000) return 0x00004000; /* CROSS */
    if (b & 0x00008000) return 0x00008000; /* SQUARE */
    if (b & 0x00002000) return 0x00002000; /* CIRCLE */
    if (b & 0x00001000) return 0x00001000; /* TRIANGLE */
    if (b & 0x00000400) return 0x00000400; /* L1 */
    if (b & 0x00000800) return 0x00000800; /* R1 */
    if (b & 0x00000010) return 0x00000010; /* UP */
    if (b & 0x00000040) return 0x00000040; /* DOWN */
    if (b & 0x00000080) return 0x00000080; /* LEFT */
    if (b & 0x00000020) return 0x00000020; /* RIGHT */
    if (b & 0x00000008) return 0x00000008; /* OPTIONS */
    return 0;
}

static const char *button_name(u32 b) {
    if (b == 0x00004000) return "CROSS";
    if (b == 0x00008000) return "SQUARE";
    if (b == 0x00002000) return "CIRCLE";
    if (b == 0x00001000) return "TRIANGLE";
    if (b == 0x00000400) return "L1";
    if (b == 0x00000800) return "R1";
    if (b == 0x00000010) return "UP";
    if (b == 0x00000040) return "DOWN";
    if (b == 0x00000080) return "LEFT";
    if (b == 0x00000020) return "RIGHT";
    if (b == 0x00000008) return "OPTIONS";
    return "none";
}

static int button_period(u32 b) {
    if (b == 0x00004000) return 54; /* CROSS */
    if (b == 0x00008000) return 48; /* SQUARE */
    if (b == 0x00002000) return 42; /* CIRCLE */
    if (b == 0x00001000) return 36; /* TRIANGLE */
    if (b == 0x00000400) return 64; /* L1 */
    if (b == 0x00000800) return 30; /* R1 */
    if (b == 0x00000010) return 72; /* UP */
    if (b == 0x00000040) return 80; /* DOWN */
    if (b == 0x00000080) return 96; /* LEFT */
    if (b == 0x00000020) return 40; /* RIGHT */
    if (b == 0x00000008) return 24; /* OPTIONS */
    return 48;
}

static void fill_square_tone(s16 *samples, int frames, int *phase, int period) {
    if (period < 8) period = 8;
    for (int i = 0; i < frames; i++) {
        int p = *phase;
        s16 v = (p < (period / 2)) ? 2200 : -2200;
        samples[i * 2 + 0] = v;
        samples[i * 2 + 1] = v;
        p++;
        if (p >= period) p = 0;
        *phase = p;
    }
}

static s32 play_short_beep(void *G, void *output_fn, s32 audio_h, s16 *samples, u32 button) {
    if (!output_fn || audio_h < 0 || !samples) return -1;
    int phase = 0;
    int period = button_period(button);
    s32 ret = 0;
    for (int n = 0; n < 8; n++) {
        fill_square_tone(samples, SAMPLES_PER_BUF, &phase, period);
        ret = (s32)NC(G, output_fn, (u64)audio_h, (u64)samples, 0, 0, 0, 0);
    }
    return ret;
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

    NC(ex.G, ex.load_mod, (u64)"libScePad.sprx", 0, 0, 0, 0, 0);
    void *pad_init = SYM(ex.G, ex.D, PAD_HANDLE, "scePadInit");
    void *pad_geth = SYM(ex.G, ex.D, PAD_HANDLE, "scePadGetHandle");
    void *pad_read = SYM(ex.G, ex.D, PAD_HANDLE, "scePadRead");
    s32 pad_init_ret = pad_init ? (s32)NC(ex.G, pad_init, 0, 0, 0, 0, 0, 0) : -1;
    s32 pad_h = pad_geth ? (s32)NC(ex.G, pad_geth, (u64)ex.user_id, 0, 0, 0, 0, 0) : -1;

    NC(ex.G, ex.load_mod, (u64)"libSceAudioOut.sprx", 0, 0, 0, 0, 0);
    void *audio_init = SYM(ex.G, ex.D, AUDIOOUT_HANDLE, AUDIOOUT_sceAudioOutInit);
    void *audio_open = SYM(ex.G, ex.D, AUDIOOUT_HANDLE, AUDIOOUT_sceAudioOutOpen);
    void *audio_output = SYM(ex.G, ex.D, AUDIOOUT_HANDLE, AUDIOOUT_sceAudioOutOutput);
    void *audio_close = SYM(ex.G, ex.D, AUDIOOUT_HANDLE, AUDIOOUT_sceAudioOutClose);

    s32 audio_init_ret = audio_init ? (s32)NC(ex.G, audio_init, 0, 0, 0, 0, 0, 0) : -1;
    if (audio_close) {
        for (int h = 0; h < 8; h++) NC(ex.G, audio_close, (u64)h, 0, 0, 0, 0, 0);
    }
    s32 audio_h = audio_open
        ? (s32)NC(ex.G, audio_open, 0xFF, 0, 0, SAMPLES_PER_BUF, SAMPLE_RATE, AUDIO_S16_STEREO)
        : -1;

    u8 *buf = ex_alloc_dialog(&ex);
    if (!buf) {
        ext->status = -3;
        return;
    }

    s16 *samples = 0;
    if (audio_h >= 0 && audio_output) {
        samples = (s16 *)NC(ex.G, ex.mmap_fn, 0, 0x1000, 3, 0x1002, (u64)-1, 0);
        if ((s64)samples == -1) samples = 0;
    }

    ext->step = 2;
    ex_dialog_begin(&ex);
    ret = ex_open_text(
        &ex,
        buf,
        "Input beep\nPulsa aceptar y toca botones.\nCada boton nuevo hace un tono.\nDuracion: 8 segundos."
    );
    if (ret == 0) {
        ex_wait_user_close(&ex);
    }
    ex_close_dialog(&ex);

    u8 *pad_buf = buf + 0x200;
    u32 prev_buttons = 0;
    u32 seen_buttons = 0;
    u32 last_button = 0;
    s32 good_reads = 0;
    s32 beep_count = 0;
    s32 last_read = -1;
    s32 last_out = -1;

    for (int i = 0; i < 240; i++) {
        for (int j = 0; j < 128; j++) pad_buf[j] = 0;
        if (pad_read && pad_h >= 0) {
            last_read = (s32)NC(ex.G, pad_read, (u64)pad_h, (u64)pad_buf, 1, 0, 0, 0);
            u32 buttons = pad_buttons_from_raw(*(u32 *)pad_buf);
            u32 pressed = buttons & ~prev_buttons;
            u32 btn = first_button(pressed);
            if (last_read > 0 && (u32)last_read < 0x80000000) good_reads++;
            seen_buttons |= buttons;
            if (btn) {
                last_button = btn;
                last_out = play_short_beep(ex.G, audio_output, audio_h, samples, btn);
                beep_count++;
            }
            prev_buttons = buttons;
        }
        if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 33333, 0, 0, 0, 0, 0);
    }

    s32 close_ret = 0;
    if (audio_close && audio_h >= 0) {
        close_ret = (s32)NC(ex.G, audio_close, (u64)audio_h, 0, 0, 0, 0, 0);
    }
    if (samples && ex.munmap_fn) {
        NC(ex.G, ex.munmap_fn, (u64)samples, 0x1000, 0, 0, 0, 0);
    }

    ext->dbg[0] = (u64)(s64)pad_h;
    ext->dbg[1] = (u64)(s64)audio_h;
    ext->dbg[2] = (u64)seen_buttons;
    ext->status = 0;

    ex_dialog_begin(&ex);
    ret = ex_open_text(
        &ex,
        buf,
        "Input beep result\npad=%x a=%x reads=%d beeps=%d\nseen=%x last=%s\nout=%x close=%x",
        (u32)pad_h,
        (u32)audio_h,
        (int)good_reads,
        (int)beep_count,
        seen_buttons,
        button_name(last_button),
        (u32)last_out,
        (u32)close_ret
    );
    if (ret == 0) ex_wait_user_close(&ex);
    ex_close_dialog(&ex);
    ex_free_dialog(&ex, buf);

    ext->step = 99;
    (void)pad_init_ret;
    (void)audio_init_ret;
}
