#include "Main.h"

#include "TuringCommon/StreamManager.h"
#include "TuringCommon/RuntimeError.h"

#include "WindowManager.h"
#include "Font.h"

std::string ExecutionDir;
void Turing_StdlibSFML_StandardOutputStream(TInt streamNum, const char *error);

extern "C" {
    void Turing_StdlibSFML_InitRun(const char *executionDir,
                                   TuringCommon::StreamManager *streamManager) {
        ExecutionDir = executionDir;
        TuringCommon::setErrorStreamManager(streamManager);
        
        Turing_StdlibSFML_Window_Init();
        Turing_StdlibSFML_Font_Init();
        
        // set up put to screen
        TInt stream = streamManager->registerStream(&Turing_StdlibSFML_StandardOutputStream, NULL);
        streamManager->setSpecialStream(TURINGCOMMON_STREAM_STDOUT, stream);
    }
    void Turing_StdlibSFML_FinalizeRun() {
        Turing_StdlibSFML_Window_Cleanup();
    }
}

static std::ostringstream msgBuffer;
static void MyFlushMsgBuffer() {
    Turing_StdlibSFML_Put_Line(msgBuffer.str());
    msgBuffer.str("");
}
//! this gets set as the standard error stream
//! if it is passed a string without a newline it is added to the buffer
//! when it sees a newline it takes the buffer and writes it to a Message::error
void Turing_StdlibSFML_StandardOutputStream(TInt streamNum, const char *error) {
    std::string errMsg(error);
    
    // consume newline-separated messages and write them as Message::error
    size_t lastFound = 0;
    size_t found = errMsg.find_first_of("\n");
    while (found!=std::string::npos)
    {
        msgBuffer << errMsg.substr(lastFound,found);
        MyFlushMsgBuffer();
        lastFound = found + 1; // +1 to skip over \n
        found = errMsg.find_first_of("\n",found+1);
    }
    msgBuffer << errMsg.substr(lastFound); // add the rest of the string to the buffer
}

std::string Turing_StdLibSFML_GetExecutionDir() {
	return ExecutionDir;
}