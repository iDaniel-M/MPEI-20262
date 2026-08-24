# Single-file build with run/clean targets  

APP := app                              # Sets the name of the final executable to 'app'
$(APP): main.c                          # Declares that 'app' depends on main.c; if main.c is newer, rebuild
	cc -Wall -O2 $< -o $@           # Compile and link: $< is 'main.c', $@ is 'app'; enable warnings and O2
.PHONY: run                             # Marks 'run' as a phony target so it always runs, even if a file named 'run' exists
run: $(APP)                             # 'run' depends on the binary so it builds first if needed
	./$(APP)                        # Executes the freshly built program
.PHONY: clean                           # Marks 'clean' as phony to avoid filename conflicts
clean:                                  # Target to remove build artifacts
	$(RM) $(APP)                    # Deletes the executable using make's portable remove command
