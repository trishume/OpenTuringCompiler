#include "TuringCommon/RuntimeError.h"

#include <iostream>
#include <sstream>

#include "TuringCommon/StreamManager.h"

namespace TuringCommon {
    static TuringCommon::StreamManager *TheStreamManager = NULL;
    void runtimeError(const char *errMsg, bool isWarning) {
        if (TheStreamManager == NULL) {
            std::cerr << "ERROR: Stream manager not set in TuringCommon::runtimeError";
            exit(1);
        }
        std::ostringstream os;
        os << errMsg << "\n";
        TheStreamManager->writeToStream(-3, os.str().c_str());
        if(!isWarning) throw 1; // unwind to executor run() function
    }
    void setErrorStreamManager(TuringCommon::StreamManager *streamManager) {
        TheStreamManager = streamManager;
    }
}
