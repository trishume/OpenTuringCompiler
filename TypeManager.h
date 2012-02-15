//
//  TypeManager.h
//  Turing Compiler
//
//  Created by Tristan Hume on 12-02-01.
//  Copyright 2012 15 Norwich Way. All rights reserved.
//

#ifndef Turing_Compiler_TypeManager__H
#define Turing_Compiler_TypeManager__H

#include <map>

#include <llvm/Type.h>
#include <llvm/Value.h>
#include "TuringType.h"

#define TURING_STRING_SIZE 255

class TypeManager {
public:
    TuringType *getType(std::string name);
    TuringType *getArrayType(TuringType *elementType, unsigned int upper);
    TuringType *getTypeLLVM(llvm::Type *llvmType, bool isReference = false);
    bool addType(std::string name,TuringType *turType);
    bool addTypeLLVM(std::string name,llvm::Type *llvmType);
    bool aliasType(std::string name, std::string aliasName);
    
    bool isType(llvm::Value *val, std::string typeName);
    
    void addDefaultTypes(llvm::LLVMContext &c);
    
private:
    
    std::map<std::string,TuringType *> NameMap;
};

#endif
