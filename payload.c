#define __PRINT
#define no_memset
#include "common/common.h"

static char *fake =  "Global variable\n";


long get_shellcode_location(long self_header, long self_len) {
  // Calculate the location of the shellcode in the binary
  return self_header + self_len;
}

ENTRY int start() {
  setuid(2);
  fake[0]= 'g';
  print(fake);
  return 1;
}