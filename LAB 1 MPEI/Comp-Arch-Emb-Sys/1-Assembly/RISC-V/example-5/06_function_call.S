# micro riscv IO demo with "subroutine"


.text

L0:
    jal x1, READ_WORD      # Call READ_WORD (jump to READ_WORD and store PC+4 in x1)

    beq x2,x0, L1          # branch to L1, if input is zero
    sw x2, 0x7fc(x0)       # write to output
    jal x0,L0              # unconditional branch to L0
L1:
    ebreak

READ_WORD:
    lw x2, 0x7fc(x0)       # load input
    jalr x0,0(x1)          # return to caller (return address is stored in x1)
