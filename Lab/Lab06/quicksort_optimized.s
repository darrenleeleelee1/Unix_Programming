	.file	"quicksort_optimized.c"
	.intel_syntax noprefix
	.text
	.p2align 4
	.globl	swap
	.type	swap, @function
swap:
.LFB23:
	endbr64
	mov	rax, QWORD PTR [rdi]
	mov	rdx, QWORD PTR [rsi]
	mov	QWORD PTR [rdi], rdx
	mov	QWORD PTR [rsi], rax
	ret
	.cfi_endproc
.LFE23:
	.size	swap, .-swap
	.p2align 4
	.globl	median_of_three
	.type	median_of_three, @function
median_of_three:
.LFB24:
	endbr64
	mov	r9d, edx
	movsx	rax, esi
	mov	rcx, rdi
	sub	r9d, esi
	lea	r8, [rdi+rax*8]
	mov	eax, r9d
	mov	rdi, QWORD PTR [r8]
	shr	eax, 31
	add	eax, r9d
	sar	eax
	add	eax, esi
	cdqe
	lea	rsi, [rcx+rax*8]
	mov	rax, QWORD PTR [rsi]
	cmp	rdi, rax
	jle	.L4
	mov	QWORD PTR [r8], rax
	mov	rax, rdi
	mov	QWORD PTR [rsi], rdi
	mov	rdi, QWORD PTR [r8]
.L4:
	movsx	rdx, edx
	lea	rcx, [rcx+rdx*8]
	mov	rdx, QWORD PTR [rcx]
	cmp	rdx, rdi
	jge	.L5
	mov	QWORD PTR [r8], rdx
	mov	rdx, rdi
	mov	QWORD PTR [rcx], rdi
	mov	rax, QWORD PTR [rsi]
.L5:
	cmp	rax, rdx
	jle	.L3
	mov	QWORD PTR [rsi], rdx
	mov	QWORD PTR [rcx], rax
	mov	rax, QWORD PTR [rsi]
.L3:
	ret
.LFE24:
	.size	median_of_three, .-median_of_three
	.p2align 4
	.globl	partition
	.type	partition, @function
partition:
.LFB25:
	endbr64
	mov	r8, rdi
	mov	edi, edx
	movsx	r9, esi
	push	rbx
	mov	esi, edi
	lea	rdx, [r8+r9*8]
	mov	rax, r9
	sub	esi, r9d
	mov	rbx, QWORD PTR [rdx]
	mov	ecx, esi
	shr	ecx, 31
	add	ecx, esi
	sar	ecx
	add	ecx, r9d
	movsx	rcx, ecx
	lea	rcx, [r8+rcx*8]
	mov	rsi, QWORD PTR [rcx]
	cmp	rbx, rsi
	jle	.L8
	mov	QWORD PTR [rdx], rsi
	mov	rsi, rbx
	mov	QWORD PTR [rcx], rbx
	mov	rbx, QWORD PTR [rdx]
.L8:
	movsx	r10, edi
	lea	r10, [r8+r10*8]
	mov	r11, QWORD PTR [r10]
	cmp	r11, rbx
	jge	.L9
	mov	QWORD PTR [rdx], r11
	mov	r11, rbx
	mov	QWORD PTR [r10], rbx
	mov	rsi, QWORD PTR [rcx]
.L9:
	cmp	rsi, r11
	jle	.L10
	mov	QWORD PTR [rcx], r11
	mov	r11, rsi
	mov	QWORD PTR [r10], rsi
	mov	rsi, QWORD PTR [rcx]
.L10:
	lea	ecx, -1[rax]
	cmp	eax, edi
	jge	.L11
	sub	edi, 1
	sub	edi, eax
	add	rdi, r9
	lea	r9, 8[r8+rdi*8]
	.p2align 4,,10
	.p2align 3
.L13:
	mov	rax, QWORD PTR [rdx]
	cmp	rax, rsi
	jge	.L12
	add	ecx, 1
	movsx	rdi, ecx
	lea	rdi, [r8+rdi*8]
	mov	r11, QWORD PTR [rdi]
	mov	QWORD PTR [rdi], rax
	mov	QWORD PTR [rdx], r11
.L12:
	add	rdx, 8
	cmp	r9, rdx
	jne	.L13
	mov	r11, QWORD PTR [r10]
	lea	eax, 1[rcx]
