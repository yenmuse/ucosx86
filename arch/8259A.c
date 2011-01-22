#include "std.h"
#include "x86.h"
#include "io.h"
#include "irq.h"

void init_8259A()
{
    /* Master 8259, ICW1 */
    outb(0x11, INT_M_CTL);
    
    /* Slave 8259, ICW1 */
    outb(0x11, INT_S_CTL);

    /* Master irqnum from INT_VECTOR_IRQ0 */
    outb(INT_VECTOR_IRQ0, INT_M_CTLMASK);

    /* Slave irqnum from INT_VECTOR_IRQ0 */
    outb(INT_VECTOR_IRQ0 + 8, INT_S_CTLMASK);

    /* Master ICW3, IR2 connect to Slave */
    outb(0x4, INT_M_CTLMASK);

    /* Slave ICW3,connect to Master IR2 */
    outb(0x2, INT_S_CTLMASK);

    /* Master ICW4 */
    outb(0x1, INT_M_CTLMASK);

    /* Slave ICW4 */
    outb(0x1, INT_S_CTLMASK);

    /* Master Mask all interrupt */
    outb(0xff, INT_M_CTLMASK);

    /* Slave Mask all interrupt */
    outb(0xff, INT_S_CTLMASK);
}

void set_mask_8259A(u16 mask)
{
    outb((u8)(mask & 0xff), INT_M_CTLMASK);
    outb((u8)((mask >> 8) & 0xff), INT_S_CTLMASK);
}
