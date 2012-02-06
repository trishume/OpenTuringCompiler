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

#include "Message.h"

using namespace llvm;

TuringType *TypeManager::getType(std::string name){
    if (NameMap.find(name) == NameMap.end()) {
        throw Message::Exception(llvm::Twine("Can't find type named ") + name);
    }
    return NameMap[name];
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
    NameMap[name] = new TuringType(name,llvmType);
    return true;
}
bool TypeManager::aliasType(std::string name, std::string aliasName){
    if (NameMap.find(name) == NameMap.end()) {
        Message::error(llvm::Twine("Can't alias type named ") + name);
        return false;
    }
    NameMap[aliasName] = new TuringType(aliasName,NameMap[name]->LLVMType);
    return true;
}

void TypeManager::addDefaultTypes(LLVMContext &c) {
    // ints
    addTypeLLVM("int",(Type*)Type::getInt64Ty(c));
    addTypeLLVM("int8",(Type*)Type::getInt8Ty(c));
    addTypeLLVM("int32",(Type*)Type::getInt32Ty(c));
    
    addTypeLLVM("boolean",(Type*)Type::getInt1Ty(c));
    
    addTypeLLVM("real",(Type*)Type::getDoublePtrTy(c));
    
    addTypeLLVM("string",(Type*)ArrayType::get((Type*)Type::getInt8Ty(c),TURING_STRING_SIZE));
    
    addTypeLLVM("void",(Type*)Type::getVoidTy(c));
}