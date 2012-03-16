#ifndef _TuringCommon_LibDefs_H_
#define _TuringCommon_LibDefs_H_

#include <stdint.h>

#define TURING_STRING_LENGTH 255

typedef int32_t TInt;
typedef double TReal;

typedef struct {
    TInt length;
    char strdata[TURING_STRING_LENGTH]; // length is dummy, can be anything
} TString;

typedef struct {
    TInt length;
    TInt data[1]; // length is dummy, can be anything
} TIntArray;

#endif