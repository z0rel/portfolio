	.section text,"x"
	.text
	.globl coro_startup
	.type coro_startup, %function
coro_startup:
.cfi_startproc
	mov x30, #0
	sub sp, sp, #16
	str x30, [sp, #0]
.cfi_def_cfa_offset 16
.cfi_offset 30, -16
	mov x0, x20
	blr x19
	b abort
.cfi_endproc
