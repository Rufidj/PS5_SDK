/* ps5_sdk_system.h - PS5 system Offsets */
#ifndef PS5_SDK_PS5_SDK_SYSTEM_H
#define PS5_SDK_PS5_SDK_SYSTEM_H

/* -- libSceSysmodule (270 offsets) -- */
#define PS5SDK_SYSMODULE_HANDLE 0x11
/* sceSysmoduleGetModuleHandleInternal (NID: 0x00000008FFFF6A1C) */
#define PS5SDK_SYSMODULE_sceSysmoduleGetModuleHandleInternal_OFFSET 0x00000680u
#define PS5SDK_SYSMODULE_sceSysmoduleGetModuleHandleInternal_EH_OFFSET 0x00000750u
#define PS5SDK_SYSMODULE_sceSysmoduleGetModuleHandleInternal_RUNTIME_VA 0x000000080c724750ULL
#define SYSMODULE_sceSysmoduleGetModuleHandleInternal      "sceSysmoduleGetModuleHandleInternal"
#define SYSMODULE_sceSysmoduleGetModuleHandleInternal_RUNTIME_VA 0x000000080c724750ULL

/* sceSysmoduleIsLoaded (NID: 0x00000025FFFF588C) */
#define PS5SDK_SYSMODULE_sceSysmoduleIsLoaded_OFFSET 0x00000240u
#define PS5SDK_SYSMODULE_sceSysmoduleIsLoaded_EH_OFFSET 0x00000310u
#define PS5SDK_SYSMODULE_sceSysmoduleIsLoaded_RUNTIME_VA 0x000000080c724310ULL
#define SYSMODULE_sceSysmoduleIsLoaded                     "sceSysmoduleIsLoaded"
#define SYSMODULE_sceSysmoduleIsLoaded_RUNTIME_VA 0x000000080c724310ULL

/* sceSysmoduleLoadModule (NID: 0x110000000100000) */
#define PS5SDK_SYSMODULE_sceSysmoduleLoadModule_OFFSET 0x00000000u
#define PS5SDK_SYSMODULE_sceSysmoduleLoadModule_EH_OFFSET 0x000000d0u
#define PS5SDK_SYSMODULE_sceSysmoduleLoadModule_RUNTIME_VA 0x000000080c7240d0ULL
#define SYSMODULE_sceSysmoduleLoadModule                   "sceSysmoduleLoadModule"
#define SYSMODULE_sceSysmoduleLoadModule_RUNTIME_VA 0x000000080c7240d0ULL

/* sceSysmoduleLoadModuleByNameInternal (NID: 0x00000008FFFF6A34) */
#define PS5SDK_SYSMODULE_sceSysmoduleLoadModuleByNameInternal_OFFSET 0x000006f0u
#define PS5SDK_SYSMODULE_sceSysmoduleLoadModuleByNameInternal_EH_OFFSET 0x000007c0u
#define PS5SDK_SYSMODULE_sceSysmoduleLoadModuleByNameInternal_RUNTIME_VA 0x000000080c7247c0ULL
#define SYSMODULE_sceSysmoduleLoadModuleByNameInternal     "sceSysmoduleLoadModuleByNameInternal"
#define SYSMODULE_sceSysmoduleLoadModuleByNameInternal_RUNTIME_VA 0x000000080c7247c0ULL

/* sceSysmoduleLoadModuleInternal (NID: 0x10060C4108070C05) */
#define PS5SDK_SYSMODULE_sceSysmoduleLoadModuleInternal_OFFSET 0x00000430u
#define PS5SDK_SYSMODULE_sceSysmoduleLoadModuleInternal_EH_OFFSET 0x00000500u
#define PS5SDK_SYSMODULE_sceSysmoduleLoadModuleInternal_RUNTIME_VA 0x000000080c724500ULL
#define SYSMODULE_sceSysmoduleLoadModuleInternal           "sceSysmoduleLoadModuleInternal"
#define SYSMODULE_sceSysmoduleLoadModuleInternal_RUNTIME_VA 0x000000080c724500ULL

