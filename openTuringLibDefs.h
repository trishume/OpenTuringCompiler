#include <stdint.h>

#define TURING_STRING_LENGTH 255

typedef int32_t TInt;
typedef double TReal;

typedef struct {
    TInt length;
    char strdata[TURING_STRING_LENGTH]; // length is dummy, can be anything
} TString;