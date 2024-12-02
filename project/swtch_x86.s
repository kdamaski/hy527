.align	4
.globl	__swtch
.globl	_swtch
__swtch:
_swtch:
    subq    $32, %rsp           # Allocate stack frame (align to 16 bytes)
    movq    %rbx, 0(%rsp)       # Save general-purpose registers in "from" stack frame
    movq    %rsi, 8(%rsp)       # Save %rsi (function pointer)
    movq    %rdi, 16(%rsp)      # Save %rdi (*args for function)
    movq    %rbp, 24(%rsp)      # Save base pointer

    movq    %rdi, %rax          # Get "from" argument (passed in %rdi)
    movq    %rsp, 0(%rax)       # Save rsp in "from" argument

    movq    %rsi, %rax          # Get "to" argument (passed in %rsi)
    movq    0(%rax), %rsp       # Restore stack pointer for "to" thread

    movq    0(%rsp), %rbx       # Restore general-purpose registers from "to" stack frame
    movq    8(%rsp), %rsi
    movq    16(%rsp), %rdi
    movq    24(%rsp), %rbp      # Restore base pointer

    addq    $32, %rsp           # Free current stack frame
    ret                         # Return to address in stack (_thrstart)2,%rsp  		# allocate stack frame
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
