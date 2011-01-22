#ifndef _H_STD_H
#define _H_STD_H
#include <stdarg.h>

typedef unsigned int uint32_t, u32;
typedef unsigned short uint16_t, u16;
typedef unsigned char uint8_t, u8;

typedef signed int sint32_t, s32;
typedef signed short sint16_t, s16;
typedef signed char sint8_t, s8;

#ifdef NULL
#undef NULL
#endif

#ifndef __cplusplus
	#define NULL    ((void*)0)
	typedef sint32_t bool;
	#define false       (0)
	#define true        (1)
#else
	#define NULL		(0)
#endif

#define BIT(n)      (0x1 << (n))

#define SECTION(section)    __attribute__((__section__(#section)))
#define CODE16()            __asm__(".code16\n")
#define CODE32()            __asm__(".code32\n")
#define __BOOTCODE          SECTION(.boot.text)
#define __BOOTDATA          SECTION(.boot.data)
#define __STAGE1CODE        SECTION(.stage1.text)
#define __STAGE1DATA        SECTION(.stage1.data)

#define INLINE              inline
#define PACKED              __attribute__((packed))
#define offset_of(f,s)      ((uint32_t)(&((s*)0)->f))
#define container_of(ptr,type,member) ((type *)((uint32_t)(ptr) - offset_of(member,type)))

#define MIN(a,b)        ((a) < (b) ? (a) : (b))
#define MAX(a,b)        ((a) > (b) ? (a) : (b))

inline static void set_bit(void *p, int nr)
{
    u8 *o = (u8*)p + nr / 8;

    *o |= (1 << (nr % 8));
}

inline static void clear_bit(void *p, int nr)
{
    u8 *o = (u8*)p + nr / 8;

    *o &= ~(1 << (nr % 8));
}

inline static bool xor_bit(void *p, int nr)
{
    u8 *o = (u8*)p + nr / 8;

    *o ^= (1 << (nr % 8));
    return !!(*o & (1 << (nr % 8)));
}

inline static bool get_bit(void *p, int nr)
{
    u8 *o = (u8*)p + nr / 8;

    return !!(*o & (1 << (nr % 8)));
}
#endif
