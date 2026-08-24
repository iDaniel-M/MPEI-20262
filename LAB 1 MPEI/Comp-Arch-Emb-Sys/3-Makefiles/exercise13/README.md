#Discover & build unit tests automatically

Assume src/ has your library code and tests/ contains test files named test_*.c. Write a Makefile that:

* builds a library libapp.a from src/*.c
* builds each tests/test_*.c into a test binary under build/tests/
* provides test target that runs all tests and reports failures
* supports make -j
