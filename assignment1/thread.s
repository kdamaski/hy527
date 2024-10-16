	.file	"thread.c"
	.text
	.globl	_STACK
	.bss
	.align 4
	.type	_STACK, @object
	.size	_STACK, 4
_STACK:
	.zero	4
	.globl	n_threads
	.align 4
	.type	n_threads, @object
	.size	n_threads, 4
n_threads:
	.zero	4
	.globl	thr_q
	.align 4
	.type	thr_q, @object
	.size	thr_q, 4
thr_q:
	.zero	4
	.section	.rodata
	.align 4
.LC0:
	.string	"Thread_init has already been called\n"
	.align 4
.LC1:
	.string	"malloc failed to allocate space\n"
	.text
	.globl	Thread_init
	.type	Thread_init, @function
Thread_init:
.LFB6:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	pushl	%ebx
	subl	$4, %esp
	.cfi_offset 3, -12
	call	__x86.get_pc_thunk.bx
	addl	$_GLOBAL_OFFSET_TABLE_, %ebx
	movl	thr_q@GOTOFF(%ebx), %eax
	testl	%eax, %eax
	je	.L2
	movl	stderr@GOT(%ebx), %eax
	movl	(%eax), %eax
	pushl	%eax
	pushl	$36
	pushl	$1
	leal	.LC0@GOTOFF(%ebx), %eax
	pushl	%eax
	call	fwrite@PLT
	addl	$16, %esp
	jmp	.L1
.L2:
	subl	$12, %esp
	pushl	$8
	call	malloc@PLT
	addl	$16, %esp
	movl	%eax, thr_q@GOTOFF(%ebx)
	movl	thr_q@GOTOFF(%ebx), %eax
	testl	%eax, %eax
	jne	.L4
	movl	stderr@GOT(%ebx), %eax
	movl	(%eax), %eax
	pushl	%eax
	pushl	$32
	pushl	$1
	leal	.LC1@GOTOFF(%ebx), %eax
	pushl	%eax
	call	fwrite@PLT
	addl	$16, %esp
	subl	$12, %esp
	pushl	$1
	call	exit@PLT
.L4:
	movl	thr_q@GOTOFF(%ebx), %eax
	movl	$0, 4(%eax)
	movl	thr_q@GOTOFF(%ebx), %edx
	movl	4(%eax), %eax
	movl	%eax, (%edx)
	subl	$4, %esp
	pushl	$65536
	pushl	$16
	leal	_STACK@GOTOFF(%ebx), %eax
	pushl	%eax
	call	posix_memalign@PLT
	addl	$16, %esp
.L1:
	movl	-4(%ebp), %ebx
	leave
	.cfi_restore 5
	.cfi_restore 3
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE6:
	.size	Thread_init, .-Thread_init
	.globl	Thread_self
	.type	Thread_self, @function
Thread_self:
.LFB7:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	call	__x86.get_pc_thunk.ax
	addl	$_GLOBAL_OFFSET_TABLE_, %eax
	movl	thr_q@GOTOFF(%eax), %eax
	movl	(%eax), %eax
	movl	4(%eax), %eax
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE7:
	.size	Thread_self, .-Thread_self
	.section	.rodata
	.align 4
.LC2:
	.string	"thread %u exited with code %d\n"
	.text
	.globl	Thread_exit
	.type	Thread_exit, @function
Thread_exit:
.LFB8:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	pushl	%ebx
	subl	$4, %esp
	.cfi_offset 3, -12
	call	__x86.get_pc_thunk.bx
	addl	$_GLOBAL_OFFSET_TABLE_, %ebx
	movl	thr_q@GOTOFF(%ebx), %eax
	movl	(%eax), %eax
	movl	4(%eax), %eax
	subl	$4, %esp
	pushl	8(%ebp)
	pushl	%eax
	leal	.LC2@GOTOFF(%ebx), %eax
	pushl	%eax
	call	printf@PLT
	addl	$16, %esp
	call	thr_dequeue@PLT
	nop
	movl	-4(%ebp), %ebx
	leave
	.cfi_restore 5
	.cfi_restore 3
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE8:
	.size	Thread_exit, .-Thread_exit
	.section	.rodata
	.align 4
.LC3:
	.string	"Thread %u cannot pause since there is no other job to run\n"
	.text
	.globl	Thread_pause
	.type	Thread_pause, @function
Thread_pause:
.LFB9:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	pushl	%ebx
	subl	$20, %esp
	.cfi_offset 3, -12
	call	__x86.get_pc_thunk.bx
	addl	$_GLOBAL_OFFSET_TABLE_, %ebx
	movl	thr_q@GOTOFF(%ebx), %eax
	movl	(%eax), %eax
	movl	12(%eax), %eax
	testl	%eax, %eax
	je	.L9
	movl	thr_q@GOTOFF(%ebx), %eax
	movl	(%eax), %eax
	movl	(%eax), %eax
	movl	%eax, -16(%ebp)
	movl	thr_q@GOTOFF(%ebx), %eax
	movl	(%eax), %eax
	movl	12(%eax), %eax
	movl	(%eax), %eax
	movl	%eax, -12(%ebp)
	call	thr_dequeue@PLT
	subl	$12, %esp
	pushl	%eax
	call	thr_enqueue@PLT
	addl	$16, %esp
	subl	$8, %esp
	pushl	-12(%ebp)
	pushl	-16(%ebp)
	call	_swtch@PLT
	addl	$16, %esp
	jmp	.L11
