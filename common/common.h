#define FUNC __attribute__((section(".func")))
#define VAR __attribute__((section(".var")))

#define ALIGN_STACK()                                                          \
  __asm__ __volatile__(                                                        \
      "mov %%rsp, %%rax;" /* Move stack pointer to rax */                      \
      "and $0xF, %%rax;"  /* Check if aligned to 16 bytes */                   \
      "jz aligned;"       /* If aligned, jump to aligned If not aligned,       \
                             adjust the stack pointer */                       \
      "sub $8, %%rsp;"    /* Decrease stack pointer by 8 bytes */              \
      "xor %0, %0;"       /* Optionally zero out the allocated space */        \
      "aligned:"                                                               \
      :        /* No output operands */                                        \
      : "r"(0) /* Input operand (to zero out) */                               \
      : "%rax" /* Clobbered register */                                        \
  )

#include <sys/syscall.h>

// 基础宏（0个参数）
#define __SYSCALL0(name, num) \
  static inline long name(void) { \
    long ret; \
    __asm__ __volatile__("syscall" \
      : "=a"(ret) \
      : "0"((long)(num)) \
      : "rcx", "r11", "memory"); \
    return ret; \
  }

// 1个参数
#define __SYSCALL1(name, num, type1, arg1) \
  static inline long name(type1 arg1) { \
    long ret; \
    __asm__ __volatile__("syscall" \
      : "=a"(ret) \
      : "0"((long)(num)), "D"((long)(arg1)) \
      : "rcx", "r11", "memory"); \
    return ret; \
  }

// 2个参数
#define __SYSCALL2(name, num, type1, arg1,type2, arg2) \
  static inline long name(type1 arg1, type2 arg2) { \
    long ret; \
    __asm__ __volatile__("syscall" \
      : "=a"(ret) \
      : "0"((long)(num)), "D"((long)(arg1)), "S"((long)(arg2)) \
      : "rcx", "r11", "memory"); \
    return ret; \
  }

// 3个参数
#define __SYSCALL3(name, num, type1, arg1, type2, arg2, type3, arg3) \
  static inline long name(type1 arg1, type2 arg2, type3 arg3) { \
    long ret; \
    __asm__ __volatile__("syscall" \
      : "=a"(ret) \
      : "0"((long)(num)), "D"((long)(arg1)), "S"((long)(arg2)), "d"((long)(arg3)) \
      : "rcx", "r11", "memory"); \
    return ret; \
  }

// 4个参数（使用r10寄存器）
#define __SYSCALL4(name, num, type1, arg1, type2, arg2, type3, arg3, type4, arg4) \
  static inline long name(type1 arg1, type2 arg2, type3 arg3, type4 arg4) { \
    long ret; \
    register long _r10 __asm__("r10") = (long)(arg4); \
    __asm__ __volatile__("syscall" \
      : "=a"(ret) \
      : "0"((long)(num)), "D"((long)(arg1)), "S"((long)(arg2)), "d"((long)(arg3)), "r"(_r10) \
      : "rcx", "r11", "memory"); \
    return ret; \
  }

// 5个参数（使用r10和r8寄存器）
#define __SYSCALL5(name, num, type1, arg1, type2, arg2, type3, arg3, type4, arg4, type5, arg5) \
  static inline long name(type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5) { \
    long ret; \
    register long _r10 __asm__("r10") = (long)(arg4); \
    register long _r8 __asm__("r8") = (long)(arg5); \
    __asm__ __volatile__("syscall" \
      : "=a"(ret) \
      : "0"((long)(num)), "D"((long)(arg1)), "S"((long)(arg2)), "d"((long)(arg3)), "r"(_r10), "r"(_r8) \
      : "rcx", "r11", "memory"); \
    return ret; \
  }

// 6个参数（使用r10、r8和r9寄存器）
#define __SYSCALL6(name, num, type1, arg1, type2, arg2, type3, arg3, type4, arg4, type5, arg5, type6, arg6) \
  static inline long name(type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5, type6 arg6) { \
    long ret; \
    register long _r10 __asm__("r10") = (long)(arg4); \
    register long _r8 __asm__("r8") = (long)(arg5); \
    register long _r9 __asm__("r9") = (long)(arg6); \
    __asm__ __volatile__("syscall" \
      : "=a"(ret) \
      : "0"((long)(num)), "D"((long)(arg1)), "S"((long)(arg2)), "d"((long)(arg3)), "r"(_r10), "r"(_r8), "r"(_r9) \
      : "rcx", "r11", "memory"); \
    return ret; \
  }

#define __SYSCALL(args, name,num, ...) \
  __SYSCALL##args(name, num, ##__VA_ARGS__)

#include <stddef.h>
#include <sys/syscall.h>