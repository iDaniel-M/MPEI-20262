# Exercise 1 — Makefile
Create a Makefile for a project with multiple C files: `main.c`, `utils.c`, and `utils.h`. 

The Makefile should:

* set as default compiler gcc
* set a gcc flags `-Wall`
* Compile main.c and utils.c into an executable named `program.elf`.
* Use object files for each .c file.
* make sure that when a file changes the minimum amount of files will be recompiled
* Include a clean rule to remove the compiled files.

```
project/
├── main.c
├── utils.c
└── utils.h
```