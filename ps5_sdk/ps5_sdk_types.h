/* ps5_sdk_types.h — standalone types, gadget call, and hardware offsets.
 * Self-contained: no libc, no EmuC0re core.h required.
 */
#ifndef PS5_SDK_TYPES_H
#define PS5_SDK_TYPES_H

typedef unsigned long  u64;
typedef unsigned int   u32;
typedef unsigned short u16;
typedef unsigned char  u8;
typedef long           s64;
typedef int            s32;
typedef short          s16;
typedef signed char    s8;

/* fw 13.00 eboot layout */
#define PS5SDK_GADGET_OFFSET   0x31AA9
#define PS5SDK_EBOOT_GS_THREAD 0x057F89B0
#define PS5SDK_EBOOT_VIDOUT    0x02d695d0

/* screen geometry */
#define PS5SDK_SCR_W       1920
#define PS5SDK_SCR_H       1080
#define PS5SDK_FB_SIZE     (PS5SDK_SCR_W * PS5SDK_SCR_H * 4)
#define PS5SDK_FB_ALIGNED  ((PS5SDK_FB_SIZE + 0x1FFFFF) & ~0x1FFFFF)
#define PS5SDK_FB_TOTAL    (PS5SDK_FB_ALIGNED * 2)

__attribute__((naked))
static u64 ps5_sdk_native_call(void *gadget, void *fn,
                                u64 a1, u64 a2, u64 a3,
                                u64 a4, u64 a5, u64 a6)
{
    __asm__ volatile (
        "pushq %%rbx\n\t"
        "movq %%rsi, %%rbx\n\t"
        "movq %%rdi, %%rax\n\t"
        "movq %%rdx, %%rdi\n\t"
        "movq %%rcx, %%rsi\n\t"
        "movq %%r8,  %%rdx\n\t"
        "movq %%r9,  %%rcx\n\t"
        "movq 16(%%rsp), %%r8\n\t"
        "movq 24(%%rsp), %%r9\n\t"
        "callq *%%rax\n\t"
        "popq %%rbx\n\t"
        "retq" ::: "memory"
    );
}

static void *ps5_sdk_resolve_sym(void *gadget, void *dlsym_fn,
                                  s32 handle, const char *name)
{
    void *addr = 0;
    ps5_sdk_native_call(gadget, dlsym_fn,
                        (u64)handle, (u64)name, (u64)&addr, 0, 0, 0);
    return addr;
}

/* Short aliases used throughout the SDK and examples */
#define PS5SDK_NC  ps5_sdk_native_call
#define PS5SDK_SYM ps5_sdk_resolve_sym

/* Backwards-compat names so existing examples compile unchanged */
#define NC              ps5_sdk_native_call
#define SYM             ps5_sdk_resolve_sym
#define GADGET_OFFSET   PS5SDK_GADGET_OFFSET
#define EBOOT_GS_THREAD PS5SDK_EBOOT_GS_THREAD
#define EBOOT_VIDOUT    PS5SDK_EBOOT_VIDOUT

#endif /* PS5_SDK_TYPES_H */
