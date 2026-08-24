# Exercise: Reverse an Array of Integers

## Objective:
Write an ARMv7 assembly program that reverses an array of integers in memory. The program should run bare metal and use a single function to perform the reversal.

## Problem Breakdown:
* Define an array of integers in the .data section.
* Write a function that reverses the elements of the array in-place.
* Call the function from the main program and store the reversed array back into memory.

## Highligts

In this program, we use a new addressing mode [R0, R2, LSL #2] where:

* R0: The base address register. It holds the starting address of the memory location you're working with.
* R2: The index register. It contains an offset, which is used to calculate the memory address relative to the base address in R0.
* LSL #2: This is a logical shift left operation. It shifts the value in R2 left by 2 bits, effectively multiplying R2 by 4 (LSL #2 is the same as multiplying by 2^2 = 4). This is often used in array indexing, where each element is 4 bytes (32-bit word size).

## Test the program:
Use the online emulator at [https://cpulator.01xz.net/?sys=arm](https://cpulator.01xz.net/?sys=arm)

