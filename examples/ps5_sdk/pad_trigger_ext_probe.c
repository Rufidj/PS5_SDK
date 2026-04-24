#include "example_common.h"

static void trig_fill_basic(u8 *p) {
    if (!p) return;
    ex_zero(p, 0x1000);
    p[0x00] = 0x01;
    p[0x01] = 0x03;
    p[0x02] = 0x60;
    p[0x03] = 0xA0;
    p[0x04] = 0xA0;
}

static void trig_zero(u8 *p) {
    if (!p) return;
    ex_zero(p, 0x1000);
}

static s32 try_fx(void *G, void *fx, void *usleep_fn, s32 h, u8 *work) {
    s32 ret;
    if (!fx || h < 0 || !work) return -1;
    trig_fill_basic(work);
    ret = (s32)NC(G, fx, (u64)h, (u64)work, 0, 0, 0, 0);
    if (usleep_fn) NC(G, usleep_fn, 700000, 0, 0, 0, 0, 0);
    trig_zero(work);
    NC(G, fx, (u64)h, (u64)work, 0, 0, 0, 0);
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

    s32 load_ret = (s32)NC(ex.G, ex.load_mod, (u64)"libScePad.sprx", 0, 0, 0, 0, 0);
    void *pad_init = SYM(ex.G, ex.D, PAD_HANDLE, "scePadInit");
    void *pad_geth = SYM(ex.G, ex.D, PAD_HANDLE, "scePadGetHandle");
    void *pad_open_ext = SYM(ex.G, ex.D, PAD_HANDLE, "scePadOpenExt");
    void *pad_open_ext2 = SYM(ex.G, ex.D, PAD_HANDLE, "scePadOpenExt2");
    void *pad_fx = SYM(ex.G, ex.D, PAD_HANDLE, "scePadSetTriggerEffect");

    s32 init_ret = pad_init ? (s32)NC(ex.G, pad_init, 0, 0, 0, 0, 0, 0) : -1;
    s32 h_get = pad_geth ? (s32)NC(ex.G, pad_geth, (u64)ex.user_id, 0, 0, 0, 0, 0) : -1;
    s32 h_ext = pad_open_ext ? (s32)NC(ex.G, pad_open_ext, (u64)ex.user_id, 0, 0, 0, 0, 0) : -1;
    s32 h_ext2 = pad_open_ext2 ? (s32)NC(ex.G, pad_open_ext2, (u64)ex.user_id, 0, 0, 0, 0, 0) : -1;

    u8 *work = (u8 *)NC(ex.G, ex.mmap_fn, 0, 0x1000, 3, 0x1002, (u64)-1, 0);
    if ((s64)work == -1) work = 0;

    s32 r_get = -1, r_ext = -1, r_ext2 = -1;

    u8 *buf = ex_alloc_dialog(&ex);
    if (!buf) {
        if (work && ex.munmap_fn) NC(ex.G, ex.munmap_fn, (u64)work, 0x1000, 0, 0, 0, 0);
        ext->status = -3;
        return;
    }

    if (pad_fx && work) {
        ex_dialog_begin(&ex);
        ret = ex_open_text(
            &ex,
            buf,
            "Pad trigger ext probe\nPon el dedo en R2.\nPulsa aceptar y nota si cambia algo."
        );
        if (ret == 0) ex_wait_user_close(&ex);
        ex_close_dialog(&ex);

        r_get = try_fx(ex.G, pad_fx, ex.usleep_fn, h_get, work);
        r_ext = try_fx(ex.G, pad_fx, ex.usleep_fn, h_ext, work);
        r_ext2 = try_fx(ex.G, pad_fx, ex.usleep_fn, h_ext2, work);
    }

    ext->dbg[0] = (u64)(s64)h_get;
    ext->dbg[1] = (u64)(s64)h_ext;
    ext->dbg[2] = (u64)(s64)h_ext2;
    ext->dbg[3] = (u64)(s64)r_get;
    ext->dbg[4] = (u64)(s64)r_ext;
    ext->dbg[5] = (u64)(s64)r_ext2;
    ext->status = 0;

    ex_dialog_begin(&ex);
    ret = ex_open_text(
        &ex,
        buf,
        "Pad trigger ext probe\nload=%x init=%x\nget=%x ext=%x ext2=%x\nrGet=%x rExt=%x rExt2=%x",
        (u32)load_ret,
        (u32)init_ret,
        (u32)h_get,
        (u32)h_ext,
        (u32)h_ext2,
        (u32)r_get,
        (u32)r_ext,
        (u32)r_ext2
    );
    if (ret == 0) ex_wait_user_close(&ex);
    ex_close_dialog(&ex);

    ex_free_dialog(&ex, buf);
    if (work && ex.munmap_fn) NC(ex.G, ex.munmap_fn, (u64)work, 0x1000, 0, 0, 0, 0);
    ext->step = 99;
}
