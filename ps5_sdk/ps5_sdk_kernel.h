#ifndef PS5_SDK_KERNEL_H
#define PS5_SDK_KERNEL_H

#define PS5SDK_KERNEL_HANDLE 0x2001
#define LIBKERNEL_HANDLE 0x2001

/* ptrace */
#define PS5SDK_KERNEL_PTRACE_RUNTIME_VA 0x00000008221900f0ULL
#define KERNEL_ptrace_RUNTIME_VA 0x00000008221900f0ULL
#define KERNEL_ptrace                                   "ptrace"

/* pipe */
#define PS5SDK_KERNEL_PIPE_RUNTIME_VA 0x0000000822190120ULL
#define KERNEL_pipe_RUNTIME_VA 0x0000000822190120ULL
#define KERNEL_pipe                                     "pipe"

/* vfork */
#define PS5SDK_KERNEL_VFORK_RUNTIME_VA 0x00000008221901d0ULL
#define KERNEL_vfork_RUNTIME_VA 0x00000008221901d0ULL
#define KERNEL_vfork                                    "vfork"

/* _exit */
#define PS5SDK_KERNEL_EXIT_RUNTIME_VA 0x00000008221903f0ULL
#define KERNEL__exit_RUNTIME_VA 0x00000008221903f0ULL
#define KERNEL__exit                                    "_exit"

/* link */
#define PS5SDK_KERNEL_LINK_RUNTIME_VA 0x00000008221904d0ULL
#define KERNEL_link_RUNTIME_VA 0x00000008221904d0ULL
#define KERNEL_link                                     "link"

/* unlink */
#define PS5SDK_KERNEL_UNLINK_RUNTIME_VA 0x00000008221904f0ULL
#define KERNEL_unlink_RUNTIME_VA 0x00000008221904f0ULL
#define KERNEL_unlink                                   "unlink"

/* chdir */
#define PS5SDK_KERNEL_CHDIR_RUNTIME_VA 0x0000000822190510ULL
#define KERNEL_chdir_RUNTIME_VA 0x0000000822190510ULL
#define KERNEL_chdir                                    "chdir"

/* chmod */
#define PS5SDK_KERNEL_CHMOD_RUNTIME_VA 0x0000000822190570ULL
#define KERNEL_chmod_RUNTIME_VA 0x0000000822190570ULL
#define KERNEL_chmod                                    "chmod"

/* chown */
#define PS5SDK_KERNEL_CHOWN_RUNTIME_VA 0x0000000822190590ULL
#define KERNEL_chown_RUNTIME_VA 0x0000000822190590ULL
#define KERNEL_chown                                    "chown"

/* getpid */
#define PS5SDK_KERNEL_GETPID_RUNTIME_VA 0x00000008221905b0ULL
#define KERNEL_getpid_RUNTIME_VA 0x00000008221905b0ULL
#define KERNEL_getpid                                   "getpid"

/* mount */
#define PS5SDK_KERNEL_MOUNT_RUNTIME_VA 0x00000008221905d0ULL
#define KERNEL_mount_RUNTIME_VA 0x00000008221905d0ULL
#define KERNEL_mount                                    "mount"

/* unmount */
#define PS5SDK_KERNEL_UNMOUNT_RUNTIME_VA 0x00000008221905f0ULL
#define KERNEL_unmount_RUNTIME_VA 0x00000008221905f0ULL
#define KERNEL_unmount                                  "unmount"

/* setuid */
#define PS5SDK_KERNEL_SETUID_RUNTIME_VA 0x0000000822190610ULL
#define KERNEL_setuid_RUNTIME_VA 0x0000000822190610ULL
#define KERNEL_setuid                                   "setuid"

/* getuid */
#define PS5SDK_KERNEL_GETUID_RUNTIME_VA 0x0000000822190630ULL
#define KERNEL_getuid_RUNTIME_VA 0x0000000822190630ULL
#define KERNEL_getuid                                   "getuid"

/* geteuid */
#define PS5SDK_KERNEL_GETEUID_RUNTIME_VA 0x0000000822190650ULL
#define KERNEL_geteuid_RUNTIME_VA 0x0000000822190650ULL
#define KERNEL_geteuid                                  "geteuid"

/* getpeername */
#define PS5SDK_KERNEL_GETPEERNAME_RUNTIME_VA 0x00000008221906f0ULL
#define KERNEL_getpeername_RUNTIME_VA 0x00000008221906f0ULL
#define KERNEL_getpeername                              "getpeername"

/* getsockname */
#define PS5SDK_KERNEL_GETSOCKNAME_RUNTIME_VA 0x0000000822190710ULL
#define KERNEL_getsockname_RUNTIME_VA 0x0000000822190710ULL
#define KERNEL_getsockname                              "getsockname"

/* access */
#define PS5SDK_KERNEL_ACCESS_RUNTIME_VA 0x0000000822190730ULL
#define KERNEL_access_RUNTIME_VA 0x0000000822190730ULL
#define KERNEL_access                                   "access"

/* sync */
#define PS5SDK_KERNEL_SYNC_RUNTIME_VA 0x0000000822190790ULL
#define KERNEL_sync_RUNTIME_VA 0x0000000822190790ULL
#define KERNEL_sync                                     "sync"

/* kill */
#define PS5SDK_KERNEL_KILL_RUNTIME_VA 0x00000008221907b0ULL
#define KERNEL_kill_RUNTIME_VA 0x00000008221907b0ULL
#define KERNEL_kill                                     "kill"

/* getppid */
#define PS5SDK_KERNEL_GETPPID_RUNTIME_VA 0x00000008221907d0ULL
#define KERNEL_getppid_RUNTIME_VA 0x00000008221907d0ULL
#define KERNEL_getppid                                  "getppid"

/* dup */
#define PS5SDK_KERNEL_DUP_RUNTIME_VA 0x00000008221907f0ULL
#define KERNEL_dup_RUNTIME_VA 0x00000008221907f0ULL
#define KERNEL_dup                                      "dup"

/* getegid */
#define PS5SDK_KERNEL_GETEGID_RUNTIME_VA 0x0000000822190810ULL
#define KERNEL_getegid_RUNTIME_VA 0x0000000822190810ULL
#define KERNEL_getegid                                  "getegid"

/* getgid */
#define PS5SDK_KERNEL_GETGID_RUNTIME_VA 0x0000000822190870ULL
#define KERNEL_getgid_RUNTIME_VA 0x0000000822190870ULL
#define KERNEL_getgid                                   "getgid"

/* ioctl */
#define PS5SDK_KERNEL_IOCTL_RUNTIME_VA 0x00000008221908d0ULL
#define KERNEL_ioctl_RUNTIME_VA 0x00000008221908d0ULL
#define KERNEL_ioctl                                    "ioctl"

/* symlink */
#define PS5SDK_KERNEL_SYMLINK_RUNTIME_VA 0x0000000822190910ULL
#define KERNEL_symlink_RUNTIME_VA 0x0000000822190910ULL
#define KERNEL_symlink                                  "symlink"

/* readlink */
#define PS5SDK_KERNEL_READLINK_RUNTIME_VA 0x0000000822190930ULL
#define KERNEL_readlink_RUNTIME_VA 0x0000000822190930ULL
#define KERNEL_readlink                                 "readlink"

/* execve */
#define PS5SDK_KERNEL_EXECVE_RUNTIME_VA 0x0000000822190950ULL
#define KERNEL_execve_RUNTIME_VA 0x0000000822190950ULL
#define KERNEL_execve                                   "execve"

/* umask */
#define PS5SDK_KERNEL_UMASK_RUNTIME_VA 0x0000000822190970ULL
#define KERNEL_umask_RUNTIME_VA 0x0000000822190970ULL
#define KERNEL_umask                                    "umask"

/* chroot */
#define PS5SDK_KERNEL_CHROOT_RUNTIME_VA 0x0000000822190990ULL
#define KERNEL_chroot_RUNTIME_VA 0x0000000822190990ULL
#define KERNEL_chroot                                   "chroot"

/* munmap */
#define PS5SDK_KERNEL_MUNMAP_RUNTIME_VA 0x00000008221909d0ULL
#define KERNEL_munmap_RUNTIME_VA 0x00000008221909d0ULL
#define KERNEL_munmap                                   "munmap"

/* mprotect */
#define PS5SDK_KERNEL_MPROTECT_RUNTIME_VA 0x00000008221909f0ULL
#define KERNEL_mprotect_RUNTIME_VA 0x00000008221909f0ULL
#define KERNEL_mprotect                                 "mprotect"

/* madvise */
#define PS5SDK_KERNEL_MADVISE_RUNTIME_VA 0x0000000822190a10ULL
#define KERNEL_madvise_RUNTIME_VA 0x0000000822190a10ULL
#define KERNEL_madvise                                  "madvise"

/* mincore */
#define PS5SDK_KERNEL_MINCORE_RUNTIME_VA 0x0000000822190a30ULL
#define KERNEL_mincore_RUNTIME_VA 0x0000000822190a30ULL
#define KERNEL_mincore                                  "mincore"

/* getgroups */
#define PS5SDK_KERNEL_GETGROUPS_RUNTIME_VA 0x0000000822190a50ULL
#define KERNEL_getgroups_RUNTIME_VA 0x0000000822190a50ULL
#define KERNEL_getgroups                                "getgroups"

/* setpgid */
#define PS5SDK_KERNEL_SETPGID_RUNTIME_VA 0x0000000822190a90ULL
#define KERNEL_setpgid_RUNTIME_VA 0x0000000822190a90ULL
#define KERNEL_setpgid                                  "setpgid"

/* dup2 */
#define PS5SDK_KERNEL_DUP2_RUNTIME_VA 0x0000000822190b30ULL
#define KERNEL_dup2_RUNTIME_VA 0x0000000822190b30ULL
#define KERNEL_dup2                                     "dup2"

/* fcntl */
#define PS5SDK_KERNEL_FCNTL_RUNTIME_VA 0x0000000822190b50ULL
#define KERNEL_fcntl_RUNTIME_VA 0x0000000822190b50ULL
#define KERNEL_fcntl                                    "fcntl"

/* setpriority */
#define PS5SDK_KERNEL_SETPRIORITY_RUNTIME_VA 0x0000000822190bb0ULL
#define KERNEL_setpriority_RUNTIME_VA 0x0000000822190bb0ULL
#define KERNEL_setpriority                              "setpriority"

/* socket */
#define PS5SDK_KERNEL_SOCKET_RUNTIME_VA 0x0000000822190bd0ULL
#define KERNEL_socket_RUNTIME_VA 0x0000000822190bd0ULL
#define KERNEL_socket                                   "socket"

/* getpriority */
#define PS5SDK_KERNEL_GETPRIORITY_RUNTIME_VA 0x0000000822190c30ULL
#define KERNEL_getpriority_RUNTIME_VA 0x0000000822190c30ULL
#define KERNEL_getpriority                              "getpriority"

/* bind */
#define PS5SDK_KERNEL_BIND_RUNTIME_VA 0x0000000822190c90ULL
#define KERNEL_bind_RUNTIME_VA 0x0000000822190c90ULL
#define KERNEL_bind                                     "bind"

/* setsockopt */
#define PS5SDK_KERNEL_SETSOCKOPT_RUNTIME_VA 0x0000000822190cb0ULL
#define KERNEL_setsockopt_RUNTIME_VA 0x0000000822190cb0ULL
#define KERNEL_setsockopt                               "setsockopt"

/* listen */
#define PS5SDK_KERNEL_LISTEN_RUNTIME_VA 0x0000000822190cd0ULL
#define KERNEL_listen_RUNTIME_VA 0x0000000822190cd0ULL
#define KERNEL_listen                                   "listen"

/* gettimeofday */
#define PS5SDK_KERNEL_GETTIMEOFDAY_RUNTIME_VA 0x0000000822190d30ULL
#define KERNEL_gettimeofday_RUNTIME_VA 0x0000000822190d30ULL
#define KERNEL_gettimeofday                             "gettimeofday"

/* getrusage */
#define PS5SDK_KERNEL_GETRUSAGE_RUNTIME_VA 0x0000000822190d50ULL
#define KERNEL_getrusage_RUNTIME_VA 0x0000000822190d50ULL
#define KERNEL_getrusage                                "getrusage"

/* getsockopt */
#define PS5SDK_KERNEL_GETSOCKOPT_RUNTIME_VA 0x0000000822190d70ULL
#define KERNEL_getsockopt_RUNTIME_VA 0x0000000822190d70ULL
#define KERNEL_getsockopt                               "getsockopt"

