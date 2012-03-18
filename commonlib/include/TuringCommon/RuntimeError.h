#ifndef _TuringCommon_RuntimeError_H_
#define _TuringCommon_RuntimeError_H_

namespace TuringCommon {
    class StreamManager; // forward decl
    //! raise a runtime error and halt the execution of the program. Does not return.
    //! \param isWarning set to true for a non-fatal error. Function returns when this is true.
	void runtimeError(const char *errMsg, bool isWarning = false);
    void setErrorStreamManager(TuringCommon::StreamManager *streamManager);
}

#endif