

// gcc -Wall -g -o debug_example debug.c

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdlib.h>

struct debug_opt
  {
  uint32_t   a;
  uint32_t   b;
  char      *c;
  uint32_t   d;
  };

void DEBUG_INTERNAL(struct debug_opt O)
{
printf("a = %u\n"
       "b = %u\n"
       "c = %s\n"
       "d = %u\n"
       "\n"
       ,O.a
       ,O.b
       ,O.c
       ,O.d
       );
}

#define debug(...) DEBUG_INTERNAL((struct debug_opt){ .a = 1, .b = 2, .c = "default", ## __VA_ARGS__ })

int main(void)
{
debug(.a=5,.c="Hello");
debug();

return 0;
}