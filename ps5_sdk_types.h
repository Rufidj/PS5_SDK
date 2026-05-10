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

/* Kernel address globals - initialized by ps5_sdk_init_kernel_addresses() */
extern u64 KERNEL_ADDRESS_DATA_BASE;
extern u64 KERNEL_ADDRESS_TEXT_BASE;
extern u64 KERNEL_ADDRESS_ALLPROC;
extern u64 KERNEL_ADDRESS_SECURITY_FLAGS;
extern u64 KERNEL_ADDRESS_PRISON0;
extern u64 KERNEL_ADDRESS_ROOTVNODE;
extern u64 KERNEL_ADDRESS_BUS_DATA_DEVICES;
extern u64 KERNEL_OFFSET_VMSPACE_P_ROOT;
extern u64 KERNEL_ADDRESS_TARGETID;
extern u64 KERNEL_ADDRESS_QA_FLAGS;
extern u64 KERNEL_ADDRESS_UTOKEN_FLAGS;

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

/* Multi-firmware kernel address initialization */
static inline s32 ps5_sdk_init_kernel_addresses(u64 data_base)
{
    KERNEL_ADDRESS_DATA_BASE = data_base;
    
    u32 fw_version = kernel_get_fw_version() & 0xffff0000;
    
    switch(fw_version) {
    case 0x1000000:
    case 0x1010000:
    case 0x1020000:
        KERNEL_ADDRESS_TEXT_BASE        = KERNEL_ADDRESS_DATA_BASE - 0x1B40000;
        KERNEL_ADDRESS_ALLPROC          = KERNEL_ADDRESS_DATA_BASE + 0x26D1BF8;
        KERNEL_ADDRESS_SECURITY_FLAGS   = KERNEL_ADDRESS_DATA_BASE + 0x6241074;
        KERNEL_ADDRESS_PRISON0          = KERNEL_ADDRESS_DATA_BASE + 0x1911E00;
        KERNEL_ADDRESS_ROOTVNODE        = KERNEL_ADDRESS_DATA_BASE + 0x6565540;
        KERNEL_ADDRESS_BUS_DATA_DEVICES = KERNEL_ADDRESS_DATA_BASE + 0x1D6D478;
        KERNEL_OFFSET_VMSPACE_P_ROOT    = 0x1c0;
        break;

    case 0x1050000:
    case 0x1100000:
    case 0x1110000:
    case 0x1120000:
    case 0x1130000:
    case 0x1140000:
        KERNEL_ADDRESS_TEXT_BASE        = KERNEL_ADDRESS_DATA_BASE - 0x1B40000;
        KERNEL_ADDRESS_ALLPROC          = KERNEL_ADDRESS_DATA_BASE + 0x26D1C18;
        KERNEL_ADDRESS_SECURITY_FLAGS   = KERNEL_ADDRESS_DATA_BASE + 0x6241074;
        KERNEL_ADDRESS_PRISON0          = KERNEL_ADDRESS_DATA_BASE + 0x1911E00;
        KERNEL_ADDRESS_ROOTVNODE        = KERNEL_ADDRESS_DATA_BASE + 0x6565540;
        KERNEL_ADDRESS_BUS_DATA_DEVICES = KERNEL_ADDRESS_DATA_BASE + 0x1D6D487;
        KERNEL_OFFSET_VMSPACE_P_ROOT    = 0x1c8;
        break;

    case 0x2000000:
        KERNEL_ADDRESS_TEXT_BASE        = KERNEL_ADDRESS_DATA_BASE - 0x1B80000;
        KERNEL_ADDRESS_ALLPROC          = KERNEL_ADDRESS_DATA_BASE + 0x2701C28;
        KERNEL_ADDRESS_SECURITY_FLAGS   = KERNEL_ADDRESS_DATA_BASE + 0x63E1274;
        KERNEL_ADDRESS_PRISON0          = KERNEL_ADDRESS_DATA_BASE + 0x194BA60;
        KERNEL_ADDRESS_ROOTVNODE        = KERNEL_ADDRESS_DATA_BASE + 0x67134C0;
        KERNEL_ADDRESS_BUS_DATA_DEVICES = KERNEL_ADDRESS_DATA_BASE + 0x1D91478;
        KERNEL_OFFSET_VMSPACE_P_ROOT    = 0x1c8;
        break;

    case 0x2200000:
    case 0x2250000:
    case 0x2260000:
    case 0x2300000:
    case 0x2500000:
    case 0x2700000:
        KERNEL_ADDRESS_TEXT_BASE        = KERNEL_ADDRESS_DATA_BASE - 0x1B80000;
        KERNEL_ADDRESS_ALLPROC          = KERNEL_ADDRESS_DATA_BASE + 0x2701C28;
        KERNEL_ADDRESS_SECURITY_FLAGS   = KERNEL_ADDRESS_DATA_BASE + 0x63E1274;
        KERNEL_ADDRESS_PRISON0          = KERNEL_ADDRESS_DATA_BASE + 0x194BD20;
        KERNEL_ADDRESS_ROOTVNODE        = KERNEL_ADDRESS_DATA_BASE + 0x67134C0;
        KERNEL_ADDRESS_BUS_DATA_DEVICES = KERNEL_ADDRESS_DATA_BASE + 0x1D91478;
        KERNEL_OFFSET_VMSPACE_P_ROOT    = 0x1c8;
        break;

    case 0x3000000:
    case 0x3100000:
    case 0x3200000:
    case 0x3210000:
        KERNEL_ADDRESS_TEXT_BASE        = KERNEL_ADDRESS_DATA_BASE - 0x0BD0000;
        KERNEL_ADDRESS_ALLPROC          = KERNEL_ADDRESS_DATA_BASE + 0x276DC58;
        KERNEL_ADDRESS_SECURITY_FLAGS   = KERNEL_ADDRESS_DATA_BASE + 0x6466474;
        KERNEL_ADDRESS_PRISON0          = KERNEL_ADDRESS_DATA_BASE + 0x1CC2670;
        KERNEL_ADDRESS_ROOTVNODE        = KERNEL_ADDRESS_DATA_BASE + 0x67AB4C0;
        KERNEL_ADDRESS_BUS_DATA_DEVICES = KERNEL_ADDRESS_DATA_BASE + 0x1DF1678;
        KERNEL_OFFSET_VMSPACE_P_ROOT    = 0x1c8;
        break;

    case 0x4020000:
        KERNEL_ADDRESS_TEXT_BASE        = KERNEL_ADDRESS_DATA_BASE - 0x0C00000;
        KERNEL_ADDRESS_ALLPROC          = KERNEL_ADDRESS_DATA_BASE + 0x27EDCB8;
        KERNEL_ADDRESS_SECURITY_FLAGS   = KERNEL_ADDRESS_DATA_BASE + 0x6506474;
        KERNEL_ADDRESS_PRISON0          = KERNEL_ADDRESS_DATA_BASE + 0x1D34D00;
        KERNEL_ADDRESS_ROOTVNODE        = KERNEL_ADDRESS_DATA_BASE + 0x66E74C0;
        KERNEL_ADDRESS_BUS_DATA_DEVICES = KERNEL_ADDRESS_DATA_BASE + 0x1E69678;
        KERNEL_OFFSET_VMSPACE_P_ROOT    = 0x1c8;
        break;

    case 0x4000000:
    case 0x4030000:
    case 0x4500000:
    case 0x4510000:
        KERNEL_ADDRESS_TEXT_BASE        = KERNEL_ADDRESS_DATA_BASE - 0x0C00000;
        KERNEL_ADDRESS_ALLPROC          = KERNEL_ADDRESS_DATA_BASE + 0x27EDCB8;
        KERNEL_ADDRESS_SECURITY_FLAGS   = KERNEL_ADDRESS_DATA_BASE + 0x6506474;
        KERNEL_ADDRESS_PRISON0          = KERNEL_ADDRESS_DATA_BASE + 0x1D34D00;
        KERNEL_ADDRESS_ROOTVNODE        = KERNEL_ADDRESS_DATA_BASE + 0x66E74C0;
        KERNEL_ADDRESS_BUS_DATA_DEVICES = KERNEL_ADDRESS_DATA_BASE + 0x1E69678;
        KERNEL_OFFSET_VMSPACE_P_ROOT    = 0x1c8;
        break;

    case 0x5000000:
    case 0x5020000:
    case 0x5100000:
        KERNEL_ADDRESS_TEXT_BASE        = KERNEL_ADDRESS_DATA_BASE - 0x0C40000;
        KERNEL_ADDRESS_ALLPROC          = KERNEL_ADDRESS_DATA_BASE + 0x291DD00;
        KERNEL_ADDRESS_SECURITY_FLAGS   = KERNEL_ADDRESS_DATA_BASE + 0x66466EC;
        KERNEL_ADDRESS_PRISON0          = KERNEL_ADDRESS_DATA_BASE + 0x1E23470;
        KERNEL_ADDRESS_ROOTVNODE        = KERNEL_ADDRESS_DATA_BASE + 0x6853510;
        KERNEL_ADDRESS_BUS_DATA_DEVICES = KERNEL_ADDRESS_DATA_BASE + 0x1F996C8;
        KERNEL_OFFSET_VMSPACE_P_ROOT    = 0x1c8;
        break;

    case 0x5500000:
        KERNEL_ADDRESS_TEXT_BASE        = KERNEL_ADDRESS_DATA_BASE - 0x0C40000;
        KERNEL_ADDRESS_ALLPROC          = KERNEL_ADDRESS_DATA_BASE + 0x291DD00;
        KERNEL_ADDRESS_SECURITY_FLAGS   = KERNEL_ADDRESS_DATA_BASE + 0x66466EC;
        KERNEL_ADDRESS_PRISON0          = KERNEL_ADDRESS_DATA_BASE + 0x1E235B0;
        KERNEL_ADDRESS_ROOTVNODE        = KERNEL_ADDRESS_DATA_BASE + 0x6853510;
        KERNEL_ADDRESS_BUS_DATA_DEVICES = KERNEL_ADDRESS_DATA_BASE + 0x1F996C8;
        KERNEL_OFFSET_VMSPACE_P_ROOT    = 0x1c8;
        break;

    case 0x6000000:
    case 0x6020000:
    case 0x6500000:
        KERNEL_ADDRESS_TEXT_BASE        = KERNEL_ADDRESS_DATA_BASE - 0x0C60000;
        KERNEL_ADDRESS_ALLPROC          = KERNEL_ADDRESS_DATA_BASE + 0x2869D20;
        KERNEL_ADDRESS_SECURITY_FLAGS   = KERNEL_ADDRESS_DATA_BASE + 0x65968EC;
        KERNEL_ADDRESS_PRISON0          = KERNEL_ADDRESS_DATA_BASE + 0x1E45560;
        KERNEL_ADDRESS_ROOTVNODE        = KERNEL_ADDRESS_DATA_BASE + 0x679F510;
        KERNEL_ADDRESS_BUS_DATA_DEVICES = KERNEL_ADDRESS_DATA_BASE + 0x1FB96C8;
        KERNEL_OFFSET_VMSPACE_P_ROOT    = 0x1d0;
        break;

    case 0x7000000:
    case 0x7010000:
    case 0x7200000:
    case 0x7400000:
    case 0x7600000:
    case 0x7610000:
        KERNEL_ADDRESS_TEXT_BASE        = KERNEL_ADDRESS_DATA_BASE - 0x0C50000;
        KERNEL_ADDRESS_ALLPROC          = KERNEL_ADDRESS_DATA_BASE + 0x2859D50;
        KERNEL_ADDRESS_SECURITY_FLAGS   = KERNEL_ADDRESS_DATA_BASE + 0x0AC8064;
        KERNEL_ADDRESS_PRISON0          = KERNEL_ADDRESS_DATA_BASE + 0x1E477F0;
        KERNEL_ADDRESS_ROOTVNODE        = KERNEL_ADDRESS_DATA_BASE + 0x30C7510;
        KERNEL_ADDRESS_BUS_DATA_DEVICES = KERNEL_ADDRESS_DATA_BASE + 0x1FA5718;
        KERNEL_OFFSET_VMSPACE_P_ROOT    = 0x1d0;
        break;

    case 0x8000000:
    case 0x8200000:
    case 0x8400000:
    case 0x8600000:
        KERNEL_ADDRESS_TEXT_BASE        = KERNEL_ADDRESS_DATA_BASE - 0x0C70000;
        KERNEL_ADDRESS_ALLPROC          = KERNEL_ADDRESS_DATA_BASE + 0x2875D50;
        KERNEL_ADDRESS_SECURITY_FLAGS   = KERNEL_ADDRESS_DATA_BASE + 0x0AC3064;
        KERNEL_ADDRESS_PRISON0          = KERNEL_ADDRESS_DATA_BASE + 0x1E484D0;
        KERNEL_ADDRESS_ROOTVNODE        = KERNEL_ADDRESS_DATA_BASE + 0x30FB510;
        KERNEL_ADDRESS_BUS_DATA_DEVICES = KERNEL_ADDRESS_DATA_BASE + 0x1FA5718;
        KERNEL_OFFSET_VMSPACE_P_ROOT    = 0x1d0;
        break;

    case 0x9000000:
        KERNEL_ADDRESS_TEXT_BASE        = KERNEL_ADDRESS_DATA_BASE - 0x0CA0000;
        KERNEL_ADDRESS_ALLPROC          = KERNEL_ADDRESS_DATA_BASE + 0x2755D50;
        KERNEL_ADDRESS_SECURITY_FLAGS   = KERNEL_ADDRESS_DATA_BASE + 0x0D72064;
        KERNEL_ADDRESS_PRISON0          = KERNEL_ADDRESS_DATA_BASE + 0x1E0ED80;
        KERNEL_ADDRESS_ROOTVNODE        = KERNEL_ADDRESS_DATA_BASE + 0x2FDB510;
        KERNEL_ADDRESS_BUS_DATA_DEVICES = KERNEL_ADDRESS_DATA_BASE + 0x1F65718;
        KERNEL_OFFSET_VMSPACE_P_ROOT    = 0x1d0;
        break;

    case 0x9050000:
    case 0x9200000:
    case 0x9400000:
    case 0x9600000:
        KERNEL_ADDRESS_TEXT_BASE        = KERNEL_ADDRESS_DATA_BASE - 0x0CA0000;
        KERNEL_ADDRESS_ALLPROC          = KERNEL_ADDRESS_DATA_BASE + 0x2755D50;
        KERNEL_ADDRESS_SECURITY_FLAGS   = KERNEL_ADDRESS_DATA_BASE + 0x0D73064;
        KERNEL_ADDRESS_PRISON0          = KERNEL_ADDRESS_DATA_BASE + 0x1E0ED80;
        KERNEL_ADDRESS_ROOTVNODE        = KERNEL_ADDRESS_DATA_BASE + 0x2FDB510;
        KERNEL_ADDRESS_BUS_DATA_DEVICES = KERNEL_ADDRESS_DATA_BASE + 0x1F65718;
        KERNEL_OFFSET_VMSPACE_P_ROOT    = 0x1d0;
        break;

    case 0x10000000:
    case 0x10010000:
    case 0x10200000:
    case 0x10400000:
    case 0x10600000:
        KERNEL_ADDRESS_TEXT_BASE        = KERNEL_ADDRESS_DATA_BASE - 0x0CC0000;
        KERNEL_ADDRESS_ALLPROC          = KERNEL_ADDRESS_DATA_BASE + 0x2765D70;
        KERNEL_ADDRESS_SECURITY_FLAGS   = KERNEL_ADDRESS_DATA_BASE + 0x0D79064;
        KERNEL_ADDRESS_PRISON0          = KERNEL_ADDRESS_DATA_BASE + 0x1E0F4E0;
        KERNEL_ADDRESS_ROOTVNODE        = KERNEL_ADDRESS_DATA_BASE + 0x2FA3510;
        KERNEL_ADDRESS_BUS_DATA_DEVICES = KERNEL_ADDRESS_DATA_BASE + 0x1F65718;
        KERNEL_OFFSET_VMSPACE_P_ROOT    = 0x1d0;
        break;

    case 0x11000000:
    case 0x11200000:
        KERNEL_ADDRESS_TEXT_BASE        = KERNEL_ADDRESS_DATA_BASE - 0x0D30000;
        KERNEL_ADDRESS_ALLPROC          = KERNEL_ADDRESS_DATA_BASE + 0x2875D70;
        KERNEL_ADDRESS_SECURITY_FLAGS   = KERNEL_ADDRESS_DATA_BASE + 0x0D8C064;
        KERNEL_ADDRESS_PRISON0          = KERNEL_ADDRESS_DATA_BASE + 0x1F21220;
        KERNEL_ADDRESS_ROOTVNODE        = KERNEL_ADDRESS_DATA_BASE + 0x30B7510;
        KERNEL_ADDRESS_BUS_DATA_DEVICES = KERNEL_ADDRESS_DATA_BASE + 0x2075718;
        KERNEL_OFFSET_VMSPACE_P_ROOT    = 0x1d0;
        break;

    case 0x11400000:
        KERNEL_ADDRESS_TEXT_BASE        = KERNEL_ADDRESS_DATA_BASE - 0x0D30000;
        KERNEL_ADDRESS_ALLPROC          = KERNEL_ADDRESS_DATA_BASE + 0x2875D70;
        KERNEL_ADDRESS_SECURITY_FLAGS   = KERNEL_ADDRESS_DATA_BASE + 0x0D8C064;
        KERNEL_ADDRESS_PRISON0          = KERNEL_ADDRESS_DATA_BASE + 0x1F21260;
        KERNEL_ADDRESS_ROOTVNODE        = KERNEL_ADDRESS_DATA_BASE + 0x30B7510;
        KERNEL_ADDRESS_BUS_DATA_DEVICES = KERNEL_ADDRESS_DATA_BASE + 0x2075718;
        KERNEL_OFFSET_VMSPACE_P_ROOT    = 0x1d0;
        break;

    case 0x11600000:
        KERNEL_ADDRESS_TEXT_BASE        = KERNEL_ADDRESS_DATA_BASE - 0x0D30000;
        KERNEL_ADDRESS_ALLPROC          = KERNEL_ADDRESS_DATA_BASE + 0x2875D70;
        KERNEL_ADDRESS_SECURITY_FLAGS   = KERNEL_ADDRESS_DATA_BASE + 0x0D8C064;
        KERNEL_ADDRESS_PRISON0          = KERNEL_ADDRESS_DATA_BASE + 0x1F212A0;
        KERNEL_ADDRESS_ROOTVNODE        = KERNEL_ADDRESS_DATA_BASE + 0x30B7510;
        KERNEL_ADDRESS_BUS_DATA_DEVICES = KERNEL_ADDRESS_DATA_BASE + 0x2075718;
        KERNEL_OFFSET_VMSPACE_P_ROOT    = 0x1d0;
        break;

    case 0x12000000:
    case 0x12020000:
    case 0x12200000:
    case 0x12400000:
    case 0x12600000:
    case 0x12700000:
        KERNEL_ADDRESS_TEXT_BASE        = KERNEL_ADDRESS_DATA_BASE - 0x0D50000;
        KERNEL_ADDRESS_ALLPROC          = KERNEL_ADDRESS_DATA_BASE + 0x2885E00;
        KERNEL_ADDRESS_SECURITY_FLAGS   = KERNEL_ADDRESS_DATA_BASE + 0x0D83064;
        KERNEL_ADDRESS_PRISON0          = KERNEL_ADDRESS_DATA_BASE + 0x1F229F0;
        KERNEL_ADDRESS_ROOTVNODE        = KERNEL_ADDRESS_DATA_BASE + 0x30D7510;
        KERNEL_ADDRESS_BUS_DATA_DEVICES = KERNEL_ADDRESS_DATA_BASE + 0x20757E8;
        KERNEL_OFFSET_VMSPACE_P_ROOT    = 0x1d0;
        break;

    case 0x13000000:
    case 0x13200000:
        KERNEL_ADDRESS_TEXT_BASE        = KERNEL_ADDRESS_DATA_BASE - 0x0CB0000;
        KERNEL_ADDRESS_ALLPROC          = KERNEL_ADDRESS_DATA_BASE + 0x28C5E00;
        KERNEL_ADDRESS_SECURITY_FLAGS   = KERNEL_ADDRESS_DATA_BASE + 0x0D99064;
        KERNEL_ADDRESS_PRISON0          = KERNEL_ADDRESS_DATA_BASE + 0x1F36240;
        KERNEL_ADDRESS_ROOTVNODE        = KERNEL_ADDRESS_DATA_BASE + 0x3133510;
        KERNEL_ADDRESS_BUS_DATA_DEVICES = KERNEL_ADDRESS_DATA_BASE + 0x20981E8;
        KERNEL_OFFSET_VMSPACE_P_ROOT    = 0x1d0;
        break;

    default:
        return -38;  /* ENOSYS */
    }

    /* Compute derived addresses */
    KERNEL_ADDRESS_TARGETID     = KERNEL_ADDRESS_SECURITY_FLAGS + 0x09;
    KERNEL_ADDRESS_QA_FLAGS     = KERNEL_ADDRESS_SECURITY_FLAGS + 0x24;
    KERNEL_ADDRESS_UTOKEN_FLAGS = KERNEL_ADDRESS_SECURITY_FLAGS + 0x8C;

    return 0;
}

/* Backwards-compat names so existing examples compile unchanged */
#define NC              ps5_sdk_native_call
#define SYM             ps5_sdk_resolve_sym
#define GADGET_OFFSET   PS5SDK_GADGET_OFFSET
#define EBOOT_GS_THREAD PS5SDK_EBOOT_GS_THREAD
#define EBOOT_VIDOUT    PS5SDK_EBOOT_VIDOUT

#endif /* PS5_SDK_TYPES_H */
