#define __PRINT
#define no_memset
#include "common/common.h"

static char *fake =  "Global variable\n";


FUNC long get_shellcode_location(long self_header, long self_len) {
  // Calculate the location of the shellcode in the binary
  return self_header + self_len;
}

int start(long self_len, long shellcode_len) {
  long rip = get_rip() - 11; // func header
  long padding_shellcode_ptr = get_shellcode_location(rip, self_len); // get shellcode location
  setuid(2);
  fake[0]= 'g';
  print(fake);
  long rip2 =  get_rip();
  return padding_shellcode_ptr - rip2 ; // This will never be reached due to exit in start()
}