#include <stdio.h>
#include "math_utils.h"

int main() {
    int num = 5;
    printf("The factorial of %d is: %d\n", num, factorial(num));
    printf("The square of %d is: %d\n", num, square(num));
    return 0;
}
