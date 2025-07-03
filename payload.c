#define __PRINT
#define no_memset
#include "common/common.h"

static char *fake =  "Global variable\n";

int start() {
  long rip = get_rip() - 8;
  setuid(2);
  fake[0]= 'g';
  print(fake);
  long rip2 =  get_rip();
  return rip2 - rip ; // This will never be reached due to exit in start()
}