	.file	"misc.c"
	.text
	.globl	thr_enqueue
	.type	thr_enqueue, @function
thr_enqueue:
.LFB0:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	call	__x86.get_pc_thunk.ax
	addl	$_GLOBAL_OFFSET_TABLE_, %eax
	movl	thr_q@GOT(%eax), %edx
	movl	(%edx), %edx
	movl	(%edx), %edx
	testl	%edx, %edx
	jne	.L2
	movl	thr_q@GOT(%eax), %edx
	movl	(%edx), %edx
	movl	8(%ebp), %ecx
	movl	%ecx, 4(%edx)
	movl	thr_q@GOT(%eax), %ecx
	movl	(%ecx), %ecx
	movl	4(%edx), %edx
	movl	%edx, (%ecx)
	movl	thr_q@GOT(%eax), %edx
	movl	(%edx), %edx
	movl	4(%edx), %edx
	movl	$0, 12(%edx)
	movl	thr_q@GOT(%eax), %ecx
	movl	(%ecx), %ecx
	movl	(%ecx), %ecx
	movl	12(%edx), %edx
	movl	%edx, 12(%ecx)
	jmp	.L3
.L2:
	movl	thr_q@GOT(%eax), %edx
	movl	(%edx), %edx
	movl	4(%edx), %edx
	movl	8(%ebp), %ecx
	movl	%ecx, 12(%edx)
	movl	8(%ebp), %edx
	movl	$0, 12(%edx)
	movl	thr_q@GOT(%eax), %edx
	movl	(%edx), %edx
	movl	8(%ebp), %ecx
	movl	%ecx, 4(%edx)
.L3:
	movl	n_threads@GOT(%eax), %edx
	movl	(%edx), %edx
	addl	$1, %edx
	movl	n_threads@GOT(%eax), %eax
	movl	%edx, (%eax)
	nop
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE0:
	.size	thr_enqueue, .-thr_enqueue
	.section	.rodata
.LC0:
	.string	"Cannot dequeue from empty\n"
	.text
	.globl	thr_dequeue
	.type	thr_dequeue, @function
thr_dequeue:
.LFB1:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	pushl	%ebx
	subl	$20, %esp
	.cfi_offset 3, -12
	call	__x86.get_pc_thunk.ax
	addl	$_GLOBAL_OFFSET_TABLE_, %eax
	movl	thr_q@GOT(%eax), %edx
	movl	(%edx), %edx
	movl	(%edx), %edx
	testl	%edx, %edx
	jne	.L5
	movl	stderr@GOT(%eax), %edx
	movl	(%edx), %edx
	pushl	%edx
	pushl	$26
	pushl	$1
	leal	.LC0@GOTOFF(%eax), %edx
	pushl	%edx
	movl	%eax, %ebx
	call	fwrite@PLT
	addl	$16, %esp
	movl	$0, %eax
	jmp	.L6
.L5:
	movl	thr_q@GOT(%eax), %edx
	movl	(%edx), %edx
	movl	(%edx), %edx
	movl	%edx, -12(%ebp)
	movl	thr_q@GOT(%eax), %edx
	movl	(%edx), %edx
	movl	(%edx), %edx
	movl	thr_q@GOT(%eax), %eax
	movl	(%eax), %eax
	movl	12(%edx), %edx
	movl	%edx, (%eax)
	movl	-12(%ebp), %eax
.L6:
	movl	-4(%ebp), %ebx
	leave
	.cfi_restore 5
	.cfi_restore 3
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE1:
	.size	thr_dequeue, .-thr_dequeue
	.section	.rodata
.LC1:
	.string	"Q empty"
.LC2:
	.string	"Thread %u with arg: %d\n"
	.text
	.globl	print_Q
	.type	print_Q, @function
print_Q:
.LFB2:
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
	movl	thr_q@GOT(%ebx), %eax
	movl	(%eax), %eax
	movl	(%eax), %eax
	testl	%eax, %eax
	jne	.L8
	subl	$12, %esp
	leal	.LC1@GOTOFF(%ebx), %eax
	pushl	%eax
	call	puts@PLT
	addl	$16, %esp
	jmp	.L7
.L8:
	movl	thr_q@GOT(%ebx), %eax
	movl	(%eax), %eax
	movl	(%eax), %eax
	movl	%eax, -12(%ebp)
	jmp	.L10
.L11:
	movl	-12(%ebp), %eax
	movl	16(%eax), %eax
	movl	(%eax), %edx
	movl	-12(%ebp), %eax
	movl	4(%eax), %eax
	subl	$4, %esp
	pushl	%edx
	pushl	%eax
	leal	.LC2@GOTOFF(%ebx), %eax
	pushl	%eax
	call	printf@PLT
	addl	$16, %esp
	movl	-12(%ebp), %eax
	movl	12(%eax), %eax
	movl	%eax, -12(%ebp)
.L10:
	cmpl	$0, -12(%ebp)
	jne	.L11
.L7:
	movl	-4(%ebp), %ebx
	leave
	.cfi_restore 5
	.cfi_restore 3
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE2:
	.size	print_Q, .-print_Q
	.section	.text.__x86.get_pc_thunk.ax,"axG",@progbits,__x86.get_pc_thunk.ax,comdat
	.globl	__x86.get_pc_thunk.ax
	.hidden	__x86.get_pc_thunk.ax
	.type	__x86.get_pc_thunk.ax, @function
__x86.get_pc_thunk.ax:
.LFB3:
	.cfi_startproc
	movl	(%esp), %eax
	ret
	.cfi_endproc
.LFE3:
	.section	.text.__x86.get_pc_thunk.bx,"axG",@progbits,__x86.get_pc_thunk.bx,comdat
	.globl	__x86.get_pc_thunk.bx
	.hidden	__x86.get_pc_thunk.bx
	.type	__x86.get_pc_thunk.bx, @function
__x86.get_pc_thunk.bx:
.LFB4:
	.cfi_startproc
	movl	(%esp), %ebx
	ret
	.cfi_endproc
.LFE4:
	.ident	"GCC: (Ubuntu 11.4.0-1ubuntu1~22.04) 11.4.0"
	.section	.note.GNU-stack,"",@progbits