.L11:
	movsx	rcx, ecx
	lea	rdx, 8[r8+rcx*8]
	mov	rcx, QWORD PTR [rdx]
	mov	QWORD PTR [rdx], r11
	mov	QWORD PTR [r10], rcx
	pop	rbx
	ret
.LFE25:
	.size	partition, .-partition
	.p2align 4
	.globl	quicksort_recursive
	.type	quicksort_recursive, @function
quicksort_recursive:
.LFB26:
	endbr64
	push	r15
	push	r14
	push	r13
	push	r12
	push	rbp
	push	rbx
	sub	rsp, 72
	mov	DWORD PTR 36[rsp], edx
	cmp	edx, esi
	jle	.L17
	mov	r15, rdi
	mov	r13d, esi
.L35:
	mov	edx, DWORD PTR 36[rsp]
	mov	esi, r13d
	mov	rdi, r15
	call	partition
	mov	DWORD PTR 40[rsp], eax
	sub	eax, 1
	mov	DWORD PTR 24[rsp], eax
	cmp	eax, r13d
	jle	.L19
	mov	r14d, r13d
	mov	r13, r15
.L34:
	mov	edx, DWORD PTR 24[rsp]
	mov	esi, r14d
	mov	rdi, r13
	call	partition
	mov	DWORD PTR 44[rsp], eax
	sub	eax, 1
	mov	DWORD PTR 28[rsp], eax
	cmp	eax, r14d
	jle	.L20
	mov	eax, r14d
	mov	r14, r13
	mov	r13d, eax
.L33:
	mov	edx, DWORD PTR 28[rsp]
	mov	esi, r13d
	mov	rdi, r14
	call	partition
	mov	DWORD PTR 48[rsp], eax
	sub	eax, 1
	mov	DWORD PTR 32[rsp], eax
	cmp	eax, r13d
	jle	.L21
.L32:
	mov	edx, DWORD PTR 32[rsp]
	mov	esi, r13d
	mov	rdi, r14
	call	partition
	lea	r12d, -1[rax]
	mov	DWORD PTR 52[rsp], eax
	cmp	r12d, r13d
	jle	.L22
.L31:
	mov	edx, r12d
	mov	esi, r13d
	mov	rdi, r14
	call	partition
	lea	ebx, -1[rax]
	mov	ebp, eax
	cmp	ebx, r13d
	jle	.L23
	mov	DWORD PTR 8[rsp], r12d
	mov	r15d, r13d
	mov	ebp, ebx
	mov	r13, r14
	mov	r14d, eax
.L30:
	mov	edx, ebp
	mov	esi, r15d
	mov	rdi, r13
	call	partition
	lea	ebx, -1[rax]
	mov	DWORD PTR 16[rsp], eax
	cmp	ebx, r15d
	jle	.L24
	mov	DWORD PTR 12[rsp], ebp
	mov	ebp, ebx
	mov	rbx, r13
.L29:
	mov	edx, ebp
	mov	esi, r15d
	mov	rdi, rbx
	call	partition
	lea	r13d, -1[rax]
	mov	DWORD PTR 56[rsp], eax
	cmp	r13d, r15d
	jle	.L25
	mov	DWORD PTR 20[rsp], ebp
	mov	rbp, rbx
	mov	ebx, r13d
.L28:
	mov	edx, ebx
	mov	esi, r15d
	mov	rdi, rbp
	call	partition
	lea	r13d, -1[rax]
	mov	r12d, eax
	cmp	r13d, r15d
	jle	.L26
	mov	r12d, ebx
	mov	ecx, eax
	mov	ebx, r13d
.L27:
	mov	esi, r15d
	mov	edx, ebx
	mov	rdi, rbp
	mov	DWORD PTR 60[rsp], ecx
	call	partition
	mov	esi, r15d
	mov	rdi, rbp
	mov	r13d, eax
	lea	edx, -1[rax]
	call	quicksort_recursive
	lea	r15d, 1[r13]
	mov	ecx, DWORD PTR 60[rsp]
	cmp	ebx, r15d
	jg	.L27
	mov	ebx, r12d
	mov	r12d, ecx
.L26:
	lea	r15d, 1[r12]
	cmp	ebx, r15d
	jg	.L28
	mov	rbx, rbp
	mov	ebp, DWORD PTR 20[rsp]
.L25:
	mov	r15d, DWORD PTR 56[rsp]
	add	r15d, 1
	cmp	ebp, r15d
	jg	.L29
	mov	ebp, DWORD PTR 12[rsp]
	mov	r13, rbx
