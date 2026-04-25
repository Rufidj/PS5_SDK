#include "example_common.h"

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

    s32 load_ret = ps5_sdk_load_sprx(ex.G, ex.load_mod, "libScePad.sprx");
    void *pad_init = SYM(ex.G, ex.D, PAD_HANDLE, "scePadInit");
    void *pad_geth = SYM(ex.G, ex.D, PAD_HANDLE, "scePadGetHandle");
    void *pad_read = SYM(ex.G, ex.D, PAD_HANDLE, "scePadRead");
    void *pad_light = SYM(ex.G, ex.D, PAD_HANDLE, "scePadSetLightBar");
    void *pad_light_reset = SYM(ex.G, ex.D, PAD_HANDLE, "scePadResetLightBar");

    s32 init_ret = pad_init ? (s32)NC(ex.G, pad_init, 0, 0, 0, 0, 0, 0) : -1;
    s32 pad_h = pad_geth ? (s32)NC(ex.G, pad_geth, (u64)ex.user_id, 0, 0, 0, 0, 0) : -1;

    u8 *buf = ex_alloc_dialog(&ex);
    if (!buf) {
        ext->status = -3;
        return;
    }

    u8 *work = (u8 *)NC(ex.G, ex.mmap_fn, 0, 0x1000, 3, 0x1002, (u64)-1, 0);
    if ((s64)work == -1) work = 0;

    ex_dialog_begin(&ex);
    ret = ex_open_text(
        &ex,
        buf,
        "Pad lightbar live\nAceptar y luego pulsa botones.\nCada boton cambia el color del mando.\nDuracion: 8 segundos."
    );
    if (ret == 0) ex_wait_user_close(&ex);
    ex_close_dialog(&ex);

    u8 *pad_buf = buf + 0x200;
    u32 seen = 0;
    u32 last_btn = 0;
    s32 changes = 0;
    s32 last_set = -1;
    u8 last_r = 0x30, last_g = 0x30, last_b = 0x30;

    for (int i = 0; i < 240; i++) {
        u32 raw = 0, buttons = 0;
        if (pad_read && pad_h >= 0) {
            ps5_sdk_pad_read_buttons(ex.G, pad_read, pad_h, pad_buf, &raw, &buttons);
            u32 btn = ps5_sdk_pad_first_button(buttons);
            if (btn) {
                u8 r, g, b;
                pick_color(btn, &r, &g, &b);
                last_set = ps5_sdk_pad_set_lightbar(ex.G, pad_light, pad_h, work, r, g, b);
                last_btn = btn;
                last_r = r; last_g = g; last_b = b;
                seen |= buttons;
                changes++;
            }
        }
        if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 33333, 0, 0, 0, 0, 0);
    }

    s32 reset_ret = ps5_sdk_pad_reset_lightbar(ex.G, pad_light_reset, pad_h);

    ext->dbg[0] = (u64)(s64)pad_h;
    ext->dbg[1] = (u64)(s64)changes;
    ext->dbg[2] = (u64)seen;
    ext->dbg[3] = (u64)(s64)last_set;
    ext->dbg[4] = (u64)(s64)reset_ret;
    ext->status = 0;

    ex_dialog_begin(&ex);
    ret = ex_open_text(
        &ex,
        buf,
        "Pad lightbar live\nload=%x init=%x h=%x\nchanges=%d last=%s\nrgb=%02x %02x %02x\nset=%x reset=%x",
        (u32)load_ret,
        (u32)init_ret,
        (u32)pad_h,
        (int)changes,
        ps5_sdk_pad_button_name(last_btn),
        last_r, last_g, last_b,
        (u32)last_set,
        (u32)reset_ret
    );
    if (ret == 0) ex_wait_user_close(&ex);
    ex_close_dialog(&ex);

    ex_free_dialog(&ex, buf);
    if (work && ex.munmap_fn) NC(ex.G, ex.munmap_fn, (u64)work, 0x1000, 0, 0, 0, 0);
    ext->step = 99;
}
