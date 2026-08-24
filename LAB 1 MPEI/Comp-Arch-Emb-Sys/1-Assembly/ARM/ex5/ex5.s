.section .data
num:       
    .word 0xF0F0F0F0        @ Example 32-bit number (binary: 11110000 11110000 ...)

.section .text
.global _start

_start:
    LDR R0, =num                  @ Load address of number into R0
    LDR R1, [R0]                  @ Load the 32-bit number into R1
    BL count_set_bits              @ Call the function to count set bits
_end_loop:	
    B _end_loop                      @ Infinite loop to halt

@ Function: count_set_bits
@ Input: R1 = 32-bit number
@ Output: R0 = Number of set bits
count_set_bits:
    MOV R2, #0                    @ Initialize count to 0

count_loop:
    TST R1, #1                    @ Test least significant bit of R1
    ADDNE R2, R2, #1              @ If bit is 1, increment count
    LSR R1, R1, #1                @ Shift R1 right by 1 bit
    CMP R1, #0                    @ Check if R1 is zero
    BNE count_loop                @ If not, continue loop

    MOV R0, R2                    @ Move the result to R0
    BX LR                         @ Return
