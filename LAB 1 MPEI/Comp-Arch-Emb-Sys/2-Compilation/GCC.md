# GCC introduction
The GNU Compiler Collection (GCC) provides a variety of commands and options for compiling C, C++, and other programming languages. Further details about how to use GCC can be found in [`GCC manuls`](https://gcc.gnu.org/onlinedocs/gcc-15.2.0/gcc/Option-Summary.html).

The basic syntax for compiling a single source file is:

```bash
gcc [options] source_file.c -o executable_name
```

Here are some common GCC commands and options:

## Basic Compilation:
```bash
gcc source.c -o program: #Compiles source.c and creates an executable named program.
```
## Output Control:
```bash
-o <file>: #Specifies the name of the output file (executable, object file, etc.).
-c: #Compiles the source file(s) into object files (.o) without linking. This is useful for compiling multiple source files separately before linking them.
-S: #Compiles the source file(s) into assembly code (.s).
-E: #Performs only preprocessing, sending the result to standard output. 
```

## Debugging and Optimization:
```bash
-g: #Generates debugging information, allowing you to use a debugger like GDB.
-O<n>: #Optimizes the code for performance, where n can be 0, 1, 2, 3, s, or fast (e.g., -O2). Higher numbers generally mean more aggressive optimization.
-Os: #Optimizes for code size instead of speed.
```
## Warnings and Errors:
```bash
-Wall: #Enables all common warning messages.
-Werror: #Treats all warnings as errors, stopping compilation if any warnings occur.
```

## Include Paths and Libraries:
```bash
-I <directory>: #Adds a directory to the search path for header files.
-L <directory>: #Adds a directory to the search path for library files.
-l <library_name>: #Links the compiled code with a specified library (e.g., -lm for the math library).
``` 
## Standards Compliance:
```bash
-std=<standard>: #Specifies the C or C++ standard to use (e.g., -std=c99, -std=c++11).
```
## Verbose Output:
```bash
-v: #Displays the commands executed at each stage of the compilation process and the compiler version.
```