/* settimeofday */
#define PS5SDK_KERNEL_SETTIMEOFDAY_RUNTIME_VA 0x0000000822190dd0ULL
#define KERNEL_settimeofday_RUNTIME_VA 0x0000000822190dd0ULL
#define KERNEL_settimeofday                             "settimeofday"

/* fchown */
#define PS5SDK_KERNEL_FCHOWN_RUNTIME_VA 0x0000000822190df0ULL
#define KERNEL_fchown_RUNTIME_VA 0x0000000822190df0ULL
#define KERNEL_fchown                                   "fchown"

/* fchmod */
#define PS5SDK_KERNEL_FCHMOD_RUNTIME_VA 0x0000000822190e10ULL
#define KERNEL_fchmod_RUNTIME_VA 0x0000000822190e10ULL
#define KERNEL_fchmod                                   "fchmod"

/* rename */
#define PS5SDK_KERNEL_RENAME_RUNTIME_VA 0x0000000822190e90ULL
#define KERNEL_rename_RUNTIME_VA 0x0000000822190e90ULL
#define KERNEL_rename                                   "rename"

/* shutdown */
#define PS5SDK_KERNEL_SHUTDOWN_RUNTIME_VA 0x0000000822190f10ULL
#define KERNEL_shutdown_RUNTIME_VA 0x0000000822190f10ULL
#define KERNEL_shutdown                                 "shutdown"

/* socketpair */
#define PS5SDK_KERNEL_SOCKETPAIR_RUNTIME_VA 0x0000000822190f30ULL
#define KERNEL_socketpair_RUNTIME_VA 0x0000000822190f30ULL
#define KERNEL_socketpair                               "socketpair"

/* mkdir */
#define PS5SDK_KERNEL_MKDIR_RUNTIME_VA 0x0000000822190f50ULL
#define KERNEL_mkdir_RUNTIME_VA 0x0000000822190f50ULL
#define KERNEL_mkdir                                    "mkdir"

/* rmdir */
#define PS5SDK_KERNEL_RMDIR_RUNTIME_VA 0x0000000822190f70ULL
#define KERNEL_rmdir_RUNTIME_VA 0x0000000822190f70ULL
#define KERNEL_rmdir                                    "rmdir"

/* setsid */
#define PS5SDK_KERNEL_SETSID_RUNTIME_VA 0x0000000822190ff0ULL
#define KERNEL_setsid_RUNTIME_VA 0x0000000822190ff0ULL
#define KERNEL_setsid                                   "setsid"

/* setegid */
#define PS5SDK_KERNEL_SETEGID_RUNTIME_VA 0x0000000822191030ULL
#define KERNEL_setegid_RUNTIME_VA 0x0000000822191030ULL
#define KERNEL_setegid                                  "setegid"

/* seteuid */
#define PS5SDK_KERNEL_SETEUID_RUNTIME_VA 0x0000000822191050ULL
#define KERNEL_seteuid_RUNTIME_VA 0x0000000822191050ULL
#define KERNEL_seteuid                                  "seteuid"

/* stat */
#define PS5SDK_KERNEL_STAT_RUNTIME_VA 0x0000000822191070ULL
#define KERNEL_stat_RUNTIME_VA 0x0000000822191070ULL
#define KERNEL_stat                                     "stat"

/* fstat */
#define PS5SDK_KERNEL_FSTAT_RUNTIME_VA 0x0000000822191090ULL
#define KERNEL_fstat_RUNTIME_VA 0x0000000822191090ULL
#define KERNEL_fstat                                    "fstat"

/* lstat */
#define PS5SDK_KERNEL_LSTAT_RUNTIME_VA 0x00000008221910b0ULL
#define KERNEL_lstat_RUNTIME_VA 0x00000008221910b0ULL
#define KERNEL_lstat                                    "lstat"

/* pathconf */
#define PS5SDK_KERNEL_PATHCONF_RUNTIME_VA 0x00000008221910d0ULL
#define KERNEL_pathconf_RUNTIME_VA 0x00000008221910d0ULL
#define KERNEL_pathconf                                 "pathconf"

/* fpathconf */
#define PS5SDK_KERNEL_FPATHCONF_RUNTIME_VA 0x00000008221910f0ULL
#define KERNEL_fpathconf_RUNTIME_VA 0x00000008221910f0ULL
#define KERNEL_fpathconf                                "fpathconf"

/* getrlimit */
#define PS5SDK_KERNEL_GETRLIMIT_RUNTIME_VA 0x0000000822191110ULL
#define KERNEL_getrlimit_RUNTIME_VA 0x0000000822191110ULL
#define KERNEL_getrlimit                                "getrlimit"

/* setrlimit */
#define PS5SDK_KERNEL_SETRLIMIT_RUNTIME_VA 0x0000000822191130ULL
#define KERNEL_setrlimit_RUNTIME_VA 0x0000000822191130ULL
#define KERNEL_setrlimit                                "setrlimit"

/* mlock */
#define PS5SDK_KERNEL_MLOCK_RUNTIME_VA 0x0000000822191190ULL
#define KERNEL_mlock_RUNTIME_VA 0x0000000822191190ULL
#define KERNEL_mlock                                    "mlock"

/* munlock */
#define PS5SDK_KERNEL_MUNLOCK_RUNTIME_VA 0x00000008221911b0ULL
#define KERNEL_munlock_RUNTIME_VA 0x00000008221911b0ULL
#define KERNEL_munlock                                  "munlock"

/* clock_gettime */
#define PS5SDK_KERNEL_CLOCK_GETTIME_RUNTIME_VA 0x0000000822191210ULL
#define KERNEL_clock_gettime_RUNTIME_VA 0x0000000822191210ULL
#define KERNEL_clock_gettime                            "clock_gettime"

/* clock_settime */
#define PS5SDK_KERNEL_CLOCK_SETTIME_RUNTIME_VA 0x0000000822191230ULL
#define KERNEL_clock_settime_RUNTIME_VA 0x0000000822191230ULL
#define KERNEL_clock_settime                            "clock_settime"

/* clock_getres */
#define PS5SDK_KERNEL_CLOCK_GETRES_RUNTIME_VA 0x0000000822191250ULL
#define KERNEL_clock_getres_RUNTIME_VA 0x0000000822191250ULL
#define KERNEL_clock_getres                             "clock_getres"

/* getdents */
#define PS5SDK_KERNEL_GETDENTS_RUNTIME_VA 0x0000000822191430ULL
#define KERNEL_getdents_RUNTIME_VA 0x0000000822191430ULL
#define KERNEL_getdents                                 "getdents"

/* mlockall */
#define PS5SDK_KERNEL_MLOCKALL_RUNTIME_VA 0x0000000822191630ULL
#define KERNEL_mlockall_RUNTIME_VA 0x0000000822191630ULL
#define KERNEL_mlockall                                 "mlockall"

/* munlockall */
#define PS5SDK_KERNEL_MUNLOCKALL_RUNTIME_VA 0x0000000822191650ULL
#define KERNEL_munlockall_RUNTIME_VA 0x0000000822191650ULL
#define KERNEL_munlockall                               "munlockall"

/* kqueue */
#define PS5SDK_KERNEL_KQUEUE_RUNTIME_VA 0x0000000822191890ULL
#define KERNEL_kqueue_RUNTIME_VA 0x0000000822191890ULL
#define KERNEL_kqueue                                   "kqueue"

/* kevent */
#define PS5SDK_KERNEL_KEVENT_RUNTIME_VA 0x00000008221918b0ULL
#define KERNEL_kevent_RUNTIME_VA 0x00000008221918b0ULL
#define KERNEL_kevent                                   "kevent"

/* sendfile */
#define PS5SDK_KERNEL_SENDFILE_RUNTIME_VA 0x0000000822191a50ULL
#define KERNEL_sendfile_RUNTIME_VA 0x0000000822191a50ULL
#define KERNEL_sendfile                                 "sendfile"

/* sigwait */
#define PS5SDK_KERNEL_SIGWAIT_RUNTIME_VA 0x0000000822191cd0ULL
#define KERNEL_sigwait_RUNTIME_VA 0x0000000822191cd0ULL
#define KERNEL_sigwait                                  "sigwait"

/* sigqueue */
#define PS5SDK_KERNEL_SIGQUEUE_RUNTIME_VA 0x0000000822191e30ULL
#define KERNEL_sigqueue_RUNTIME_VA 0x0000000822191e30ULL
#define KERNEL_sigqueue                                 "sigqueue"

/* shm_open */
#define PS5SDK_KERNEL_SHM_OPEN_RUNTIME_VA 0x0000000822192050ULL
#define KERNEL_shm_open_RUNTIME_VA 0x0000000822192050ULL
#define KERNEL_shm_open                                 "shm_open"

/* shm_unlink */
#define PS5SDK_KERNEL_SHM_UNLINK_RUNTIME_VA 0x0000000822192070ULL
#define KERNEL_shm_unlink_RUNTIME_VA 0x0000000822192070ULL
#define KERNEL_shm_unlink                               "shm_unlink"

/* fstatat */
#define PS5SDK_KERNEL_FSTATAT_RUNTIME_VA 0x0000000822192170ULL
#define KERNEL_fstatat_RUNTIME_VA 0x0000000822192170ULL
#define KERNEL_fstatat                                  "fstatat"

/* mkdirat */
#define PS5SDK_KERNEL_MKDIRAT_RUNTIME_VA 0x00000008221921d0ULL
#define KERNEL_mkdirat_RUNTIME_VA 0x00000008221921d0ULL
#define KERNEL_mkdirat                                  "mkdirat"

/* renameat */
#define PS5SDK_KERNEL_RENAMEAT_RUNTIME_VA 0x0000000822192250ULL
#define KERNEL_renameat_RUNTIME_VA 0x0000000822192250ULL
#define KERNEL_renameat                                 "renameat"

/* unlinkat */
#define PS5SDK_KERNEL_UNLINKAT_RUNTIME_VA 0x0000000822192290ULL
#define KERNEL_unlinkat_RUNTIME_VA 0x0000000822192290ULL
#define KERNEL_unlinkat                                 "unlinkat"

/* fdatasync */
#define PS5SDK_KERNEL_FDATASYNC_RUNTIME_VA 0x0000000822193150ULL
#define KERNEL_fdatasync_RUNTIME_VA 0x0000000822193150ULL
#define KERNEL_fdatasync                                "fdatasync"

/* pthread_attr_destroy */
#define PS5SDK_KERNEL_PTHREAD_ATTR_DESTROY_RUNTIME_VA 0x00000008221948a0ULL
#define KERNEL_pthread_attr_destroy_RUNTIME_VA 0x00000008221948a0ULL
#define KERNEL_pthread_attr_destroy                     "pthread_attr_destroy"

/* pthread_attr_getstacksize */
#define PS5SDK_KERNEL_PTHREAD_ATTR_GETSTACKSIZE_RUNTIME_VA 0x0000000822194c70ULL
#define KERNEL_pthread_attr_getstacksize_RUNTIME_VA 0x0000000822194c70ULL
#define KERNEL_pthread_attr_getstacksize                "pthread_attr_getstacksize"

/* pthread_attr_init */
#define PS5SDK_KERNEL_PTHREAD_ATTR_INIT_RUNTIME_VA 0x0000000822194ca0ULL
#define KERNEL_pthread_attr_init_RUNTIME_VA 0x0000000822194ca0ULL
#define KERNEL_pthread_attr_init                        "pthread_attr_init"

/* pthread_attr_setstacksize */
#define PS5SDK_KERNEL_PTHREAD_ATTR_SETSTACKSIZE_RUNTIME_VA 0x0000000822194f00ULL
#define KERNEL_pthread_attr_setstacksize_RUNTIME_VA 0x0000000822194f00ULL
#define KERNEL_pthread_attr_setstacksize                "pthread_attr_setstacksize"

/* pthread_cancel */
#define PS5SDK_KERNEL_PTHREAD_CANCEL_RUNTIME_VA 0x0000000822195980ULL
#define KERNEL_pthread_cancel_RUNTIME_VA 0x0000000822195980ULL
#define KERNEL_pthread_cancel                           "pthread_cancel"

/* pthread_cond_init */
#define PS5SDK_KERNEL_PTHREAD_COND_INIT_RUNTIME_VA 0x0000000822195e90ULL
#define KERNEL_pthread_cond_init_RUNTIME_VA 0x0000000822195e90ULL
#define KERNEL_pthread_cond_init                        "pthread_cond_init"

