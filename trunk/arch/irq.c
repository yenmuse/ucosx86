#include "std.h"
#include "pm.h"
#include "io.h"
#include "x86.h"
#include "irq.h"

static void (*g_interrupt_handler[256])(struct irq_ctx *ctx) = {0,};
extern idt_desc_t sys_idt_table[];

void init_idt_desc(u8 vector, u8 type, void (*handler)(void), u8 dpl)
{
    u32 offset = (u32)handler;
    idt_desc_t *pi = sys_idt_table + vector;

    pi->low = (offset & 0xffff) | (0x8 << 16);
    pi->high = (offset & 0xffff0000) 
        | BIT(15)   /* P */
        | ((dpl & 0x3) << 13)   /* dpl */
        | ((type & 0xf) << 8)   /* type */
    ;
}

void register_handler(u32 vector, void (*handler)(struct irq_ctx*))
{
    if(g_interrupt_handler[vector])
    {
        panic("Already has handler!\n");
    }
    else
    {
        g_interrupt_handler[vector] = handler;
    }
}

void user_isr(struct irq_ctx ctx)
{
    u32 vector = ctx.vector;

    if(g_interrupt_handler[vector])
        g_interrupt_handler[vector](&ctx);
    else
        panic("No Handler for Interrupt %d!\n", vector);
}

