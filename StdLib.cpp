//
//  StdLib.cpp
//  Turing Compiler
//
//  Created by Tristan Hume on 12-02-05.
//  Copyright 2012 15 Norwich Way. All rights reserved.
//

#include <iostream>

typedef long TInt;


extern "C" {
    void TuringPrintInt(TInt num) {
        std::cout << num << std::endl;
    }
    void TuringPrintBool(bool value) {
        std::cout << (value ? "true" : "false") << std::endl;
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
}