/* pthread_cond_destroy */
#define PS5SDK_KERNEL_PTHREAD_COND_DESTROY_RUNTIME_VA 0x0000000822195f60ULL
#define KERNEL_pthread_cond_destroy_RUNTIME_VA 0x0000000822195f60ULL
#define KERNEL_pthread_cond_destroy                     "pthread_cond_destroy"

/* pthread_cond_wait */
#define PS5SDK_KERNEL_PTHREAD_COND_WAIT_RUNTIME_VA 0x0000000822196950ULL
#define KERNEL_pthread_cond_wait_RUNTIME_VA 0x0000000822196950ULL
#define KERNEL_pthread_cond_wait                        "pthread_cond_wait"

/* pthread_cond_timedwait */
#define PS5SDK_KERNEL_PTHREAD_COND_TIMEDWAIT_RUNTIME_VA 0x0000000822196b20ULL
#define KERNEL_pthread_cond_timedwait_RUNTIME_VA 0x0000000822196b20ULL
#define KERNEL_pthread_cond_timedwait                   "pthread_cond_timedwait"

/* pthread_cond_signal */
#define PS5SDK_KERNEL_PTHREAD_COND_SIGNAL_RUNTIME_VA 0x0000000822196fa0ULL
#define KERNEL_pthread_cond_signal_RUNTIME_VA 0x0000000822196fa0ULL
#define KERNEL_pthread_cond_signal                      "pthread_cond_signal"

/* pthread_cond_broadcast */
#define PS5SDK_KERNEL_PTHREAD_COND_BROADCAST_RUNTIME_VA 0x0000000822197240ULL
#define KERNEL_pthread_cond_broadcast_RUNTIME_VA 0x0000000822197240ULL
#define KERNEL_pthread_cond_broadcast                   "pthread_cond_broadcast"

/* pthread_create */
#define PS5SDK_KERNEL_PTHREAD_CREATE_RUNTIME_VA 0x0000000822197ba0ULL
#define KERNEL_pthread_create_RUNTIME_VA 0x0000000822197ba0ULL
#define KERNEL_pthread_create                           "pthread_create"

/* pthread_detach */
#define PS5SDK_KERNEL_PTHREAD_DETACH_RUNTIME_VA 0x0000000822198370ULL
#define KERNEL_pthread_detach_RUNTIME_VA 0x0000000822198370ULL
#define KERNEL_pthread_detach                           "pthread_detach"

/* pthread_equal */
#define PS5SDK_KERNEL_PTHREAD_EQUAL_RUNTIME_VA 0x0000000822198410ULL
#define KERNEL_pthread_equal_RUNTIME_VA 0x0000000822198410ULL
#define KERNEL_pthread_equal                            "pthread_equal"

/* pthread_exit */
#define PS5SDK_KERNEL_PTHREAD_EXIT_RUNTIME_VA 0x00000008221985f0ULL
#define KERNEL_pthread_exit_RUNTIME_VA 0x00000008221985f0ULL
#define KERNEL_pthread_exit                             "pthread_exit"

/* fork */
#define PS5SDK_KERNEL_FORK_RUNTIME_VA 0x0000000822198bd0ULL
#define KERNEL_fork_RUNTIME_VA 0x0000000822198bd0ULL
#define KERNEL_fork                                     "fork"

/* pthread_getname_np */
#define PS5SDK_KERNEL_PTHREAD_GETNAME_NP_RUNTIME_VA 0x0000000822199400ULL
#define KERNEL_pthread_getname_np_RUNTIME_VA 0x0000000822199400ULL
#define KERNEL_pthread_getname_np                       "pthread_getname_np"

/* pthread_join */
#define PS5SDK_KERNEL_PTHREAD_JOIN_RUNTIME_VA 0x0000000822199710ULL
#define KERNEL_pthread_join_RUNTIME_VA 0x0000000822199710ULL
#define KERNEL_pthread_join                             "pthread_join"

/* pthread_once */
#define PS5SDK_KERNEL_PTHREAD_ONCE_RUNTIME_VA 0x000000082219a490ULL
#define KERNEL_pthread_once_RUNTIME_VA 0x000000082219a490ULL
#define KERNEL_pthread_once                             "pthread_once"

/* pthread_rwlock_destroy */
#define PS5SDK_KERNEL_PTHREAD_RWLOCK_DESTROY_RUNTIME_VA 0x000000082219b170ULL
#define KERNEL_pthread_rwlock_destroy_RUNTIME_VA 0x000000082219b170ULL
#define KERNEL_pthread_rwlock_destroy                   "pthread_rwlock_destroy"

/* pthread_rwlock_init */
#define PS5SDK_KERNEL_PTHREAD_RWLOCK_INIT_RUNTIME_VA 0x000000082219b270ULL
#define KERNEL_pthread_rwlock_init_RUNTIME_VA 0x000000082219b270ULL
#define KERNEL_pthread_rwlock_init                      "pthread_rwlock_init"

/* pthread_rwlock_rdlock */
#define PS5SDK_KERNEL_PTHREAD_RWLOCK_RDLOCK_RUNTIME_VA 0x000000082219b5b0ULL
#define KERNEL_pthread_rwlock_rdlock_RUNTIME_VA 0x000000082219b5b0ULL
#define KERNEL_pthread_rwlock_rdlock                    "pthread_rwlock_rdlock"

/* pthread_rwlock_tryrdlock */
#define PS5SDK_KERNEL_PTHREAD_RWLOCK_TRYRDLOCK_RUNTIME_VA 0x000000082219b8e0ULL
#define KERNEL_pthread_rwlock_tryrdlock_RUNTIME_VA 0x000000082219b8e0ULL
#define KERNEL_pthread_rwlock_tryrdlock                 "pthread_rwlock_tryrdlock"

/* pthread_rwlock_wrlock */
#define PS5SDK_KERNEL_PTHREAD_RWLOCK_WRLOCK_RUNTIME_VA 0x000000082219be90ULL
#define KERNEL_pthread_rwlock_wrlock_RUNTIME_VA 0x000000082219be90ULL
#define KERNEL_pthread_rwlock_wrlock                    "pthread_rwlock_wrlock"

/* pthread_rwlock_unlock */
#define PS5SDK_KERNEL_PTHREAD_RWLOCK_UNLOCK_RUNTIME_VA 0x000000082219c100ULL
#define KERNEL_pthread_rwlock_unlock_RUNTIME_VA 0x000000082219c100ULL
#define KERNEL_pthread_rwlock_unlock                    "pthread_rwlock_unlock"

/* pthread_self */
#define PS5SDK_KERNEL_PTHREAD_SELF_RUNTIME_VA 0x000000082219c450ULL
#define KERNEL_pthread_self_RUNTIME_VA 0x000000082219c450ULL
#define KERNEL_pthread_self                             "pthread_self"

/* pause */
#define PS5SDK_KERNEL_PAUSE_RUNTIME_VA 0x000000082219ce70ULL
#define KERNEL_pause_RUNTIME_VA 0x000000082219ce70ULL
#define KERNEL_pause                                    "pause"

/* sigprocmask */
#define PS5SDK_KERNEL_SIGPROCMASK_RUNTIME_VA 0x000000082219cf70ULL
#define KERNEL_sigprocmask_RUNTIME_VA 0x000000082219cf70ULL
#define KERNEL_sigprocmask                              "sigprocmask"

/* sigsuspend */
#define PS5SDK_KERNEL_SIGSUSPEND_RUNTIME_VA 0x000000082219d000ULL
#define KERNEL_sigsuspend_RUNTIME_VA 0x000000082219d000ULL
#define KERNEL_sigsuspend                               "sigsuspend"

/* raise */
#define PS5SDK_KERNEL_RAISE_RUNTIME_VA 0x000000082219d0e0ULL
#define KERNEL_raise_RUNTIME_VA 0x000000082219d0e0ULL
#define KERNEL_raise                                    "raise"

/* sigaction */
#define PS5SDK_KERNEL_SIGACTION_RUNTIME_VA 0x000000082219d100ULL
#define KERNEL_sigaction_RUNTIME_VA 0x000000082219d100ULL
#define KERNEL_sigaction                                "sigaction"

/* accept */
#define PS5SDK_KERNEL_ACCEPT_RUNTIME_VA 0x000000082219e8a0ULL
#define KERNEL_accept_RUNTIME_VA 0x000000082219e8a0ULL
#define KERNEL_accept                                   "accept"

/* close */
#define PS5SDK_KERNEL_CLOSE_RUNTIME_VA 0x000000082219e950ULL
#define KERNEL_close_RUNTIME_VA 0x000000082219e950ULL
#define KERNEL_close                                    "close"

/* connect */
#define PS5SDK_KERNEL_CONNECT_RUNTIME_VA 0x000000082219ea50ULL
#define KERNEL_connect_RUNTIME_VA 0x000000082219ea50ULL
#define KERNEL_connect                                  "connect"

/* fsync */
#define PS5SDK_KERNEL_FSYNC_RUNTIME_VA 0x000000082219ec50ULL
#define KERNEL_fsync_RUNTIME_VA 0x000000082219ec50ULL
#define KERNEL_fsync                                    "fsync"

/* nanosleep */
#define PS5SDK_KERNEL_NANOSLEEP_RUNTIME_VA 0x000000082219ece0ULL
#define KERNEL_nanosleep_RUNTIME_VA 0x000000082219ece0ULL
#define KERNEL_nanosleep                                "nanosleep"

/* open */
#define PS5SDK_KERNEL_OPEN_RUNTIME_VA 0x000000082219ed30ULL
#define KERNEL_open_RUNTIME_VA 0x000000082219ed30ULL
#define KERNEL_open                                     "open"

/* openat */
#define PS5SDK_KERNEL_OPENAT_RUNTIME_VA 0x000000082219ee70ULL
#define KERNEL_openat_RUNTIME_VA 0x000000082219ee70ULL
#define KERNEL_openat                                   "openat"

/* poll */
#define PS5SDK_KERNEL_POLL_RUNTIME_VA 0x000000082219efb0ULL
#define KERNEL_poll_RUNTIME_VA 0x000000082219efb0ULL
#define KERNEL_poll                                     "poll"

/* read */
#define PS5SDK_KERNEL_READ_RUNTIME_VA 0x000000082219f080ULL
#define KERNEL_read_RUNTIME_VA 0x000000082219f080ULL
#define KERNEL_read                                     "read"

/* recvfrom */
#define PS5SDK_KERNEL_RECVFROM_RUNTIME_VA 0x000000082219f140ULL
#define KERNEL_recvfrom_RUNTIME_VA 0x000000082219f140ULL
#define KERNEL_recvfrom                                 "recvfrom"

/* recvmsg */
#define PS5SDK_KERNEL_RECVMSG_RUNTIME_VA 0x000000082219f1c0ULL
#define KERNEL_recvmsg_RUNTIME_VA 0x000000082219f1c0ULL
#define KERNEL_recvmsg                                  "recvmsg"

/* select */
#define PS5SDK_KERNEL_SELECT_RUNTIME_VA 0x000000082219f220ULL
#define KERNEL_select_RUNTIME_VA 0x000000082219f220ULL
#define KERNEL_select                                   "select"

/* sendmsg */
#define PS5SDK_KERNEL_SENDMSG_RUNTIME_VA 0x000000082219f290ULL
#define KERNEL_sendmsg_RUNTIME_VA 0x000000082219f290ULL
#define KERNEL_sendmsg                                  "sendmsg"

/* sendto */
#define PS5SDK_KERNEL_SENDTO_RUNTIME_VA 0x000000082219f2f0ULL
#define KERNEL_sendto_RUNTIME_VA 0x000000082219f2f0ULL
#define KERNEL_sendto                                   "sendto"

/* sleep */
#define PS5SDK_KERNEL_SLEEP_RUNTIME_VA 0x000000082219f370ULL
#define KERNEL_sleep_RUNTIME_VA 0x000000082219f370ULL
#define KERNEL_sleep                                    "sleep"

/* usleep */
#define PS5SDK_KERNEL_USLEEP_RUNTIME_VA 0x000000082219f430ULL
#define KERNEL_usleep_RUNTIME_VA 0x000000082219f430ULL
#define KERNEL_usleep                                   "usleep"

/* wait */
#define PS5SDK_KERNEL_WAIT_RUNTIME_VA 0x000000082219f470ULL
#define KERNEL_wait_RUNTIME_VA 0x000000082219f470ULL
#define KERNEL_wait                                     "wait"

