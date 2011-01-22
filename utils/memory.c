#include "std.h"
#include "x86.h"

void* memset(void *base, int c, int size)
{
    u8 *p = (u8*)base;

	while(size--)
		*p++ = (u8)c;
	return base;
}

int strlen(const char *str)
{
    const char *p = str;

    while(*p++);
    return p - str;
}

void* memcpy(void *dst, const void *src, int size)
{
    u8 *pdst = dst;
    const u8 *psrc = src;

    while(size--)
        *pdst++ = *psrc++;

	return dst;
}
