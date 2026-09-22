#include "myfunc.h"

#include <stdint.h>
#include <stddef.h>

uint32_t b1;
uint32_t b2[4];


void *memset(void *s, int c, size_t n) {
    unsigned char *p = (unsigned char *)s;
    while (n--) {
        *p++ = (unsigned char)c;
    }
    return s;
}
int main()
{
	char s[100] = { 1,2,3 };

	return myfunc(s);
}
