// Include function prototypes. The function itself will be linked at the end
#include "function.h"
#include <stdio.h>

// Define a constant to see what the preprocessor does
#define N 5

int main(int argc, char **argv){

	const int a = 2 + N;
	const int b = N - 3;

	int c = add(a, b);

	printf("Your result is %d!\n", c);

	return 0;
}
