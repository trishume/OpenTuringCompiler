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
#include "TuringValue.h"

#define TURING_STRING_SIZE 255

class TypeManager {
public:
    TuringType *getType(std::string name);
    bool addType(std::string name,TuringType *turType);
    bool addTypeLLVM(std::string name,llvm::Type *llvmType);
    bool aliasType(TuringType *type, std::string aliasName);
    
    bool isType(TuringValue *val, std::string typeName);
    bool isArrayRef(llvm::Type *llvmType);
    
    void addDefaultTypes(llvm::LLVMContext &c);
    
private:
    
    std::map<std::string,TuringType *> NameMap;
};

#endif
