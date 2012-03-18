#include "Main.h"

#include "TuringCommon/StreamManager.h"
#include "TuringCommon/RuntimeError.h"

std::string ExecutionDir;

extern "C" {
    void Turing_StdlibSFML_InitRun(const char *executionDir,
                                   TuringCommon::StreamManager *streamManager) {
        ExecutionDir = executionDir;
        TuringCommon::setErrorStreamManager(streamManager);
    }
}

std::string Turing_StdLibSFML_GetExecutionDir() {
	return ExecutionDir;
}