#include "example_common.h"

#define SDK_AF_INET     2
#define SDK_SOCK_DGRAM  2

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
    void *net_init = SYM(ex.G, ex.D, NET_HANDLE, "sceNetInit");
    void *net_term = SYM(ex.G, ex.D, NET_HANDLE, "sceNetTerm");
    void *net_socket = SYM(ex.G, ex.D, NET_HANDLE, "sceNetSocket");
    void *net_close = SYM(ex.G, ex.D, NET_HANDLE, "sceNetSocketClose");

    s32 init_ret = net_init ? (s32)NC(ex.G, net_init, 0, 0, 0, 0, 0, 0) : -1;

    s32 fd = -1;
    s32 close_ret = 0x7fffffff;
    if (net_socket) {
        fd = (s32)NC(ex.G, net_socket, (u64)"sdk_net_init_probe", SDK_AF_INET, SDK_SOCK_DGRAM, 0, 0, 0);
        if (net_close && fd >= 0) {
            close_ret = (s32)NC(ex.G, net_close, (u64)fd, 0, 0, 0, 0, 0);
        }
    }

    s32 term_ret = net_term ? (s32)NC(ex.G, net_term, 0, 0, 0, 0, 0, 0) : -1;

    ext->dbg[0] = (u64)(s64)init_ret;
    ext->dbg[1] = (u64)(s64)fd;
    ext->dbg[2] = (u64)(s64)term_ret;
    ext->status = 0;

    u8 *buf = ex_alloc_dialog(&ex);
    if (!buf) {
        ext->status = -3;
        return;
    }

    ext->step = 2;
    ex_dialog_begin(&ex);
    if (fd >= 0) {
        ret = ex_open_text(
            &ex,
            buf,
            "Net init probe\nnet=%x init=%x\nfd=%d close=%x term=%x\nsceNet OK",
            (u32)net_mod,
            (u32)init_ret,
            (int)fd,
            (u32)close_ret,
            (u32)term_ret
        );
    } else {
        ret = ex_open_text(
            &ex,
            buf,
            "Net init probe\nnet=%x init=%x\nsocket=%x term=%x\ninit=%p socket=%p",
            (u32)net_mod,
            (u32)init_ret,
            (u32)fd,
            (u32)term_ret,
            net_init,
            net_socket
        );
    }
    if (ret == 0) ex_wait_user_close(&ex);
    ex_close_dialog(&ex);
    ex_free_dialog(&ex, buf);
    ext->step = 99;
}
