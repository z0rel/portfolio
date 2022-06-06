	.section text,"x"
	.text
	.globl coro_transfer
	.type coro_transfer, %function
coro_transfer:
	sub x2, sp, #8 * 20
	stp x19, x20, [x2, #16 * 0]
	stp x21, x22, [x2, #16 * 1]
	stp x23, x24, [x2, #16 * 2]
	stp x25, x26, [x2, #16 * 3]
	stp x27, x28, [x2, #16 * 4]
	stp x29, x30, [x2, #16 * 5]
	stp d8,  d9,  [x2, #16 * 6]
	stp d10, d11, [x2, #16 * 7]
	stp d12, d13, [x2, #16 * 8]
	stp d14, d15, [x2, #16 * 9]
	str x2, [x0, #0]
	ldr x3, [x1, #0]
	ldp x19, x20, [x3, #16 * 0]
	ldp x21, x22, [x3, #16 * 1]
	ldp x23, x24, [x3, #16 * 2]
	ldp x25, x26, [x3, #16 * 3]
	ldp x27, x28, [x3, #16 * 4]
	ldp x29, x30, [x3, #16 * 5]
	ldp d8,  d9,  [x3, #16 * 6]
	ldp d10, d11, [x3, #16 * 7]
	ldp d12, d13, [x3, #16 * 8]
	ldp d14, d15, [x3, #16 * 9]
	add sp, x3, #8 * 20
	ret
