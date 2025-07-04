#define __PRINT
#define no_memset
#include "common/common.h"

#define LOADER_LEN 200

FUNC long get_shellcode_location(long self_header) {
  // Calculate the location of the shellcode in the binary
  return self_header + LOADER_LEN;
}

int start(long elf_flie_length) {
  long start_addr = get_rip() - 8 ; // func header
  long padding_shellcode_ptr = get_shellcode_location(start_addr); // get shellcode location

  print("[+] Shellcode written to /tmp/sc\n");
  return start_addr; // This will never be reached due to exit in start()
}