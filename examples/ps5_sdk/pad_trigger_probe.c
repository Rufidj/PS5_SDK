#include "example_common.h"

static void pad_trigger_fill_variant_a(u8 *p) {
    if (!p) return;
    ex_zero(p, 0x1000);

    /* Conservative guess: two trigger blocks, right side only. */
    p[0x10] = 0x01;
    p[0x11] = 0x03;
    p[0x12] = 0x90;
    p[0x13] = 0xA0;
    p[0x14] = 0xA0;
    p[0x15] = 0x00;
}

static void pad_trigger_fill_zero(u8 *p) {
    if (!p) return;
    ex_zero(p, 0x1000);
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
            "Pad trigger probe\nPon el dedo en R2.\nPulsa aceptar y espera 2 segundos."
        );
        if (ret == 0) ex_wait_user_close(&ex);
        ex_close_dialog(&ex);
    }

    s32 fx_ret = -1;
    s32 reset_ret = -1;

    if (pad_fx && pad_h >= 0 && work) {
        pad_trigger_fill_variant_a(work);
        fx_ret = (s32)NC(ex.G, pad_fx, (u64)pad_h, (u64)work, 0, 0, 0, 0);
        if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 2000000, 0, 0, 0, 0, 0);

        pad_trigger_fill_zero(work);
        reset_ret = (s32)NC(ex.G, pad_fx, (u64)pad_h, (u64)work, 0, 0, 0, 0);
    }

    ext->dbg[0] = (u64)(s64)pad_h;
    ext->dbg[1] = (u64)(s64)fx_ret;
    ext->dbg[2] = (u64)(s64)reset_ret;
    ext->dbg[3] = work ? *(u32 *)(work + 0x10) : 0;
    ext->dbg[4] = work ? *(u32 *)(work + 0x14) : 0;
    ext->status = 0;

    ex_dialog_begin(&ex);
    ret = ex_open_text(
        &ex,
        buf,
        "Pad trigger probe\nload=%x init=%x h=%x\nfx=%x reset=%x\nb10=%02x %02x %02x %02x\nb14=%02x %02x",
        (u32)load_ret,
        (u32)init_ret,
        (u32)pad_h,
        (u32)fx_ret,
        (u32)reset_ret,
        work ? work[0x10] : 0,
        work ? work[0x11] : 0,
        work ? work[0x12] : 0,
        work ? work[0x13] : 0,
        work ? work[0x14] : 0,
        work ? work[0x15] : 0
    );
    if (ret == 0) ex_wait_user_close(&ex);
    ex_close_dialog(&ex);

    ex_free_dialog(&ex, buf);
    if (work && ex.munmap_fn) NC(ex.G, ex.munmap_fn, (u64)work, 0x1000, 0, 0, 0, 0);
    ext->step = 99;
}
