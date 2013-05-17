#ifndef _TURING_RGB_H_
#define _TURING_RGB_H_

#include "TuringCommon/LibDefs.h"
#include "TuringCommon/RuntimeError.h"

//! lookup table for all the turing colour numbers
extern char TuringPalette[256][3];

void setRGBCoulourForNum(TInt num, char r, char g, char b);
char *getRGBColourFromNum(TInt num);

#endif