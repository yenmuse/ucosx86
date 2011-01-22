ENTRY(start_16)

/* MBR */
_bootsect_vma 	= 0x7c00;
_stage1_vma_cs	= 0x9000;
_kernel_vma_cs 	= 0x1000;
_kernel_vma 	= _kernel_vma_cs << 4;
SECTIONS
{
	. = 0;
	.loader : AT(0)
	{
		*(.boot.text);
		*(.boot.data);
		. = 510;
		SHORT(0xaa55);
		stage1_entry = .;
		*(.stage1.text);
		*(.stage1.rodata);
		*(.stage1.data);
		. = ALIGN(0x200);
	}
	_loader_size = SIZEOF(.loader);

	. = _kernel_vma;
	.kernel : 
	{
        	*(.entry32);
		*(.text);
		*(.text.*);
		*(.rodata);
		*(.rodata.*);
		*(.note*);
		
		. = ALIGN(4);
		__CTOR_LIST__ = .;
		LONG((__CTOR_END__ - __CTOR_LIST__) / 4 - 2);
		*(SORT(.ctors));
		LONG(0);
		__CTOR_END__ = .;

		. = ALIGN(4);
		__DTOR_LIST__ = .;
		LONG((__DTOR_END__ - __DTOR_LIST__) / 4 - 2);
		*(SORT(.dtors));
		LONG(0);
		__DTOR_END__ = .;

		*(.data);
		. = ALIGN(512);
	}
	_kernel_sectors = (SIZEOF(.kernel) + 511) >> 9;	

    .kernel_bss :
    {
		_kernel_bss = .;
        *(.tbss);
		*(.bss);
		*(.common);
		*(.COMMON);
		*(*COM*);
		. = ALIGN(512);
        ASSERT(. < 0xa0000 , "Memory Overflow to 0xa0000");
		_kernel_bss_end = .;
    }
/*
	/DISCARD/ :
	{
		*(*);
	}
*/
}
