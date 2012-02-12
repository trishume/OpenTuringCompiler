//
//  Type.h
//  Turing Compiler
//
//  Created by Tristan Hume on 12-02-01.
//  Copyright 2012 15 Norwich Way. All rights reserved.
//

#ifndef Turing_Compiler_Type_H
#define Turing_Compiler_Type_H

#include <string>

#include <llvm/Type.h>

#include "TuringType.h"

class TuringType {
public:
    //! \param isReference  wether this is being passed by reference
    //!                     some types like arrays have a different type when 
    //!                     passed as parameters.
    virtual llvm::Type *getLLVMType(bool isReference = false) = 0;
    virtual std::string getName() = 0;
    
    virtual bool isArrayTy() = 0;
};

class BasicTuringType : public TuringType {
public:
    BasicTuringType(std::string n, llvm::Type *type) : LLVMType(type), Name(n) {}
    //! \param isReference has no effect for basic types
    virtual llvm::Type *getLLVMType(bool isReference = false);
    virtual std::string getName();
    
    virtual bool isArrayTy() {return false;}
protected:
    llvm::Type *LLVMType;
    std::string Name;
};

class TuringArrayType : public TuringType {
public:
    TuringArrayType(TuringType *elementType, unsigned int upper) : ElementType(elementType), Size(upper) {}
    //! \param isReference returns {i32, [Size x Type]} normally {i32, [0 x Type]} by reference
    virtual llvm::Type *getLLVMType(bool isReference = false);
    TuringType *getElementType() {return ElementType;};
    virtual std::string getName();
    
    virtual bool isArrayTy() {return true;}
protected:
    TuringType *ElementType;
    unsigned int Size;
};

#endif
