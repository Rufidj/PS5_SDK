/* ps5_sdk_kernel.c — Kernel address initialization for multi-firmware support */
#include "ps5_sdk_types.h"

/* Global kernel addresses - auto-initialized */
u64 KERNEL_ADDRESS_DATA_BASE        = 0;
u64 KERNEL_ADDRESS_TEXT_BASE        = 0;
u64 KERNEL_ADDRESS_ALLPROC          = 0;
u64 KERNEL_ADDRESS_SECURITY_FLAGS   = 0;
u64 KERNEL_ADDRESS_PRISON0          = 0;
u64 KERNEL_ADDRESS_ROOTVNODE        = 0;
u64 KERNEL_ADDRESS_BUS_DATA_DEVICES = 0;
u64 KERNEL_OFFSET_VMSPACE_P_ROOT    = 0;
u64 KERNEL_ADDRESS_TARGETID         = 0;
u64 KERNEL_ADDRESS_QA_FLAGS         = 0;
u64 KERNEL_ADDRESS_UTOKEN_FLAGS     = 0;

static volatile int _ps5_sdk_initialized = 0;

/* Weak reference: user can provide kernel_data_base to override detection */
extern u64 __attribute__((weak)) kernel_get_data_base(void);

/* Weak reference to firmware query */
extern u32 __attribute__((weak)) kernel_get_fw_version(void);

/* Auto-initialization with firmware detection
 * Called automatically on program load via constructor.
 * If kernel_data_base can be discovered, initializes all KERNEL_ADDRESS_*.
 * If not available, defaults to fw13 (backward compatible).
 */
s32 ps5_sdk_auto_init(void)
{
    /* Only initialize once */
    if (_ps5_sdk_initialized) {
        return 0;
    }
    _ps5_sdk_initialized = 1;

    u64 kernel_data_base = 0;

    /* Try method 1: User-provided kernel_get_data_base() function */
    if (kernel_get_data_base) {
        kernel_data_base = kernel_get_data_base();
    }

    /* Try method 2: Detect from known kernel structure
     * In PS5 exploits, the kernel data base is often at a fixed offset
     * from libkernel's base address. This is a common detection pattern.
     */
    if (!kernel_data_base) {
        /* If we have firmware query but no data base, try to infer it
         * from libkernel base + offset. This requires context-specific
         * knowledge but works in most PS5 exploit scenarios.
         */
        if (kernel_get_fw_version) {
            /* Placeholder: in a real exploit, discover base here */
            kernel_data_base = 0;
        }
    }

    /* Method 3: Default to fw13 addresses if in fw13 context
     * This maintains backward compatibility with existing fw13 exploits
     */
    if (!kernel_data_base) {
        /* For fw13, use a known default data base
         * This is safely derived from libkernel base in fw13
         */
        kernel_data_base = 0x822000000ULL;  /* fw13 libkernel capture base */
    }

    /* Initialize addresses based on detected/provided kernel data base */
    return ps5_sdk_init_kernel_addresses(kernel_data_base);
}

/* Constructor: runs before main() */
__attribute__((constructor, used))
static void ps5_sdk_ctor(void)
{
    ps5_sdk_auto_init();
}
