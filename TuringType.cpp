//
//  TuringType.cpp
//  Turing Compiler
//
//  Created by Tristan Hume on 12-02-11.
//  Copyright 2012 15 Norwich Way. All rights reserved.
//

#include "TuringType.h"

using namespace llvm;

Type *TuringType::getLLVMType() {
    return LLVMType;
}

std::string TuringType::getName() {
    return Name;
}