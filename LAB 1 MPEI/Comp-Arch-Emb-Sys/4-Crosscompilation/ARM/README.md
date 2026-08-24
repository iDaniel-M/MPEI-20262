# Baremetal examples

### These exercises were adapted to work with ARM QEMU, the original exercises were taken from the repository https://github.com/johnwinans/rvddt/tree/master of John Winans

These illustrate an approach to initializing an address space suitable
for running a C program that uses newlib.

*** freestanding example apps:


stand01:
	Implement crt0.S for ARM Cortex-M processor

stand02:
	- add some variables to the data and bss regions
	- now there is somehting to zero-out
	- note that the text region now is zero-padded in order to be able to initialize the page-aligned data region
	- see the bss region get zeroed out this time

stand03:
	BL main  -->  main() { return 0x8675309 }

	- how does 'BL' know to use LR as the return address register?????? 
		Because the ABI says so!

stand04:
	call main() and set the value of a variable in the BSS

stand05:
	call main() and set the values from variables in the DATA and BSS regions.

stand06:
	Create a char array in main() that is not used.  Even with an initializer it is optimized away!

stand07: (error)
	Create char array in main() and then use it.
	This examle fails due to missing memset()!!

stand08:
	Add -lgcc and -lc libraries.

stand09: (error)
	Add a call to malloc() that fails because there is no sbrk()

stand10: (note that the text region is too big to hardcode the data to start at 0x1000)
	Add stubs for the newlib's stdlib dependancies in stub_stdlib.c
	Demonstrate that malloc() is working properly.


Conclusion: 

We can now execute single-threaded C programs that can use functions from the
standard library.  There are no provisions, however, for things like I/O.


