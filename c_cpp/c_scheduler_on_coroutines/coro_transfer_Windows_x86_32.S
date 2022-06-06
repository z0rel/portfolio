	.section text,"x"
	.text
	.globl _coro_transfer
	.type _coro_transfer, @function
_coro_transfer:
	pushl %ebp
	pushl %ebx
	pushl %esi
	pushl %edi
	pushl %fs:0
	pushl %fs:4
	pushl %fs:8
	movl %esp, (%eax)
	movl (%edx), %esp
	popl %fs:8
	popl %fs:4
	popl %fs:0
	popl %edi
	popl %esi
	popl %ebx
	popl %ebp
	popl %ecx
	jmpl *%ecx
