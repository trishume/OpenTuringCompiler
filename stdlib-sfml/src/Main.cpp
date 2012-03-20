#include "Main.h"

#include "TuringCommon/StreamManager.h"
#include "TuringCommon/RuntimeError.h"

#include "WindowManager.h"

std::string ExecutionDir;

extern "C" {
    void Turing_StdlibSFML_InitRun(const char *executionDir,
                                   TuringCommon::StreamManager *streamManager) {
        ExecutionDir = executionDir;
        TuringCommon::setErrorStreamManager(streamManager);
        
        Turing_StdlibSFML_Window_Init();
    }
    void Turing_StdlibSFML_FinalizeRun() {
        Turing_StdlibSFML_Window_Cleanup();
    }
}

std::string Turing_StdLibSFML_GetExecutionDir() {
	return ExecutionDir;
}