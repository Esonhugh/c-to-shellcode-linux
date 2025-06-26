#include "common/common.h"

__SYSCALL(3, write, SYS_write, int,fd, const char *,str, size_t,len);

FUNC int test() {
  write(1, "Hello, World2!\n", 15);
  return 0;
}

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

FUNC char const* get_hello() {
  return "Hello, World!\n";
}


int start() {
  print(get_hello());
  return 0;
}