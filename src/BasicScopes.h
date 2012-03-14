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
public:
    virtual Symbol *resolveVarThis(std::string name);
    //! sets a variable name to reference a specific value
    virtual void setVar(std::string name, llvm::Value *val, TuringType *type = NULL);
    virtual void setVar(std::string name, Symbol *val);
    
    virtual std::string getScopeName() {return ScopeName;}
    virtual void setScopeName(const std::string &name) {
        ScopeName = name;
    }
    
    virtual Symbol *declareVar(std::string name, TuringType *type);
    
    virtual ~BasicScope() {}
    
protected:
    BasicScope(Scope *parent) : Scope(parent) {}
    virtual bool isDeclaredThis(std::string name);
    //! overriden by subclasses to allocate the memory in the right place
    virtual llvm::Value *allocateSpace(TuringType *type,
                                       const std::string &name) = 0;
    
    std::map<std::string,Symbol*> symbols;
    std::string ScopeName;
};

class LocalScope : public BasicScope {
public:
    //! \param curFunc Function in wich the variables should be allocated
    LocalScope(llvm::Function *curFunc, Scope *parent);
    
    virtual Scope *createChildScope();
    
    
protected:
    virtual llvm::Value *allocateSpace(TuringType *type,
                                       const std::string &name);
    llvm::Function *TheFunction;
};

class GlobalScope : public BasicScope {
public:
    //! \param mod The module that the globals are declared in
    GlobalScope(llvm::Module *mod, Scope *parent);
    virtual Scope *createChildScope();
    
protected:
    virtual llvm::Value *allocateSpace(TuringType *type,
                                       const std::string &name);
    llvm::Module *TheModule;
};

#endif
