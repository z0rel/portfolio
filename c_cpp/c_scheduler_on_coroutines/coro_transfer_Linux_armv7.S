	.section text,"x"
	.text
	.globl coro_transfer
	.type coro_transfer, %function
coro_transfer:
	vpush {d8-d15}
	push {r4-r11,lr}
	str sp, [r0]
	ldr sp, [r1]
	pop {r4-r11,lr}
	vpop {d8-d15}
	mov r15, lr
