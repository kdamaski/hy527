.align	4
.globl	__swtch
.globl	_swtch
__swtch:              # _swtch has two arguments that are pointers
_swtch:
	subl	$16,%esp      # move stack pointer to fit 4 new 32bit values
	movl	%ebx,0(%esp)  # save three general purpose registers in this stack
	movl	%esi,4(%esp) 
	movl	%edi,8(%esp)
	movl	%ebp,12(%esp) # above arguments place the base pointer.
	movl	20(%esp),%eax # gets the from argument of _swtch into eax
	movl	%esp,0(%eax)  # place current sp into the *from argument
	movl	24(%esp),%eax # place the to argument of _swtch into eax
	movl	0(%eax),%esp  # place the *to value into esp
	movl	0(%esp),%ebx  # restore three general purpose registers from *to stack
	movl	4(%esp),%esi
	movl	8(%esp),%edi
	movl	12(%esp),%ebp # place the frame pointer of *to stack into current ebp
	addl	$16, %esp     # free current stack frame
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
