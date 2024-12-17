.align	4
.globl	__swtch
.globl	_swtch
__swtch:
_swtch:
	subl	$24,%esp  		# allocate stack frame
	movl	%ebx,0(%esp)	# save general purpose regs in ("from") stack frame
	movl	%esi,4(%esp)	# $esi holds the func. We create such stack
	movl	%edi,8(%esp)  # $edi holds the *args of func
	movl	%ebp,12(%esp)	# save stack base ptr
	movl	%ecx,16(%esp)
	movl	%edx,20(%esp)
	movl	28(%esp),%eax	# get from argument 
	movl	%esp,0(%eax)	# save esp in from argument 
	movl	32(%esp),%eax	# get to argument
	movl	0(%eax),%esp	# restore stack pointer for "to" thread (actual swtch)
	movl	0(%esp),%ebx	# restore general purpose regs from ("to") strack frame
	movl	4(%esp),%esi
	movl	8(%esp),%edi
	movl	12(%esp),%ebp	# restore stack base pointer
	movl	16(%esp),%ecx
	movl	20(%esp),%edx
	addl	$24, %esp			# free current stack frame
	ret						      # return to address in stack (_thrstart) manually planted
.align	4
.globl	__thrstart
.globl	_thrstart
__thrstart:
_thrstart:
	pushl	%edi
	call	*%esi
	pushl	%eax
	call	Thread_exit
.globl	__ENDMONITOR
.globl	_ENDMONITOR
__ENDMONITOR:
_ENDMONITOR:
