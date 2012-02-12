//
//  TuringType.cpp
//  Turing Compiler
//
//  Created by Tristan Hume on 12-02-11.
//  Copyright 2012 15 Norwich Way. All rights reserved.
//

#include "TuringType.h"

#include <vector>

#include <llvm/LLVMContext.h>
#include <llvm/DerivedTypes.h>
#include <llvm/ADT/Twine.h>
#include <llvm/ADT/ArrayRef.h>

using namespace llvm;

Type *BasicTuringType::getLLVMType(bool isReference) {
    return LLVMType;
}

std::string BasicTuringType::getName() {
    return Name;
}

Type *TuringArrayType::getLLVMType(bool isReference) {
    std::vector<Type*> structElements;
    structElements.push_back(Type::getInt32Ty(getGlobalContext())); //size
    structElements.push_back(ArrayType::get(ElementType->getLLVMType(),(isReference ? 0 : Size))); // array
    return StructType::get(getGlobalContext(),ArrayRef<Type*>(structElements));
}

std::string TuringArrayType::getName() {
    return (Twine("array 1..") + Twine(Size) + " of " + ElementType->getName()).str();
}