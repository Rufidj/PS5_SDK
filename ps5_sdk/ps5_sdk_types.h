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
    if (addr) return addr;

    /* Portable fallback:
     * if static handle lookup failed, map known handles to module paths,
     * load the module dynamically, and retry dlsym with the runtime handle.
     */
    if (!gadget || !dlsym_fn || !name) return 0;

    const char *mod_path = 0;
    switch (handle) {
        case 0x2:  mod_path = "libSceLibcInternal.sprx"; break;
        case 0x11: mod_path = "libSceSysmodule.sprx"; break;
        case 0x13: mod_path = "libSceFios2.prx"; break;
        case 0x14: mod_path = "libc.prx"; break;
        case 0x1e: mod_path = "libSceNet.sprx"; break;
        case 0x2d: mod_path = "libSceIpmi.sprx"; break;
        case 0x2e: mod_path = "libSceMbus.sprx"; break;
        case 0x2f: mod_path = "libSceRegMgr.sprx"; break;
        case 0x30: mod_path = "libSceRtc.sprx"; break;
        case 0x31: mod_path = "libSceAvSetting.sprx"; break;
        case 0x32: mod_path = "libSceVideoOut.sprx"; break;
        case 0x33: mod_path = "libSceGnmDriver.sprx"; break;
        case 0x35: mod_path = "libSceAudioOut.sprx"; break;
        case 0x39: mod_path = "libSceAudioIn.sprx"; break;
        case 0x3c: mod_path = "libSceAjm.sprx"; break;
        case 0x3d: mod_path = "libScePad.sprx"; break;
        case 0x3e: mod_path = "libSceCamera.sprx"; break;
        case 0x44: mod_path = "libSceNetCtl.sprx"; break;
        case 0x46: mod_path = "libSceHttp.sprx"; break;
        case 0x48: mod_path = "libSceSsl.sprx"; break;
        case 0x4a: mod_path = "libSceNpCommon.sprx"; break;
        case 0x4b: mod_path = "libSceNpManager.sprx"; break;
        case 0x77: mod_path = "libSceNpWebApi.sprx"; break;
        case 0x78: mod_path = "libSceSaveData.sprx"; break;
        case 0x79: mod_path = "libSceSystemService.sprx"; break;
        case 0x8a: mod_path = "libSceUserService.sprx"; break;
        case 0x8b: mod_path = "libSceCommonDialog.sprx"; break;
        case 0x8d: mod_path = "libSceSysUtil.sprx"; break;
        case 0x8e: mod_path = "libSceSisrMgr.sprx"; break;
        case 0x93: mod_path = "libScePngEnc.sprx"; break;
        case 0x94: mod_path = "libSceAppContent.sprx"; break;
        case 0x96: mod_path = "libSceNpTrophy.sprx"; break;
        case 0x9d: mod_path = "libSceGameLiveStreaming.sprx"; break;
        case 0x9f: mod_path = "libSceRemoteplay.sprx"; break;
        case 0xa1: mod_path = "libSceScreenShot.sprx"; break;
        case 0xa2: mod_path = "libSceResourceArbitrator.sprx"; break;
        case 0xa4: mod_path = "libSceAvcap2.sprx"; break;
        case 0xa5: mod_path = "libSceVideoRecording.sprx"; break;
        case 0xaa: mod_path = "libSceSharePlay.sprx"; break;
        case 0xac: mod_path = "libSceCompanionHttpd.sprx"; break;
        case 0xb5: mod_path = "libSceMsgDialog.sprx"; break;
        case 0xb7: mod_path = "libSceSaveDataDialog.sprx"; break;
        case 0xb9: mod_path = "libScePs2EmuMenuDialog.sprx"; break;
        default: break;
    }
    if (!mod_path) return 0;

    void *load_start = 0;
    ps5_sdk_native_call(gadget, dlsym_fn,
                        (u64)0x2001, (u64)"sceKernelLoadStartModule",
                        (u64)&load_start, 0, 0, 0);
    if (!load_start) return 0;

    s64 dyn_h = (s64)ps5_sdk_native_call(gadget, load_start,
                                         (u64)mod_path, 0, 0, 0, 0, 0);
    if (dyn_h <= 0) return 0;

    ps5_sdk_native_call(gadget, dlsym_fn,
                        (u64)(s32)dyn_h, (u64)name, (u64)&addr, 0, 0, 0);
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
