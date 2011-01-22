#ifndef _H_MEM_H
#define _H_MEM_H
#include "std.h"
#include "list.h"

#define PAGE_SIZE   (4096)
#define PAGE_MASK   (~(PAGE_SIZE - 1))
#define PAGE_SHIFT  (12)
#define MAP_NR(addr)    ((addr) >> PAGE_SHIFT)
#define PAGE_ALIGN(addr)    ((((addr) + PAGE_SIZE - 1) & PAGE_MASK))

struct page
{
    struct list_head list;
};

#ifdef __cplusplus
extern "C" {
#endif

void mem_init(u32, u32);
u32 page_address(struct page *page);
struct page* address_page(u32 addr);
struct page* alloc_page(int order);
void free_page(struct page*, int order);
void show_mem(void);

void kmalloc_init(void);
void kfree(void *p);
void *krealloc(void *p, int size);
void * kmalloc(int size);

#ifdef __cplusplus
}
#endif
#endif
