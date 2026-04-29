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
    return ps5_sdk_audio_play_square_tone(G, output_fn, audio_h, samples, 6, button_period(button), 2200);
}

static void pick_color(u32 btn, u8 *r, u8 *g, u8 *b) {
    *r = 0x30; *g = 0x30; *b = 0x30;
    if (btn == PS5SDK_PAD_BTN_CROSS)    { *r = 0x20; *g = 0x60; *b = 0xFF; return; }
    if (btn == PS5SDK_PAD_BTN_CIRCLE)   { *r = 0xFF; *g = 0x30; *b = 0x30; return; }
    if (btn == PS5SDK_PAD_BTN_TRIANGLE) { *r = 0x30; *g = 0xFF; *b = 0x50; return; }
    if (btn == PS5SDK_PAD_BTN_SQUARE)   { *r = 0xFF; *g = 0x30; *b = 0xD0; return; }
    if (btn == PS5SDK_PAD_BTN_UP)       { *r = 0xFF; *g = 0xFF; *b = 0x40; return; }
    if (btn == PS5SDK_PAD_BTN_DOWN)     { *r = 0x20; *g = 0xD0; *b = 0xD0; return; }
    if (btn == PS5SDK_PAD_BTN_LEFT)     { *r = 0xFF; *g = 0x80; *b = 0x20; return; }
    if (btn == PS5SDK_PAD_BTN_RIGHT)    { *r = 0x90; *g = 0x60; *b = 0xFF; return; }
    if (btn == PS5SDK_PAD_BTN_L1)       { *r = 0xFF; *g = 0xFF; *b = 0xFF; return; }
    if (btn == PS5SDK_PAD_BTN_R1)       { *r = 0x10; *g = 0x10; *b = 0x10; return; }
    if (btn == PS5SDK_PAD_BTN_OPTIONS)  { *r = 0xFF; *g = 0xA0; *b = 0x00; return; }
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

    s32 pad_mod = ex_module_handle(&ex, "libScePad.sprx");
    void *pad_init = ex_sym_module(&ex, "libScePad.sprx", "scePadInit");
    void *pad_geth = ex_sym_module(&ex, "libScePad.sprx", "scePadGetHandle");
    void *pad_read = ex_sym_module(&ex, "libScePad.sprx", "scePadRead");
    s32 pad_init_ret = pad_init ? (s32)NC(ex.G, pad_init, 0, 0, 0, 0, 0, 0) : -1;
    s32 pad_h = pad_geth ? (s32)NC(ex.G, pad_geth, (u64)ex.user_id, 0, 0, 0, 0, 0) : -1;

    s32 audio_mod = ex_module_handle(&ex, "libSceAudioOut.sprx");
    void *audio_init = ex_sym_module(&ex, "libSceAudioOut.sprx", "sceAudioOutInit");
    void *audio_open = ex_sym_module(&ex, "libSceAudioOut.sprx", "sceAudioOutOpen");
    void *audio_output = ex_sym_module(&ex, "libSceAudioOut.sprx", "sceAudioOutOutput");
    void *audio_close = ex_sym_module(&ex, "libSceAudioOut.sprx", "sceAudioOutClose");
    s32 audio_init_ret = audio_init ? (s32)NC(ex.G, audio_init, 0, 0, 0, 0, 0, 0) : -1;
    s32 audio_h = ps5_sdk_audio_open_default(ex.G, audio_open);

    u8 *buf = ex_alloc_dialog(&ex);
    if (!buf) {
        ext->status = -3;
        return;
    }

    u8 *work = 0;

    s16 *samples = 0;
    if (audio_h >= 0 && audio_output) {
        samples = (s16 *)NC(ex.G, ex.mmap_fn, 0, 0x1000, 3, 0x1002, (u64)-1, 0);
        if ((s64)samples == -1) samples = 0;
    }

    ex_dialog_begin(&ex);
    ret = ex_open_text(
        &ex,
        buf,
        "Pad party (safe)\nAceptar y luego pulsa botones.\nCada boton hace beep.\nDuracion: 8 segundos."
    );
    if (ret == 0) {
        ext->status = 0;
        ex_wait_user_close(&ex);
    } else {
        ext->status = -4;
    }
    ex_close_dialog(&ex);

    u8 *pad_buf = buf + 0x200;
    u32 prev_buttons = 0;
    u32 seen_buttons = 0;
    u32 last_button = 0;
    s32 changes = 0;
    s32 beep_count = 0;
    s32 last_set = -1;
    s32 last_beep = -1;
    u8 last_r = 0x30, last_g = 0x30, last_b = 0x30;

    for (int i = 0; i < 240; i++) {
        u32 raw = 0, buttons = 0;
        if (pad_read && pad_h >= 0) {
            ps5_sdk_pad_read_buttons(ex.G, pad_read, pad_h, pad_buf, &raw, &buttons);
            u32 pressed = buttons & ~prev_buttons;
            u32 btn = ps5_sdk_pad_first_button(pressed);
            seen_buttons |= buttons;
            if (btn) {
                u8 r, g, b;
                pick_color(btn, &r, &g, &b);
                (void)r; (void)g; (void)b;
                last_set = -1;
                last_beep = play_short_beep(ex.G, audio_output, audio_h, samples, btn);
                last_button = btn;
                last_r = 0; last_g = 0; last_b = 0;
                changes++;
                beep_count++;
            }
            prev_buttons = buttons;
        }
        if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 33333, 0, 0, 0, 0, 0);
    }

    s32 reset_ret = -1;
    s32 close_ret = 0;
    if (audio_close && audio_h >= 0) {
        close_ret = (s32)NC(ex.G, audio_close, (u64)audio_h, 0, 0, 0, 0, 0);
    }

    ext->dbg[0] = (u64)(s64)pad_h;
    ext->dbg[1] = (u64)(s64)audio_h;
    ext->dbg[2] = (u64)(s64)changes;
    ext->dbg[3] = (u64)seen_buttons;
    ext->dbg[4] = (u64)(s64)last_set;
    ext->dbg[5] = (u64)(s64)last_beep;
    ext->status = 0;

    ex_dialog_begin(&ex);
    ret = ex_open_text(
        &ex,
        buf,
        "Pad party (safe)\npad=%x a=%x chg=%d beep=%d\nlast=%s rgb=%02x%02x%02x\nset=%x out=%x reset=%x close=%x",
        (u32)pad_h,
        (u32)audio_h,
        (int)changes,
        (int)beep_count,
        ps5_sdk_pad_button_name(last_button),
        last_r, last_g, last_b,
        (u32)last_set,
        (u32)last_beep,
        (u32)reset_ret,
        (u32)close_ret
    );
    if (ret == 0) ex_wait_user_close(&ex);
    ex_close_dialog(&ex);

    ex_free_dialog(&ex, buf);
    if (samples && ex.munmap_fn) NC(ex.G, ex.munmap_fn, (u64)samples, 0x1000, 0, 0, 0, 0);
    (void)work;

    ext->step = 99;
    (void)pad_mod;
    (void)audio_mod;
    (void)pad_init_ret;
    (void)audio_init_ret;
    ex_finish(&ex, ext, ext->status, 0);
}
