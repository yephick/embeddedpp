#include <stdio.h>
#include "lib1/lib1.h"
#include "lib2/lib2.h"

int main(void){
  printf("%s\n", greeting_msg());
  for(unsigned n = 1; n < 20; ++n)
    print_fib(n);
  return 0;
}

