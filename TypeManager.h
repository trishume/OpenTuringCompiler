//
//  TypeManager.h
//  Turing Compiler
//
//  Created by Tristan Hume on 12-02-01.
//  Copyright 2012 15 Norwich Way. All rights reserved.
//

#ifndef Turing_Compiler_TypeManager__H
#define Turing_Compiler_TypeManager__H

#include <llvm/Type.h>
#include "TuringType.h"

class TypeManager {
public:
    TuringType *getType(std::string name);
    bool addType(std::string name,TuringType *turType);
    bool addTypeLLVM(std::string name,Type *llvmType);
    bool aliasType(std::string name, std::string aliasName);
    
}

#endif
