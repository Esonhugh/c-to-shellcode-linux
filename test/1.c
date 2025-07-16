#define __PRINT
#include "../common/syscalls.h"


static void init_func_this(int argc, char ** argv, char ** envp) {
  int i = 0;
  for (; i < 10; i++){
    write(1, "Hello from init function!\n", 26);
  }
 	return;
}