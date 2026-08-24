# Makefiles intro
Makefile commands are instructions executed by the make utility to build or manage projects. These commands are defined within a Makefile and are associated with specific targets.

## Key Concepts:

### Targets:
These represent files to be created or actions to be performed (e.g., `all`, `clean`, `program.exe`).
### Dependencies:
Files or other targets that a target relies on. If a dependency is newer than the target, or if the target doesn't exist, `make` will execute the associated commands.
### Commands:
Shell commands that `make` executes to build a target or perform an action. Each command line in a rule must begin with a tab character. 

## Common Makefile Commands/Targets and their Purpose:
`make`: When executed without a specific target, `make` will build the first target defined in the Makefile (often named `all`).

`make [target_name]`: Executes the commands associated with the specified `target_name`.

`make clean`: A common target used to remove generated files (object files, executables, temporary files) from a project, ensuring a clean build environment.

`make install`: Installs the built project files into a designated location.

`make test`: Runs tests defined within the project.

## Example of a Basic Makefile Rule:
```makefile
program.exe: main.o helper.o
	gcc -o program.exe main.o helper.o

main.o: main.c
	gcc -c main.c

clean:
	rm -f *.o program.exe
```

### In this example:
* `program.exe` is a target that depends on `main.o` and `helper.o`. The command `gcc -o program.exe main.o helper.o` builds the executable. 
* `main.o` is a target that depends on `main.c`. The command `gcc -c main.c compiles main.c` into an object file.

* `clean` is a target that removes object files and the executable, or anything decided by the programmer.

## Important Notes:
* Commands within a Makefile are executed by the shell.

* Command modifiers like `@` (to suppress command echoing) or `-` (to ignore errors) can be used before commands.

* `.PHONY` targets are used to declare targets that do not correspond to actual files, ensuring they are always executed when called (e.g., `.PHONY: clean`).