/* wait4 */
#define PS5SDK_KERNEL_WAIT4_RUNTIME_VA 0x000000082219f510ULL
#define KERNEL_wait4_RUNTIME_VA 0x000000082219f510ULL
#define KERNEL_wait4                                    "wait4"

/* waitpid */
#define PS5SDK_KERNEL_WAITPID_RUNTIME_VA 0x000000082219f570ULL
#define KERNEL_waitpid_RUNTIME_VA 0x000000082219f570ULL
#define KERNEL_waitpid                                  "waitpid"

/* write */
#define PS5SDK_KERNEL_WRITE_RUNTIME_VA 0x000000082219f5c0ULL
#define KERNEL_write_RUNTIME_VA 0x000000082219f5c0ULL
#define KERNEL_write                                    "write"

/* execvp */
#define PS5SDK_KERNEL_EXECVP_RUNTIME_VA 0x00000008221a06b0ULL
#define KERNEL_execvp_RUNTIME_VA 0x00000008221a06b0ULL
#define KERNEL_execvp                                   "execvp"

/* sem_init */
#define PS5SDK_KERNEL_SEM_INIT_RUNTIME_VA 0x00000008221a0c70ULL
#define KERNEL_sem_init_RUNTIME_VA 0x00000008221a0c70ULL
#define KERNEL_sem_init                                 "sem_init"

/* sem_open */
#define PS5SDK_KERNEL_SEM_OPEN_RUNTIME_VA 0x00000008221a0d50ULL
#define KERNEL_sem_open_RUNTIME_VA 0x00000008221a0d50ULL
#define KERNEL_sem_open                                 "sem_open"

/* sem_close */
#define PS5SDK_KERNEL_SEM_CLOSE_RUNTIME_VA 0x00000008221a1210ULL
#define KERNEL_sem_close_RUNTIME_VA 0x00000008221a1210ULL
#define KERNEL_sem_close                                "sem_close"

/* sem_unlink */
#define PS5SDK_KERNEL_SEM_UNLINK_RUNTIME_VA 0x00000008221a12f0ULL
#define KERNEL_sem_unlink_RUNTIME_VA 0x00000008221a12f0ULL
#define KERNEL_sem_unlink                               "sem_unlink"

/* sem_destroy */
#define PS5SDK_KERNEL_SEM_DESTROY_RUNTIME_VA 0x00000008221a13f0ULL
#define KERNEL_sem_destroy_RUNTIME_VA 0x00000008221a13f0ULL
#define KERNEL_sem_destroy                              "sem_destroy"

/* sem_trywait */
#define PS5SDK_KERNEL_SEM_TRYWAIT_RUNTIME_VA 0x00000008221a1470ULL
#define KERNEL_sem_trywait_RUNTIME_VA 0x00000008221a1470ULL
#define KERNEL_sem_trywait                              "sem_trywait"

/* sem_timedwait */
#define PS5SDK_KERNEL_SEM_TIMEDWAIT_RUNTIME_VA 0x00000008221a14c0ULL
#define KERNEL_sem_timedwait_RUNTIME_VA 0x00000008221a14c0ULL
#define KERNEL_sem_timedwait                            "sem_timedwait"

/* sem_wait */
#define PS5SDK_KERNEL_SEM_WAIT_RUNTIME_VA 0x00000008221a19d0ULL
#define KERNEL_sem_wait_RUNTIME_VA 0x00000008221a19d0ULL
#define KERNEL_sem_wait                                 "sem_wait"

/* sem_post */
#define PS5SDK_KERNEL_SEM_POST_RUNTIME_VA 0x00000008221a19e0ULL
#define KERNEL_sem_post_RUNTIME_VA 0x00000008221a19e0ULL
#define KERNEL_sem_post                                 "sem_post"

/* signal */
#define PS5SDK_KERNEL_SIGNAL_RUNTIME_VA 0x00000008221a1af0ULL
#define KERNEL_signal_RUNTIME_VA 0x00000008221a1af0ULL
#define KERNEL_signal                                   "signal"

/* sysconf */
#define PS5SDK_KERNEL_SYSCONF_RUNTIME_VA 0x00000008221a1d10ULL
#define KERNEL_sysconf_RUNTIME_VA 0x00000008221a1d10ULL
#define KERNEL_sysconf                                  "sysconf"

/* sysctl */
#define PS5SDK_KERNEL_SYSCTL_RUNTIME_VA 0x00000008221a2150ULL
#define KERNEL_sysctl_RUNTIME_VA 0x00000008221a2150ULL
#define KERNEL_sysctl                                   "sysctl"

/* sysctlbyname */
#define PS5SDK_KERNEL_SYSCTLBYNAME_RUNTIME_VA 0x00000008221a2260ULL
#define KERNEL_sysctlbyname_RUNTIME_VA 0x00000008221a2260ULL
#define KERNEL_sysctlbyname                             "sysctlbyname"

/* tcgetattr */
#define PS5SDK_KERNEL_TCGETATTR_RUNTIME_VA 0x00000008221a2380ULL
#define KERNEL_tcgetattr_RUNTIME_VA 0x00000008221a2380ULL
#define KERNEL_tcgetattr                                "tcgetattr"

/* tcsetattr */
#define PS5SDK_KERNEL_TCSETATTR_RUNTIME_VA 0x00000008221a2390ULL
#define KERNEL_tcsetattr_RUNTIME_VA 0x00000008221a2390ULL
#define KERNEL_tcsetattr                                "tcsetattr"

/* inet_ntop */
#define PS5SDK_KERNEL_INET_NTOP_RUNTIME_VA 0x00000008221a28a0ULL
#define KERNEL_inet_ntop_RUNTIME_VA 0x00000008221a28a0ULL
#define KERNEL_inet_ntop                                "inet_ntop"

/* inet_pton */
#define PS5SDK_KERNEL_INET_PTON_RUNTIME_VA 0x00000008221a2e00ULL
#define KERNEL_inet_pton_RUNTIME_VA 0x00000008221a2e00ULL
#define KERNEL_inet_pton                                "inet_pton"

/* htonl */
#define PS5SDK_KERNEL_HTONL_RUNTIME_VA 0x00000008221a33d0ULL
#define KERNEL_htonl_RUNTIME_VA 0x00000008221a33d0ULL
#define KERNEL_htonl                                    "htonl"

/* htons */
#define PS5SDK_KERNEL_HTONS_RUNTIME_VA 0x00000008221a33e0ULL
#define KERNEL_htons_RUNTIME_VA 0x00000008221a33e0ULL
#define KERNEL_htons                                    "htons"

/* ntohl */
#define PS5SDK_KERNEL_NTOHL_RUNTIME_VA 0x00000008221a33f0ULL
#define KERNEL_ntohl_RUNTIME_VA 0x00000008221a33f0ULL
#define KERNEL_ntohl                                    "ntohl"

/* ntohs */
#define PS5SDK_KERNEL_NTOHS_RUNTIME_VA 0x00000008221a3400ULL
#define KERNEL_ntohs_RUNTIME_VA 0x00000008221a3400ULL
#define KERNEL_ntohs                                    "ntohs"

/* recv */
#define PS5SDK_KERNEL_RECV_RUNTIME_VA 0x00000008221a3410ULL
#define KERNEL_recv_RUNTIME_VA 0x00000008221a3410ULL
#define KERNEL_recv                                     "recv"

/* send */
#define PS5SDK_KERNEL_SEND_RUNTIME_VA 0x00000008221a3420ULL
#define KERNEL_send_RUNTIME_VA 0x00000008221a3420ULL
#define KERNEL_send                                     "send"

/* ftruncate */
#define PS5SDK_KERNEL_FTRUNCATE_RUNTIME_VA 0x00000008221a35f0ULL
#define KERNEL_ftruncate_RUNTIME_VA 0x00000008221a35f0ULL
#define KERNEL_ftruncate                                "ftruncate"

/* lseek */
#define PS5SDK_KERNEL_LSEEK_RUNTIME_VA 0x00000008221a3600ULL
#define KERNEL_lseek_RUNTIME_VA 0x00000008221a3600ULL
#define KERNEL_lseek                                    "lseek"

/* mmap */
#define PS5SDK_KERNEL_MMAP_RUNTIME_VA 0x00000008221a3610ULL
#define KERNEL_mmap_RUNTIME_VA 0x00000008221a3610ULL
#define KERNEL_mmap                                     "mmap"

/* pread */
#define PS5SDK_KERNEL_PREAD_RUNTIME_VA 0x00000008221a3620ULL
#define KERNEL_pread_RUNTIME_VA 0x00000008221a3620ULL
#define KERNEL_pread                                    "pread"

/* pwrite */
#define PS5SDK_KERNEL_PWRITE_RUNTIME_VA 0x00000008221a3630ULL
#define KERNEL_pwrite_RUNTIME_VA 0x00000008221a3630ULL
#define KERNEL_pwrite                                   "pwrite"

/* truncate */
#define PS5SDK_KERNEL_TRUNCATE_RUNTIME_VA 0x00000008221a3650ULL
#define KERNEL_truncate_RUNTIME_VA 0x00000008221a3650ULL
#define KERNEL_truncate                                 "truncate"

/* scePthreadAttrDestroy */
#define PS5SDK_KERNEL_SCEPTHREADATTRDESTROY_RUNTIME_VA 0x00000008221a4710ULL
#define KERNEL_scePthreadAttrDestroy_RUNTIME_VA 0x00000008221a4710ULL
#define KERNEL_scePthreadAttrDestroy                    "scePthreadAttrDestroy"

/* scePthreadAttrGetstacksize */
#define PS5SDK_KERNEL_SCEPTHREADATTRGETSTACKSIZE_RUNTIME_VA 0x00000008221a4750ULL
#define KERNEL_scePthreadAttrGetstacksize_RUNTIME_VA 0x00000008221a4750ULL
#define KERNEL_scePthreadAttrGetstacksize               "scePthreadAttrGetstacksize"

/* scePthreadAttrGetdetachstate */
#define PS5SDK_KERNEL_SCEPTHREADATTRGETDETACHSTATE_RUNTIME_VA 0x00000008221a47b0ULL
#define KERNEL_scePthreadAttrGetdetachstate_RUNTIME_VA 0x00000008221a47b0ULL
#define KERNEL_scePthreadAttrGetdetachstate             "scePthreadAttrGetdetachstate"

/* scePthreadAttrInit */
#define PS5SDK_KERNEL_SCEPTHREADATTRINIT_RUNTIME_VA 0x00000008221a47d0ULL
#define KERNEL_scePthreadAttrInit_RUNTIME_VA 0x00000008221a47d0ULL
#define KERNEL_scePthreadAttrInit                       "scePthreadAttrInit"

/* scePthreadAttrSetstacksize */
#define PS5SDK_KERNEL_SCEPTHREADATTRSETSTACKSIZE_RUNTIME_VA 0x00000008221a47f0ULL
#define KERNEL_scePthreadAttrSetstacksize_RUNTIME_VA 0x00000008221a47f0ULL
#define KERNEL_scePthreadAttrSetstacksize               "scePthreadAttrSetstacksize"

/* scePthreadAttrSetdetachstate */
#define PS5SDK_KERNEL_SCEPTHREADATTRSETDETACHSTATE_RUNTIME_VA 0x00000008221a4870ULL
#define KERNEL_scePthreadAttrSetdetachstate_RUNTIME_VA 0x00000008221a4870ULL
#define KERNEL_scePthreadAttrSetdetachstate             "scePthreadAttrSetdetachstate"

/* scePthreadCondBroadcast */
#define PS5SDK_KERNEL_SCEPTHREADCONDBROADCAST_RUNTIME_VA 0x00000008221a4ab0ULL
#define KERNEL_scePthreadCondBroadcast_RUNTIME_VA 0x00000008221a4ab0ULL
#define KERNEL_scePthreadCondBroadcast                  "scePthreadCondBroadcast"

/* scePthreadCondInit */
#define PS5SDK_KERNEL_SCEPTHREADCONDINIT_RUNTIME_VA 0x00000008221a4ad0ULL
#define KERNEL_scePthreadCondInit_RUNTIME_VA 0x00000008221a4ad0ULL
#define KERNEL_scePthreadCondInit                       "scePthreadCondInit"

/* scePthreadCondDestroy */
#define PS5SDK_KERNEL_SCEPTHREADCONDDESTROY_RUNTIME_VA 0x00000008221a4b70ULL
#define KERNEL_scePthreadCondDestroy_RUNTIME_VA 0x00000008221a4b70ULL
#define KERNEL_scePthreadCondDestroy                    "scePthreadCondDestroy"

