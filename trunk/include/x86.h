#ifndef _H_X86_H
#define _H_X86_H
#include "std.h"
#include "pm.h"
#include "pci.h"

struct e820_entry
{
	u32 base_low;
	u32 base_high;
	u32 length_low;
	u32 length_high;
	u32 type;
};

struct e820_map
{
	u32 count;
	struct e820_entry item[256];
};

#define panic(fmt, args...)	\
	do{printk(fmt, ##args); while(1);}while(0)

void init_idt_table(void);
void init_idt_desc(u8 vector, u8 type, void (*handler)(void), u8 dpl);

#endif