.L24:
	mov	r15d, DWORD PTR 16[rsp]
	add	r15d, 1
	cmp	ebp, r15d
	jg	.L30
	mov	r12d, DWORD PTR 8[rsp]
	mov	ebp, r14d
	mov	r14, r13
.L23:
	lea	r13d, 1[rbp]
	cmp	r12d, r13d
	jg	.L31
.L22:
	mov	r13d, DWORD PTR 52[rsp]
	add	r13d, 1
	cmp	DWORD PTR 32[rsp], r13d
	jg	.L32
.L21:
	mov	r13d, DWORD PTR 48[rsp]
	add	r13d, 1
	cmp	DWORD PTR 28[rsp], r13d
	jg	.L33
	mov	r13, r14
.L20:
	mov	r14d, DWORD PTR 44[rsp]
	add	r14d, 1
	cmp	DWORD PTR 24[rsp], r14d
	jg	.L34
	mov	r15, r13
.L19:
	mov	r13d, DWORD PTR 40[rsp]
	add	r13d, 1
	cmp	r13d, DWORD PTR 36[rsp]
	jl	.L35
.L17:
	add	rsp, 72
	pop	rbx
	pop	rbp
	pop	r12
	pop	r13
	pop	r14
	pop	r15
	ret
.LFE26:
	.size	quicksort_recursive, .-quicksort_recursive
	.p2align 4
	.globl	quicksort
	.type	quicksort, @function
quicksort:
.LFB27:
	endbr64
	push	r13
	push	r12
	lea	r12d, -1[rsi]
	push	rbp
	push	rbx
	sub	rsp, 8
	test	r12d, r12d
	jle	.L46
	mov	r13, rdi
	xor	ebp, ebp
.L48:
	mov	esi, ebp
	mov	edx, r12d
	mov	rdi, r13
	call	partition
	mov	esi, ebp
	mov	rdi, r13
	mov	ebx, eax
	lea	edx, -1[rax]
	call	quicksort_recursive
	lea	ebp, 1[rbx]
	cmp	r12d, ebp
	jg	.L48
.L46:
	add	rsp, 8
	pop	rbx
	pop	rbp
	pop	r12
	pop	r13
	ret
.LFE27:
	.size	quicksort, .-quicksort
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC3:
	.string	"Sorted array: "
.LC4:
	.string	"%ld "
	.section	.text.startup,"ax",@progbits
	.p2align 4
	.globl	main
	.type	main, @function
main:
.LFB28:
	endbr64
	push	r12
	mov	edx, 5
	xor	esi, esi
	push	rbp
	lea	rbp, .LC4[rip]
	push	rbx
	sub	rsp, 48
	movdqa	xmm0, XMMWORD PTR .LC0[rip]
	mov	rbx, rsp
	lea	r12, 48[rsp]
	movaps	XMMWORD PTR [rsp], xmm0
	movdqa	xmm0, XMMWORD PTR .LC1[rip]
	mov	rdi, rbx
	movaps	XMMWORD PTR 16[rsp], xmm0
	movdqa	xmm0, XMMWORD PTR .LC2[rip]
	movaps	XMMWORD PTR 32[rsp], xmm0
	call	quicksort_recursive
	lea	rdi, .LC3[rip]
	call	puts@PLT
	.p2align 4,,10
	.p2align 3
.L52:
	mov	rdx, QWORD PTR [rbx]
	mov	rsi, rbp
	mov	edi, 1
	xor	eax, eax
	add	rbx, 8
	call	__printf_chk@PLT
	cmp	r12, rbx
	jne	.L52
	mov	edi, 10
	call	putchar@PLT
	add	rsp, 48
	xor	eax, eax
	pop	rbx
	pop	rbp
	pop	r12
	ret
.LFE28:
	.size	main, .-main
	.section	.rodata.cst16,"aM",@progbits,16
	.align 16
.LC0:
	.quad	10
	.quad	7
	.align 16
.LC1:
	.quad	8
	.quad	9
	.align 16
.LC2:
	.quad	1
	.quad	5
	.ident	"GCC: (Ubuntu 11.3.0-1ubuntu1~22.04) 11.3.0"
	.section	.note.GNU-stack,"",@progbits
	.section	.note.gnu.property,"a"
	.align 8
	.long	1f - 0f
	.long	4f - 1f
	.long	5
0:
	.string	"GNU"
1:
	.align 8
	.long	0xc0000002
	.long	3f - 2f
2:
	.long	0x3
3:
	.align 8
4:
