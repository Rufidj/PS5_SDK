#include "example_common.h"

struct rtc_clock_data {
    u16 year;
    u16 month;
    u16 day;
    u16 hour;
    u16 minute;
    u16 second;
    u32 microsecond;
};

static const char *net_state_name(s32 st) {
    if (st == 0) return "OFFLINE";
    if (st == 1) return "CONNECT";
    if (st == 2) return "IP WAIT";
    if (st == 3) return "ONLINE";
    return "UNKNOWN";
}

static void safe_copy_name(char *dst, const char *src, u32 max) {
    u32 i = 0;
    if (!dst || max == 0) return;
    if (src) {
        while (i + 1 < max && src[i]) {
            char c = src[i];
            dst[i] = (c >= 32 && c <= 126) ? c : '?';
            i++;
        }
    }
    dst[i] = 0;
}

static s64 get_sys_param_int(void *G, void *fn, s32 id, s32 *out) {
    if (!fn || !out) return -1;
    *out = 0;
    return (s64)NC(G, fn, (u64)id, (u64)out, 0, 0, 0, 0);
}

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

static u32 wait_choice(struct ps5_example *ex, void *pad_read, s32 pad_h, u8 *pad_buf, s32 *reads, u32 *seen_buttons) {
    u32 prev_buttons = 0;
    u32 last_buttons = 0;

    for (int i = 0; i < 180; i++) {
        if (pad_read && pad_h >= 0) {
            u32 last_raw = 0;
            s32 last_read = ps5_sdk_pad_read_buttons(ex->G, pad_read, pad_h, pad_buf, &last_raw, &last_buttons);
            if (last_read > 0 && (u32)last_read < 0x80000000) (*reads)++;
            *seen_buttons |= last_buttons;
        } else {
            last_buttons = 0;
        }

        u32 pressed = last_buttons & ~prev_buttons;
        prev_buttons = last_buttons;

        if (pressed & PS5SDK_PAD_BTN_CROSS) return PS5SDK_PAD_BTN_CROSS;
        if (pressed & PS5SDK_PAD_BTN_SQUARE) return PS5SDK_PAD_BTN_SQUARE;
        if (pressed & PS5SDK_PAD_BTN_CIRCLE) return PS5SDK_PAD_BTN_CIRCLE;
        if (pressed & PS5SDK_PAD_BTN_TRIANGLE) return PS5SDK_PAD_BTN_TRIANGLE;

        if (ex->usleep_fn) NC(ex->G, ex->usleep_fn, 33333, 0, 0, 0, 0, 0);
    }
    return 0;
}

