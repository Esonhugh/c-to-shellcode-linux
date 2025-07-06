#define __PRINT
#define no_memset
#include "common/common.h"
#include "mojoelf/mojoelf.c"

#define LOADER_LEN 7768
#define SO_LENGTH 15112

FUNC long get_shellcode_location(long self_header) {
  // Calculate the location of the shellcode in the binary
  return self_header + LOADER_LEN;
}


int start() {
  long start_addr = get_rip() - 8 ; // func header
  long padding_shellcode_ptr = get_shellcode_location(start_addr); // get shellcode location
  MOJOELF_dlopen_mem((void*)padding_shellcode_ptr, SO_LENGTH, NULL); // load the shellcode
  print("[+] Shellcode written to /tmp/sc\n");
  return start_addr; // This will never be reached due to exit in start()
}