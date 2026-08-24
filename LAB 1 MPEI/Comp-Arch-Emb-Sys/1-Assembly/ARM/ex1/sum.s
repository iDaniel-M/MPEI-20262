// Write a program that counts from 1 to 31
// Run and test the program using https://cpulator.01xz.net/?sys=arm

.global _start
_start:
	mov r0, #0 			// Initialize sum to 0       
	mov r1, #1        	// Initialize counter to 1
loop:
	add r0, r0, r1    	// Add counter to sum
	add r1, r1, #1      // Increment counter
	cmp r1, #31         // Compare counter with 31
	blt loop      		// If counter < 31, branch to loop    
end:
    b .                 // finish in an infity loop