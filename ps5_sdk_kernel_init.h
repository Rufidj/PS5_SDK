/* ps5_sdk_kernel_init.h — Usage guide for multi-firmware kernel support */

/*
AUTOMATIC INITIALIZATION (NO SETUP REQUIRED):
==============================================

The SDK automatically initializes kernel addresses on program load via 
a constructor function. This means:

  ✓ No manual initialization calls needed
  ✓ Works transparently in all builds
  ✓ Backward compatible with existing fw13 code
  ✓ Automatically detects firmware version at runtime

How it works:
  1. On program load, ps5_sdk_kernel.c constructor runs
  2. Detects kernel data base (tries multiple methods)
  3. Calls ps5_sdk_init_kernel_addresses() automatically
  4. All KERNEL_ADDRESS_* variables ready to use in main()


OPTIONAL: CUSTOM KERNEL DATA BASE DETECTION
============================================

If you want to override the automatic detection (e.g., for a custom 
exploit or non-standard environment), provide a weak symbol:

  // In your code:
  u64 kernel_get_data_base(void)
  {
      // Your custom detection logic here
      return your_detected_kernel_data_base;
  }

The SDK will use your function instead of the default detection.


MANUAL INITIALIZATION (ADVANCED)
================================

If you need manual control over initialization:

  #include "ps5_sdk_types.h"
  
  s32 result = ps5_sdk_init_kernel_addresses(kernel_data_base);
  if (result != 0) {
      printf("ERROR: Unsupported firmware version\n");
      return result;
  }

Returns:
  0 on success
  -38 (ENOSYS) if firmware version is not supported


SUPPORTED FIRMWARE VERSIONS:
   - 1.0x (0x1000000)
   - 1.01-1.04 (0x1010000, 0x1020000)
   - 1.05 (0x1050000)
   - 1.1x-1.14 (0x1100000-0x1140000)
   - 2.0x (0x2000000)
   - 2.2x-2.7x (0x2200000-0x2700000)
   - 3.0x-3.21 (0x3000000-0x3210000)
   - 4.0x-4.51 (0x4000000-0x4510000)
   - 5.0x-5.1x (0x5000000-0x5100000)
   - 5.5x (0x5500000)
   - 6.0x-6.5x (0x6000000-0x6500000)
   - 7.0x-7.61 (0x7000000-0x7610000)
   - 8.0x-8.6x (0x8000000-0x8600000)
   - 9.0x (0x9000000)
   - 9.05-9.6x (0x9050000-0x9600000)
   - 10.0x-10.6x (0x10000000-0x10600000)
   - 11.0x-11.4x (0x11000000-0x11600000)
   - 12.0x-12.7x (0x12000000-0x12700000)
   - 13.0x-13.2x (0x13000000-0x13200000)

AVAILABLE GLOBAL ADDRESSES (auto-initialized):
   - KERNEL_ADDRESS_DATA_BASE
   - KERNEL_ADDRESS_TEXT_BASE
   - KERNEL_ADDRESS_ALLPROC
   - KERNEL_ADDRESS_SECURITY_FLAGS
   - KERNEL_ADDRESS_PRISON0
   - KERNEL_ADDRESS_ROOTVNODE
   - KERNEL_ADDRESS_BUS_DATA_DEVICES
   - KERNEL_OFFSET_VMSPACE_P_ROOT
   - KERNEL_ADDRESS_TARGETID
   - KERNEL_ADDRESS_QA_FLAGS
   - KERNEL_ADDRESS_UTOKEN_FLAGS

*/

