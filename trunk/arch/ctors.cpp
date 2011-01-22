#include "std.h"
#include "mem.h"
#include "utils.h"

extern "C" {
void *operator new(u32 size);
void *operator new[](u32 size);
void operator delete(void *p);
void operator delete[](void *p);
}

void *operator new(u32 size)
{
	printk("function new(%d) called!\n", size);
	return kmalloc(size);
}

void *operator new[](u32 size)
{
	printk("function new[](%d) called!\n", size);
	return kmalloc(size);
}

void operator delete(void *p)
{
	printk("function delete(%x) called!\n", p);
	kfree(p);
}

void operator delete[](void *p)
{
	printk("function delete(%x) called!\n", p);
	kfree(p);
}