/* scePthreadCondSignal */
#define PS5SDK_KERNEL_SCEPTHREADCONDSIGNAL_RUNTIME_VA 0x00000008221a4b90ULL
#define KERNEL_scePthreadCondSignal_RUNTIME_VA 0x00000008221a4b90ULL
#define KERNEL_scePthreadCondSignal                     "scePthreadCondSignal"

/* scePthreadCondTimedwait */
#define PS5SDK_KERNEL_SCEPTHREADCONDTIMEDWAIT_RUNTIME_VA 0x00000008221a4bd0ULL
#define KERNEL_scePthreadCondTimedwait_RUNTIME_VA 0x00000008221a4bd0ULL
#define KERNEL_scePthreadCondTimedwait                  "scePthreadCondTimedwait"

/* scePthreadCondWait */
#define PS5SDK_KERNEL_SCEPTHREADCONDWAIT_RUNTIME_VA 0x00000008221a4bf0ULL
#define KERNEL_scePthreadCondWait_RUNTIME_VA 0x00000008221a4bf0ULL
#define KERNEL_scePthreadCondWait                       "scePthreadCondWait"

/* scePthreadCreate */
#define PS5SDK_KERNEL_SCEPTHREADCREATE_RUNTIME_VA 0x00000008221a4c10ULL
#define KERNEL_scePthreadCreate_RUNTIME_VA 0x00000008221a4c10ULL
#define KERNEL_scePthreadCreate                         "scePthreadCreate"

/* scePthreadDetach */
#define PS5SDK_KERNEL_SCEPTHREADDETACH_RUNTIME_VA 0x00000008221a4c30ULL
#define KERNEL_scePthreadDetach_RUNTIME_VA 0x00000008221a4c30ULL
#define KERNEL_scePthreadDetach                         "scePthreadDetach"

/* scePthreadEqual */
#define PS5SDK_KERNEL_SCEPTHREADEQUAL_RUNTIME_VA 0x00000008221a4c50ULL
#define KERNEL_scePthreadEqual_RUNTIME_VA 0x00000008221a4c50ULL
#define KERNEL_scePthreadEqual                          "scePthreadEqual"

/* scePthreadExit */
#define PS5SDK_KERNEL_SCEPTHREADEXIT_RUNTIME_VA 0x00000008221a4c70ULL
#define KERNEL_scePthreadExit_RUNTIME_VA 0x00000008221a4c70ULL
#define KERNEL_scePthreadExit                           "scePthreadExit"

/* scePthreadGetspecific */
#define PS5SDK_KERNEL_SCEPTHREADGETSPECIFIC_RUNTIME_VA 0x00000008221a4c80ULL
#define KERNEL_scePthreadGetspecific_RUNTIME_VA 0x00000008221a4c80ULL
#define KERNEL_scePthreadGetspecific                    "scePthreadGetspecific"

/* scePthreadJoin */
#define PS5SDK_KERNEL_SCEPTHREADJOIN_RUNTIME_VA 0x00000008221a4cb0ULL
#define KERNEL_scePthreadJoin_RUNTIME_VA 0x00000008221a4cb0ULL
#define KERNEL_scePthreadJoin                           "scePthreadJoin"

/* scePthreadKeyCreate */
#define PS5SDK_KERNEL_SCEPTHREADKEYCREATE_RUNTIME_VA 0x00000008221a4cd0ULL
#define KERNEL_scePthreadKeyCreate_RUNTIME_VA 0x00000008221a4cd0ULL
#define KERNEL_scePthreadKeyCreate                      "scePthreadKeyCreate"

/* scePthreadKeyDelete */
#define PS5SDK_KERNEL_SCEPTHREADKEYDELETE_RUNTIME_VA 0x00000008221a4cf0ULL
#define KERNEL_scePthreadKeyDelete_RUNTIME_VA 0x00000008221a4cf0ULL
#define KERNEL_scePthreadKeyDelete                      "scePthreadKeyDelete"

/* scePthreadMutexInit */
#define PS5SDK_KERNEL_SCEPTHREADMUTEXINIT_RUNTIME_VA 0x00000008221a4e10ULL
#define KERNEL_scePthreadMutexInit_RUNTIME_VA 0x00000008221a4e10ULL
#define KERNEL_scePthreadMutexInit                      "scePthreadMutexInit"

/* scePthreadMutexDestroy */
#define PS5SDK_KERNEL_SCEPTHREADMUTEXDESTROY_RUNTIME_VA 0x00000008221a4fd0ULL
#define KERNEL_scePthreadMutexDestroy_RUNTIME_VA 0x00000008221a4fd0ULL
#define KERNEL_scePthreadMutexDestroy                   "scePthreadMutexDestroy"

/* scePthreadMutexLock */
#define PS5SDK_KERNEL_SCEPTHREADMUTEXLOCK_RUNTIME_VA 0x00000008221a4ff0ULL
#define KERNEL_scePthreadMutexLock_RUNTIME_VA 0x00000008221a4ff0ULL
#define KERNEL_scePthreadMutexLock                      "scePthreadMutexLock"

/* scePthreadMutexTrylock */
#define PS5SDK_KERNEL_SCEPTHREADMUTEXTRYLOCK_RUNTIME_VA 0x00000008221a5010ULL
#define KERNEL_scePthreadMutexTrylock_RUNTIME_VA 0x00000008221a5010ULL
#define KERNEL_scePthreadMutexTrylock                   "scePthreadMutexTrylock"

/* scePthreadMutexTimedlock */
#define PS5SDK_KERNEL_SCEPTHREADMUTEXTIMEDLOCK_RUNTIME_VA 0x00000008221a5030ULL
#define KERNEL_scePthreadMutexTimedlock_RUNTIME_VA 0x00000008221a5030ULL
#define KERNEL_scePthreadMutexTimedlock                 "scePthreadMutexTimedlock"

/* scePthreadMutexUnlock */
#define PS5SDK_KERNEL_SCEPTHREADMUTEXUNLOCK_RUNTIME_VA 0x00000008221a5050ULL
#define KERNEL_scePthreadMutexUnlock_RUNTIME_VA 0x00000008221a5050ULL
#define KERNEL_scePthreadMutexUnlock                    "scePthreadMutexUnlock"

/* scePthreadRwlockInit */
#define PS5SDK_KERNEL_SCEPTHREADRWLOCKINIT_RUNTIME_VA 0x00000008221a5090ULL
#define KERNEL_scePthreadRwlockInit_RUNTIME_VA 0x00000008221a5090ULL
#define KERNEL_scePthreadRwlockInit                     "scePthreadRwlockInit"

/* scePthreadRwlockDestroy */
#define PS5SDK_KERNEL_SCEPTHREADRWLOCKDESTROY_RUNTIME_VA 0x00000008221a5130ULL
#define KERNEL_scePthreadRwlockDestroy_RUNTIME_VA 0x00000008221a5130ULL
#define KERNEL_scePthreadRwlockDestroy                  "scePthreadRwlockDestroy"

/* scePthreadRwlockRdlock */
#define PS5SDK_KERNEL_SCEPTHREADRWLOCKRDLOCK_RUNTIME_VA 0x00000008221a5150ULL
#define KERNEL_scePthreadRwlockRdlock_RUNTIME_VA 0x00000008221a5150ULL
#define KERNEL_scePthreadRwlockRdlock                   "scePthreadRwlockRdlock"

/* scePthreadRwlockUnlock */
#define PS5SDK_KERNEL_SCEPTHREADRWLOCKUNLOCK_RUNTIME_VA 0x00000008221a51f0ULL
#define KERNEL_scePthreadRwlockUnlock_RUNTIME_VA 0x00000008221a51f0ULL
#define KERNEL_scePthreadRwlockUnlock                   "scePthreadRwlockUnlock"

/* scePthreadRwlockWrlock */
#define PS5SDK_KERNEL_SCEPTHREADRWLOCKWRLOCK_RUNTIME_VA 0x00000008221a5210ULL
#define KERNEL_scePthreadRwlockWrlock_RUNTIME_VA 0x00000008221a5210ULL
#define KERNEL_scePthreadRwlockWrlock                   "scePthreadRwlockWrlock"

/* scePthreadSelf */
#define PS5SDK_KERNEL_SCEPTHREADSELF_RUNTIME_VA 0x00000008221a54a0ULL
#define KERNEL_scePthreadSelf_RUNTIME_VA 0x00000008221a54a0ULL
#define KERNEL_scePthreadSelf                           "scePthreadSelf"

/* scePthreadCancel */
#define PS5SDK_KERNEL_SCEPTHREADCANCEL_RUNTIME_VA 0x00000008221a54b0ULL
#define KERNEL_scePthreadCancel_RUNTIME_VA 0x00000008221a54b0ULL
#define KERNEL_scePthreadCancel                         "scePthreadCancel"

/* scePthreadSetprio */
#define PS5SDK_KERNEL_SCEPTHREADSETPRIO_RUNTIME_VA 0x00000008221a55a0ULL
#define KERNEL_scePthreadSetprio_RUNTIME_VA 0x00000008221a55a0ULL
#define KERNEL_scePthreadSetprio                        "scePthreadSetprio"

/* scePthreadYield */
#define PS5SDK_KERNEL_SCEPTHREADYIELD_RUNTIME_VA 0x00000008221a55c0ULL
#define KERNEL_scePthreadYield_RUNTIME_VA 0x00000008221a55c0ULL
#define KERNEL_scePthreadYield                          "scePthreadYield"

/* scePthreadAttrGetschedparam */
#define PS5SDK_KERNEL_SCEPTHREADATTRGETSCHEDPARAM_RUNTIME_VA 0x00000008221a56b0ULL
#define KERNEL_scePthreadAttrGetschedparam_RUNTIME_VA 0x00000008221a56b0ULL
#define KERNEL_scePthreadAttrGetschedparam              "scePthreadAttrGetschedparam"

/* scePthreadAttrSetschedparam */
#define PS5SDK_KERNEL_SCEPTHREADATTRSETSCHEDPARAM_RUNTIME_VA 0x00000008221a5750ULL
#define KERNEL_scePthreadAttrSetschedparam_RUNTIME_VA 0x00000008221a5750ULL
#define KERNEL_scePthreadAttrSetschedparam              "scePthreadAttrSetschedparam"

/* scePthreadGetaffinity */
#define PS5SDK_KERNEL_SCEPTHREADGETAFFINITY_RUNTIME_VA 0x00000008221a5930ULL
#define KERNEL_scePthreadGetaffinity_RUNTIME_VA 0x00000008221a5930ULL
#define KERNEL_scePthreadGetaffinity                    "scePthreadGetaffinity"

/* scePthreadSetaffinity */
#define PS5SDK_KERNEL_SCEPTHREADSETAFFINITY_RUNTIME_VA 0x00000008221a5990ULL
#define KERNEL_scePthreadSetaffinity_RUNTIME_VA 0x00000008221a5990ULL
#define KERNEL_scePthreadSetaffinity                    "scePthreadSetaffinity"

/* scePthreadGetthreadid */
#define PS5SDK_KERNEL_SCEPTHREADGETTHREADID_RUNTIME_VA 0x00000008221a5ab0ULL
#define KERNEL_scePthreadGetthreadid_RUNTIME_VA 0x00000008221a5ab0ULL
#define KERNEL_scePthreadGetthreadid                    "scePthreadGetthreadid"

/* sceKernelMprotect */
#define PS5SDK_KERNEL_SCEKERNELMPROTECT_RUNTIME_VA 0x00000008221a5d40ULL
#define KERNEL_sceKernelMprotect_RUNTIME_VA 0x00000008221a5d40ULL
#define KERNEL_sceKernelMprotect                        "sceKernelMprotect"

/* sceKernelMunmap */
#define PS5SDK_KERNEL_SCEKERNELMUNMAP_RUNTIME_VA 0x00000008221a5dd0ULL
#define KERNEL_sceKernelMunmap_RUNTIME_VA 0x00000008221a5dd0ULL
#define KERNEL_sceKernelMunmap                          "sceKernelMunmap"

/* sceKernelSleep */
#define PS5SDK_KERNEL_SCEKERNELSLEEP_RUNTIME_VA 0x00000008221a5e90ULL
#define KERNEL_sceKernelSleep_RUNTIME_VA 0x00000008221a5e90ULL
#define KERNEL_sceKernelSleep                           "sceKernelSleep"

