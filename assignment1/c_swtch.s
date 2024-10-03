.align	8
.globl	__swtch
.globl	_swtch
__swtch:
_swtch:
	subq	$32,%rsp
	movq	%rbx,0(%rsp)
	movq	%rsi,4(%rsp)
	movq	%rdi,8(%rsp)
	movq	%rbp,12(%rsp)
	movq	40(%rsp),%rax
	movq	%rsp,0(%rax)
	movq	48(%rsp),%rax
	movq	0(%rax),%rsp
	movq	0(%rsp),%rbx
	movq	8(%rsp),%rsi
	movq	16(%rsp),%rdi
	movq	24(%rsp),%rbp
	addq	$32, %rsp
	ret
.align	8
.globl	__thrstart
.globl	_thrstart
__thrstart:
_thrstart:
	pushq	%rdi
	call	*%rsi
	pushq	%rax
	call	Thread_exit
.globl	__ENDMONITOR
.globl	_ENDMONITOR
__ENDMONITOR:
_ENDMONITOR:
