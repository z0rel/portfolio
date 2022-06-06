	.section text,"x"
	.text
	.globl coro_startup
	.type coro_startup, %function
coro_startup:
.fnstart
.save {lr}
.pad #12
	mov lr, #0
	push {lr}
	sub sp, #12
	mov r0, r5
	blx r4
	b abort
.fnend
