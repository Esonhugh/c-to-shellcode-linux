#ifndef COMMON_H
#define COMMON_H

#include "syscalls.h"

#define FUNC __attribute__((section(".func")))
#define INLINE __attribute__((always_inline)) inline
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

#ifndef no_memset
FUNC void* memset(void* b, int c, size_t len) {
    char* p = (char*)b;
    for (size_t i = 0; i != len; ++i) {
        p[i] = c;
    }
    return b;
}

FUNC void* memcpy(void* dest, const void* src, size_t len) {
    char* d = (char*)dest;
    const char* s = (const char*)src;
    for (size_t i = 0; i != len; ++i) {
        d[i] = s[i];
    }
    return dest;
}
#endif

#ifndef malloc

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

/* Other flags.  */
#define MAP_FIXED	0x10		/* Interpret addr exactly.  */
# define MAP_FILE	0
# ifdef __MAP_ANONYMOUS
#  define MAP_ANONYMOUS	__MAP_ANONYMOUS	/* Don't use a file.  */
# else
#  define MAP_ANONYMOUS	0x20		/* Don't use a file.  */
# endif
# define MAP_ANON	MAP_ANONYMOUS
/* When MAP_HUGETLB is set bits [26:31] encode the log2 of the huge page size.  */
# define MAP_HUGE_SHIFT	26
# define MAP_HUGE_MASK	0x3f

#define MAP_FAILED	((void *) -1)

// Function of simple memory allocation
FUNC void *malloc(size_t size) {
  void* ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  return ptr;
}

#define Malloc(size) malloc(size) // Define malloc to use the custom implementation

#endif

#ifndef no_get_rip
FUNC long get_rip() {
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

FUNC int strlen(const char *str) {
  int len = 0;
  while (str[len] != '\0') {
    len++;
  }
  return len;
}

FUNC int print(const char *str) {
  return write(1, str, strlen(str));
}

FUNC int fprint(int fd, const char *str) {
  return write(fd, str, strlen(str));
}

FUNC int print_hex(void* ptr) {
  char hex[18];
  hex[0] = '0';
  hex[1] = 'x';
  const char* hex_chars = "0123456789abcdef";
  unsigned long addr = (unsigned long)ptr;
  for (int i = 17; i >= 2; i--) {
    hex[i] = hex_chars[addr % 16];
    addr /= 16;
  }
  return write(1, hex, 18);
}

#endif

#endif // COMMON_H