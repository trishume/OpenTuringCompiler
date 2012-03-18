#include "Main.h"

std::string ExecutionDir;

extern "C" {
    void Turing_StdlibSFML_InitRun(const char *executionDir) {
        ExecutionDir = executionDir;
    }
}

std::string Turing_StdLibSFML_GetExecutionDir() {
	return ExecutionDir;
}