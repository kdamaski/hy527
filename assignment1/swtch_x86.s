.align	4
.globl	__swtch
.globl	_swtch
__swtch:
_swtch:
	subl	$16,%esp  		# allocate stack frame
	movl	%ebx,0(%esp)	# save general purpose regs in ("from") stack frame
	movl	%esi,4(%esp)		
	movl	%edi,8(%esp)
	movl	%ebp,12(%esp)	# save stack base ptr
	movl	20(%esp),%eax	# get from argument 
	movl	%esp,0(%eax)	# save esp in from argument 
	movl	24(%esp),%eax	# get to argument
	movl	0(%eax),%esp	# restore stack pointer for "to" thread
	movl	0(%esp),%ebx	# restore general purpose regs from ("to") strack frame
	movl	4(%esp),%esi
	movl	8(%esp),%edi
	movl	12(%esp),%ebp	# restore stack base pointer
	addl	$16, %esp			# free current stack frame
	ret						      # return to address in stack ("to" return address)
.align	4
.globl	__thrstart
.globl	_thrstart
__thrstart:
_thrstart:
	pushl	%edi
	call	      *%esi
	pushl	%eax
	call	Thread_exit
.globl	__ENDMONITOR
.globl	_ENDMONITOR
__ENDMONITOR:
_ENDMONITOR:
