.align	4
.globl	__swtch
.globl	_swtch
__swtch:
_swtch:
    subq    $192, %rsp               
    movq    %rbx, 128(%rsp)           
    movq    %rsi, 136(%rsp)          
    movq    %rdi, 144(%rsp)          
    movq    %r12, 152(%rsp)
    movq    %r13, 160(%rsp)
    movq    %r14, 168(%rsp)
    movq    %r15, 176(%rsp)          
    movq    %rbp, 184(%rsp)

    movq    %rdi, %rax              
    movq    %rsp, 0(%rax)           

    movq    %rsi, %rax              
    movq    0(%rax), %rsp           

    movq    128(%rsp), %rbx           
    movq    136(%rsp), %rsi          
    movq    144(%rsp), %rdi          
    movq    152(%rsp), %r12
    movq    160(%rsp), %r13
    movq    168(%rsp), %r14
    movq    176(%rsp), %r15          
    movq    184(%rsp), %rbp

    addq    $192, %rsp               
    ret                   
.align	4
.globl	__thrstart
.globl	_thrstart
__thrstart:
_thrstart:
	pushq	%rdi
	call  *%rsi
	pushq	%rax
	call	Thread_exit
.globl	__ENDMONITOR
.globl	_ENDMONITOR
__ENDMONITOR:
_ENDMONITOR:
