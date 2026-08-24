# Exercise 5 — makefile

You are given the following Makefile:

```makefile
CC = gcc
CFLAGS = -Wall -g
TARGET = program
SRC = main.c math_operations.c string_operations.c
OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

```

Questions:

* What is the purpose of all the rules in this Makefile?
* Explain what the %.o: %.c rule does.
* What does the clean rule do, and when would you use it?
