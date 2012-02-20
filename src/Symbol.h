//
//  Symbol.h
//  TuringCompiler
//
//  Created by Tristan Hume on 12-02-19.
//  Copyright 2012 15 Norwich Way. All rights reserved.
//

#ifndef TuringCompiler_Symbol_H
#define TuringCompiler_Symbol_H

#include <string>


#include <llvm/Function.h>
#include <llvm/Value.h>

#include "TuringType.h"


//! abstract class representing things in the scope system: variables and functions
class Symbol {
public:
    virtual TuringType *getType() = 0;
    virtual llvm::Value *getVal() = 0;
    
    virtual bool isFunction() = 0;
protected:
    Symbol(){}
};

//! represents a variable. The value must be a pointer type unless it is an argument
class VarSymbol : public Symbol {
public:
    VarSymbol() : Val(NULL), Type(NULL) {}
    VarSymbol(llvm::Value *val, TuringType *type) : Val(val), Type(type) {}
    virtual TuringType *getType() {return Type;}
    virtual llvm::Value *getVal();
    
    virtual bool isFunction() {return false;}
protected:
    llvm::Value *Val;
    TuringType *Type;
};

//! represents a function. getType is the return type and getVal is the Function*
class FunctionSymbol : public Symbol {
public:
    FunctionSymbol(llvm::Function *llvmFunc, TuringType *type) : IsSRet(false), Func(llvmFunc), RetType(type)  {}
    virtual TuringType *getType() {return RetType;}
    virtual llvm::Value *getVal() {return Func;}
    
    virtual bool isFunction() {return true;}
    llvm::Function *getFunc() {return Func;}
    
    //! is the first parameter a pointer to where the return value should go?
    bool IsSRet;
protected:
    llvm::Function *Func;
    TuringType *RetType;
};

#endif
