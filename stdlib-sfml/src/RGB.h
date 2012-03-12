#ifndef _TURING_RGB_H_
#define _TURING_RGB_H_

#include "openTuringLibDefs.h"
#include "openTuringRuntimeError.h"

//! lookup table for all the turing colour numbers
extern char TuringPalette[256][3];

char *getRGBColourFromNum(TInt num);

#endif