void init_idt_table()
{
    /* init system trap */
#define _EXCEPTION_HANDLER_NO_ERRCODE(name, vector) \
    extern void name(void); \
    init_idt_desc(vector, TYPE_INT_GATE_32, name, 0);

#define _EXCEPTION_HANDLER_ERRCODE(name, vector) \
    extern void name(void); \
    init_idt_desc(vector, TYPE_INT_GATE_32, name, 0);

#define _IRQ_HANDLER(vector)   \
    extern void _IRQ_##vector(void);    \
    init_idt_desc(vector, TYPE_INT_GATE_32, _IRQ_##vector, 0);

#define _IRQ_HANDLER_10(vector)   \
    extern void _IRQ_##vector##0(void);    \
    extern void _IRQ_##vector##1(void);    \
    extern void _IRQ_##vector##2(void);    \
    extern void _IRQ_##vector##3(void);    \
    extern void _IRQ_##vector##4(void);    \
    extern void _IRQ_##vector##5(void);    \
    extern void _IRQ_##vector##6(void);    \
    extern void _IRQ_##vector##7(void);    \
    extern void _IRQ_##vector##8(void);    \
    extern void _IRQ_##vector##9(void);    \
    init_idt_desc(vector##0, TYPE_INT_GATE_32, _IRQ_##vector##0, 0);   \
    init_idt_desc(vector##1, TYPE_INT_GATE_32, _IRQ_##vector##1, 0);   \
    init_idt_desc(vector##2, TYPE_INT_GATE_32, _IRQ_##vector##2, 0);   \
    init_idt_desc(vector##3, TYPE_INT_GATE_32, _IRQ_##vector##3, 0);   \
    init_idt_desc(vector##4, TYPE_INT_GATE_32, _IRQ_##vector##4, 0);   \
    init_idt_desc(vector##5, TYPE_INT_GATE_32, _IRQ_##vector##5, 0);   \
    init_idt_desc(vector##6, TYPE_INT_GATE_32, _IRQ_##vector##6, 0);   \
    init_idt_desc(vector##7, TYPE_INT_GATE_32, _IRQ_##vector##7, 0);   \
    init_idt_desc(vector##8, TYPE_INT_GATE_32, _IRQ_##vector##8, 0);   \
    init_idt_desc(vector##9, TYPE_INT_GATE_32, _IRQ_##vector##9, 0);   

    _EXCEPTION_HANDLER_NO_ERRCODE(_DE_fault, 0)
    _EXCEPTION_HANDLER_NO_ERRCODE(_UNUSED_0, 1)
    _EXCEPTION_HANDLER_NO_ERRCODE(_NMI_interrupt, 2)
    _EXCEPTION_HANDLER_NO_ERRCODE(_BP_trap, 3)
    _EXCEPTION_HANDLER_NO_ERRCODE(_OF_trap, 4)
    _EXCEPTION_HANDLER_NO_ERRCODE(_BR_fault, 5)
    _EXCEPTION_HANDLER_NO_ERRCODE(_UD_fault, 6)
    _EXCEPTION_HANDLER_NO_ERRCODE(_NM_fault, 7)
    _EXCEPTION_HANDLER_ERRCODE(_DF_abort, 8)
    _EXCEPTION_HANDLER_ERRCODE(_TS_fault, 10)
    _EXCEPTION_HANDLER_ERRCODE(_NP_fault, 11)
    _EXCEPTION_HANDLER_ERRCODE(_SS_fault, 12)
    _EXCEPTION_HANDLER_ERRCODE(_GP_fault, 13)
    _EXCEPTION_HANDLER_ERRCODE(_PF_fault, 14)
    _EXCEPTION_HANDLER_NO_ERRCODE(_MF_fault, 16)
    _EXCEPTION_HANDLER_ERRCODE(_AC_fault, 17)
    _EXCEPTION_HANDLER_NO_ERRCODE(_MC_abort, 18)
    _EXCEPTION_HANDLER_NO_ERRCODE(_XF_fault, 19)
    
    _EXCEPTION_HANDLER_NO_ERRCODE(_UNUSED_1, 20)
    _EXCEPTION_HANDLER_NO_ERRCODE(_UNUSED_2, 21)
    _EXCEPTION_HANDLER_NO_ERRCODE(_UNUSED_3, 22)
    _EXCEPTION_HANDLER_NO_ERRCODE(_UNUSED_4, 23)
    _EXCEPTION_HANDLER_NO_ERRCODE(_UNUSED_5, 24)
    _EXCEPTION_HANDLER_NO_ERRCODE(_UNUSED_6, 25)
    _EXCEPTION_HANDLER_NO_ERRCODE(_UNUSED_7, 26)
    _EXCEPTION_HANDLER_NO_ERRCODE(_UNUSED_8, 27)
    _EXCEPTION_HANDLER_NO_ERRCODE(_UNUSED_9, 28)
    _EXCEPTION_HANDLER_NO_ERRCODE(_UNUSED_10, 29)
    _EXCEPTION_HANDLER_NO_ERRCODE(_UNUSED_11, 30)
    _EXCEPTION_HANDLER_NO_ERRCODE(_UNUSED_12, 31)
    
    _IRQ_HANDLER(32)   
    _IRQ_HANDLER(33)   
    _IRQ_HANDLER(34)   
    _IRQ_HANDLER(35)   
    _IRQ_HANDLER(36)   
    _IRQ_HANDLER(37)   
    _IRQ_HANDLER(38)   
    _IRQ_HANDLER(39)   

    _IRQ_HANDLER_10(4)
    _IRQ_HANDLER_10(5)
    _IRQ_HANDLER_10(6)
    _IRQ_HANDLER_10(7)
    _IRQ_HANDLER_10(8)
    _IRQ_HANDLER_10(9)
    _IRQ_HANDLER_10(10)
    _IRQ_HANDLER_10(11)
    _IRQ_HANDLER_10(12)
    _IRQ_HANDLER_10(13)
    _IRQ_HANDLER_10(14)
    _IRQ_HANDLER_10(15)
    _IRQ_HANDLER_10(16)
    _IRQ_HANDLER_10(17)
    _IRQ_HANDLER_10(18)
    _IRQ_HANDLER_10(19)
    _IRQ_HANDLER_10(20)
    _IRQ_HANDLER_10(21)
    _IRQ_HANDLER_10(22)
    _IRQ_HANDLER_10(23)
    _IRQ_HANDLER_10(24)

    _IRQ_HANDLER(250)   
    _IRQ_HANDLER(251)   
    _IRQ_HANDLER(252)   
    _IRQ_HANDLER(253)   
    _IRQ_HANDLER(254)   
    _IRQ_HANDLER(255)   

#undef _IRQ_HANDLER
#undef _IRQ_HANDLER_10
#undef _EXCEPTION_HANDLER_NO_ERRCODE
#undef _EXCEPTION_HANDLER_ERRCODE
}
