//
//  StdLib.cpp
//  Turing Compiler
//
//  Created by Tristan Hume on 12-02-05.
//  Copyright 2012 15 Norwich Way. All rights reserved.
//

#include <iostream>

#include <llvm/ADT/Twine.h>

#include "TypeManager.h"
#include "Message.h"

typedef int TInt;

using namespace llvm;


typedef struct {
    int length;
    char strdata[255]; // length is dummy, can be anything
} TString;

extern "C" {
    void TuringPrintInt(TInt num) {
        std::cout << num;
    }
    void TuringPrintBool(bool value) {
        std::cout << (value ? "true" : "false");
    }
    void TuringPrintString(TString *string) {
        std::cout << string->strdata;
    }
    void TuringPrintNewline() {
        std::cout << std::endl;
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
    void TuringCopyArray(void *from, void *to, int fromLength, int toLength) {
//        Message::log(Twine("copying array of length ") + Twine(fromLength) + 
//                     " to one of length " + Twine(toLength));
        if (fromLength > toLength) {
            // TODO better runtime error handling
            Message::error("Tried to copy an array to a smaller one.");
            exit(1);
        }
        size_t fromLen = 0; // int and size_t may be different sizes so make sure the passed value is correct
        fromLen = fromLength;
        memcpy(to,from,fromLen);
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
    
    //! \param index the 1-based index, unchecked
    //! \param length the length of the array
    //! \returns the 0-based index, stops program if there is an index problem
    int TuringIndexArray(int index, int length) {
        if (index <= 0) {
            Message::error(Twine("Can't index an array with the negative value of ") + Twine(index));
            exit(1);
        }
        if (index > length) {
            Message::error(Twine("Can't index an array of size") + Twine(length) +  " with the number " + Twine(index));
            exit(1);
        }
        return index - 1;
    }
}
