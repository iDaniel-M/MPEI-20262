.text
                      # x0 always has value 0
                      # x2 is our stack pointer
                      # x5-x7 are temporary registers
 addi    x2,x0,0x700  # initialize stack pointer

 addi    x5,x0,1      # x5 = 1;
 addi    x6,x0,2      # x6 = 2;
 addi    x7,x0,3      # x7 = 3;

 addi    x2,x2,-4     # push x5
 sw      x5,0(x2)
 addi    x2,x2,-4     # push x6
 sw      x6,0(x2)
 addi    x2,x2,-4     # push x7
 sw      x7,0(x2)

 lw      x7,0(x2)     # pop x7
 addi    x2,x2,4
 lw      x6,0(x2)     # pop x6
 addi    x2,x2,4
 lw      x5,0(x2)     # pop x5
 addi    x2,x2,4

 ebreak
