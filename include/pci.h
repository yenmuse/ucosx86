#ifndef _H_PCI_H
#define _H_PCI_H
#include "std.h"
#include "utils.h"
#include "pci_regs.h"
#include "pci_ids.h"
#include "list.h"
#include "io.h"

#define IO_RESOURCE     (0x1)
#define MEM_RESOURCE    (0x2)

struct resource
{
    u32 type;   /* IO or MEM */
    u32 start;
    u32 length;
};

struct pci_device
{
    u16 vendor_id, device_id;
    u16 subsystem_vendor_id;
    u16 subsystem_id;
    u8 interrupt_pin, interrupt_line;
    u32 class;

    u8 bus, slot, func;
    u8 res_cnt;
    struct resource res[6];
    struct list_head list;
};

static inline void pci_device_init(struct pci_device *dev, u8 bus, u8 slot, u8 func)
{
    memset((void*)dev, 0, sizeof(*dev));
    INIT_LIST_HEAD(&dev->list);
    dev->bus = bus;
    dev->slot = slot;
    dev->func = func;
}

#define _out_u32(val, addr)  outl((val), (addr))
#define _out_u16(val, addr)  outw((val), (addr))
#define _out_u8(val, addr)  outb((val), (addr))
#define _in_u32(addr)  inl((addr))
#define _in_u32(addr)  inl((addr))
#define _in_u16(addr)  inw((addr))
#define _in_u8(addr)  inb((addr))

#define _GEN_PCI_WRITE_FUNC(type)    \
    static inline void write_pci_config_##type(u8 bus, u8 slot, u8 func, u8 offset, type val)   \
    {   \
        outl(0x80000000 | (bus << 16) | (slot << 11) | (func << 8) | offset, 0xcf8);    \
        _out_##type(val, 0xcfc + (offset & (4 - sizeof(type)))); \
    }   \
    static inline void pci_write_config_##type(struct pci_device *dev, u8 offset, type val)  \
    {   \
        write_pci_config_##type(dev->bus, dev->slot, dev->func, offset, val);   \
    }

#define _GEN_PCI_READ_FUNC(type)    \
    static inline type read_pci_config_##type(u8 bus, u8 slot, u8 func, u8 offset)   \
    {   \
        outl(0x80000000 | (bus << 16) | (slot << 11) | (func << 8) | offset, 0xcf8);    \
        return _in_##type(0xcfc + (offset & (4 - sizeof(type)))); \
    }   \
    static inline type pci_read_config_##type(struct pci_device *dev, u8 offset)  \
    {   \
        return read_pci_config_##type(dev->bus, dev->slot, dev->func, offset);   \
    }

_GEN_PCI_WRITE_FUNC(u32)
_GEN_PCI_WRITE_FUNC(u16)
_GEN_PCI_WRITE_FUNC(u8)

_GEN_PCI_READ_FUNC(u32)
_GEN_PCI_READ_FUNC(u16)
_GEN_PCI_READ_FUNC(u8)

void pci_devices_scan();
void dump_pci_device();
#endif
