#include <iostream>

void turingRuntimeError(const char *errMsg, bool isWarning) {
	std::string message = isWarning ? "WARNING" : "ERROR";
	std::cout << message << ": " << errMsg << std::endl;
	if(!isWarning) exit(2);
}