//
//  TypeManager.cpp
//  Turing Compiler
//
//  Created by Tristan Hume on 12-02-01.
//  Copyright 2012 15 Norwich Way. All rights reserved.
//
#include "TypeManager.h"

#include <llvm/ADT/Twine.h>
#include <llvm/DerivedTypes.h>
#include <llvm/LLVMContext.h>

#include "Message.h"

using namespace llvm;

TuringType *TypeManager::getType(std::string name){
    if (NameMap.find(name) == NameMap.end()) {
        throw Message::Exception(llvm::Twine("Can't find type named ") + name);
    }
    return NameMap[name];
}
TuringType *TypeManager::getArrayType(TuringType *elementType, unsigned int upper) {
    //TODO LEAK this may never get released
    return new TuringArrayType(elementType, upper);
}

//! finds an existing TuringType of the specified LLVM Type.
TuringType *TypeManager::getTypeLLVM(Type *llvmType, bool isReference){
    std::map<std::string,TuringType*>::const_iterator it;
    for (it = NameMap.begin(); it != NameMap.end(); ++it) {
        TuringType *type = it->second;
        if (type->getLLVMType(isReference) == llvmType) {
            return type;
        }
    }
    
    /*if (llvmType->isPointerTy()) {
        TuringType *sub = getTypeLLVM(cast<PointerType>(llvmType)->getElementType());
        return BasicTuringType(Twine("pointer to ") + sub->getName(),
     }*/
    
    // is it an array
    if (llvmType->isStructTy() && cast<StructType>(llvmType)->getNumElements() == 2 && 
        cast<StructType>(llvmType)->getElementType(0) == getType("int")->getLLVMType() &&
        cast<StructType>(llvmType)->getElementType(1)->isArrayTy()) 
    {
        // it's an array so construct a type for it
        ArrayType *arrTy = cast<ArrayType>(cast<StructType>(llvmType)->getElementType(1));
        return getArrayType(getTypeLLVM(arrTy->getElementType()), arrTy->getNumElements());
    }
    
    // couldn't find it, throw exception    
    throw Message::Exception("Can't find correct type.");
    return NULL;
}
bool TypeManager::addType(std::string name,TuringType *turType){
    if (NameMap.find(name) != NameMap.end()) {
        Message::error(llvm::Twine("Type ") + name + " already exists.");
        return false;
    }
    NameMap[name] = turType;
    return true;
}
bool TypeManager::addTypeLLVM(std::string name,Type *llvmType){
    if (NameMap.find(name) != NameMap.end()) {
        Message::error(Twine("Type ") + name + " already exists.");
        return false;
    }
    NameMap[name] = new BasicTuringType(name,llvmType);
    return true;
}
bool TypeManager::aliasType(std::string name, std::string aliasName){
    if (NameMap.find(name) == NameMap.end()) {
        Message::error(llvm::Twine("Can't alias type named ") + name);
        return false;
    }
    NameMap[aliasName] = NameMap[name];
    return true;
}

bool TypeManager::isType(Value *val, std::string typeName) {
    return val->getType() == getType(typeName)->getLLVMType(true);
}

void TypeManager::addDefaultTypes(LLVMContext &c) {
    // ints
    addTypeLLVM("int",(Type*)Type::getInt32Ty(c));
    addTypeLLVM("int8",(Type*)Type::getInt8Ty(c));
    addTypeLLVM("int64",(Type*)Type::getInt64Ty(c));
    
    addTypeLLVM("boolean",(Type*)Type::getInt1Ty(c));
    
    addTypeLLVM("real",(Type*)Type::getDoublePtrTy(c));
    
    //addTypeLLVM("string",(Type*)ArrayType::get((Type*)Type::getInt8Ty(c),TURING_STRING_SIZE)); // char[255]
    TuringArrayType *strType = new TuringArrayType(getType("int8"),256);
    strType->setName("string");
    addType("string",strType); // char*
    
    addTypeLLVM("pointer to void",(Type*)Type::getInt8Ty(c)->getPointerTo());
    //aliasType("string","pointer to void");
    
    addTypeLLVM("void",(Type*)Type::getVoidTy(c));
    addTypeLLVM("auto",(Type*)Type::getVoidTy(c));
}