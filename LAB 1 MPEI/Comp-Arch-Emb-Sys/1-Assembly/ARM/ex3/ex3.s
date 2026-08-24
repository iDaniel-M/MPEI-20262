.section .data
array:      .word 1, 2, 3, 4, 5    @ Array of 5 integers to reverse

.section .text
.global _start

_start:
    LDR R0, =array                @ Load address of the array. Cannot use ADR because it is in a different section and is too far
    MOV R1, #5                    @ Length of array
    BL reverse_array              @ Call the reverse_array function

_end_loop:	
	B _end_loop                      @ Infinite loop to halt

reverse_array:
    SUBS R1, R1, #1               @ R1 = length - 1 (index of last element)
    MOV R2, #0                    @ R2 is the index of the first element

reverse_loop:
    CMP R2, R1                    @ Compare first index (R2) with last (R1)
    BGE reverse_done              @ If R2 >= R1, we're done


				  @ we use a new addressing mode [R0, R2, LSL #2]: 
				  @ R0: The base address register. It holds the starting address of the memory location you're working with.
				  @ R2: The index register. It contains an offset, which is used to calculate the memory address relative to the base address in R0.
				  @ LSL #2: This is a logical shift left operation. It shifts the value in R2 left by 2 bits, effectively multiplying R2 by 4 
				  @(LSL #2 is the same as multiplying by 2^2 = 4) 
				  @This is often used in array indexing, where each element is 4 bytes (32-bit word size).

    LDR R3, [R0, R2, LSL #2]      @ Load element from start of array
    LDR R4, [R0, R1, LSL #2]      @ Load element from end of array
    STR R3, [R0, R1, LSL #2]      @ Swap element from start with end
    STR R4, [R0, R2, LSL #2]      @ Swap element from end with start

    ADD R2, R2, #1                @ Increment start index (R2)
    SUB R1, R1, #1                @ Decrement end index (R1)
    B reverse_loop                @ Continue loop

reverse_done:
    BX LR                         @ Return from function



