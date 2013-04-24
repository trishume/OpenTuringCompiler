#include "TuringCommon/StreamManager.h"
#include "TuringCommon/RuntimeError.h"

extern "C" {
    void Turing_Stdlib_InitRun(const char *executionDir,
                                   TuringCommon::StreamManager *streamManager) {
        TuringCommon::setErrorStreamManager(streamManager);
    }
}