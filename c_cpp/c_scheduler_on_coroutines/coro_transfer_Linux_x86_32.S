	.section text,"x"
	.text
	.globl coro_transfer
	.type coro_transfer, @function
coro_transfer:
	pushl %ebp
	pushl %ebx
	pushl %esi
	pushl %edi
	movl %esp, (%eax)
	movl (%edx), %esp
	popl %edi
	popl %esi
	popl %ebx
	popl %ebp
	popl %ecx
	jmpl *%ecx

