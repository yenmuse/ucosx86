#include "std.h"
#include "pm.h"
#include "io.h"
#include "x86.h"
#include "irq.h"
#include "mem.h"
#include "utils.h"
#include "includes.h"
#include <stdarg.h>

/* system global var */
u32 sys_free_mem = 0, sys_free_mem_end = 0;
gdt_desc_t sys_gdt_table[] = {
    {0x0, 0x0},                         /* first segment */
    {(0x0 << 16) | 0xffff, 0x00df9800}, /* code segment */
    {(0x0 << 16) | 0xffff, 0x00df9300}, /* data segment */
};
idt_desc_t sys_idt_table[256] = {0,};
struct e820_map sys_e820_map = {0,};
/* system global var end */


static void timer_handler(struct irq_ctx *ctx)
{
    OSTimeTick();
    outb(0x20, INT_VECTOR_IRQ0);
}

static void soft80_handler(struct irq_ctx *ctx)
{

}

static void restart(void)
{
    int i;
    u32 pgdtr;
    idtr_t idtr = {sizeof(sys_idt_table), (u32)sys_idt_table}; 
    gdtr_t  gdtr = {sizeof(sys_gdt_table), (u32)sys_gdt_table};
    struct e820_map *e820 = (struct e820_map*)0x90000;

    /* init the e820 map */
    memcpy(&sys_e820_map, e820, sizeof(struct e820_map) + e820->count * sizeof(struct e820_entry)); 

    /* init the free_memory */
    e820 = &sys_e820_map;
    sys_free_mem = 0x0;
    sys_free_mem_end = 0x0;

    for(i = 0; i < e820->count; i++)
    {
        if((e820->item[i].base_low != 0) && (e820->item[i].type == 1))
        {
            sys_free_mem = e820->item[i].base_low;
            sys_free_mem_end = sys_free_mem + e820->item[i].length_low;
            break;
        }
    }
    if(sys_free_mem_end == 0x0)
        panic("free_mem_end detect error!\n");

    printk("free_mem_start = %x, free_mem_end = %x\n",
       sys_free_mem,sys_free_mem_end);

    /* change cs, ds, es, gs segment register */  
    asm(
        "lgdtw (%0);\r\n"
        "ljmpl $8, $1f;\r\n"
        "1:nop;\r\n"
        :   /* output define */
        : "a"(&gdtr)  /* input define */
    );

    /* reload ds, es, ss */
    asm(
        "mov $16, %%ax;\r\n"
        "mov %%ax, %%ss;\r\n"
        "mov %%ax, %%ds;\r\n"
        "mov %%ax, %%es;\r\n"
        "mov %%ax, %%gs;\r\n"
        :   /* output define */
        :   /* input define */
    );
    /* init 8259 */
    init_8259A();

    /* init Interrupt table register */
    asm(
        "lidtw (%0);\r\n"
        :
        : "a"(&idtr)
    );

    init_idt_table();
}

void dump_e820_map(void)
{

    int i;
    u32 type;
    char* e820_type[] = {
        "Invalid",
        "AddressRange Memory",
        "AddressRageReserved",
        "Undefined"
    };
    struct e820_map *e820 = &sys_e820_map;

    printk("memory layout: \n");
    for(i = 0; i < e820->count; i++)
    {
        type = e820->item[i].type;
        printk("    %08x - %08x : %d (%s)\n", 
            e820->item[i].base_low, 
            e820->item[i].base_low + e820->item[i].length_low, 
            type,
            (type > 3) ? e820_type[3] : e820_type[type]
        );
    }
}

static void cpp_env_init(void)
{
	extern void (*__CTOR_LIST__[])(void);

	u32 i;
	u32 nptrs = (u32)__CTOR_LIST__[0];

	printk("nptrs = %d\n", nptrs);

	for(i = nptrs; i >= 1; i--)
	{
		printk("__CTOR_LIST__[%d] = 0x%x\n", i, __CTOR_LIST__[i]);
		__CTOR_LIST__[i]();
	}
}

void Task1(void *ctx)
{
    char ch = 'a';
    asm("sti;\r\n");
  
	cpp_env_init();
//	Test();
    fs_main();
//    dump_e820_map();
    while(1)
    {
        putch_at(ch, 10, 60);
        if(ch == 'z')
            ch = 'a';
        else
            ch++;
        OSTimeDly(10);
    }
}

//static u8 sector0[512] = {0,};

void Task2(void *ctx)
{
    u32 i;
    void *p = NULL;
    char ch = 'A';
    asm("sti;\r\n");

//    show_mem();
//    dump_pci_device();
    while(1)
    {
        putch_at(ch, 20, 60);
        if(ch == 'Z')
            ch = 'A';
        else
            ch++;
    }
}

struct ucos_task
{
    u32 prior;
    void (*route)(void *);
    u32 stack_size;
    void *context;
    OS_STK* stack_base;
}ucos_task_table[] = {
    {3, Task1, 1024},
    {4, Task2, 1024}
};

void ucos_task_init(void)
{
    OS_STK *stk;
    struct page *page;
    struct ucos_task *task;

    for(task = ucos_task_table; 
            task < ucos_task_table + sizeof(ucos_task_table) / sizeof(ucos_task_table[0]);
            task++)
    {
        stk = (OS_STK*)kmalloc(task->stack_size);
        if(!stk)
            panic("memory limit!\n"); 
        task->stack_base = stk;
        OSTaskCreate(task->route, task->context, 
                task->stack_base + task->stack_size/sizeof(OS_STK), task->prior);
    }
}

int c_entry(void)
{
    extern u8 _kernel_bss;
    extern u8 _kernel_bss_end;
    
    /* init bss segment */
    memset((void*)&_kernel_bss, 0, (u32)(&_kernel_bss_end - &_kernel_bss));

    /* init video memory */
    memset((void*)0xb8000, 0x0, 80 * 25 * 2);

    /* setup gdt, idt table */
    restart();

    /* setup buddy system */
    mem_init(sys_free_mem, sys_free_mem_end);

    /* register interrupt 80 for ucos */
    register_handler(0x80, soft80_handler);

    /* register timer handler */
    register_handler(0x20, timer_handler); 

    /* set 8259 interrupt mask */
    set_mask_8259A(0xfffe);
//    set_mask_8259A(0x0);

    /* scan all pci devices */
    pci_devices_scan();

    OSInit();
    ucos_task_init();
    OSStart();
    
    return 0;
}
