# Exercise: Sum of Array using ARMv7 Assembly (Function Call)

## Objective:
Write an ARMv7 assembly program that computes the sum of an array of integers using a function call. 

The program must execute in a bare-metal environment and output the result (for example, via a memory-mapped I/O register, such as a UART).

## Problem Breakdown:

* Define an array of integers in the .data section of the program.
* Implement a function that takes the array's address and length as arguments, computes the sum of the array elements, and returns the result.
* Call the function from the main program to compute the sum and store the result in a register.
* Output the result to a memory-mapped I/O register, such as a UART (if available in the bare-metal setup), or store the result in memory.

## Program Requirements:

* Use ARMv7 assembly instructions.
* The sum function should use function calling conventions: passing arguments through registers (e.g., R0 for the address, R1 for the array length) and returning the result in R0.
* The program should run bare-metal (no OS).


## Test the program:
Use the online emulator at [https://cpulator.01xz.net/?sys=arm](https://cpulator.01xz.net/?sys=arm)


##  Highlights

On ARMv7 (AArch32, the classic 32-bit ARM rchitecture), the standard function call convention is the **AAPCS (ARM Architecture Procedure Call Standard)**, sometimes referred to as the ARM EABI calling convention.

Here’s the breakdown:

**Register usage for parameters and return value**

*	`R0 – R3`: Used to pass the first four arguments to a function (in order). If there are fewer than four arguments, only the necessary registers are used.
If there are more than four arguments, the extra ones are passed on the stack.
* `R0`: Holds the return value of a function (integer, pointer, or small struct). For larger results (e.g., 64-bit integers), R0 and R1 are used together.
For floating-point operations, if VFP registers are enabled and the ABI permits, s0–s15 may be used.
* Stack:
	* Arguments beyond the fourth are pushed to the stack (aligned to 4 or 8 bytes, depending on type).
	* The stack must remain 8-byte aligned at a function call boundary.

**Callee-saved vs caller-saved registers**

When a program executes a function call (BL in ARM), both the caller and the callee may want to use the same registers. The AAPCS defines who is responsible for saving which registers so that values aren’t lost unexpectedly.

_Caller-saved (volatile) registers_

* `R0 – R3` Hold function arguments and the return value.
If the caller needs their values after a function call, they must save them (usually on the stack) before making the call.

*	`R12` (IP, “intra-procedure-call scratch”)
A temporary scratch register that compilers often use during calls. The caller cannot expect it to survive across a function call.

In short, the caller is responsible for saving these if they need them after the call.

_Callee-saved (non-volatile) registers_

*	`R4 – R11` These must be preserved by the callee.
If a function wants to use them, it must push them onto the stack at entry and pop them before returning.

* `SP (R13)` The stack pointer must always point to the correct location and be restored before returning.

* `LR (R14, Link Register)` – Holds the return address (set by BL). A function usually saves it on the stack at the beginning (with PUSH {LR}) if it is going to call other functions or use BL again.

*	`PC (R15)` – The program counter. Execution flow depends on it. Functions normally restore PC indirectly (via BX LR or POP {PC}).

In short, the callee is responsible for restoring these before it returns.



