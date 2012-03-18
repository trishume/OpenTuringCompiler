#ifndef _TuringCommon_StreamManager_H_
#define _TuringCommon_StreamManager_H_

#include <map>

#include "TuringCommon/IDManager.h"
#include "TuringCommon/LibDefs.h"

#define TURINGCOMMON_STREAM_READ_TOKEN -1
#define TURINGCOMMON_STREAM_READ_LINE -2

#define TURINGCOMMON_STREAM_STDOUT -1
#define TURINGCOMMON_STREAM_STDIN -2
#define TURINGCOMMON_STREAM_STDERR -3

namespace TuringCommon {
    /*!
     * Manages streams for the runtime. Keeps track of all the streams that are provided by plugin libraries.
     * A pointer to this is passed to the plugin libraries so they can create and manage streams.
     * 
     * Streams are used by the Turing "get" and "put" commands to read and write textual data to a variety
     * of locations. For example, the screen, files and the network.
     *
     * Streams are really just function pointers. Both types of stream function take a TInt stream number and a (TString *)
     * as parameters. The stream number is passed so that one function can be used for multiple streams and the
     * number can be used to disambiguate. Read streams are passed an aditional TInt length parameter.
     * The function pointers are provided by libraries using registerStream.
     *
     * For write streams the function should write the TString to the stream.
     *
     * For read streams the function should fill the TString with `length` characters of data. 
     * Length can also take on special negative values that represent reading one token
     * or one whole line. (see TURINGCOMMON_STREAM_READ_LINE and TURINGCOMMON_STREAM_READ_TOKEN)
     *
     * Streams are referred to by stream number. When a program wants to read or write to a stream the stream
     * manager looks up the function for the stream number provided and calls it. There are also special default
     * streams with negative numbers. For example standard output for when "put" is called without a stream number
     * is -1. The standard error stream is -3 (different from the original Turing where it is 0). These can be set
     * by libraries to refer to their streams. For example the SFML standard library redirects standard output to
     * write to the screen.
     */
    class StreamManager {
    public:
        // typedefs for the function pointers so we don't have
        // to use the ugly syntax...
        typedef void (*WriteStreamFunc)(TInt,const char*);
        typedef void (*ReadStreamFunc)(TInt,TString*,TInt);
        
        StreamManager() : Streams("Stream") {}
        
        //! convenience method to initialize all the special streams
        //! with stdin, stdout and stderr
        void initWithStandardStreams();

        //! registers a stream. Passed two stream functions.
        //! Either of the function pointers may be NULL if the stream
        //! does not support reading or writing. If both streams are NULL
        //! -1 is returned. THIS IS NOT A NEW STREAM NUMBER.
        //! \returns a stream number for the new stream. Or a negative number on error.
        TInt registerStream(WriteStreamFunc outStream,ReadStreamFunc inStream);
        //! removes a stream from the registry so it is no longer valid.
        void closeStream(TInt streamNumber);
        //! redirects a special stream
        //! \param specialStream the stream to redirect. Must be a negative number.
        //! \param streamNumber the normal stream to direct it to.
        //! \returns true on success. returns false and sets errMsg on error.
        bool setSpecialStream(TInt specialStream, TInt streamNumber, std::string *errMsg = NULL);

        //! reads from a stream and fills buffer with the result.
        //! \returns true on success. returns false and sets errMsg on error.
        bool readFromStream(TInt streamNumber, TString *buffer, TInt length, std::string *errMsg = NULL);
        //! writes text to a stream.
        //! \returns true on success. returns false and sets errMsg on error.
        bool writeToStream(TInt streamNumber, const char *text, std::string *errMsg = NULL);
    protected:
        struct TuringStream {
            TuringStream() : WriteFunc(NULL), ReadFunc(NULL) {}
            WriteStreamFunc WriteFunc;
            ReadStreamFunc ReadFunc;
        };
        bool assertValidStream(TInt streamNumber, std::string *errMsg);
        //! gets a new stream number, following special stream redirection
        TInt getSpecialStream(TInt specialStream);
        
        //! manages normal stream numbers
        IDManager<TuringStream> Streams;
        //! maps special stream number -> normal stream
        std::map<TInt,TInt> SpecialStreams;
    };
}

#endif