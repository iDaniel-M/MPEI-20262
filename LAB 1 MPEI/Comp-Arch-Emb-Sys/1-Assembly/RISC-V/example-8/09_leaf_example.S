.text

_start:
main:

la t0, g
lw x10,0(t0)

la t0, h
lw x11,0(t0)

la t0, i
lw x12,0(t0)

la t0, j
lw x13,0(t0)

jal x1,leaf_example
loop:
beq zero,zero,loop


leaf_example:
	addi sp,sp,-12
	sw   x5,8(sp)
	sw   x6,4(sp)
	sw   x20,0(sp)
	add  x5,x10,x11
	add  x6,x12,x1
	sub  x20,x5,x6
	addi x10,x20,0
	lw   x20,0(sp)
	lw   x6,4(sp)
	lw   x5,8(sp)
	addi sp,sp,12
	jalr x0,0(x1)

.org 0x400
.data
    g: .word  10
    h: .word  10
    i: .word  10
    j: .word  20