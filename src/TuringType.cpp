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

#include "Message.h"

using namespace llvm;

bool TuringType::compare(TuringType *other) {
    // LLVM types are all unique. Pointer comparison always checks for type equivelancy.
    return getLLVMType(false) == other->getLLVMType(false);
}

bool TuringArrayType::compare(TuringType *other) {
    if (!other->isArrayTy()) {
        return false;
    }
    
    TuringArrayType *arrTy = static_cast<TuringArrayType*>(other);
    
    return getSize() == arrTy->getSize() && getElementType()->compare(arrTy->getElementType());
}

Type *BasicTuringType::getLLVMType(bool isReference) {
    return LLVMType;
}

std::string BasicTuringType::getName() {
    return Name;
}

void BasicTuringType::setName(const std::string &newName) {
    Name = newName;
}

TuringArrayType::TuringArrayType(TuringType *elementType, unsigned int upper) : ElementType(elementType), Size(upper) {
    Twine sizeTwine = Size == 0 ? "*" : Twine(Size);
    Name = (Twine("array 1..") + sizeTwine + " of " + ElementType->getName()).str();
}

Type *TuringArrayType::getLLVMType(bool isReference) {
    std::vector<Type*> structElements;
    structElements.push_back(Type::getInt32Ty(getGlobalContext())); //size
    structElements.push_back(ArrayType::get(ElementType->getLLVMType(),(isReference ? 0 : Size))); // array
    Type *arrType = StructType::get(getGlobalContext(),ArrayRef<Type*>(structElements));
    
    if (isReference) {
        return arrType->getPointerTo();
    } else {
        return arrType;
    }
}

std::string TuringArrayType::getName() {
    return Name;
}

TuringRecordType::TuringRecordType(std::vector<VarDecl> elements) : Elems(elements) {
    Twine nameBuilder("record of");
    for (unsigned int i = 0; i < Elems.size(); ++i) {
        // build name
        nameBuilder.concat(" (");
        nameBuilder.concat(Elems[i].Type->getName());
        nameBuilder.concat(")");
        // build field map
        NameToIndex[Elems[i].Name] = i;
    }
    Name = nameBuilder.str();
}

Type *TuringRecordType::getLLVMType(bool isReference) {
    std::vector<Type*> structElements;
    
    for (unsigned int i = 0; i < Elems.size(); ++i) {
        structElements.push_back(Elems[i].Type->getLLVMType(false));
    }
    
    Type *type = StructType::get(getGlobalContext(),ArrayRef<Type*>(structElements));
    
    if (isReference) {
        return type->getPointerTo();
    } else {
        return type;
    }
}

unsigned int TuringRecordType::getIndex(std::string field) {
    if (NameToIndex.find(field) == NameToIndex.end()) {
        throw Message::Exception(Twine("This record type does not have a field named ") + field);
    }
    return NameToIndex[field];
}