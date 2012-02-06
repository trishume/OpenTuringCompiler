//
//  Type.h
//  Turing Compiler
//
//  Created by Tristan Hume on 12-02-01.
//  Copyright 2012 15 Norwich Way. All rights reserved.
//

#ifndef Turing_Compiler_Type_H
#define Turing_Compiler_Type_H

#include <string>

#include <llvm/Type.h>

#include "TuringType.h"

struct TuringType {
    TuringType(std::string n, llvm::Type *type) : LLVMType(type), Name(n) {}
    llvm::Type *LLVMType;
    std::string Name;
};

#endif
