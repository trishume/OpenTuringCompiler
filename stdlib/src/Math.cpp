//
//  StdLib.cpp
//  Turing Compiler
//
//  Created by Tristan Hume on 12-02-05.
//  Copyright 2012 15 Norwich Way. All rights reserved.
//

#include <math.h>
#include <ctime>
#include <cstdlib>

#include "TuringCommon/LibDefs.h" 

static TReal MyDegToRad(TReal deg) {
	return deg * (M_PI / 180.0);
}



extern "C" {
    TReal Turing_Stdlib_Math_Arctand(TReal deg) {
        return atan(MyDegToRad(deg));
    }
    TReal Turing_Stdlib_Math_Cosd(TReal deg) {
        return cos(MyDegToRad(deg));
    }
    TReal Turing_Stdlib_Math_Sind(TReal deg) {
        return sin(MyDegToRad(deg));
    }

    // these wrappers are needed to allow C++ overloading to the double versions
    TReal Turing_Stdlib_Math_Arctan(TReal rad) {
        return atan(rad);
    }
    TReal Turing_Stdlib_Math_Cos(TReal rad) {
        return cos(rad);
    }
    TReal Turing_Stdlib_Math_Sin(TReal rad) {
        return sin(rad);
    }
    TReal Turing_Stdlib_Math_Sqrt(TReal val) {
        return sqrt(val);
    }
    TReal Turing_Stdlib_Math_Exp(TReal val) {
        return exp(val);
    }
    TReal Turing_Stdlib_Math_Ln(TReal val) {
        return log(val);
    }

    //
	// Generate a random number between 0 and 1
	// return a uniform number in [0,1].
	double Turing_Stdlib_Rand_Real() {
	    return rand() / double(RAND_MAX);
	}
	TInt Turing_Stdlib_Rand_Int(TInt start, TInt end) {
	    return (rand() % (end+1-start)) + start;
	}
	//
	// Reset the random number generator with the system clock.
	void Turing_Stdlib_Rand_Set(TInt s) {
	    srand(s);
	}

	void Turing_Stdlib_Rand_Randomize(TInt s) {
	    Turing_Stdlib_Rand_Set(time(0));
	}
}