.L9:
	movl	thr_q@GOTOFF(%ebx), %eax
	movl	(%eax), %eax
	movl	4(%eax), %eax
	subl	$8, %esp
	pushl	%eax
	leal	.LC3@GOTOFF(%ebx), %eax
	pushl	%eax
	call	printf@PLT
	addl	$16, %esp
.L11:
	nop
	movl	-4(%ebp), %ebx
	leave
	.cfi_restore 5
	.cfi_restore 3
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE9:
	.size	Thread_pause, .-Thread_pause
	.globl	Thread_join
	.type	Thread_join, @function
Thread_join:
.LFB10:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	call	__x86.get_pc_thunk.ax
	addl	$_GLOBAL_OFFSET_TABLE_, %eax
	call	Thread_self
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE10:
	.size	Thread_join, .-Thread_join
	.globl	Thread_new
	.type	Thread_new, @function
Thread_new:
.LFB11:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	pushl	%ebx
	subl	$20, %esp
	.cfi_offset 3, -12
	call	__x86.get_pc_thunk.bx
	addl	$_GLOBAL_OFFSET_TABLE_, %ebx
	movl	16(%ebp), %eax
	leal	20(%eax), %edx
	movl	n_threads@GOTOFF(%ebx), %eax
	imull	%eax, %edx
	movl	_STACK@GOTOFF(%ebx), %eax
	addl	%edx, %eax
	movl	%eax, -12(%ebp)
	movl	-12(%ebp), %eax
	leal	400(%eax), %edx
	movl	-12(%ebp), %eax
	movl	%edx, (%eax)
	movl	-12(%ebp), %edx
	movl	-12(%ebp), %eax
	movl	%edx, 4(%eax)
	movl	-12(%ebp), %eax
	leal	20(%eax), %edx
	movl	-12(%ebp), %eax
	movl	%edx, 16(%eax)
	movl	-12(%ebp), %eax
	movl	16(%eax), %eax
	testl	%eax, %eax
	jne	.L15
	movl	stderr@GOT(%ebx), %eax
	movl	(%eax), %eax
	pushl	%eax
	pushl	$32
	pushl	$1
	leal	.LC1@GOTOFF(%ebx), %eax
	pushl	%eax
	call	fwrite@PLT
	addl	$16, %esp
	movl	$-1, %eax
	jmp	.L16
.L15:
	movl	-12(%ebp), %eax
	movl	16(%eax), %eax
	subl	$4, %esp
	pushl	16(%ebp)
	pushl	12(%ebp)
	pushl	%eax
	call	memcpy@PLT
	addl	$16, %esp
	movl	-12(%ebp), %eax
	movl	8(%ebp), %edx
	movl	%edx, 8(%eax)
	subl	$12, %esp
	pushl	-12(%ebp)
	call	thr_enqueue@PLT
	addl	$16, %esp
	movl	$0, %eax
.L16:
	movl	-4(%ebp), %ebx
	leave
	.cfi_restore 5
	.cfi_restore 3
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE11:
	.size	Thread_new, .-Thread_new
	.section	.rodata
	.align 4
.LC4:
	.string	"Thread %u is running with arg=%d\n"
	.align 4
.LC5:
	.string	"Thread %u is back to running with arg=%d\n"
	.text
	.globl	mytestfunc
	.type	mytestfunc, @function
mytestfunc:
.LFB12:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	pushl	%ebx
	subl	$20, %esp
	.cfi_offset 3, -12
	call	__x86.get_pc_thunk.bx
	addl	$_GLOBAL_OFFSET_TABLE_, %ebx
	movl	8(%ebp), %eax
	movl	(%eax), %eax
	movl	%eax, -12(%ebp)
	movl	thr_q@GOTOFF(%ebx), %eax
	movl	(%eax), %eax
	movl	4(%eax), %eax
	subl	$4, %esp
	pushl	-12(%ebp)
	pushl	%eax
	leal	.LC4@GOTOFF(%ebx), %eax
	pushl	%eax
	call	printf@PLT
	addl	$16, %esp
	call	Thread_pause
	movl	thr_q@GOTOFF(%ebx), %eax
	movl	(%eax), %eax
	movl	4(%eax), %eax
	subl	$4, %esp
	pushl	-12(%ebp)
	pushl	%eax
	leal	.LC5@GOTOFF(%ebx), %eax
	pushl	%eax
	call	printf@PLT
	addl	$16, %esp
	call	Thread_self
	movl	-4(%ebp), %ebx
	leave
	.cfi_restore 5
	.cfi_restore 3
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE12:
	.size	mytestfunc, .-mytestfunc
	.section	.rodata
