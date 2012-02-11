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

class TuringType {
public:
    TuringType(std::string n, llvm::Type *type) : LLVMType(type), Name(n) {}
    virtual llvm::Type *getLLVMType();
    virtual std::string getName();
    
protected:
    llvm::Type *LLVMType;
    std::string Name;
};

#endif
