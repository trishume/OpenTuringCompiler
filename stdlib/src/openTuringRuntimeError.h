#include <iostream>

void turingRuntimeError(const char *errMsg) {
	std::cout << "ERROR: " << errMsg << std::endl;
	exit(2);
}