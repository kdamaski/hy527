.align	4
.globl	__swtch
.globl	_swtch
__swtch:
_swtch:
  subq  $80, %rsp           # Allocate stack frame (align to 16 bytes)
  movq  %rbx, 0(%rsp)       # Save general-purpose registers in "from" stack frame
  movq  %rsi, 8(%rsp)       # Save %rsi (function pointer)
  movq  %rdi, 16(%rsp)      # Save %rdi (*args for function)
  movq  %rbp, 24(%rsp)      # Save base pointer
	movq	%rcx,32(%rsp)
	movq	%rdx,40(%rsp)
	movq	56(%rsp),%rax	# get from argument 
	movq	%rsp,0(%rax)	# save esp in from argument 
	movq	64(%rsp),%rax	# get to argument
	movq	0(%rax),%rsp	# restore stack pointer for "to" thread (actual swtch)
	movq	0(%rsp),%rbx	# rrstorr grnrral purposr rrgs from ("to") strack framr
	movq	8(%rsp),%rsi
	movq	16(%rsp),%rdi
	movq	24(%rsp),%rbp	# rrstorr stack basr pointrr
	movq	32(%rsp),%rcx
	movq	40(%rsp),%rdx
	addl	$80, %rsp			# frrr currrnt stack framr
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