/* sceKernelNanosleep */
#define PS5SDK_KERNEL_SCEKERNELNANOSLEEP_RUNTIME_VA 0x00000008221a5f60ULL
#define KERNEL_sceKernelNanosleep_RUNTIME_VA 0x00000008221a5f60ULL
#define KERNEL_sceKernelNanosleep                       "sceKernelNanosleep"

/* sceKernelUsleep */
#define PS5SDK_KERNEL_SCEKERNELUSLEEP_RUNTIME_VA 0x00000008221a6000ULL
#define KERNEL_sceKernelUsleep_RUNTIME_VA 0x00000008221a6000ULL
#define KERNEL_sceKernelUsleep                          "sceKernelUsleep"

/* sceKernelGetProcessTime */
#define PS5SDK_KERNEL_SCEKERNELGETPROCESSTIME_RUNTIME_VA 0x00000008221a6170ULL
#define KERNEL_sceKernelGetProcessTime_RUNTIME_VA 0x00000008221a6170ULL
#define KERNEL_sceKernelGetProcessTime                  "sceKernelGetProcessTime"

/* sceKernelGetCurrentCpu */
#define PS5SDK_KERNEL_SCEKERNELGETCURRENTCPU_RUNTIME_VA 0x00000008221a6320ULL
#define KERNEL_sceKernelGetCurrentCpu_RUNTIME_VA 0x00000008221a6320ULL
#define KERNEL_sceKernelGetCurrentCpu                   "sceKernelGetCurrentCpu"

/* sceKernelGetThreadName */
#define PS5SDK_KERNEL_SCEKERNELGETTHREADNAME_RUNTIME_VA 0x00000008221a6530ULL
#define KERNEL_sceKernelGetThreadName_RUNTIME_VA 0x00000008221a6530ULL
#define KERNEL_sceKernelGetThreadName                   "sceKernelGetThreadName"

/* sceKernelGetCpumode */
#define PS5SDK_KERNEL_SCEKERNELGETCPUMODE_RUNTIME_VA 0x00000008221a6ab0ULL
#define KERNEL_sceKernelGetCpumode_RUNTIME_VA 0x00000008221a6ab0ULL
#define KERNEL_sceKernelGetCpumode                      "sceKernelGetCpumode"

/* sceKernelRead */
#define PS5SDK_KERNEL_SCEKERNELREAD_RUNTIME_VA 0x00000008221a6de0ULL
#define KERNEL_sceKernelRead_RUNTIME_VA 0x00000008221a6de0ULL
#define KERNEL_sceKernelRead                            "sceKernelRead"

/* sceKernelWrite */
#define PS5SDK_KERNEL_SCEKERNELWRITE_RUNTIME_VA 0x00000008221a6e10ULL
#define KERNEL_sceKernelWrite_RUNTIME_VA 0x00000008221a6e10ULL
#define KERNEL_sceKernelWrite                           "sceKernelWrite"

/* sceKernelOpen */
#define PS5SDK_KERNEL_SCEKERNELOPEN_RUNTIME_VA 0x00000008221a6e40ULL
#define KERNEL_sceKernelOpen_RUNTIME_VA 0x00000008221a6e40ULL
#define KERNEL_sceKernelOpen                            "sceKernelOpen"

/* sceKernelClose */
#define PS5SDK_KERNEL_SCEKERNELCLOSE_RUNTIME_VA 0x00000008221a6ea0ULL
#define KERNEL_sceKernelClose_RUNTIME_VA 0x00000008221a6ea0ULL
#define KERNEL_sceKernelClose                           "sceKernelClose"

/* sceKernelUnlink */
#define PS5SDK_KERNEL_SCEKERNELUNLINK_RUNTIME_VA 0x00000008221a6ed0ULL
#define KERNEL_sceKernelUnlink_RUNTIME_VA 0x00000008221a6ed0ULL
#define KERNEL_sceKernelUnlink                          "sceKernelUnlink"

/* sceKernelChmod */
#define PS5SDK_KERNEL_SCEKERNELCHMOD_RUNTIME_VA 0x00000008221a6f00ULL
#define KERNEL_sceKernelChmod_RUNTIME_VA 0x00000008221a6f00ULL
#define KERNEL_sceKernelChmod                           "sceKernelChmod"

/* sceKernelFsync */
#define PS5SDK_KERNEL_SCEKERNELFSYNC_RUNTIME_VA 0x00000008221a6f40ULL
#define KERNEL_sceKernelFsync_RUNTIME_VA 0x00000008221a6f40ULL
#define KERNEL_sceKernelFsync                           "sceKernelFsync"

/* sceKernelFcntl */
#define PS5SDK_KERNEL_SCEKERNELFCNTL_RUNTIME_VA 0x00000008221a7780ULL
#define KERNEL_sceKernelFcntl_RUNTIME_VA 0x00000008221a7780ULL
#define KERNEL_sceKernelFcntl                           "sceKernelFcntl"

/* sceKernelRename */
#define PS5SDK_KERNEL_SCEKERNELRENAME_RUNTIME_VA 0x00000008221a7900ULL
#define KERNEL_sceKernelRename_RUNTIME_VA 0x00000008221a7900ULL
#define KERNEL_sceKernelRename                          "sceKernelRename"

/* sceKernelMkdir */
#define PS5SDK_KERNEL_SCEKERNELMKDIR_RUNTIME_VA 0x00000008221a7960ULL
#define KERNEL_sceKernelMkdir_RUNTIME_VA 0x00000008221a7960ULL
#define KERNEL_sceKernelMkdir                           "sceKernelMkdir"

/* sceKernelRmdir */
#define PS5SDK_KERNEL_SCEKERNELRMDIR_RUNTIME_VA 0x00000008221a7990ULL
#define KERNEL_sceKernelRmdir_RUNTIME_VA 0x00000008221a7990ULL
#define KERNEL_sceKernelRmdir                           "sceKernelRmdir"

/* sceKernelStat */
#define PS5SDK_KERNEL_SCEKERNELSTAT_RUNTIME_VA 0x00000008221a79f0ULL
#define KERNEL_sceKernelStat_RUNTIME_VA 0x00000008221a79f0ULL
#define KERNEL_sceKernelStat                            "sceKernelStat"

/* sceKernelFstat */
#define PS5SDK_KERNEL_SCEKERNELFSTAT_RUNTIME_VA 0x00000008221a7a20ULL
#define KERNEL_sceKernelFstat_RUNTIME_VA 0x00000008221a7a20ULL
#define KERNEL_sceKernelFstat                           "sceKernelFstat"

/* sceKernelGetdents */
#define PS5SDK_KERNEL_SCEKERNELGETDENTS_RUNTIME_VA 0x00000008221a7ab0ULL
#define KERNEL_sceKernelGetdents_RUNTIME_VA 0x00000008221a7ab0ULL
#define KERNEL_sceKernelGetdents                        "sceKernelGetdents"

/* sceKernelPread */
#define PS5SDK_KERNEL_SCEKERNELPREAD_RUNTIME_VA 0x00000008221a7b40ULL
#define KERNEL_sceKernelPread_RUNTIME_VA 0x00000008221a7b40ULL
#define KERNEL_sceKernelPread                           "sceKernelPread"

/* sceKernelPwrite */
#define PS5SDK_KERNEL_SCEKERNELPWRITE_RUNTIME_VA 0x00000008221a7b70ULL
#define KERNEL_sceKernelPwrite_RUNTIME_VA 0x00000008221a7b70ULL
#define KERNEL_sceKernelPwrite                          "sceKernelPwrite"

/* sceKernelMmap */
#define PS5SDK_KERNEL_SCEKERNELMMAP_RUNTIME_VA 0x00000008221a7ba0ULL
#define KERNEL_sceKernelMmap_RUNTIME_VA 0x00000008221a7ba0ULL
#define KERNEL_sceKernelMmap                            "sceKernelMmap"

/* sceKernelLseek */
#define PS5SDK_KERNEL_SCEKERNELLSEEK_RUNTIME_VA 0x00000008221a7bd0ULL
#define KERNEL_sceKernelLseek_RUNTIME_VA 0x00000008221a7bd0ULL
#define KERNEL_sceKernelLseek                           "sceKernelLseek"

/* sceKernelTruncate */
#define PS5SDK_KERNEL_SCEKERNELTRUNCATE_RUNTIME_VA 0x00000008221a7c00ULL
#define KERNEL_sceKernelTruncate_RUNTIME_VA 0x00000008221a7c00ULL
#define KERNEL_sceKernelTruncate                        "sceKernelTruncate"

/* sceKernelFtruncate */
#define PS5SDK_KERNEL_SCEKERNELFTRUNCATE_RUNTIME_VA 0x00000008221a7c30ULL
#define KERNEL_sceKernelFtruncate_RUNTIME_VA 0x00000008221a7c30ULL
#define KERNEL_sceKernelFtruncate                       "sceKernelFtruncate"

/* sceKernelCheckReachability */
#define PS5SDK_KERNEL_SCEKERNELCHECKREACHABILITY_RUNTIME_VA 0x00000008221a7c60ULL
#define KERNEL_sceKernelCheckReachability_RUNTIME_VA 0x00000008221a7c60ULL
#define KERNEL_sceKernelCheckReachability               "sceKernelCheckReachability"

/* sceKernelGetDirectMemorySize */
#define PS5SDK_KERNEL_SCEKERNELGETDIRECTMEMORYSIZE_RUNTIME_VA 0x00000008221a85c0ULL
#define KERNEL_sceKernelGetDirectMemorySize_RUNTIME_VA 0x00000008221a85c0ULL
#define KERNEL_sceKernelGetDirectMemorySize             "sceKernelGetDirectMemorySize"

/* sceKernelMapFlexibleMemory */
#define PS5SDK_KERNEL_SCEKERNELMAPFLEXIBLEMEMORY_RUNTIME_VA 0x00000008221a8900ULL
#define KERNEL_sceKernelMapFlexibleMemory_RUNTIME_VA 0x00000008221a8900ULL
#define KERNEL_sceKernelMapFlexibleMemory               "sceKernelMapFlexibleMemory"

/* sceKernelAllocateDirectMemory */
#define PS5SDK_KERNEL_SCEKERNELALLOCATEDIRECTMEMORY_RUNTIME_VA 0x00000008221a8ca0ULL
#define KERNEL_sceKernelAllocateDirectMemory_RUNTIME_VA 0x00000008221a8ca0ULL
#define KERNEL_sceKernelAllocateDirectMemory            "sceKernelAllocateDirectMemory"

/* sceKernelAllocateMainDirectMemory */
#define PS5SDK_KERNEL_SCEKERNELALLOCATEMAINDIRECTMEMORY_RUNTIME_VA 0x00000008221a8e20ULL
#define KERNEL_sceKernelAllocateMainDirectMemory_RUNTIME_VA 0x00000008221a8e20ULL
#define KERNEL_sceKernelAllocateMainDirectMemory        "sceKernelAllocateMainDirectMemory"

/* sceKernelReleaseDirectMemory */
#define PS5SDK_KERNEL_SCEKERNELRELEASEDIRECTMEMORY_RUNTIME_VA 0x00000008221a9110ULL
#define KERNEL_sceKernelReleaseDirectMemory_RUNTIME_VA 0x00000008221a9110ULL
#define KERNEL_sceKernelReleaseDirectMemory             "sceKernelReleaseDirectMemory"

/* sceKernelMapDirectMemory */
#define PS5SDK_KERNEL_SCEKERNELMAPDIRECTMEMORY_RUNTIME_VA 0x00000008221a9240ULL
#define KERNEL_sceKernelMapDirectMemory_RUNTIME_VA 0x00000008221a9240ULL
#define KERNEL_sceKernelMapDirectMemory                 "sceKernelMapDirectMemory"

/* sceKernelGetDirectMemoryType */
#define PS5SDK_KERNEL_SCEKERNELGETDIRECTMEMORYTYPE_RUNTIME_VA 0x00000008221a9560ULL
#define KERNEL_sceKernelGetDirectMemoryType_RUNTIME_VA 0x00000008221a9560ULL
#define KERNEL_sceKernelGetDirectMemoryType             "sceKernelGetDirectMemoryType"

/* sceKernelBatchMap */
#define PS5SDK_KERNEL_SCEKERNELBATCHMAP_RUNTIME_VA 0x00000008221a9950ULL
#define KERNEL_sceKernelBatchMap_RUNTIME_VA 0x00000008221a9950ULL
#define KERNEL_sceKernelBatchMap                        "sceKernelBatchMap"

