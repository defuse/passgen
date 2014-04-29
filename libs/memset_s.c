#include <stdlib.h>

#include "memset_s.h"

/* 
 * This code was taken from (url split into two lines):
 * https://www.securecoding.cert.org/confluence/display/cplusplus/
 * MSC06-CPP.+Be+aware+of+compiler+optimization+when+dealing+with+sensitive+data
 */
void *memset_s(void *v, int c, size_t n) {
  volatile unsigned char *p = v;
  while (n--)
    *p++ = c;
 
  return v;
}
