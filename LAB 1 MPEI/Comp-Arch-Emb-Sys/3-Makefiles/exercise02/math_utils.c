#include "math_utils.h"

int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

int square(int x) {
    return x * x;
}
