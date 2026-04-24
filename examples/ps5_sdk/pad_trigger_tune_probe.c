#include "example_common.h"

static void trig_zero(u8 *p) {
    if (!p) return;
    ex_zero(p, 0x1000);
}

static void trig_fill(u8 *p, u8 mode, u8 a, u8 b, u8 c) {
    trig_zero(p);
    p[0x00] = 0x01;
    p[0x01] = mode;
    p[0x02] = a;
    p[0x03] = b;
    p[0x04] = c;
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

    s32 load_ret = (s32)NC(ex.G, ex.load_mod, (u64)"libScePad.sprx", 0, 0, 0, 0, 0);
    void *pad_init = SYM(ex.G, ex.D, PAD_HANDLE, "scePadInit");
    void *pad_geth = SYM(ex.G, ex.D, PAD_HANDLE, "scePadGetHandle");
    void *pad_fx = SYM(ex.G, ex.D, PAD_HANDLE, "scePadSetTriggerEffect");

    s32 init_ret = pad_init ? (s32)NC(ex.G, pad_init, 0, 0, 0, 0, 0, 0) : -1;
    s32 pad_h = pad_geth ? (s32)NC(ex.G, pad_geth, (u64)ex.user_id, 0, 0, 0, 0, 0) : -1;

    u8 *work = (u8 *)NC(ex.G, ex.mmap_fn, 0, 0x1000, 3, 0x1002, (u64)-1, 0);
    if ((s64)work == -1) work = 0;

    s32 r1 = -1, r2 = -1, r3 = -1, rr1 = -1, rr2 = -1;

    u8 *buf = ex_alloc_dialog(&ex);
    if (!buf) {
        if (work && ex.munmap_fn) NC(ex.G, ex.munmap_fn, (u64)work, 0x1000, 0, 0, 0, 0);
        ext->status = -3;
        return;
    }

    if (pad_fx && pad_h >= 0 && work) {
        ex_dialog_begin(&ex);
        ret = ex_open_text(
            &ex,
            buf,
            "Pad trigger tune\nPon el dedo en R2.\nPulsa aceptar y nota si alguna fase ofrece resistencia."
        );
        if (ret == 0) ex_wait_user_close(&ex);
        ex_close_dialog(&ex);

        trig_fill(work, 0x03, 0x40, 0x80, 0x80);
        r1 = (s32)NC(ex.G, pad_fx, (u64)pad_h, (u64)work, 0, 0, 0, 0);
        if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 1000000, 0, 0, 0, 0, 0);

        trig_fill(work, 0x03, 0x60, 0xA0, 0xA0);
        r2 = (s32)NC(ex.G, pad_fx, (u64)pad_h, (u64)work, 0, 0, 0, 0);
        if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 1000000, 0, 0, 0, 0, 0);

        trig_fill(work, 0x02, 0x20, 0x90, 0x00);
        r3 = (s32)NC(ex.G, pad_fx, (u64)pad_h, (u64)work, 0, 0, 0, 0);
        if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 1000000, 0, 0, 0, 0, 0);

        trig_fill(work, 0x00, 0x00, 0x00, 0x00);
        rr1 = (s32)NC(ex.G, pad_fx, (u64)pad_h, (u64)work, 0, 0, 0, 0);
        if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 300000, 0, 0, 0, 0, 0);

        trig_fill(work, 0x01, 0x00, 0x00, 0x00);
        rr2 = (s32)NC(ex.G, pad_fx, (u64)pad_h, (u64)work, 0, 0, 0, 0);
    }

    ext->dbg[0] = (u64)(s64)pad_h;
    ext->dbg[1] = (u64)(s64)r1;
    ext->dbg[2] = (u64)(s64)r2;
    ext->dbg[3] = (u64)(s64)r3;
    ext->dbg[4] = (u64)(s64)rr1;
    ext->dbg[5] = (u64)(s64)rr2;
    ext->status = 0;

    ex_dialog_begin(&ex);
    ret = ex_open_text(
        &ex,
        buf,
        "Pad trigger tune\nload=%x init=%x h=%x\np1=%x p2=%x p3=%x\nrz0=%x rz1=%x",
        (u32)load_ret,
        (u32)init_ret,
        (u32)pad_h,
        (u32)r1,
        (u32)r2,
        (u32)r3,
        (u32)rr1,
        (u32)rr2
    );
    if (ret == 0) ex_wait_user_close(&ex);
    ex_close_dialog(&ex);

    ex_free_dialog(&ex, buf);
    if (work && ex.munmap_fn) NC(ex.G, ex.munmap_fn, (u64)work, 0x1000, 0, 0, 0, 0);
    ext->step = 99;
}
