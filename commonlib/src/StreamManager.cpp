#include "TuringCommon/StreamManager.h"

#include <sstream>
#include <iostream>
#include <limits>

// copy-pasted from the internet. Stops stupid behaviour with stream buffering
template <typename CharT>
static std::streamsize ignore_line (
                             std::basic_istream<CharT>& in, bool always_discard = false )
{
    std::streamsize nread = 0;
    
    if ( always_discard
        || ( in.rdbuf()->sungetc() != std::char_traits<CharT>::eof()
            && in.get() != in.widen ( '\n' ) ) )
    {
        // The stream is good, and we haven't
        // read a full line yet, so clear it out
        in.ignore ( std::numeric_limits<std::streamsize>::max(), in.widen ( '\n' ) );
        nread = in.gcount();
    }
    
    return nread;
}

// TODO these are just testing methods. For example, they can overflow the buffer.
// maybe make them more bulletproof?
static void StreamStdOut(TInt streamNum,const char* text) {
    std::cout << text;
}
static void StreamStdErr(TInt streamNum,const char* text) {
    std::cerr << text;
}
static void StreamStdIn(TInt streamNum,TString* buffer, TInt length) {
    if (length == TURINGCOMMON_STREAM_READ_LINE) {
        ignore_line(std::cin);
        std::string inString;
        std::getline(std::cin,inString);
        strncpy(buffer->strdata, inString.c_str(), buffer->length);
    } else {
        std::cin >> buffer->strdata;
    }
}

namespace TuringCommon {
    void StreamManager::initWithStandardStreams() {
        TInt stream;
        
        stream = registerStream(&StreamStdErr, NULL);
        setSpecialStream(TURINGCOMMON_STREAM_STDERR, stream);
        
        stream = registerStream(&StreamStdOut, NULL);
        setSpecialStream(TURINGCOMMON_STREAM_STDOUT, stream);
        
        stream = registerStream(NULL, &StreamStdIn);
        setSpecialStream(TURINGCOMMON_STREAM_STDIN, stream);
    }
    
	TInt StreamManager::registerStream(WriteStreamFunc outStream, ReadStreamFunc inStream) {
        TInt newStreamId = Streams.getNew();
        TuringStream *newStream = Streams.get(newStreamId);
        // can't create a stream that can't read or write
        if (outStream == NULL && inStream == NULL) {
            return -1;
        }
        newStream->WriteFunc = outStream;
        newStream->ReadFunc = inStream;
        return newStreamId;
    }
    
    void StreamManager::closeStream(TInt streamNumber) {
        Streams.remove(streamNumber);
    }
    
    bool StreamManager::setSpecialStream(TInt specialStream, TInt streamNumber, std::string *errMsg) {
        if (specialStream >= 0) {
            if(errMsg) *errMsg = "Special stream number to redirect must be negative.";
            return false;
        }
        if (!assertValidStream(streamNumber, errMsg)) return false;
        
        SpecialStreams[specialStream] = streamNumber;
        
        return true;
    }
    
    bool StreamManager::readFromStream(TInt streamNumber, TString *buffer, TInt length, std::string *errMsg) {
        streamNumber = getSpecialStream(streamNumber);
        if (!assertValidStream(streamNumber, errMsg)) return false;
        TuringStream *stream = Streams.get(streamNumber);
        if (stream->ReadFunc == NULL) {
            if(errMsg) *errMsg = "Tried to read from a stream with no read functionality.";
            return false;
        }
        // call the function pointer
        (*(stream->ReadFunc))(streamNumber,buffer,length);
        return true;
    }
    
    bool StreamManager::writeToStream(TInt streamNumber, const char *text, std::string *errMsg) {
        streamNumber = getSpecialStream(streamNumber);
        if (!assertValidStream(streamNumber, errMsg)) return false;
        TuringStream *stream = Streams.get(streamNumber);
        if (stream->WriteFunc == NULL) {
            if(errMsg) *errMsg = "Tried to write to a stream with no write functionality.";
            return false;
        }
        // call the function pointer
        (*(stream->WriteFunc))(streamNumber,text);
        return true;
    }
    
    bool StreamManager::assertValidStream(TInt streamNumber, std::string *errMsg) {
        if (!Streams.exists(streamNumber)) {
            if(errMsg) {
                std::ostringstream os;
                os << "Stream number " << streamNumber << " does not exist.";
                *errMsg = os.str();
            }
            return false;
        }
        return true;
    }
    
    TInt StreamManager::getSpecialStream(TInt streamNum) {
        if (SpecialStreams.find(streamNum) == SpecialStreams.end()) {
            return streamNum;
        }
        return SpecialStreams[streamNum];
    }
}