.LC6:
	.string	"Cannot run Q empty"
	.text
	.globl	schedule
	.type	schedule, @function
schedule:
.LFB13:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	pushl	%ebx
	subl	$4, %esp
	.cfi_offset 3, -12
	call	__x86.get_pc_thunk.ax
	addl	$_GLOBAL_OFFSET_TABLE_, %eax
	movl	thr_q@GOTOFF(%eax), %edx
	movl	(%edx), %edx
	testl	%edx, %edx
	je	.L20
	movl	thr_q@GOTOFF(%eax), %edx
	movl	(%edx), %edx
	movl	(%edx), %edx
#APP
# 96 "thread.c" 1
	movl %edx, %esp
# 0 "" 2
#NO_APP
	movl	thr_q@GOTOFF(%eax), %edx
	movl	(%edx), %edx
	movl	8(%edx), %ecx
	movl	thr_q@GOTOFF(%eax), %edx
	movl	(%edx), %edx
	movl	16(%edx), %edx
	subl	$8, %esp
	pushl	%ecx
	pushl	%edx
	movl	%eax, %ebx
	call	_thrstart@PLT
	addl	$16, %esp
	movl	$0, %eax
	jmp	.L21
.L20:
	subl	$12, %esp
	leal	.LC6@GOTOFF(%eax), %edx
	pushl	%edx
	movl	%eax, %ebx
	call	puts@PLT
	addl	$16, %esp
	movl	$-1, %eax
.L21:
	movl	-4(%ebp), %ebx
	leave
	.cfi_restore 5
	.cfi_restore 3
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE13:
	.size	schedule, .-schedule
	.globl	main
	.type	main, @function
main:
.LFB14:
	.cfi_startproc
	leal	4(%esp), %ecx
	.cfi_def_cfa 1, 0
	andl	$-16, %esp
	pushl	-4(%ecx)
	pushl	%ebp
	movl	%esp, %ebp
	.cfi_escape 0x10,0x5,0x2,0x75,0
	pushl	%ebx
	pushl	%ecx
	.cfi_escape 0xf,0x3,0x75,0x78,0x6
	.cfi_escape 0x10,0x3,0x2,0x75,0x7c
	subl	$16, %esp
	call	__x86.get_pc_thunk.bx
	addl	$_GLOBAL_OFFSET_TABLE_, %ebx
	movl	%gs:20, %eax
	movl	%eax, -12(%ebp)
	xorl	%eax, %eax
	movl	$4, -20(%ebp)
	movl	$5, -16(%ebp)
	call	Thread_init
	subl	$12, %esp
	pushl	$0
	pushl	$1
	pushl	$4
	leal	-20(%ebp), %eax
	pushl	%eax
	leal	mytestfunc@GOTOFF(%ebx), %eax
	pushl	%eax
	call	Thread_new
	addl	$32, %esp
	subl	$12, %esp
	pushl	$0
	pushl	$2
	pushl	$4
	leal	-16(%ebp), %eax
	pushl	%eax
	leal	mytestfunc@GOTOFF(%ebx), %eax
	pushl	%eax
	call	Thread_new
	addl	$32, %esp
	call	print_Q@PLT
	call	schedule
	subl	$12, %esp
	pushl	%eax
	call	Thread_exit
	addl	$16, %esp
	call	schedule
	subl	$12, %esp
	pushl	%eax
	call	Thread_exit
	addl	$16, %esp
	movl	$0, %eax
	movl	-12(%ebp), %edx
	subl	%gs:20, %edx
	je	.L24
	call	__stack_chk_fail_local
.L24:
	leal	-8(%ebp), %esp
	popl	%ecx
	.cfi_restore 1
	.cfi_def_cfa 1, 0
	popl	%ebx
	.cfi_restore 3
	popl	%ebp
	.cfi_restore 5
	leal	-4(%ecx), %esp
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE14:
	.size	main, .-main
	.section	.text.__x86.get_pc_thunk.ax,"axG",@progbits,__x86.get_pc_thunk.ax,comdat
	.globl	__x86.get_pc_thunk.ax
	.hidden	__x86.get_pc_thunk.ax
	.type	__x86.get_pc_thunk.ax, @function
__x86.get_pc_thunk.ax:
.LFB15:
	.cfi_startproc
	movl	(%esp), %eax
	ret
	.cfi_endproc
.LFE15:
	.section	.text.__x86.get_pc_thunk.bx,"axG",@progbits,__x86.get_pc_thunk.bx,comdat
	.globl	__x86.get_pc_thunk.bx
	.hidden	__x86.get_pc_thunk.bx
	.type	__x86.get_pc_thunk.bx, @function
__x86.get_pc_thunk.bx:
.LFB16:
	.cfi_startproc
	movl	(%esp), %ebx
	ret
	.cfi_endproc
.LFE16:
	.hidden	__stack_chk_fail_local
	.ident	"GCC: (Ubuntu 11.4.0-1ubuntu1~22.04) 11.4.0"
	.section	.note.GNU-stack,"",@progbits
