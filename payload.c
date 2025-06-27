#define __PRINT
#include "common/common.h"

FUNC int test() {
  print("Hello, World2!\n");
  return 0;
}

int start() {
  setuid(33); // set uid to 0 (root)
  VAR(int, uid, getuid());
  if (uid == 0) {
    print("uid = 0 (root)\n");
  } else {
    print("uid != 0 (not root)\n");
  }
  const char *filename = "/etc/passwd";
  VAR(int, fd, open(filename, 0));
  char buffer[1024];
  if (fd < 0) {
    print("Failed to open file: ");
    print(filename);
    print("\n");
  } else {
    VAR(int, len, 0);
    read_again:
    len =  read(fd, buffer, 20);
    if (len < 0) {
      print("Failed to read file: ");
      print(filename);
      print("\n");
    } else if (len == 20) {
      write(1, buffer, len);
      goto read_again;
    } else {
      write(1, buffer, len);
    }
    close(fd);
  }
  exit(0);
  return 0;
}