/* sceKernelVirtualQuery */
#define PS5SDK_KERNEL_SCEKERNELVIRTUALQUERY_RUNTIME_VA 0x00000008221a9ae0ULL
#define KERNEL_sceKernelVirtualQuery_RUNTIME_VA 0x00000008221a9ae0ULL
#define KERNEL_sceKernelVirtualQuery                    "sceKernelVirtualQuery"

/* sceKernelSetVirtualRangeName */
#define PS5SDK_KERNEL_SCEKERNELSETVIRTUALRANGENAME_RUNTIME_VA 0x00000008221a9b50ULL
#define KERNEL_sceKernelSetVirtualRangeName_RUNTIME_VA 0x00000008221a9b50ULL
#define KERNEL_sceKernelSetVirtualRangeName             "sceKernelSetVirtualRangeName"

/* sceKernelMapNamedDirectMemory */
#define PS5SDK_KERNEL_SCEKERNELMAPNAMEDDIRECTMEMORY_RUNTIME_VA 0x00000008221a9f90ULL
#define KERNEL_sceKernelMapNamedDirectMemory_RUNTIME_VA 0x00000008221a9f90ULL
#define KERNEL_sceKernelMapNamedDirectMemory            "sceKernelMapNamedDirectMemory"

/* sceKernelGetTscFrequency */
#define PS5SDK_KERNEL_SCEKERNELGETTSCFREQUENCY_RUNTIME_VA 0x00000008221acf40ULL
#define KERNEL_sceKernelGetTscFrequency_RUNTIME_VA 0x00000008221acf40ULL
#define KERNEL_sceKernelGetTscFrequency                 "sceKernelGetTscFrequency"

/* sceKernelReadTsc */
#define PS5SDK_KERNEL_SCEKERNELREADTSC_RUNTIME_VA 0x00000008221acfb0ULL
#define KERNEL_sceKernelReadTsc_RUNTIME_VA 0x00000008221acfb0ULL
#define KERNEL_sceKernelReadTsc                         "sceKernelReadTsc"

/* sceKernelGetProcessTimeCounter */
#define PS5SDK_KERNEL_SCEKERNELGETPROCESSTIMECOUNTER_RUNTIME_VA 0x00000008221ad020ULL
#define KERNEL_sceKernelGetProcessTimeCounter_RUNTIME_VA 0x00000008221ad020ULL
#define KERNEL_sceKernelGetProcessTimeCounter           "sceKernelGetProcessTimeCounter"

/* sceKernelGetFsSandboxRandomWord */
#define PS5SDK_KERNEL_SCEKERNELGETFSSANDBOXRANDOMWORD_RUNTIME_VA 0x00000008221ad100ULL
#define KERNEL_sceKernelGetFsSandboxRandomWord_RUNTIME_VA 0x00000008221ad100ULL
#define KERNEL_sceKernelGetFsSandboxRandomWord          "sceKernelGetFsSandboxRandomWord"

/* sceKernelGetSystemSwVersion */
#define PS5SDK_KERNEL_SCEKERNELGETSYSTEMSWVERSION_RUNTIME_VA 0x00000008221ad240ULL
#define KERNEL_sceKernelGetSystemSwVersion_RUNTIME_VA 0x00000008221ad240ULL
#define KERNEL_sceKernelGetSystemSwVersion              "sceKernelGetSystemSwVersion"

/* sceKernelGetCompiledSdkVersion */
#define PS5SDK_KERNEL_SCEKERNELGETCOMPILEDSDKVERSION_RUNTIME_VA 0x00000008221ad570ULL
#define KERNEL_sceKernelGetCompiledSdkVersion_RUNTIME_VA 0x00000008221ad570ULL
#define KERNEL_sceKernelGetCompiledSdkVersion           "sceKernelGetCompiledSdkVersion"

/* sceKernelCreateEqueue */
#define PS5SDK_KERNEL_SCEKERNELCREATEEQUEUE_RUNTIME_VA 0x00000008221adb90ULL
#define KERNEL_sceKernelCreateEqueue_RUNTIME_VA 0x00000008221adb90ULL
#define KERNEL_sceKernelCreateEqueue                    "sceKernelCreateEqueue"

/* sceKernelDeleteEqueue */
#define PS5SDK_KERNEL_SCEKERNELDELETEEQUEUE_RUNTIME_VA 0x00000008221adc10ULL
#define KERNEL_sceKernelDeleteEqueue_RUNTIME_VA 0x00000008221adc10ULL
#define KERNEL_sceKernelDeleteEqueue                    "sceKernelDeleteEqueue"

/* sceKernelWaitEqueue */
#define PS5SDK_KERNEL_SCEKERNELWAITEQUEUE_RUNTIME_VA 0x00000008221adc70ULL
#define KERNEL_sceKernelWaitEqueue_RUNTIME_VA 0x00000008221adc70ULL
#define KERNEL_sceKernelWaitEqueue                      "sceKernelWaitEqueue"

/* sceKernelAddReadEvent */
#define PS5SDK_KERNEL_SCEKERNELADDREADEVENT_RUNTIME_VA 0x00000008221ae2d0ULL
#define KERNEL_sceKernelAddReadEvent_RUNTIME_VA 0x00000008221ae2d0ULL
#define KERNEL_sceKernelAddReadEvent                    "sceKernelAddReadEvent"

/* sceKernelAddWriteEvent */
#define PS5SDK_KERNEL_SCEKERNELADDWRITEEVENT_RUNTIME_VA 0x00000008221ae3e0ULL
#define KERNEL_sceKernelAddWriteEvent_RUNTIME_VA 0x00000008221ae3e0ULL
#define KERNEL_sceKernelAddWriteEvent                   "sceKernelAddWriteEvent"

/* sceKernelAddUserEvent */
#define PS5SDK_KERNEL_SCEKERNELADDUSEREVENT_RUNTIME_VA 0x00000008221ae600ULL
#define KERNEL_sceKernelAddUserEvent_RUNTIME_VA 0x00000008221ae600ULL
#define KERNEL_sceKernelAddUserEvent                    "sceKernelAddUserEvent"

/* sceKernelDeleteUserEvent */
#define PS5SDK_KERNEL_SCEKERNELDELETEUSEREVENT_RUNTIME_VA 0x00000008221ae700ULL
#define KERNEL_sceKernelDeleteUserEvent_RUNTIME_VA 0x00000008221ae700ULL
#define KERNEL_sceKernelDeleteUserEvent                 "sceKernelDeleteUserEvent"

/* sceKernelTriggerUserEvent */
#define PS5SDK_KERNEL_SCEKERNELTRIGGERUSEREVENT_RUNTIME_VA 0x00000008221ae780ULL
#define KERNEL_sceKernelTriggerUserEvent_RUNTIME_VA 0x00000008221ae780ULL
#define KERNEL_sceKernelTriggerUserEvent                "sceKernelTriggerUserEvent"

/* sceKernelInternalMemoryGetAvailableSize */
#define PS5SDK_KERNEL_SCEKERNELINTERNALMEMORYGETAVAILABLESIZE_RUNTIME_VA 0x00000008221b5630ULL
#define KERNEL_sceKernelInternalMemoryGetAvailableSize_RUNTIME_VA 0x00000008221b5630ULL
#define KERNEL_sceKernelInternalMemoryGetAvailableSize  "sceKernelInternalMemoryGetAvailableSize"

/* sceKernelGetHwModelName */
#define PS5SDK_KERNEL_SCEKERNELGETHWMODELNAME_RUNTIME_VA 0x00000008221b7510ULL
#define KERNEL_sceKernelGetHwModelName_RUNTIME_VA 0x00000008221b7510ULL
#define KERNEL_sceKernelGetHwModelName                  "sceKernelGetHwModelName"

/* sceKernelDebugOutText */
#define PS5SDK_KERNEL_SCEKERNELDEBUGOUTTEXT_RUNTIME_VA 0x00000008221bb320ULL
#define KERNEL_sceKernelDebugOutText_RUNTIME_VA 0x00000008221bb320ULL
#define KERNEL_sceKernelDebugOutText                    "sceKernelDebugOutText"

/* sceKernelCreateEventFlag */
#define PS5SDK_KERNEL_SCEKERNELCREATEEVENTFLAG_RUNTIME_VA 0x00000008221bb8c0ULL
#define KERNEL_sceKernelCreateEventFlag_RUNTIME_VA 0x00000008221bb8c0ULL
#define KERNEL_sceKernelCreateEventFlag                 "sceKernelCreateEventFlag"

/* sceKernelDeleteEventFlag */
#define PS5SDK_KERNEL_SCEKERNELDELETEEVENTFLAG_RUNTIME_VA 0x00000008221bb910ULL
#define KERNEL_sceKernelDeleteEventFlag_RUNTIME_VA 0x00000008221bb910ULL
#define KERNEL_sceKernelDeleteEventFlag                 "sceKernelDeleteEventFlag"

/* sceKernelWaitEventFlag */
#define PS5SDK_KERNEL_SCEKERNELWAITEVENTFLAG_RUNTIME_VA 0x00000008221bb940ULL
#define KERNEL_sceKernelWaitEventFlag_RUNTIME_VA 0x00000008221bb940ULL
#define KERNEL_sceKernelWaitEventFlag                   "sceKernelWaitEventFlag"

/* sceKernelSetEventFlag */
#define PS5SDK_KERNEL_SCEKERNELSETEVENTFLAG_RUNTIME_VA 0x00000008221bb9e0ULL
#define KERNEL_sceKernelSetEventFlag_RUNTIME_VA 0x00000008221bb9e0ULL
#define KERNEL_sceKernelSetEventFlag                    "sceKernelSetEventFlag"

/* sceKernelClearEventFlag */
#define PS5SDK_KERNEL_SCEKERNELCLEAREVENTFLAG_RUNTIME_VA 0x00000008221bba10ULL
#define KERNEL_sceKernelClearEventFlag_RUNTIME_VA 0x00000008221bba10ULL
#define KERNEL_sceKernelClearEventFlag                  "sceKernelClearEventFlag"

/* sceKernelCreateSema */
#define PS5SDK_KERNEL_SCEKERNELCREATESEMA_RUNTIME_VA 0x00000008221bbba0ULL
#define KERNEL_sceKernelCreateSema_RUNTIME_VA 0x00000008221bbba0ULL
#define KERNEL_sceKernelCreateSema                      "sceKernelCreateSema"

/* sceKernelDeleteSema */
#define PS5SDK_KERNEL_SCEKERNELDELETESEMA_RUNTIME_VA 0x00000008221bbbf0ULL
#define KERNEL_sceKernelDeleteSema_RUNTIME_VA 0x00000008221bbbf0ULL
#define KERNEL_sceKernelDeleteSema                      "sceKernelDeleteSema"

/* sceKernelWaitSema */
#define PS5SDK_KERNEL_SCEKERNELWAITSEMA_RUNTIME_VA 0x00000008221bbc20ULL
#define KERNEL_sceKernelWaitSema_RUNTIME_VA 0x00000008221bbc20ULL
#define KERNEL_sceKernelWaitSema                        "sceKernelWaitSema"

/* sceKernelSignalSema */
#define PS5SDK_KERNEL_SCEKERNELSIGNALSEMA_RUNTIME_VA 0x00000008221bbcb0ULL
#define KERNEL_sceKernelSignalSema_RUNTIME_VA 0x00000008221bbcb0ULL
#define KERNEL_sceKernelSignalSema                      "sceKernelSignalSema"

/* sceKernelLoadStartModule */
#define PS5SDK_KERNEL_SCEKERNELLOADSTARTMODULE_RUNTIME_VA 0x00000008221c5c70ULL
#define KERNEL_sceKernelLoadStartModule_RUNTIME_VA 0x00000008221c5c70ULL
#define KERNEL_sceKernelLoadStartModule                 "sceKernelLoadStartModule"

/* sceKernelStopUnloadModule */
#define PS5SDK_KERNEL_SCEKERNELSTOPUNLOADMODULE_RUNTIME_VA 0x00000008221c6400ULL
#define KERNEL_sceKernelStopUnloadModule_RUNTIME_VA 0x00000008221c6400ULL
#define KERNEL_sceKernelStopUnloadModule                "sceKernelStopUnloadModule"

/* sceKernelGetModuleList */
#define PS5SDK_KERNEL_SCEKERNELGETMODULELIST_RUNTIME_VA 0x00000008221c6950ULL
#define KERNEL_sceKernelGetModuleList_RUNTIME_VA 0x00000008221c6950ULL
#define KERNEL_sceKernelGetModuleList                   "sceKernelGetModuleList"