static s32 show_text_page(struct ps5_example *ex, u8 *dialog, const char *fmt, ...) {
    s32 ret;
    char text[192];
    ex_dialog_begin(ex);
    __builtin_va_list ap;
    __builtin_va_start(ap, fmt);
    ps5_sdk_vsnprintf(text, sizeof(text), fmt, ap);
    __builtin_va_end(ap);
    ret = ex_open_text(ex, dialog, "%s", text);
    if (ret == 0) ex_wait_user_close(ex);
    ex_close_dialog(ex);
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

    void *pad_init = ex_sym_module(&ex, "libScePad.sprx", "scePadInit");
    void *pad_geth = ex_sym_module(&ex, "libScePad.sprx", "scePadGetHandle");
    void *pad_read = ex_sym_module(&ex, "libScePad.sprx", "scePadRead");

    void *audio_init = ex_sym_module(&ex, "libSceAudioOut.sprx", "sceAudioOutInit");
    void *audio_open = ex_sym_module(&ex, "libSceAudioOut.sprx", "sceAudioOutOpen");
    void *audio_output = ex_sym_module(&ex, "libSceAudioOut.sprx", "sceAudioOutOutput");
    void *audio_close = ex_sym_module(&ex, "libSceAudioOut.sprx", "sceAudioOutClose");

    void *get_login_list = ex_sym_module(&ex, "libSceUserService.sprx", "sceUserServiceGetLoginUserIdList");
    void *get_user_name = ex_sym_module(&ex, "libSceUserService.sprx", "sceUserServiceGetUserName");
    void *get_clock_local = ex_sym_module(&ex, "libSceRtc.sprx", "sceRtcGetCurrentClockLocalTime");
    void *get_state_fn = ex_sym_module(&ex, "libSceNetCtl.sprx", "sceNetCtlGetState");
    void *param_get_int = ex_sym_module(&ex, "libSceSystemService.sprx", "sceSystemServiceParamGetInt");
    void *get_clock = ex_sym_module(&ex, "libSceGnmDriver.sprx", "sceGnmGetGpuCoreClockFrequency");
    void *is_pa_enabled = ex_sym_module(&ex, "libSceGnmDriver.sprx", "sceGnmIsUserPaEnabled");

    s32 pad_init_ret = pad_init ? (s32)NC(ex.G, pad_init, 0, 0, 0, 0, 0, 0) : -1;
    s32 pad_h = pad_geth ? (s32)NC(ex.G, pad_geth, (u64)ex.user_id, 0, 0, 0, 0, 0) : -1;
    s32 audio_init_ret = audio_init ? (s32)NC(ex.G, audio_init, 0, 0, 0, 0, 0, 0) : -1;
    s32 audio_h = ps5_sdk_audio_open_default(ex.G, audio_open);

    u8 *work = (u8 *)NC(ex.G, ex.mmap_fn, 0, 0x2000, 3, 0x1002, (u64)-1, 0);
    if ((s64)work == -1) work = 0;

    u8 *dialog = ex_alloc_dialog(&ex);
    if (!dialog || !work) {
        ext->status = -3;
        if (dialog) ex_free_dialog(&ex, dialog);
        if (work && ex.munmap_fn) NC(ex.G, ex.munmap_fn, (u64)work, 0x2000, 0, 0, 0, 0);
        return;
    }

    s16 *samples = (s16 *)(work + 0x1000);
    u8 *pad_buf = work + 0x1800;
    u32 seen_buttons = 0;
    s32 last_audio_out = -1;
    s32 reads = 0;

    ext->step = 2;
    for (;;) {
        show_text_page(
            &ex,
            dialog,
            "SDK Launcher\n\nCROSS  Dashboard\nSQUARE Pad live\nCIRCLE Audio beep\nTRIANGLE System info\n\nCierra y pulsa un boton."
        );

        u32 choice = wait_choice(&ex, pad_read, pad_h, pad_buf, &reads, &seen_buttons);
        if (!choice) break;

        if (choice == PS5SDK_PAD_BTN_CROSS) {
            char user_name[32];
            struct rtc_clock_data clk;
            s32 ids[4];
            s32 net_state = -1;
            s32 lang = 0;
            s32 time_zone = 0;
            u64 clock_hz = 0;
            s32 pa_enabled = -1;

            user_name[0] = 0;
            for (int i = 0; i < 4; i++) ids[i] = -1;
            clk.year = clk.month = clk.day = 0;
            clk.hour = clk.minute = clk.second = 0;
            clk.microsecond = 0;

            if (get_login_list) {
                for (int i = 0; i < 16; i++) ((s32 *)work)[i] = -1;
                if ((s32)NC(ex.G, get_login_list, (u64)work, 0, 0, 0, 0, 0) == 0) {
                    for (int i = 0; i < 4; i++) ids[i] = ((s32 *)work)[i];
                }
            }
            if (get_user_name) {
                s32 target_id = (ids[0] != -1) ? ids[0] : ex.user_id;
                ex_zero(work + 0x100, 0x100);
                if ((s32)NC(ex.G, get_user_name, (u64)target_id, (u64)(work + 0x100), 0x100, 0, 0, 0) == 0) {
                    safe_copy_name(user_name, (char *)(work + 0x100), sizeof(user_name));
                }
            }
            if (!user_name[0]) ps5_sdk_snprintf(user_name, sizeof(user_name), "user#%d", (int)ex.user_id);
            if (get_clock_local) NC(ex.G, get_clock_local, (u64)&clk, 0, 0, 0, 0, 0);
            if (get_state_fn) NC(ex.G, get_state_fn, (u64)&net_state, 0, 0, 0, 0, 0);
            get_sys_param_int(ex.G, param_get_int, 1, &lang);
            get_sys_param_int(ex.G, param_get_int, 4, &time_zone);
            if (get_clock) clock_hz = (u64)NC(ex.G, get_clock, 0, 0, 0, 0, 0, 0);
            if (is_pa_enabled) pa_enabled = (s32)NC(ex.G, is_pa_enabled, 0, 0, 0, 0, 0, 0);
            show_text_page(
                &ex,
                dialog,
                "Dashboard\n%s\n%04d-%02d-%02d %02d:%02d\nNet:%s GPU:%uMHz\nLang:%d TZ:%d PA:%d\n\nCIRCLE volver",
                user_name,
                (int)clk.year,
                (int)clk.month,
                (int)clk.day,
                (int)clk.hour,
                (int)clk.minute,
                net_state_name(net_state),
                (u32)(clock_hz / 1000000ULL),
                (int)lang,
                (int)time_zone,
                (int)pa_enabled
            );
        } else if (choice == PS5SDK_PAD_BTN_SQUARE) {
            u32 last_buttons = 0;
            if (pad_read && pad_h >= 0) {
                u32 last_raw = 0;
                s32 last_read = ps5_sdk_pad_read_buttons(ex.G, pad_read, pad_h, pad_buf, &last_raw, &last_buttons);
                if (last_read > 0 && (u32)last_read < 0x80000000) reads++;
                seen_buttons |= last_buttons;
            }
            show_text_page(
                &ex,
                dialog,
                "Pad live\nh=%x init=%x reads=%d\nlast=%x %s\nseen=%x\n\nCIRCLE volver",
                (u32)pad_h,
                (u32)pad_init_ret,
                (int)reads,
                (u32)last_buttons,
                ps5_sdk_pad_button_name(ps5_sdk_pad_first_button(last_buttons)),
                (u32)seen_buttons
            );
        } else if (choice == PS5SDK_PAD_BTN_CIRCLE) {
            last_audio_out = play_short_beep(ex.G, audio_output, audio_h, samples, PS5SDK_PAD_BTN_SQUARE);
            show_text_page(
                &ex,
                dialog,
                "Audio beep\nh=%x init=%x\nlast_out=%x\nCROSS pitido\nCIRCLE volver",
                (u32)audio_h,
                (u32)audio_init_ret,
                (u32)last_audio_out
            );
        } else {
            s32 lang = 0;
            s32 date_fmt = 0;
            s32 time_fmt = 0;
            s32 time_zone = 0;
            s32 summer = 0;
            get_sys_param_int(ex.G, param_get_int, 1, &lang);
            get_sys_param_int(ex.G, param_get_int, 2, &date_fmt);
            get_sys_param_int(ex.G, param_get_int, 3, &time_fmt);
            get_sys_param_int(ex.G, param_get_int, 4, &time_zone);
            get_sys_param_int(ex.G, param_get_int, 5, &summer);
            show_text_page(
                &ex,
                dialog,
                "System info\nlang=%d date=%d time=%d\ntz=%d summer=%d\nuser=%d\n\nCIRCLE volver",
                (int)lang,
                (int)date_fmt,
                (int)time_fmt,
                (int)time_zone,
                (int)summer,
                (int)ex.user_id
            );
        }
    }

    ext->dbg[0] = (u64)(s64)pad_h;
    ext->dbg[1] = (u64)(s64)audio_h;
    ext->dbg[2] = (u64)seen_buttons;
    ext->dbg[3] = (u64)(s64)last_audio_out;
    ext->status = 0;

    if (audio_close && audio_h >= 0) {
        NC(ex.G, audio_close, (u64)audio_h, 0, 0, 0, 0, 0);
    }
    ex_close_dialog(&ex);
    ex_free_dialog(&ex, dialog);
    if (work && ex.munmap_fn) NC(ex.G, ex.munmap_fn, (u64)work, 0x2000, 0, 0, 0, 0);
    ext->step = 99;
}
