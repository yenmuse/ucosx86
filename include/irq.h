#ifndef _H_IRQ_H
#define _H_IRQ_H
#include "std.h"

/* 8259A IO port */
#define INT_M_CTL       (0x20)
#define INT_M_CTLMASK   (0x21)
#define INT_S_CTL       (0xa0)
#define INT_S_CTLMASK   (0xa1)
#define INT_VECTOR_IRQ0 (0x20)

struct irq_ctx
{
    u32 pdata;
    u32 edi;
    u32 esi;
    u32 ebp;
    u32 esp;
    u32 ebx;
    u32 edx;
    u32 ecx;
    u32 eax;
    u32 vector;
    u32 errcode;
    u32 eip;
    u32 cs;
    u32 eflags;
};

struct irq_handler
{

};

void init_8259A(void);
void set_mask_8259A(u16 mask);
void register_handler(u32 vector, void (*handler)(struct irq_ctx*));
#endif
