#include <sys/syscall.h>
#include <stddef.h>

// 基础宏（0个参数）
#define __SYSCALL0(name, num, ret_t)                \
  static inline ret_t name(void)                    \
  {                                                 \
    ret_t ret;                                      \
    __asm__ __volatile__("syscall"                  \
                         : "=a"(ret)                \
                         : "0"((long)(num))         \
                         : "rcx", "r11", "memory"); \
    return ret;                                     \
  }

// 1个参数
#define __SYSCALL1(name, num, type1, arg1, ret_t)              \
  static inline ret_t name(type1 arg1)                         \
  {                                                            \
    ret_t ret;                                                 \
    __asm__ __volatile__("syscall"                             \
                         : "=a"(ret)                           \
                         : "0"((long)(num)), "D"((long)(arg1)) \
                         : "rcx", "r11", "memory");            \
    return ret;                                                \
  }

// 2个参数
#define __SYSCALL2(name, num, type1, arg1, type2, arg2, ret_t)                    \
  static inline ret_t name(type1 arg1, type2 arg2)                                \
  {                                                                               \
    ret_t ret;                                                                    \
    __asm__ __volatile__("syscall"                                                \
                         : "=a"(ret)                                              \
                         : "0"((long)(num)), "D"((long)(arg1)), "S"((long)(arg2)) \
                         : "rcx", "r11", "memory");                               \
    return ret;                                                                   \
  }

// 3个参数
#define __SYSCALL3(name, num, type1, arg1, type2, arg2, type3, arg3, ret_t)                          \
  static inline ret_t name(type1 arg1, type2 arg2, type3 arg3)                                       \
  {                                                                                                  \
    ret_t ret;                                                                                       \
    __asm__ __volatile__("syscall"                                                                   \
                         : "=a"(ret)                                                                 \
                         : "0"((long)(num)), "D"((long)(arg1)), "S"((long)(arg2)), "d"((long)(arg3)) \
                         : "rcx", "r11", "memory");                                                  \
    return ret;                                                                                      \
  }

// 4个参数（使用r10寄存器）
#define __SYSCALL4(name, num, type1, arg1, type2, arg2, type3, arg3, type4, arg4, ret_t)                        \
  static inline ret_t name(type1 arg1, type2 arg2, type3 arg3, type4 arg4)                                      \
  {                                                                                                             \
    ret_t ret;                                                                                                  \
    register long _r10 __asm__("r10") = (long)(arg4);                                                           \
    __asm__ __volatile__("syscall"                                                                              \
                         : "=a"(ret)                                                                            \
                         : "0"((long)(num)), "D"((long)(arg1)), "S"((long)(arg2)), "d"((long)(arg3)), "r"(_r10) \
                         : "rcx", "r11", "memory");                                                             \
    return ret;                                                                                                 \
  }

// 5个参数（使用r10和r8寄存器）
#define __SYSCALL5(name, num, type1, arg1, type2, arg2, type3, arg3, type4, arg4, type5, arg5, ret_t)                     \
  static inline ret_t name(type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5)                                    \
  {                                                                                                                       \
    ret_t ret;                                                                                                            \
    register long _r10 __asm__("r10") = (long)(arg4);                                                                     \
    register long _r8 __asm__("r8") = (long)(arg5);                                                                       \
    __asm__ __volatile__("syscall"                                                                                        \
                         : "=a"(ret)                                                                                      \
                         : "0"((long)(num)), "D"((long)(arg1)), "S"((long)(arg2)), "d"((long)(arg3)), "r"(_r10), "r"(_r8) \
                         : "rcx", "r11", "memory");                                                                       \
    return ret;                                                                                                           \
  }

// 6个参数（使用r10、r8和r9寄存器）
#define __SYSCALL6(name, num, type1, arg1, type2, arg2, type3, arg3, type4, arg4, type5, arg5, type6, arg6, ret_t)                  \
  static inline ret_t name(type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5, type6 arg6)                                  \
  {                                                                                                                                 \
    ret_t ret;                                                                                                                      \
    register long _r10 __asm__("r10") = (long)(arg4);                                                                               \
    register long _r8 __asm__("r8") = (long)(arg5);                                                                                 \
    register long _r9 __asm__("r9") = (long)(arg6);                                                                                 \
    __asm__ __volatile__("syscall"                                                                                                  \
                         : "=a"(ret)                                                                                                \
                         : "0"((long)(num)), "D"((long)(arg1)), "S"((long)(arg2)), "d"((long)(arg3)), "r"(_r10), "r"(_r8), "r"(_r9) \
                         : "rcx", "r11", "memory");                                                                                 \
    return ret;                                                                                                                     \
  }

