
#include <sys/syscall.h>
#include <stddef.h>

// 基础宏（0个参数）
#define __SYSCALL0(name, num)                       \
  static inline long name(void)                     \
  {                                                 \
    long ret;                                       \
    __asm__ __volatile__("syscall"                  \
                         : "=a"(ret)                \
                         : "0"((long)(num))         \
                         : "rcx", "r11", "memory"); \
    return ret;                                     \
  }

// 1个参数
#define __SYSCALL1(name, num, type1, arg1)                     \
  static inline long name(type1 arg1)                          \
  {                                                            \
    long ret;                                                  \
    __asm__ __volatile__("syscall"                             \
                         : "=a"(ret)                           \
                         : "0"((long)(num)), "D"((long)(arg1)) \
                         : "rcx", "r11", "memory");            \
    return ret;                                                \
  }

// 2个参数
#define __SYSCALL2(name, num, type1, arg1, type2, arg2)                           \
  static inline long name(type1 arg1, type2 arg2)                                 \
  {                                                                               \
    long ret;                                                                     \
    __asm__ __volatile__("syscall"                                                \
                         : "=a"(ret)                                              \
                         : "0"((long)(num)), "D"((long)(arg1)), "S"((long)(arg2)) \
                         : "rcx", "r11", "memory");                               \
    return ret;                                                                   \
  }

// 3个参数
#define __SYSCALL3(name, num, type1, arg1, type2, arg2, type3, arg3)                                 \
  static inline long name(type1 arg1, type2 arg2, type3 arg3)                                        \
  {                                                                                                  \
    long ret;                                                                                        \
    __asm__ __volatile__("syscall"                                                                   \
                         : "=a"(ret)                                                                 \
                         : "0"((long)(num)), "D"((long)(arg1)), "S"((long)(arg2)), "d"((long)(arg3)) \
                         : "rcx", "r11", "memory");                                                  \
    return ret;                                                                                      \
  }

// 4个参数（使用r10寄存器）
#define __SYSCALL4(name, num, type1, arg1, type2, arg2, type3, arg3, type4, arg4)                               \
  static inline long name(type1 arg1, type2 arg2, type3 arg3, type4 arg4)                                       \
  {                                                                                                             \
    long ret;                                                                                                   \
    register long _r10 __asm__("r10") = (long)(arg4);                                                           \
    __asm__ __volatile__("syscall"                                                                              \
                         : "=a"(ret)                                                                            \
                         : "0"((long)(num)), "D"((long)(arg1)), "S"((long)(arg2)), "d"((long)(arg3)), "r"(_r10) \
                         : "rcx", "r11", "memory");                                                             \
    return ret;                                                                                                 \
  }

// 5个参数（使用r10和r8寄存器）
#define __SYSCALL5(name, num, type1, arg1, type2, arg2, type3, arg3, type4, arg4, type5, arg5)                            \
  static inline long name(type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5)                                     \
  {                                                                                                                       \
    long ret;                                                                                                             \
    register long _r10 __asm__("r10") = (long)(arg4);                                                                     \
    register long _r8 __asm__("r8") = (long)(arg5);                                                                       \
    __asm__ __volatile__("syscall"                                                                                        \
                         : "=a"(ret)                                                                                      \
                         : "0"((long)(num)), "D"((long)(arg1)), "S"((long)(arg2)), "d"((long)(arg3)), "r"(_r10), "r"(_r8) \
                         : "rcx", "r11", "memory");                                                                       \
    return ret;                                                                                                           \
  }

// 6个参数（使用r10、r8和r9寄存器）
#define __SYSCALL6(name, num, type1, arg1, type2, arg2, type3, arg3, type4, arg4, type5, arg5, type6, arg6)                         \
  static inline long name(type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5, type6 arg6)                                   \
  {                                                                                                                                 \
    long ret;                                                                                                                       \
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
#define __SYSCALL(arg_num, name, num, ...) \
  __SYSCALL##arg_num(name, num, ##__VA_ARGS__)

__SYSCALL(3, mprotect, SYS_mprotect, void *, addr, size_t, len, int, prot);
__SYSCALL(3, write, SYS_write, int, fd, const char *, str, size_t, len);
__SYSCALL(3, read, SYS_read, int, fd, char *, buf, size_t, count);
__SYSCALL(1, exit, SYS_exit, int, status);
__SYSCALL(1, setuid, SYS_setuid, int, uid);
__SYSCALL(0, getuid, SYS_getuid);
__SYSCALL(0, getpid, SYS_getpid);
__SYSCALL(2, open, SYS_open, const char *, filename, int, oflags);
__SYSCALL(1, close, SYS_close, int, fd);
