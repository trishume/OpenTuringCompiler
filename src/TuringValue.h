//
//  TuringValue.h
//  TuringCompiler
//
//  Created by Tristan Hume on 12-02-20.
//  Copyright 2012 15 Norwich Way. All rights reserved.
//

#ifndef TuringCompiler_TuringValue_H
#define TuringCompiler_TuringValue_H

#include <llvm/Value.h>

//! an LLVM value that includes a type
class TuringValue
{
public:
    TuringValue(llvm::Value *val, TuringType *type) : Type(type), Value(val) {}
    ~TuringValue();

    TuringType *getType() {return Type;};
    llvm::Value *getVal() {return Value;};

    /* data */
protected:
    TuringType *Type;
    llvm::Value *Value;
};

#endif