/* sceKernelGetModuleInfo */
#define PS5SDK_KERNEL_SCEKERNELGETMODULEINFO_RUNTIME_VA 0x00000008221c6b80ULL
#define KERNEL_sceKernelGetModuleInfo_RUNTIME_VA 0x00000008221c6b80ULL
#define KERNEL_sceKernelGetModuleInfo                   "sceKernelGetModuleInfo"

/* sceKernelGetModuleInfoFromAddr */
#define PS5SDK_KERNEL_SCEKERNELGETMODULEINFOFROMADDR_RUNTIME_VA 0x00000008221c6d50ULL
#define KERNEL_sceKernelGetModuleInfoFromAddr_RUNTIME_VA 0x00000008221c6d50ULL
#define KERNEL_sceKernelGetModuleInfoFromAddr           "sceKernelGetModuleInfoFromAddr"

/* dlopen */
#define PS5SDK_KERNEL_DLOPEN_RUNTIME_VA 0x00000008221c7520ULL
#define KERNEL_dlopen_RUNTIME_VA 0x00000008221c7520ULL
#define KERNEL_dlopen                                   "dlopen"

/* dlclose */
#define PS5SDK_KERNEL_DLCLOSE_RUNTIME_VA 0x00000008221c7530ULL
#define KERNEL_dlclose_RUNTIME_VA 0x00000008221c7530ULL
#define KERNEL_dlclose                                  "dlclose"

/* dlerror */
#define PS5SDK_KERNEL_DLERROR_RUNTIME_VA 0x00000008221c7540ULL
#define KERNEL_dlerror_RUNTIME_VA 0x00000008221c7540ULL
#define KERNEL_dlerror                                  "dlerror"

/* dlsym */
#define PS5SDK_KERNEL_DLSYM_RUNTIME_VA 0x00000008221c7550ULL
#define KERNEL_dlsym_RUNTIME_VA 0x00000008221c7550ULL
#define KERNEL_dlsym                                    "dlsym"

/* pthread_mutex_init */
#define PS5SDK_KERNEL_PTHREAD_MUTEX_INIT_RUNTIME_VA 0x00000008221cd5f0ULL
#define KERNEL_pthread_mutex_init_RUNTIME_VA 0x00000008221cd5f0ULL
#define KERNEL_pthread_mutex_init                       "pthread_mutex_init"

/* pthread_mutex_timedlock */
#define PS5SDK_KERNEL_PTHREAD_MUTEX_TIMEDLOCK_RUNTIME_VA 0x00000008221cd860ULL
#define KERNEL_pthread_mutex_timedlock_RUNTIME_VA 0x00000008221cd860ULL
#define KERNEL_pthread_mutex_timedlock                  "pthread_mutex_timedlock"

/* pthread_mutex_destroy */
#define PS5SDK_KERNEL_PTHREAD_MUTEX_DESTROY_RUNTIME_VA 0x00000008221ce600ULL
#define KERNEL_pthread_mutex_destroy_RUNTIME_VA 0x00000008221ce600ULL
#define KERNEL_pthread_mutex_destroy                    "pthread_mutex_destroy"

/* pthread_mutex_trylock */
#define PS5SDK_KERNEL_PTHREAD_MUTEX_TRYLOCK_RUNTIME_VA 0x00000008221ce910ULL
#define KERNEL_pthread_mutex_trylock_RUNTIME_VA 0x00000008221ce910ULL
#define KERNEL_pthread_mutex_trylock                    "pthread_mutex_trylock"

/* pthread_mutex_lock */
#define PS5SDK_KERNEL_PTHREAD_MUTEX_LOCK_RUNTIME_VA 0x00000008221cee50ULL
#define KERNEL_pthread_mutex_lock_RUNTIME_VA 0x00000008221cee50ULL
#define KERNEL_pthread_mutex_lock                       "pthread_mutex_lock"

/* pthread_mutex_unlock */
#define PS5SDK_KERNEL_PTHREAD_MUTEX_UNLOCK_RUNTIME_VA 0x00000008221cfee0ULL
#define KERNEL_pthread_mutex_unlock_RUNTIME_VA 0x00000008221cfee0ULL
#define KERNEL_pthread_mutex_unlock                     "pthread_mutex_unlock"

/* pthread_key_create */
#define PS5SDK_KERNEL_PTHREAD_KEY_CREATE_RUNTIME_VA 0x00000008221d2010ULL
#define KERNEL_pthread_key_create_RUNTIME_VA 0x00000008221d2010ULL
#define KERNEL_pthread_key_create                       "pthread_key_create"

/* pthread_key_delete */
#define PS5SDK_KERNEL_PTHREAD_KEY_DELETE_RUNTIME_VA 0x00000008221d2150ULL
#define KERNEL_pthread_key_delete_RUNTIME_VA 0x00000008221d2150ULL
#define KERNEL_pthread_key_delete                       "pthread_key_delete"

/* scePthreadSetspecific */
#define PS5SDK_KERNEL_SCEPTHREADSETSPECIFIC_RUNTIME_VA 0x00000008221d24e0ULL
#define KERNEL_scePthreadSetspecific_RUNTIME_VA 0x00000008221d24e0ULL
#define KERNEL_scePthreadSetspecific                    "scePthreadSetspecific"

/* pthread_setspecific */
#define PS5SDK_KERNEL_PTHREAD_SETSPECIFIC_RUNTIME_VA 0x00000008221d2620ULL
#define KERNEL_pthread_setspecific_RUNTIME_VA 0x00000008221d2620ULL
#define KERNEL_pthread_setspecific                      "pthread_setspecific"

/* pthread_getspecific */
#define PS5SDK_KERNEL_PTHREAD_GETSPECIFIC_RUNTIME_VA 0x00000008221d2680ULL
#define KERNEL_pthread_getspecific_RUNTIME_VA 0x00000008221d2680ULL
#define KERNEL_pthread_getspecific                      "pthread_getspecific"

/* gadget_syscall */
#define PS5SDK_KERNEL_GADGET_SYSCALL_RUNTIME_VA 0x0000000809c636f0ULL
#define KERNEL_gadget_syscall_RUNTIME_VA 0x0000000809c636f0ULL
#define KERNEL_gadget_syscall                           "gadget_syscall"

/* gadget_pop_rdi */
#define PS5SDK_KERNEL_GADGET_POP_RDI_RUNTIME_VA 0x0000000809c65354ULL
#define KERNEL_gadget_pop_rdi_RUNTIME_VA 0x0000000809c65354ULL
#define KERNEL_gadget_pop_rdi                           "gadget_pop_rdi"

/* gadget_pop_rsi */
#define PS5SDK_KERNEL_GADGET_POP_RSI_RUNTIME_VA 0x00000008073903a0ULL
#define KERNEL_gadget_pop_rsi_RUNTIME_VA 0x00000008073903a0ULL
#define KERNEL_gadget_pop_rsi                           "gadget_pop_rsi"

/* gadget_syscall_09ccd06b */
#define PS5SDK_KERNEL_GADGET_SYSCALL_09CCD06B_RUNTIME_VA 0x0000000809ccd06bULL
#define KERNEL_gadget_syscall_09ccd06b_RUNTIME_VA 0x0000000809ccd06bULL
#define KERNEL_gadget_syscall_09ccd06b                  "gadget_syscall_09ccd06b"

/* gadget_syscall_1a18c02c */
#define PS5SDK_KERNEL_GADGET_SYSCALL_1A18C02C_RUNTIME_VA 0x000000081a18c02cULL
#define KERNEL_gadget_syscall_1a18c02c_RUNTIME_VA 0x000000081a18c02cULL
#define KERNEL_gadget_syscall_1a18c02c                  "gadget_syscall_1a18c02c"

/* gadget_syscall_35f40bcf */
#define PS5SDK_KERNEL_GADGET_SYSCALL_35F40BCF_RUNTIME_VA 0x0000000835f40bcfULL
#define KERNEL_gadget_syscall_35f40bcf_RUNTIME_VA 0x0000000835f40bcfULL
#define KERNEL_gadget_syscall_35f40bcf                  "gadget_syscall_35f40bcf"

/* gadget_pop_rdi_09ca5020 */
#define PS5SDK_KERNEL_GADGET_POP_RDI_09CA5020_RUNTIME_VA 0x0000000809ca5020ULL
#define KERNEL_gadget_pop_rdi_09ca5020_RUNTIME_VA 0x0000000809ca5020ULL
#define KERNEL_gadget_pop_rdi_09ca5020                  "gadget_pop_rdi_09ca5020"

/* gadget_pop_rdi_09cb71d6 */
#define PS5SDK_KERNEL_GADGET_POP_RDI_09CB71D6_RUNTIME_VA 0x0000000809cb71d6ULL
#define KERNEL_gadget_pop_rdi_09cb71d6_RUNTIME_VA 0x0000000809cb71d6ULL
#define KERNEL_gadget_pop_rdi_09cb71d6                  "gadget_pop_rdi_09cb71d6"

/* gadget_pop_rdi_09cbc16c */
#define PS5SDK_KERNEL_GADGET_POP_RDI_09CBC16C_RUNTIME_VA 0x0000000809cbc16cULL
#define KERNEL_gadget_pop_rdi_09cbc16c_RUNTIME_VA 0x0000000809cbc16cULL
#define KERNEL_gadget_pop_rdi_09cbc16c                  "gadget_pop_rdi_09cbc16c"

/* gadget_pop_rdi_09cdfe60 */
#define PS5SDK_KERNEL_GADGET_POP_RDI_09CDFE60_RUNTIME_VA 0x0000000809cdfe60ULL
#define KERNEL_gadget_pop_rdi_09cdfe60_RUNTIME_VA 0x0000000809cdfe60ULL
#define KERNEL_gadget_pop_rdi_09cdfe60                  "gadget_pop_rdi_09cdfe60"

/* gadget_pop_rdi_1a1baa18 */
#define PS5SDK_KERNEL_GADGET_POP_RDI_1A1BAA18_RUNTIME_VA 0x000000081a1baa18ULL
#define KERNEL_gadget_pop_rdi_1a1baa18_RUNTIME_VA 0x000000081a1baa18ULL
#define KERNEL_gadget_pop_rdi_1a1baa18                  "gadget_pop_rdi_1a1baa18"

/* gadget_pop_rsi_09cbb1b4 */
#define PS5SDK_KERNEL_GADGET_POP_RSI_09CBB1B4_RUNTIME_VA 0x0000000809cbb1b4ULL
#define KERNEL_gadget_pop_rsi_09cbb1b4_RUNTIME_VA 0x0000000809cbb1b4ULL
#define KERNEL_gadget_pop_rsi_09cbb1b4                  "gadget_pop_rsi_09cbb1b4"

/* gadget_pop_rsi_9000c0c99 */
#define PS5SDK_KERNEL_GADGET_POP_RSI_9000C0C99_RUNTIME_VA 0x00000009000c0c99ULL
#define KERNEL_gadget_pop_rsi_9000c0c99_RUNTIME_VA 0x00000009000c0c99ULL
#define KERNEL_gadget_pop_rsi_9000c0c99                 "gadget_pop_rsi_9000c0c99"

/* gadget_pop_rsi_9000c0fec */
#define PS5SDK_KERNEL_GADGET_POP_RSI_9000C0FEC_RUNTIME_VA 0x00000009000c0fecULL
#define KERNEL_gadget_pop_rsi_9000c0fec_RUNTIME_VA 0x00000009000c0fecULL
#define KERNEL_gadget_pop_rsi_9000c0fec                 "gadget_pop_rsi_9000c0fec"

/* gadget_syscall_900908283 */
#define PS5SDK_KERNEL_GADGET_SYSCALL_900908283_RUNTIME_VA 0x0000000900908283ULL
#define KERNEL_gadget_syscall_900908283_RUNTIME_VA 0x0000000900908283ULL
#define KERNEL_gadget_syscall_900908283                 "gadget_syscall_900908283"

/* gadget_syscall_900908a7b */
#define PS5SDK_KERNEL_GADGET_SYSCALL_900908A7B_RUNTIME_VA 0x0000000900908a7bULL
#define KERNEL_gadget_syscall_900908a7b_RUNTIME_VA 0x0000000900908a7bULL
#define KERNEL_gadget_syscall_900908a7b                 "gadget_syscall_900908a7b"

#endif