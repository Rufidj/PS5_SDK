#include "example_common.h"

static u32 pad_buttons_from_raw(u32 raw) {
    return raw & 0x001FFFFF;
}

static const char *pad_name_from_buttons(u32 b) {
    if (b & 0x00004000) return "CROSS";
    if (b & 0x00008000) return "SQUARE";
    if (b & 0x00002000) return "CIRCLE";
    if (b & 0x00001000) return "TRIANGLE";
    if (b & 0x00000400) return "L1";
    if (b & 0x00000800) return "R1";
    if (b & 0x00000010) return "UP";
    if (b & 0x00000040) return "DOWN";
    if (b & 0x00000080) return "LEFT";
    if (b & 0x00000020) return "RIGHT";
    if (b & 0x00000008) return "OPTIONS";
    return "none";
}

static void append_text(char *dst, u32 dst_size, const char *src) {
    u32 pos = 0;
    while (pos + 1 < dst_size && dst[pos]) pos++;
    while (pos + 1 < dst_size && *src) dst[pos++] = *src++;
    dst[pos] = 0;
}

static void append_button_name(char *dst, u32 dst_size, u32 *first, const char *name) {
    if (!*first) append_text(dst, dst_size, " ");
    append_text(dst, dst_size, name);
    *first = 0;
}

static void pad_names_from_buttons(u32 b, char *dst, u32 dst_size) {
    u32 first = 1;
    if (!dst || dst_size == 0) return;
    dst[0] = 0;

    if (b & 0x00004000) append_button_name(dst, dst_size, &first, "CROSS");
    if (b & 0x00008000) append_button_name(dst, dst_size, &first, "SQUARE");
    if (b & 0x00002000) append_button_name(dst, dst_size, &first, "CIRCLE");
    if (b & 0x00001000) append_button_name(dst, dst_size, &first, "TRIANGLE");
    if (b & 0x00000400) append_button_name(dst, dst_size, &first, "L1");
    if (b & 0x00000800) append_button_name(dst, dst_size, &first, "R1");
    if (b & 0x00000010) append_button_name(dst, dst_size, &first, "UP");
    if (b & 0x00000040) append_button_name(dst, dst_size, &first, "DOWN");
    if (b & 0x00000080) append_button_name(dst, dst_size, &first, "LEFT");
    if (b & 0x00000020) append_button_name(dst, dst_size, &first, "RIGHT");
    if (b & 0x00000008) append_button_name(dst, dst_size, &first, "OPTIONS");
    if (first) append_text(dst, dst_size, "none");
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

    s32 init_ret = pad_init ? (s32)NC(ex.G, pad_init, 0, 0, 0, 0, 0, 0) : -1;
    s32 pad_h = pad_geth ? (s32)NC(ex.G, pad_geth, (u64)ex.user_id, 0, 0, 0, 0, 0) : -1;

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
        "Pad capture\nPulsa aceptar y luego toca botones durante 5 segundos."
    );
    if (ret != 0) {
        ext->status = -4;
        ex_free_dialog(&ex, buf);
        return;
    }
    ex_wait_user_close(&ex);
    ex_close_dialog(&ex);

    u8 *pad_buf = buf + 0x200;
    s32 last_read = -1;
    u32 last_raw = 0;
    u32 last_buttons = 0;
    u32 seen_buttons = 0;
    s32 good_reads = 0;
    s32 pressed_reads = 0;

    for (int i = 0; i < 150; i++) {
        for (int j = 0; j < 128; j++) pad_buf[j] = 0;
        if (pad_read && pad_h >= 0) {
            last_read = (s32)NC(ex.G, pad_read, (u64)pad_h, (u64)pad_buf, 1, 0, 0, 0);
            last_raw = *(u32 *)pad_buf;
            last_buttons = pad_buttons_from_raw(last_raw);
            if (last_read > 0 && (u32)last_read < 0x80000000) good_reads++;
            if (last_buttons) pressed_reads++;
            seen_buttons |= last_buttons;
        }

        if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 33333, 0, 0, 0, 0, 0);
    }

    ext->dbg[0] = (u64)(s64)pad_h;
    ext->dbg[1] = (u64)(s64)last_read;
    ext->dbg[2] = (u64)last_raw;
    ext->status = 0;

    char *seen_names = (char *)(buf + 0x280);
    char *last_name = (char *)(buf + 0x340);
    pad_names_from_buttons(seen_buttons, seen_names, 160);
    pad_names_from_buttons(last_buttons, last_name, 64);

    ex_dialog_begin(&ex);
    ret = ex_open_text(
        &ex,
        buf,
        "Pad result\nh=%x init=%x reads=%d pressed=%d\nlast=%x %s\nseen=%x\n%s",
        (u32)pad_h,
        (u32)init_ret,
        (int)good_reads,
        (int)pressed_reads,
        last_buttons,
        last_name,
        seen_buttons,
        seen_names
    );
    if (ret != 0) {
        ret = ex_open_text(
            &ex,
            buf,
            "Pad result\nh=%x init=%x reads=%d p=%d\nlast=%x seen=%x",
            (u32)pad_h,
            (u32)init_ret,
            (int)good_reads,
            (int)pressed_reads,
            last_buttons,
            seen_buttons
        );
    }
    if (ret == 0) {
        ex_wait_user_close(&ex);
    }
    ex_close_dialog(&ex);
    ex_free_dialog(&ex, buf);
    ext->step = 99;
}