/* sceSysmoduleUnloadModule (NID: 0xFFFF5850000008DC) */
#define PS5SDK_SYSMODULE_sceSysmoduleUnloadModule_OFFSET 0x000001d0u
#define PS5SDK_SYSMODULE_sceSysmoduleUnloadModule_EH_OFFSET 0x000002a0u
#define PS5SDK_SYSMODULE_sceSysmoduleUnloadModule_RUNTIME_VA 0x000000080c7242a0ULL
#define SYSMODULE_sceSysmoduleUnloadModule                 "sceSysmoduleUnloadModule"
#define SYSMODULE_sceSysmoduleUnloadModule_RUNTIME_VA 0x000000080c7242a0ULL

/* sceSysmoduleUnloadModuleInternal (NID: 0x00000013FFFF5E3C) */
#define PS5SDK_SYSMODULE_sceSysmoduleUnloadModuleInternal_OFFSET 0x00000490u
#define PS5SDK_SYSMODULE_sceSysmoduleUnloadModuleInternal_EH_OFFSET 0x00000560u
#define PS5SDK_SYSMODULE_sceSysmoduleUnloadModuleInternal_RUNTIME_VA 0x000000080c724560ULL
#define SYSMODULE_sceSysmoduleUnloadModuleInternal         "sceSysmoduleUnloadModuleInternal"
#define SYSMODULE_sceSysmoduleUnloadModuleInternal_RUNTIME_VA 0x000000080c724560ULL

/* -- libSceSystemService (1240 offsets) -- */
#define PS5SDK_SYSTEMSERVICE_HANDLE 0x79
/* sceSystemServiceGetAppStatus (NID: 0xFFFDC5B000003AEC) */
#define PS5SDK_SYSTEMSERVICE_sceSystemServiceGetAppStatus_OFFSET 0x00004c70u
#define PS5SDK_SYSTEMSERVICE_sceSystemServiceGetAppStatus_EH_OFFSET 0x00004d40u
#define PS5SDK_SYSTEMSERVICE_sceSystemServiceGetAppStatus_RUNTIME_VA 0x0000000839928d40ULL
#define SYSTEMSERVICE_sceSystemServiceGetAppStatus             "sceSystemServiceGetAppStatus"
#define SYSTEMSERVICE_sceSystemServiceGetAppStatus_RUNTIME_VA 0x0000000839928d40ULL

/* sceSystemServiceKillApp (NID: 0x070C6D0203834506) */
#define PS5SDK_SYSTEMSERVICE_sceSystemServiceKillApp_OFFSET 0x00004c20u
#define PS5SDK_SYSTEMSERVICE_sceSystemServiceKillApp_EH_OFFSET 0x00004cf0u
#define PS5SDK_SYSTEMSERVICE_sceSystemServiceKillApp_RUNTIME_VA 0x0000000839928cf0ULL
#define SYSTEMSERVICE_sceSystemServiceKillApp                  "sceSystemServiceKillApp"
#define SYSTEMSERVICE_sceSystemServiceKillApp_RUNTIME_VA 0x0000000839928cf0ULL

/* sceSystemServiceReceiveEvent (NID: 0x058349060D430286) */
#define PS5SDK_SYSTEMSERVICE_sceSystemServiceReceiveEvent_OFFSET 0x00000000u
#define PS5SDK_SYSTEMSERVICE_sceSystemServiceReceiveEvent_EH_OFFSET 0x000000d0u
#define PS5SDK_SYSTEMSERVICE_sceSystemServiceReceiveEvent_RUNTIME_VA 0x00000008399240d0ULL
#define SYSTEMSERVICE_sceSystemServiceReceiveEvent             "sceSystemServiceReceiveEvent"
#define SYSTEMSERVICE_sceSystemServiceReceiveEvent_RUNTIME_VA 0x00000008399240d0ULL

/* -- libSceUserService (816 offsets) -- */
#define PS5SDK_USERSERVICE_HANDLE 0x8a
/* sceUserServiceTerminate (NID: 0x060C4108070C6602) */
#define PS5SDK_USERSERVICE_sceUserServiceTerminate_OFFSET 0x00000000u
#define PS5SDK_USERSERVICE_sceUserServiceTerminate_EH_OFFSET 0x000000d0u
#define PS5SDK_USERSERVICE_sceUserServiceTerminate_RUNTIME_VA 0x00000008003d40d0ULL
#define USERSERVICE_sceUserServiceTerminate                  "sceUserServiceTerminate"
#define USERSERVICE_sceUserServiceTerminate_RUNTIME_VA 0x00000008003d40d0ULL


#endif /* PS5_SDK_PS5_SDK_SYSTEM_H */