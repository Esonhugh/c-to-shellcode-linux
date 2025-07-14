#ifndef _SHELLCODE_COMMON_H
#define _SHELLCODE_COMMON_H
#include "syscalls.h"

#define ENTRY __attribute__((section(".entry")))
#define INLINE __attribute__((always_inline)) inline

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

#ifndef no_memset
void* memset(void* b, int c, size_t len) {
    char* p = (char*)b;
    for (size_t i = 0; i != len; ++i) {
        p[i] = c;
    }
    return b;
}

void* memcpy(void* dest, const void* src, size_t len) {
    char* d = (char*)dest;
    const char* s = (const char*)src;
    for (size_t i = 0; i != len; ++i) {
        d[i] = s[i];
    }
    return dest;
}
#endif

#ifndef no_get_rip
long get_rip() {
  void* location;
  __asm__ __volatile__(
      "mov %%rsp, %0;" // Move stack pointer to location
      : "=r"(location)  // Output operand
      :                  // No input operands
      :                  // No clobbered registers
  );
  return *((long*)location);
}

#endif

#ifdef __PRINT

int strlen(const char *str) {
  int len = 0;
  while (str[len] != '\0') {
    len++;
  }
  return len;
}

int print(const char *str) {
  int len = strlen(str);
  return write(1, str, len);
}

#endif
#endif