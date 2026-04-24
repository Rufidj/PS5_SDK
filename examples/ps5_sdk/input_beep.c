#include "example_common.h"

static int button_period(u32 b) {
    if (b == PS5SDK_PAD_BTN_CROSS) return 54;
    if (b == PS5SDK_PAD_BTN_SQUARE) return 48;
    if (b == PS5SDK_PAD_BTN_CIRCLE) return 42;
    if (b == PS5SDK_PAD_BTN_TRIANGLE) return 36;
    if (b == PS5SDK_PAD_BTN_L1) return 64;
    if (b == PS5SDK_PAD_BTN_R1) return 30;
    if (b == PS5SDK_PAD_BTN_UP) return 72;
    if (b == PS5SDK_PAD_BTN_DOWN) return 80;
    if (b == PS5SDK_PAD_BTN_LEFT) return 96;
    if (b == PS5SDK_PAD_BTN_RIGHT) return 40;
    if (b == PS5SDK_PAD_BTN_OPTIONS) return 24;
    return 48;
}

static s32 play_short_beep(void *G, void *output_fn, s32 audio_h, s16 *samples, u32 button) {
    if (!output_fn || audio_h < 0 || !samples) return -1;
    return ps5_sdk_audio_play_square_tone(G, output_fn, audio_h, samples, 8, button_period(button), 2200);
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
    void *audio_init = SYM(ex.G, ex.D, AUDIOOUT_HANDLE, "sceAudioOutInit");
    void *audio_open = SYM(ex.G, ex.D, AUDIOOUT_HANDLE, "sceAudioOutOpen");
    void *audio_output = SYM(ex.G, ex.D, AUDIOOUT_HANDLE, "sceAudioOutOutput");
    void *audio_close = SYM(ex.G, ex.D, AUDIOOUT_HANDLE, "sceAudioOutClose");

    s32 audio_init_ret = audio_init ? (s32)NC(ex.G, audio_init, 0, 0, 0, 0, 0, 0) : -1;
    ps5_sdk_audio_close_known_handles(ex.G, audio_close, 8);
    s32 audio_h = ps5_sdk_audio_open_default(ex.G, audio_open);

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
            u32 last_raw = 0;
            u32 buttons = 0;
            last_read = ps5_sdk_pad_read_buttons(ex.G, pad_read, pad_h, pad_buf, &last_raw, &buttons);
            u32 pressed = buttons & ~prev_buttons;
            u32 btn = ps5_sdk_pad_first_button(pressed);
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
        ps5_sdk_pad_button_name(last_button),
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
