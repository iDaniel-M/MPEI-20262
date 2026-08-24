# Debugging Tools (GDB and LLDB)

# GDB commands
GDB (GNU Debugger) is a powerful command-line debugger for various programming languages. Here are some commonly used GDB commands, for further details please refer to [`GNU Debugger`](https://sourceware.org/gdb/current/onlinedocs/gdb.html/):

## Starting and Exiting:
```bash
gdb <program>: #Starts GDB with the specified program.
run or r: #Starts the execution of the program.
quit or q: #Exits GDB.
```
## Breakpoints:
``` bash
break <function_name>: #Sets a breakpoint at the entry of a function.
break <file>:<line_number>: #Sets a breakpoint at a specific line in a file.
break *<address>: #Sets a breakpoint at a specific memory address.
info breakpoints: #Lists all currently set breakpoints.
delete <breakpoint_number>: #Deletes a specific breakpoint.
disable <breakpoint_number>: #Disables a specific breakpoint without deleting it.
enable <breakpoint_number>: #Enables a previously disabled breakpoint.
```
## Execution Control:
```bash
continue or c: #Continues program execution until the next breakpoint or program termination.
next or n: #Executes the next line of code, stepping over function calls.
step or s: #Executes the next line of code, stepping into function calls.
finish: #Continues execution until the current function returns.
until <line_number>: #Continues execution until the specified line number is reached. 
```
## Examining Data:
```bash
print <expression> or p <expression>: #Prints the value of a variable or expression.
info locals: #Prints the values of local variables in the current function.
info args: #Prints the values of arguments passed to the current function.
x/<format> <address>: #Examines memory at a specific address with a specified format (e.g., x/i for instructions, x/s for string, x/xw for hex words).
```
## Stack and Frame Information:
```bash
backtrace or bt: #Prints a stack trace, showing the call history.
frame <frame_number>: #Selects a specific stack frame.
up: #Moves up one level in the call stack.
down: #Moves down one level in the call stack.
```
## Other Useful Commands:
```bash
list <line_number>: #Displays source code around a specific line number.
help <command>: #Provides help on a specific GDB command.
set <variable_name> = <value>: #Sets the value of a variable during debugging.
```

# LLDB Commands
LLDB is a powerful debugger with a structured command syntax. Commands are generally in the format of `<noun> <verb> [-options [option-value]] [argument [argument...]]`. 

Here the list of more common LLDB commands, for further details please refer to [`LLDB Tutorial`](https://lldb.llvm.org/use/tutorial.html):

## Execution Control:
```bash
run or r: #Launch the executable in the debugger.
continue or c: #Continue execution until the next breakpoint or program completion.
next or n or thread step-over: #Execute the next line of code, stepping over function calls.
step or s or thread step-in: #Execute the next line of code, stepping into function calls.
finish or f or thread step-out: #Step out of the current function and return to the calling frame.
```

## Breakpoints:
```bash
breakpoint set -n <function_name>: #Set a breakpoint on a function by name.
breakpoint set -f <file_name> -l <line_number>: #Set a breakpoint at a specific file and line number.
breakpoint list: #List all active breakpoints.
breakpoint delete <breakpoint_id>: #Delete a specific breakpoint.
```

## Inspecting Data:
```bash
print <variable_name> or p <variable_name>: #Print the value of a variable.
print *<pointer_variable>: #Print the value pointed to by a pointer.
po <object_variable>: #Print a more human-readable description of an object.
frame variable: #Display all arguments and local variables in the current frame.
x/<format> <address>: #Examine memory at a given address with specified formatting.
```

## Stack and Threads:
```bash
bt or thread backtrace: #Show the current call stack.
thread list: #List all active threads in the process.
thread select <thread_id>: #Select a specific thread for debugging.
``` 

## Other Useful Commands:
```bash
watchpoint set variable <variable_name>: #Set a watchpoint to stop execution when a variable's value changes.
expr <expression>: #Evaluate an expression on the current thread.
quit or q: #Exit the LLDB debugger.
```