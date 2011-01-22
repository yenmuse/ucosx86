#ifndef _H_PM_H
#define _H_PM_H

#define DA_32		0x4000
#define DA_DPL0		0x00
#define DA_DPL1		0x20
#define DA_DPL2		0x40
#define DA_DPL3		0x60
#define DA_DR		0x90
#define DA_DRW		0x92
#define DA_DRWA		0x93
#define DA_C		0x98
#define DA_CCO		0x9c
#define DA_CCOR		0x9e
#define DA_G		0x8000

#ifdef __ASSEMBLY__

#define DESC(Base, Limit, Attr)	\
	.2byte ((Limit) & 0xffff); \
	.2byte ((Base) & 0xff); \
	.byte ((Base) >> 16) & 0xff; \
	.2byte ((((Limit) >> 8) & 0xf00) | ((Attr) & 0xf0ff));\
	.byte ((Base) >> 24) & 0xff;


#define GDT(Base, Limit)	\
	.2byte ((Limit) & 0xffff); \
	.4byte (Base);

#endif

#ifndef __ASSEMBLY__
#include "std.h"

enum{
    TYPE_TSS_16         = 0x1,
    TYPE_LDT            = 0x2,
    TYPE_TSS_16_BUSY    = 0x3,
    TYPE_CALL_GATE_16   = 0x4,
    TYPE_TASK_GATE      = 0x5,
    TYPE_INT_GATE_16    = 0x6,
    TYPE_TRAP_GATE_16   = 0x7,
    TYPE_TSS_32         = 0x9,
    TYPE_TSS_32_BUSY    = 0xa,
    TYPE_CALL_GATE_32   = 0xb,
    TYPE_INT_GATE_32    = 0xe,
    TYPE_TRAP_GATE_32   = 0xf,
};

/* global descriptor */
typedef struct _gdt_desc
{
	u32 low;
	u32 high;
}PACKED gdt_desc_t;

/* interrupt descriptor */
typedef struct _idt_desc
{
    u32 low;
    u32 high;
}PACKED idt_desc_t;

/* local descriptor */
typedef struct _ldt_desc
{
    u32 low;
    u32 high;
}PACKED ldt_desc_t;

/* task state descriptor */
typedef struct _tss_desc
{
    u32 low;
    u32 high;
}PACKED tss_desc_t;

/* global descriptor table register and interrupt descriptor table register */
typedef struct
{
   u16 limit;
   u32 base;
}PACKED gdtr_t, idtr_t;

/* local descriptor table register and task register */
typedef struct
{
    u32 low;
    u32 base;
}PACKED ldtr_t, tr_t;
#endif

#endif
