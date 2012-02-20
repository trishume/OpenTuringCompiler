//
//  Symbol.cpp
//  TuringCompiler
//
//  Created by Tristan Hume on 12-02-19.
//  Copyright 2012 15 Norwich Way. All rights reserved.
//

#include "Symbol.h"

#include <llvm/Instructions.h>

using namespace llvm;

Value *VarSymbol::getVal() {
    /*if (getType()->isComplexTy()) {
        return new BitCastInst(Val,getType()->getLLVMType(true),"castedLHS");
    }*/
    return Val;
}