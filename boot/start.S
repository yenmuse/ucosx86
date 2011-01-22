.section .boot.text
.global start_16

#define TEMP_SP			0x8000

.extern _stage1_vma_cs
.extern _loader_size

.code16
start_16:
	jmp _start_16
	nop

_start_16:
	mov %cs, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %ss
	mov $TEMP_SP, %ax
	mov %ax, %sp

	mov $_loader_size,%cx
	shr $9, %cx
	mov $_stage1_vma_cs,%ax
	mov %ax,%es
	mov $0,%bx
	mov $0,%ax
	call read_sector	

	ljmp $0x9000,$0x0200

/*read %cl sectors from %ax sector(floppy) to %es:%bx(memory)*/
read_sector:
	push %bp
	mov %sp,%bp
	sub $2,%sp
	mov %cl,-2(%bp)
	push %bx
	mov $18, %bl
	div %bl
	inc %ah
	mov %ah,%cl
	mov %al,%dh
	shr $1,%al
	mov %al,%ch
	and $1,%dh
	and $1,%dh
	pop %bx
	mov $0,%dl	/* BS_DrvNum */
1:
	mov $2,%ah
	mov -2(%ebp),%al
	int $0x13
	jc 1b
	add $2,%sp
	pop %bp
	ret

