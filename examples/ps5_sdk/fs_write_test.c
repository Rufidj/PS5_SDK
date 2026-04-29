#include "example_common.h"

#define O_RDONLY    0x0000
#define O_WRONLY    0x0001
#define O_RDWR      0x0002
#define O_CREAT     0x0200
#define O_TRUNC     0x0400

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

    void *kopen = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, KERNEL_sceKernelOpen);
    void *kwrite = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, KERNEL_sceKernelWrite);
    void *kread = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, KERNEL_sceKernelRead);
    void *kclose = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, KERNEL_sceKernelClose);

    if (!kopen || !kwrite || !kread || !kclose) {
        ext->status = -2;
        return;
    }

    const char *paths[] = {
        "/data/ps5_sdk_test.txt",
        "/temp0/ps5_sdk_test.txt",
        "/savedata0/ps5_sdk_test.txt",
        "/app0/ps5_sdk_test.txt"
    };
    const char *filepath = "N/A";
    const char *msg_write = "Hello from PS5 SDK fs_write_test!\n";
    s32 fd_w = -1, fd_r = -1;
    s64 written = -1, read_bytes = -1;

    u8 *buf = ex_alloc_dialog(&ex);
    if (!buf) {
        ext->status = -3;
        return;
    }

    /* Buffer for reading from file */
    u8 *file_buf = (u8 *)NC(ex.G, ex.mmap_fn, 0, 0x1000, 3, 0x1002, (u64)-1, 0);
    if ((s64)file_buf != -1) {
        ex_zero(file_buf, 0x1000);

        /* Write phase: try paths until one works */
        for (int i = 0; i < 4; i++) {
            fd_w = (s32)NC(ex.G, kopen, (u64)paths[i], O_CREAT | O_TRUNC | O_RDWR, 0777, 0, 0, 0);
            if (fd_w >= 0) {
                filepath = paths[i];
                u64 len = 0;
                while (msg_write[len]) len++;
                written = (s64)NC(ex.G, kwrite, (u64)fd_w, (u64)msg_write, len, 0, 0, 0);
                NC(ex.G, kclose, (u64)fd_w, 0, 0, 0, 0, 0);
                break;
            }
        }

        /* Read phase: if we found a working path */
        if (fd_w >= 0) {
            fd_r = (s32)NC(ex.G, kopen, (u64)filepath, O_RDONLY, 0, 0, 0, 0);
            if (fd_r >= 0) {
                read_bytes = (s64)NC(ex.G, kread, (u64)fd_r, (u64)file_buf, 0xFFF, 0, 0, 0);
                NC(ex.G, kclose, (u64)fd_r, 0, 0, 0, 0, 0);
            }
        }
    }

    ext->step = 2;
    ex_dialog_begin(&ex);
    ret = ex_open_text(&ex, buf, 
        "FS Write Test\nPath: %s\nfd_w: %d, written: %d\nfd_r: %d, read: %d\nContent: %s",
        filepath, (int)fd_w, (int)written, (int)fd_r, (int)read_bytes, 
        (file_buf && read_bytes > 0) ? (char*)file_buf : "N/A");

    if (ret == 0) ex_wait_user_close(&ex);
    ex_close_dialog(&ex);

    if ((s64)file_buf != -1) NC(ex.G, ex.munmap_fn, (u64)file_buf, 0x1000, 0, 0, 0, 0);
    ex_free_dialog(&ex, buf);

    ext->status = 0;
    ext->step = 99;
}
