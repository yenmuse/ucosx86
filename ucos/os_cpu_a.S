//;********************************************************************************************************
//;                                               uC/OS-II
//;                                         The Real-Time Kernel
//;
//;                          (c) Copyright 1992-2002, Jean J. Labrosse, Weston, FL
//;                                          All Rights Reserved
//;
//;
//;                                       80x86/80x88 Specific code
//;                                          LARGE MEMORY MODEL
//;
//;                                           Borland C/C++ V4.51
//;                                       (IBM/PC Compatible Target)
//;
//; File         : OS_CPU_A.ASM
//; By           : Jean J. Labrosse
//;********************************************************************************************************
//
//;********************************************************************************************************
//;                                    PUBLIC and EXTERNAL REFERENCES
//;********************************************************************************************************
.section .text

.global OSStartHighRdy
.global OSIntCtxSw
.global OSIntHandler
.global user_isr

.extern OSIntExit
.extern OSIntEnter
.extern OSTimeTick
.extern OSTaskSwHook

.extern OSIntNesting
.extern OSPrioHighRdy
.extern OSPrioCur
.extern OSRunning
.extern OSTCBCur
.extern OSTCBHighRdy

.code32
OSIntHandler:
    pusha
    pushl $0
    call OSIntEnter
    movl OSTCBCur, %ebx
    movb OSIntNesting, %al

    dec %al
    jnz 1f
    movl %esp, (%ebx)
1:
    call user_isr
    call OSIntExit
    popl %eax
    popa
    addl $8, %esp
    iret
    
OSStartHighRdy:
    call OSTaskSwHook
    movb $1, OSRunning
    movl OSTCBHighRdy, %ebx
    movl (%ebx), %esp
    popl %eax
    popa
    addl $8, %esp
    iret

OSIntCtxSw:
    CALL   OSTaskSwHook 
    
    movl OSPrioHighRdy, %eax
    movl %eax, OSPrioCur
    
    movl OSTCBHighRdy, %eax
    movl %eax, OSTCBCur
    
    movl OSTCBHighRdy, %ebx
    movl (%ebx), %esp
    
    popl %eax
    popa
    
    addl $8, %esp
    iret

