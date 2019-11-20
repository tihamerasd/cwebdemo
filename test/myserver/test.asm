	.file	"test.c"
	.text
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC0:
	.string	"tls"
	.text
	.p2align 4
	.globl	asmer
	.type	asmer, @function
asmer:
.LFB23:
	.cfi_startproc
	subq	$8, %rsp
	.cfi_def_cfa_offset 16
	xorl	%edx, %edx
	movl	$1, %esi
	movl	$2, %edi
	call	socket@PLT
	movl	$4, %r8d
	movl	$31, %edx
	leaq	.LC0(%rip), %rcx
	movl	%eax, %edi
	movl	$6, %esi
	addq	$8, %rsp
	.cfi_def_cfa_offset 8
	jmp	setsockopt@PLT
	.cfi_endproc
.LFE23:
	.size	asmer, .-asmer
	.ident	"GCC: (GNU) 9.2.0"
	.section	.note.GNU-stack,"",@progbits
