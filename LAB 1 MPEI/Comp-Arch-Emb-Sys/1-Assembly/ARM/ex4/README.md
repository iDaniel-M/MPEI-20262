# Exercise: Fibonacci Sequence

## Objective:
Write an ARMv7 assembly program that calculates the Nth Fibonacci. The result should be returned in a register. The program should run bare-metal.

## Problem Breakdown:
* Define the Fibonacci function.
* Use a simple main program that calls the function to calculate the 10th Fibonacci number.
* Return the result in a register.


## Test the program:
Use the online emulator at [https://cpulator.01xz.net/?sys=arm](https://cpulator.01xz.net/?sys=arm)

## Explanation:
This program recursively computes the Nth Fibonacci number using the standard recursive formula Fib(n) = Fib(n-1) + Fib(n-2).

* The base cases (Fib(0) = 0, Fib(1) = 1) are handled separately.
* The recursive calls are managed using the stack (PUSH and POP instructions) to save and restore the current state.
* The result is returned in R0.
