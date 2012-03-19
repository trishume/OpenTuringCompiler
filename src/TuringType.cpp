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
    
    if (isFlexible() && !arrTy->isFlexible()) {
        return false;
    }
    if (isAnySize() && !arrTy->isAnySize()) {
        return false;
    }
    
    return getSize() == arrTy->getSize() && // same size
    //        getLower() == arrTy->getLower() && // same lower bound, should this be enabled?
            getElementType()->compare(arrTy->getElementType()); // same type
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

TuringArrayType::TuringArrayType(TuringType *elementType, unsigned int size, bool anySize, bool flexible, int lower) : ElementType(elementType), Lower(lower), Size(size), AnySize(anySize) , Flexible(flexible)  {
    int upperSize = Size + Lower - 1;
    Twine sizeTwine = AnySize ? "*" : Twine(upperSize);
    Twine flexTwine = Flexible ? Twine("flexible ") : Twine("");
    Name = (flexTwine + "array " + Twine(Lower) + ".." + sizeTwine + " of " + ElementType->getName()).str();
}

Type *TuringArrayType::getLLVMType(bool isReference) {
    unsigned int size = AnySize ? 0 : Size;
    
    std::vector<Type*> structElements;
    structElements.push_back(Type::getInt32Ty(getGlobalContext())); //size
    structElements.push_back(ArrayType::get(ElementType->getLLVMType(),
                                            ((isReference||Flexible) ? 0 : size))); // array
    Type *arrType = StructType::get(getGlobalContext(),ArrayRef<Type*>(structElements));
    
    // flexible arrays are always pointers to a buffer
    if (Flexible) {
        return arrType->getPointerTo();
    }
    
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