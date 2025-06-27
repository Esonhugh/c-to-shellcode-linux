#include "syscalls.h"

#define FUNC __attribute__((section(".func")))
#define CONST(type, name, init) \
  FUNC type name() { return init; } 

#define VAR(type, name, init) __volatile__ type name = init

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


typedef int bool;
#define True 1
#define False 0

#ifdef __PRINT

FUNC int strlen(const char *str) {
  int len = 0;
  while (str[len] != '\0') {
    len++;
  }
  return len;
}

FUNC int print(const char *str) {
  int len = strlen(str);
  return write(1, str, len);
}

#endif



