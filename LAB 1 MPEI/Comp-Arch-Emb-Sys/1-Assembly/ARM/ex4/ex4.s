 .section .text
 .global _start

_start:
    LDR SP, =0x20008000     @ Initialize stack pointer to the top of RAM
    MOV R0, #10             @ Calculate the 10th Fibonacci number
    BL fibonacci            @ Call the Fibonacci function
_end_loop:	
    B _end_loop                      @ Infinite loop to halt

@ Fibonacci function
@ Input: R0 = n
@ Output: R0 = Fibonacci(n)

fibonacci:
    MOV R1, #0                        @ R1 = fib(0) = 0
    MOV R2, #1                        @ R2 = fib(1) = 1
    CMP R0, #0                        @ If n == 0, return 0
    BEQ ret_zero                      @ If n == 0, end with result 0
    CMP R0, #1                        @ If n == 1, return 1
    BEQ end_fibonacci                 @ If n == 1, end with result 1

    SUBS R0, R0, #1                   @ Decrease n by 1, since we've handled n == 0 and n == 1 cases

fib_loop:
    SUBS R0, R0, #1                   @ Decrement n
    ADD R3, R1, R2                    @ R3 = R1 + R2 (next Fibonacci number)
    MOV R1, R2                        @ Move fib(n-1) -> fib(n-2)
    MOV R2, R3                        @ Move next Fibonacci number -> fib(n-1)
    BNE fib_loop                      @ If n != 0, continue loop

end_fibonacci:
    MOV R0, R2                        @ The result (fib(n)) is in R2
    BX LR                             @ Return to caller

ret_zero:
    MOV  R0, R1                       @ 0
    BX   LR
