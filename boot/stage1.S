.section .stage1.text
#include "pm.h"

.extern _kernel_sectors
.extern _kernel_vma_cs
.extern test

#define TEMP_SS		0x0
#define TEMP_SP		0x7e00

.macro Descriptor	Base, Limit, Attr
	.2byte	\Limit & 0xffff
	.2byte \Base & 0xff
	.byte (\Base >> 16) & 0xff
	.2byte ((\Limit >> 8) & 0xf00) | (\Attr & 0xf0ff)
	.byte (\Base >> 24) & 0xff
.endm

.code16
stage1_entry:
	mov %cs,%ax
	mov %ax,%ds
	mov %ax,%es

	mov $TEMP_SS, %ax
	mov %ax,%ss
	mov $TEMP_SP, %sp

	/* load kernel */
	mov $_kernel_vma_cs, %ax
	mov %ax, %es
	mov $0, %bx

	mov $_kernel_sectors, %cx

	mov $128, %ax
	call load_sector

	call loade820

	/* enter protected mode */
	lgdtw GdtPtr
	cli

	/* enable A20 */
	inb $0x92, %al
	orb $0b00000010, %al
	outb %al, $0x92

	/* switch to protected mode */
	movl %cr0, %eax
	or $1, %eax
	movl %eax, %cr0

	ljmpl $8, $0x10000

loade820:
	push %bp
	push %di
	push %ebx
    push %es

    mov $0x9000, %ax
    mov %ax, %es

	xor %ebx, %ebx
	movl %ebx, 0x0
	mov $0x4, %di
1:
	movl $0xe820, %eax
	movl $20, %ecx
	movl $0x534d4150, %edx
	int $0x015
	jc 2f
	add $20, %di
	incl 0x0
	cmp $0, %ebx
	jne 1b
	jmp 3f
2:
	movl $0, 0x0
3:
    pop %es
	pop %ebx
	pop %di
	pop %bp
	ret

/*read %cl sectors from %ax sector(floppy) to %es:%bx(memory)*/
load_sector:
    push %bp
    mov %sp, %bp
    push %bx

    xor %dx, %dx
    mov %cl, %dl
    mov $1, %cl
    
1:
    call load_sector_1
    add $512, %bx
    jnz 2f

    push %ax
    mov %es, %ax
    add $0x1000, %ax
    mov %ax, %es
    pop %ax
2:
    inc %ax
    sub $1, %dx
    jne 1b 

    pop %bx
    mov %bp, %sp
    pop %bp
    ret

/*read %cl sectors from %ax sector(floppy) to %es:%bx(memory)*/
load_sector_1:
	push %bp
	mov %sp,%bp
	sub $4,%sp
	push %bx
    push %cx
    push %dx
    push %ax

    mov %bx, -4(%bp) 
	mov %cl,-2(%bp)
	mov $18, %bl
	div %bl
    mov -4(%bp), %bx
	inc %ah
	mov %ah,%cl
	mov %al,%dh
	shr $1,%al
	mov %al,%ch
	and $1,%dh
	and $1,%dh
	mov $0,%dl	/* BS_DrvNum */
1:
	mov $2,%ah
	mov -2(%ebp),%al
	int $0x13
	jc 1b 

    pop %ax
    pop %dx
    pop %cx
	pop %bx
    mov %bp, %sp
	pop %bp
	ret

message: 		.ascii "OS Loading...!"
message_end:	.byte 0

/* Global Descript Table */
LABEL_GDT:			DESC(0,	0, 0)
LABEL_DESC_CODE32:	DESC(0,	0xffff,	(DA_C + DA_32 + DA_G))
LABEL_DESC_DATA32:	DESC(0,	0xffff,	(DA_DRW + DA_G + DA_32))
LABEL_DESC_STACK32:	DESC(0,	0xffff,	(DA_DRW + DA_G + DA_32))
LABEL_DESC_LAST	:	DESC(0,0,0)	 

GdtPtr:				GDT(0x90000 + LABEL_GDT, LABEL_DESC_LAST - LABEL_GDT)

.section .entry32
.global kernel_entry
.extern OSIntHandler
.extern c_entry
.code32

/* because the stack is invalid here, so this function should not written by C code */
kernel_entry:
	movl $LABEL_DESC_DATA32 - LABEL_GDT, %eax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %gs
	mov $LABEL_DESC_STACK32 - LABEL_GDT, %ax
	mov %ax, %ss
	movl $0x90000 , %esp
	call c_entry
	jmp .

#define _EXCEPTION_HANDLER_ERRCODE(name,vector)  \
    .global name;    \
    .align 16; \
    name:   \
        pushl $vector;   \
        jmp OSIntHandler;

#define _EXCEPTION_HANDLER_NO_ERRCODE(name, vector)    \
    .global name;    \
    .align 16; \
    name:   \
        pushl $0xffffffff;    \
        pushl $vector;   \
        jmp OSIntHandler;

#define _IRQ_HANDLER(vector)   \
    _EXCEPTION_HANDLER_NO_ERRCODE(_IRQ_##vector, vector) 

#define _IRQ_HANDLER_10(vector)   \
    _EXCEPTION_HANDLER_NO_ERRCODE(_IRQ_##vector##0, vector##0) \ 
    _EXCEPTION_HANDLER_NO_ERRCODE(_IRQ_##vector##1, vector##1) \ 
    _EXCEPTION_HANDLER_NO_ERRCODE(_IRQ_##vector##2, vector##2) \ 
    _EXCEPTION_HANDLER_NO_ERRCODE(_IRQ_##vector##3, vector##3) \ 
    _EXCEPTION_HANDLER_NO_ERRCODE(_IRQ_##vector##4, vector##4) \ 
    _EXCEPTION_HANDLER_NO_ERRCODE(_IRQ_##vector##5, vector##5) \ 
    _EXCEPTION_HANDLER_NO_ERRCODE(_IRQ_##vector##6, vector##6) \ 
    _EXCEPTION_HANDLER_NO_ERRCODE(_IRQ_##vector##7, vector##7) \ 
    _EXCEPTION_HANDLER_NO_ERRCODE(_IRQ_##vector##8, vector##8) \ 
    _EXCEPTION_HANDLER_NO_ERRCODE(_IRQ_##vector##9, vector##9) 

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
