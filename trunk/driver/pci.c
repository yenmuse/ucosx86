#include "std.h"
#include "x86.h"
#include "io.h"
#include "pci.h"
#include "list.h"

LIST_HEAD(pci_device_list);

void pci_devices_scan()
{
    u32 i;
    u16 vid, pid;
    u32 classcode;
    u32 bus, slot, func;
    struct pci_device *dev;

    for(bus = 0; bus < 256; bus++)
    {
        for(slot = 0; slot < 32; slot++)
        {
            for(func = 0; func < 8; func++)
            {
                vid = read_pci_config_u16(bus, slot, func, 0x0);
                pid = read_pci_config_u16(bus, slot, func, 0x2);
                if((vid == 0xffff) || (vid == 0))
                    continue;
                
                dev = (struct pci_device*)kmalloc(sizeof(*dev));
                pci_device_init(dev, bus, slot, func);
                
                dev->vendor_id = pci_read_config_u16(dev, PCI_VENDOR_ID);
                dev->device_id = pci_read_config_u16(dev, PCI_DEVICE_ID);
                dev->subsystem_id = pci_read_config_u16(dev, PCI_SUBSYSTEM_ID);
                dev->subsystem_vendor_id = pci_read_config_u16(dev, PCI_SUBSYSTEM_VENDOR_ID);
                dev->interrupt_pin = pci_read_config_u8(dev, PCI_INTERRUPT_PIN);
                dev->interrupt_line = pci_read_config_u8(dev, PCI_INTERRUPT_LINE);
                dev->class = pci_read_config_u32(dev,PCI_CLASS_REVISION) >> 8; 

                for(i = 0; i < 6; i++)
                {
                    dev->res[i].start = pci_read_config_u32(dev, PCI_BASE_ADDRESS_0 + i * 4);
                }
                
                list_add_tail(&dev->list, &pci_device_list);
            }
        }
    }
}

void pci_dump_one_device(struct pci_device *dev)
{
    printk("PCI: %02x:%02x:%02x vid=%04x, pid=%04x, class = 0x%06x, intpin = %d, intline = %d\n" 
            " res = [%x,%x,%x,%x,%x,%x], cmd=%x\n",
            dev->bus, dev->slot, dev->func, dev->vendor_id, dev->device_id, 
            dev->class, dev->interrupt_pin, dev->interrupt_line,
            dev->res[0].start, 
            dev->res[1].start, 
            dev->res[2].start, 
            dev->res[3].start, 
            dev->res[4].start, 
            dev->res[5].start,
            pci_read_config_u16(dev, PCI_COMMAND)
    );
}

void dump_pci_device()
{
    struct list_head *p;
    struct pci_device *dev;
    u32 bus, slot, func;

    printk("---------------PCI List-------------\n");
    list_for_each(p, &pci_device_list)
    {
        dev = list_entry(p, struct pci_device, list);
        pci_dump_one_device(dev);
    }
    printk("------------------------------------\n");
}


