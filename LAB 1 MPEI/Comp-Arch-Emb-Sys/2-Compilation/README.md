# Intro to GNU tools

This directory contains some examples regarding the basic usage of GNU tools for compiling and debugging computer programs written in C/C++. 

A basic list of GCC options and usage can be found in [GCC.md](GCC.md), and a basic list of commands for the debugger can be found in [GDB.md](GDB.md). Also, some basic intro about the usage of Make and Makefiles is located in [Makefile.md](Makefile.md).

The current directory contains the following directories:

# 0\_simple
First example with a dummy main which calls a custom function to add two integer numbers.

# 1\_stdio
IO example using printf. The main calls the addition function and prints the results. From now on the functionality of the C code will be the same, but we will see different usage of the Makefile.

# 2\_make\_variables
Use variables in Makefile to generalize compilation

# 3\_include\_path
Store header files in dedicated directory and soecify its path in the Makefile

# 4\_make\_generic
Generic compilation flow to generate object files starting from C files

# 5\_advanced\_make
Phony targets and run commands without printing what is being executed, simply print the output of the command.
