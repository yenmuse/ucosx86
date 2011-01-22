#ifndef _H_UTILS_H
#define _H_UTILS_H
#include "std.h"

#ifdef __cplusplus
extern "C" {
#endif
void printk(const char *fmt,...);
void sprintk(char *buf, const char *fmt,...);
void putch(char ch);
void putch_at(char ch, int x, int y);
void* memset(void *base, int c, int size);
int strlen(const char *str);
void* memcpy(void *dst, const void *src, int size);

#ifdef __cplusplus
}
#endif

#endif
