#include <stdint.h>

typedef int32_t TInt;
typedef double TReal;

typedef struct {
    TInt length;
    char strdata[255]; // length is dummy, can be anything
} TString;