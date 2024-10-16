.align	4
.globl	__swtch
.globl	_swtch
__swtch:              # _swtch has two arguments that are pointers
_swtch:
	subl	$32,%esp      # move stack pointer to fit 4 new 32bit values
	movl	%eax,0(%esp)  # save general purpose registers in this stack
	movl	%ebx,4(%esp)
	movl	%ecx,8(%esp)
	movl	%edx,12(%esp)
	movl	%esi,16(%esp) 
	movl	%edi,20(%esp)
	movl	%ebp,24(%esp) # above arguments place the base pointer.
	movl	36(%esp),%eax # gets the from argument of _swtch into eax
	movl	%esp,0(%eax)  # place current sp into the *from argument
	movl	40(%esp),%eax # place the to argument of _swtch into eax
	movl	0(%eax),%esp  # place the *to value into esp
	movl	0(%esp),%eax  # restore general purpose registers from *to stack
	movl	4(%esp),%ebx
	movl	8(%esp),%ecx
	movl	12(%esp),%edx
	movl	16(%esp),%esi
	movl	20(%esp),%edi
	movl	24(%esp),%ebp # place the frame pointer of *to stack into current ebp
	addl	$32, %esp     # free current stack frame
	ret                 # return to address in stack ("to" return address)
.align	4
.globl	__thrstart
.globl	_thrstart
__thrstart:
_thrstart:
	pushl	%edi          # edi register holds void *args of my threads 
	call	*%esi         # esi holds the func and here we call it
	pushl	%eax          # eax here holds the return value of the thread func which is an integer
	call	Thread_exit   # call thread exit with the exit code from eax
.globl	__ENDMONITOR
.globl	_ENDMONITOR
__ENDMONITOR:
_ENDMONITOR:
