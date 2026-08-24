# Exercise 2 — Makefile
Create a Makefile for a project with multiple C files: `main.c`, `utils.c`, and `utils.h`. The project does not require cross-compilation and creates native code.

The Makefile should:

* set as default compiler gcc
* set a gcc flags `-Wall`
* Compiles `main.c` and `math_utils.c` into object files (main.o and math_utils.o).
* Links the object files into an executable named `math_program.elf`.
* Includes a rule that recompiles `math_utils.c` if `math_utils.h` is modified.
* Has a clean rule to remove the executable and any object files.

```
project/
├── main.c
├── maths_util.c
└── maths_util.h
```
