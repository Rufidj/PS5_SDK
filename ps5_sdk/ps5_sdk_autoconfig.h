/* ps5_sdk_autoconfig.h — Automatic kernel address detection and initialization
 *
 * This header provides transparent, automatic initialization of kernel addresses
 * based on the PS5 firmware version detected at runtime. No manual setup required.
 *
 * Include this early in your code (e.g., in your main entry point or early init)
 * to enable automatic firmware detection and address resolution.
 */
#ifndef PS5_SDK_AUTOCONFIG_H
#define PS5_SDK_AUTOCONFIG_H

#include "ps5_sdk_types.h"

/* Automatic initialization function
 * Detects kernel data base and initializes all KERNEL_ADDRESS_* variables.
 * Called automatically on first use via constructor attribute.
 * Safe to call multiple times (idempotent).
 *
 * Returns:
 *   0 on success
 *   -ENOSYS if firmware version is not supported
 */
extern s32 ps5_sdk_auto_init(void);

#endif /* PS5_SDK_AUTOCONFIG_H */
