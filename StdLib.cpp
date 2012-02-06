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
}
