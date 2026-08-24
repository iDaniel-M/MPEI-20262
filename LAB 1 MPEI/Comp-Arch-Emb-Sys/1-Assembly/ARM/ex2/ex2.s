.section .data
    @ The array array is defined in the .data 
    @ section and contains five integers (1, 2, 3, 4, 5). 
    @This can be changed to test different arrays.	
    array:      .word 1, 2, 3, 4, 5  @ Array of integers (example)

.section .text
    .global _start                @ Entry point for the program

_start:
    LDR R0, =array                @ Load address of array into R0
    MOV R1, #5                    @ Load the length of the array (5 elements) into R1

    BL sum_array                  @ Branch to sum_array function and link (store return address)

_end_loop:
    @ The sum is now in R0, you could output it here or store it in memory for later use.
    B _end_loop                      @ Infinite loop to halt program (bare-metal)

@ Function: sum_array
@ Arguments:
@   R0: Address of the array
@   R1: Length of the array
@ Return:
@   R0: Sum of the array elements
sum_array:
    MOV R2, #0                    @ Initialize sum (R2) to 0
    MOV R3, #0                    @ Initialize index (R3) to 0

sum_loop:
    CMP R3, R1                    @ Compare index (R3) with the array length (R1)
    BEQ sum_done                  @ If index == length, jump to sum_done
    LDR R12, [R0, R3, LSL #2]      @ Load word from array (R0 + R3 * 4)
    ADD R2, R2, R12                @ Add the array element to sum (R2)
    ADD R3, R3, #1                @ Increment index (R3)
    B sum_loop                    @ Repeat the loop

sum_done:
    MOV R0, R2                    @ Move the sum to R0 (return value)
    BX LR                         @ Return from function (branch to link register)



