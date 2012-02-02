//
//  Type.h
//  Turing Compiler
//
//  Created by Tristan Hume on 12-02-01.
//  Copyright 2012 15 Norwich Way. All rights reserved.
//

#ifndef Turing_Compiler_Type_H
#define Turing_Compiler_Type_H

#include <llvm/Type.h>

class TuringType {
    llvm::Type CompileType;
    std::string Name;
};

#endif
