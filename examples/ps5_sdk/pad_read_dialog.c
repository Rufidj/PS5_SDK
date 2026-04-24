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
            last_read = ps5_sdk_pad_read_buttons(ex.G, pad_read, pad_h, pad_buf, &last_raw, &last_buttons);
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
    ps5_sdk_pad_names_from_buttons(seen_buttons, seen_names, 160);
    ps5_sdk_pad_names_from_buttons(last_buttons, last_name, 64);

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
