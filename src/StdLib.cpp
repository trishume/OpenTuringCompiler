//
//  StdLib.cpp
//  Turing Compiler
//
//  Created by Tristan Hume on 12-02-05.
//  Copyright 2012 15 Norwich Way. All rights reserved.
//

#include <iostream>
#include <stdlib.h>

#include <llvm/ADT/Twine.h>

#include "TypeManager.h"
#include "Message.h"

#include "TuringCommon/LibDefs.h"
#include "TuringCommon/StreamManager.h"

using namespace llvm;

static void MySendStringToStream(const char *text,TuringCommon::StreamManager *streamManager, TInt stream) {
    std::string errMsg;
    bool worked = streamManager->writeToStream(stream, text, &errMsg);
    if (!worked) {
        Message::runtimeError(errMsg);
    }
}
static void MyReadStringFromStream(TString *text, TInt length, 
                                   TuringCommon::StreamManager *streamManager, TInt stream) {
    std::string errMsg;
    bool worked = streamManager->readFromStream(stream, text, length, &errMsg);
    if (!worked) {
        Message::runtimeError(errMsg);
    }
}

extern "C" {
    void TuringAssert(bool val, TString *exprStr, TInt line, TString *file) {
        if (!val) {
            Message::runtimeError(Twine("Assertion failed: \"") + exprStr->strdata + 
                                  "\" at " + file->strdata + ":" + Twine(line));
        }
    }
    void TuringQuitWithCode(TInt errorCode) {
        throw errorCode;
    }
    
    void TuringSetApproximatePos(TInt line,TString *file, TInt lineRange) {
        Message::setCurLine(line, file->strdata,lineRange);
    }
    
    void TuringPrintInt(TInt num,TuringCommon::StreamManager *streamManager, TInt stream) {
        char text[TURING_STRING_LENGTH];
        sprintf(text, "%d",num);
        MySendStringToStream(text, streamManager, stream);
    }
    void TuringPrintReal(TReal num,TuringCommon::StreamManager *streamManager, TInt stream) {
        char text[TURING_STRING_LENGTH];
        sprintf(text, "%g",num);
        MySendStringToStream(text, streamManager, stream);
    }
    void TuringPrintBool(bool value,TuringCommon::StreamManager *streamManager, TInt stream) {
        MySendStringToStream((value ? "true" : "false"), streamManager, stream);
    }
    void TuringPrintString(TString *string,TuringCommon::StreamManager *streamManager, TInt stream) {
        MySendStringToStream(string->strdata, streamManager, stream);
    }
    void TuringPrintNewline(TuringCommon::StreamManager *streamManager, TInt stream) {
        char text[TURING_STRING_LENGTH];
        strncpy(text, "\n", TURING_STRING_LENGTH);
        MySendStringToStream(text, streamManager, stream);
    }
    int TuringPower(TInt a,TInt ex) {
        if ( 0==ex )  return 1;
        else
        {
            int z = a;
            int y = 1;
            while ( 1 )
            {
                if ( ex & 1 )  y *= z;
                ex /= 2;
                if ( 0==ex )  break;
                z *= z;
            }
            return y;
        }
    }
    void TuringGetString(TString *string, TInt length, TuringCommon::StreamManager *streamManager, TInt stream) {
        MyReadStringFromStream(string,length, streamManager, stream);
    }
    void TuringGetInt(TInt *numRef, TInt length, TuringCommon::StreamManager *streamManager, TInt stream) {
        TString text;
        text.length = TURING_STRING_LENGTH;
        MyReadStringFromStream(&text,length, streamManager, stream);
        *numRef = atoi(text.strdata);
    }
    int TuringStringLength(TString *string) {
        return strlen(string->strdata);
    }
    // fancy struct return. Turing signature is fcn TuringStringConcat(lhs,rhs : string) : string
    void TuringStringConcat(TString *retStr, TString* lhs, TString *rhs) {
        // TODO this is suboptimal because it uses strlen. Ignore safety and forge ahead?
        if (TuringStringLength(lhs) + TuringStringLength(rhs) > retStr->length) {
            Message::runtimeError("Tried to add two strings to create a string larger than the maximum string length");
        }
        strcpy(retStr->strdata, lhs->strdata);
        strcat(retStr->strdata, rhs->strdata);
    }
    
    bool TuringStringCompare(TString *from, TString *to) {
        return strcmp(from->strdata, to->strdata) == 0;
    }
    //! optimization of TuringCopyArray for strings
    //! also allows copying of strings of different sizes
    void TuringStringCopy(TString *from, TString *to) {
        //! TODO don't rely on strncpy to catch overflows. Implement proper length checking.
        //std::cout << "Copying string " << from->strdata << std::endl;
        strncpy(to->strdata, from->strdata,to->length);
    }
    //! copy an array, leaving the destination length element intact
    void TuringCopyArray(void *from, void *to, int fromLength, int toLength) {
        if (fromLength != toLength) {
            // TODO better runtime error handling
            Message::runtimeError(Twine("Tried to copy an array of length ") + Twine(fromLength) + 
                                  " to one of length " + Twine(toLength));
        }
        size_t fromLen = 0; // int and size_t may be different sizes so make sure the passed value is correct
        fromLen = fromLength;
        
        // doesn't have to be a string, we just want to access the length part of the struct.
        TString *castedTo = ((TString*)to);
        TInt prevLength = castedTo->length;
        
        memcpy(to,from,fromLen);
        // restore the length part of the array struct
        castedTo->length = prevLength;
    }
    bool TuringCompareArray(void *from, void *to, int fromLength, int toLength) {
        //        Message::log(Twine("copying array of length ") + Twine(fromLength) + 
        //                     " to one of length " + Twine(toLength));
        if (fromLength != toLength) {
            return false;
        }
        size_t fromLen = 0; // int and size_t may be different sizes so make sure the passed value is correct
        fromLen = fromLength;
        return memcmp(to,from,fromLen) == 0;
    }
    
    // TODO maybe get rid of this and do it in compiler to improve optimization
    bool TuringCompareRecord(void *from, void *to, int fromLen) {
        return memcmp(to,from,fromLen) == 0;
    }
    
    //! \param index the 1-based index, unchecked
    //! \param lower the lower limit of the array
    //! \param length the length of the array
    //! \returns the 0-based index, stops program if there is an index problem
    int TuringIndexArray(int index, int lower, int length) {
        if (index < lower) {
            Message::runtimeError(Twine("Can't index an array with lower bound ") + Twine(lower) +  
                                  " with the number " + Twine(index));
        }
        if (index >= lower + length) {
            int upper = lower + length - 1;
            Message::runtimeError(Twine("Can't index an array with upper limit ") + Twine(upper) +  
                                  " with the number " + Twine(index));
        }
        return index - lower;
    }
    //! arr may be null to allocate a new array
    void *TuringAllocateFlexibleArray(void *arr,TInt byteSize, TInt length) {
        size_t byteSize_size; // TInt and size_t may be different sizes so allocate a buffer.
        byteSize_size = byteSize;
        void *buffer = realloc(arr,byteSize_size);
        
        // set the length part of the struct. TString is just used for the length.
        TString *castedBuf = ((TString*)buffer);
        castedBuf->length = length;
        
        return buffer;
    }
}
