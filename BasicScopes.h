//
//  BasicScopes.h
//  Turing Compiler
//
//  Created by Tristan Hume on 12-02-07.
//  Copyright 2012 15 Norwich Way. All rights reserved.
//

#ifndef Turing_Compiler_BasicScopes_H
#define Turing_Compiler_BasicScopes_H

#include <string>
#include <map>

#include <llvm/Function.h>
#include <llvm/Module.h>
#include <llvm/Value.h>

#include "Scope.h"
#include "TuringType.h"

//! simple common functionality
class BasicScope : public Scope {
    virtual llvm::Value *resolveVarThis(std::string name);
    //! sets a variable name to reference a specific value
    virtual void setVar(std::string name, llvm::Value *val);
    
protected:
    BasicScope(Scope *parent) : Scope(parent) {}
    virtual bool isDeclaredThis(std::string name);
    
    std::map<std::string,llvm::Value*> symbols;
};

class LocalScope : public BasicScope {
public:
    //! \param curFunc Function in wich the variables should be allocated
    LocalScope(llvm::Function *curFunc, Scope *parent);
    virtual Scope *createChildScope();
    
    virtual llvm::Value *declareVar(std::string name, TuringType *type);
protected:
    llvm::Function *TheFunction;
};

class GlobalScope : public BasicScope {
public:
    //! \param mod The module that the globals are declared in
    GlobalScope(llvm::Module *mod, Scope *parent);
    virtual Scope *createChildScope();
    
    virtual llvm::Value *declareVar(std::string name, TuringType *type);
protected:
    llvm::Module *TheModule;
};

#endif
