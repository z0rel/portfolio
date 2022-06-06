	.section text,"x"
	.text
	.globl _coro_transfer
	.type _coro_transfer, @function
_coro_transfer:
	subq $168, %rsp ; one dummy qword to improve alignment 
	movaps %xmm6, (%rsp)
	movaps %xmm7, 16(%rsp)
	movaps %xmm8, 32(%rsp)
	movaps %xmm9, 48(%rsp)
	movaps %xmm10, 64(%rsp)
	movaps %xmm11, 80(%rsp)
	movaps %xmm12, 96(%rsp)
	movaps %xmm13, 112(%rsp)
	movaps %xmm14, 128(%rsp)
	movaps %xmm15, 144(%rsp)
	pushq %rsi
	pushq %rdi
	pushq %rbp
	pushq %rbx
	pushq %r12
	pushq %r13
	pushq %r14
	pushq %r15
	pushq %fs:0x0
	pushq %fs:0x8
	pushq %fs:0xc
	movq %rsp, (%rcx)
	movq (%rdx), %rsp
	popq %fs:0xc
	popq %fs:0x8
	popq %fs:0x0
	popq %r15
	popq %r14
	popq %r13
	popq %r12
	popq %rbx
	popq %rbp
	popq %rdi
	popq %rsi
	movaps (%rsp), %xmm6
	movaps 16(%rsp), %xmm7
	movaps 32(%rsp), %xmm8
	movaps 48(%rsp), %xmm9
	movaps 64(%rsp), %xmm10
	movaps 80(%rsp), %xmm11
	movaps 96(%rsp), %xmm12
	movaps 112(%rsp), %xmm13
	movaps 128(%rsp), %xmm14
	movaps 144(%rsp), %xmm15
	addq $168, %rsp
	popq %rcx
	jmpq *%rcx

