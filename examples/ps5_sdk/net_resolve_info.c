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

    s32 net_mod = (s32)NC(ex.G, ex.load_mod, (u64)"libSceNet.sprx", 0, 0, 0, 0, 0);
    s32 netctl_mod = (s32)NC(ex.G, ex.load_mod, (u64)"libSceNetCtl.sprx", 0, 0, 0, 0, 0);

    void *socket_fn = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, "socket");
    void *close_fn = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, "close");
    void *bind_fn = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, "bind");
    void *listen_fn = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, "listen");
    void *accept_fn = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, "accept");
    void *sendto_fn = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, "sendto");
    void *recvfrom_fn = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, "recvfrom");
    void *poll_fn = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, "poll");
    void *setsockopt_fn = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, "setsockopt");
    void *getsockname_fn = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, "getsockname");

    u32 resolved = 0;
    if (socket_fn) resolved++;
    if (close_fn) resolved++;
    if (bind_fn) resolved++;
    if (listen_fn) resolved++;
    if (accept_fn) resolved++;
    if (sendto_fn) resolved++;
    if (recvfrom_fn) resolved++;
    if (poll_fn) resolved++;
    if (setsockopt_fn) resolved++;
    if (getsockname_fn) resolved++;

    ext->dbg[0] = (u64)(s64)net_mod;
    ext->dbg[1] = (u64)(s64)netctl_mod;
    ext->dbg[2] = (u64)resolved;
    ext->status = 0;

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
        "Network resolve\nnet=%x netctl=%x\nlibkernel sockets=%d/10\nsock=%p send=%p recv=%p\nresolve-only OK",
        (u32)net_mod,
        (u32)netctl_mod,
        (int)resolved,
        socket_fn,
        sendto_fn,
        recvfrom_fn
    );
    if (ret == 0) ex_wait_user_close(&ex);
    ex_close_dialog(&ex);
    ex_free_dialog(&ex, buf);
    ext->step = 99;
}