// defined SYSCALL macros for 0 to 6 arguments
#define __SYSCALL(arg_num, ret_t, name, num, ...) \
  __SYSCALL##arg_num(name, num, ##__VA_ARGS__, ret_t)

// int mprotect(void addr[.size], size_t size, int prot);
__SYSCALL(3, int, mprotect, SYS_mprotect, void *, addr, size_t, len, int, prot);
// ssize_t write(int fd, const char *str, size_t len);
__SYSCALL(3, long, write, SYS_write, int, fd, const char *, str, size_t, len);
//  ssize_t read(int fd, void buf[.count], size_t count);
__SYSCALL(3, long, read, SYS_read, int, fd, char *, buf, size_t, count);
// void exit(int status);
__SYSCALL(1, long, exit, SYS_exit, int, status); // never returns
// int setuid(uid_t uid);
__SYSCALL(1, int, setuid, SYS_setuid, unsigned int, uid);
// uid_t getuid(void);
__SYSCALL(0, unsigned int, getuid, SYS_getuid);
//  pid_t getpid(void);
__SYSCALL(0, int, getpid, SYS_getpid);
// int open(const char *pathname, int flags, .../* mode_t mode */ );
__SYSCALL(2, int, open, SYS_open, const char *, filename, int, oflags);

#define O_RDONLY             00
#define O_WRONLY             01
#define O_RDWR               02
#ifndef O_CREAT
# define O_CREAT           0100 /* Not fcntl.  */
#endif
#ifndef O_EXCL
# define O_EXCL            0200 /* Not fcntl.  */
#endif
#ifndef O_NOCTTY
# define O_NOCTTY          0400 /* Not fcntl.  */
#endif
#ifndef O_TRUNC
# define O_TRUNC          01000 /* Not fcntl.  */
#endif
#ifndef O_APPEND
# define O_APPEND         02000
#endif
#ifndef O_NONBLOCK
# define O_NONBLOCK       04000
#endif
#ifndef O_NDELAY
# define O_NDELAY       O_NONBLOCK
#endif
#ifndef O_SYNC
# define O_SYNC        04010000
#endif
#define O_FSYNC         O_SYNC
#ifndef O_ASYNC
# define O_ASYNC         020000
#endif
#ifndef __O_LARGEFILE
# define __O_LARGEFILE  0100000
#endif

#ifndef __O_DIRECTORY
# define __O_DIRECTORY  0200000
#endif
#ifndef __O_NOFOLLOW
# define __O_NOFOLLOW   0400000
#endif
#ifndef __O_CLOEXEC
# define __O_CLOEXEC   02000000
#endif
#ifndef __O_DIRECT
# define __O_DIRECT      040000
#endif
#ifndef __O_NOATIME
# define __O_NOATIME   01000000
#endif
#ifndef __O_PATH
# define __O_PATH     010000000
#endif
#ifndef __O_DSYNC
# define __O_DSYNC       010000
#endif
#ifndef __O_TMPFILE
# define __O_TMPFILE   (020000000 | __O_DIRECTORY)
#endif

// int close(int fd);
__SYSCALL(1, int, close, SYS_close, int, fd);
// int brk(void *addr);
__SYSCALL(1, int, brk, SYS_brk, void *, addr);

// void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
typedef long int off_t;
__SYSCALL(6, void *, mmap, SYS_mmap, void *, addr, size_t, length, int, prot, int, flags, int, fd, off_t, offset);
// int munmap(void *addr, size_t length);
__SYSCALL(2, int, munmap, SYS_munmap, void *, addr, size_t, length);


// mmap flags
#define PROT_READ	0x1		/* Page can be read.  */
#define PROT_WRITE	0x2		/* Page can be written.  */
#define PROT_EXEC	0x4		/* Page can be executed.  */
#define PROT_NONE	0x0		/* Page can not be accessed.  */
#define PROT_GROWSDOWN	0x01000000	/* Extend change to start of
					   growsdown vma (mprotect only).  */
#define PROT_GROWSUP	0x02000000	/* Extend change to start of
					   growsup vma (mprotect only).  */

/* Sharing types (must choose one and only one of these).  */
#define MAP_SHARED	0x01		/* Share changes.  */
#define MAP_PRIVATE	0x02		/* Changes are private.  */
#define MAP_ANONYMOUS	0x20		/* Anonymous mapping.  */