//
//  BasicScopes.cpp
//  Turing Compiler
//
//  Created by Tristan Hume on 12-02-07.
//  Copyright 2012 15 Norwich Way. All rights reserved.
//

#include "BasicScopes.h"

#include <llvm/Support/IRBuilder.h>

#include "Message.h"

using namespace llvm;

Value *BasicScope::resolveVarThis(std::string name) {
    if (symbols.find(name) == symbols.end()) {
        Message::error(Twine("Could not find variable ") + name);
        return NULL;
    }
    return symbols[name];
}

bool BasicScope::isDeclaredThis(std::string name) {
    return symbols.find(name) != symbols.end();
}

GlobalScope::GlobalScope(llvm::Module *mod, Scope *parent) : TheModule(mod), BasicScope(parent) {
    
}

Scope *GlobalScope::createChildScope() {
    return new GlobalScope(TheModule,this);
}

bool GlobalScope::declareVar(std::string name, TuringType *type) {
    if (isDeclaredThis(name)) {
        Message::error(Twine("Variable ") + name + " is already defined");
        return false;
    }
    
    GlobalVariable* gvar = new GlobalVariable(/*Module=*/*TheModule, 
                                              /*Type=*/type->LLVMType,
                                              /*isConstant=*/false,
                                              /*Linkage=*/GlobalValue::CommonLinkage,
                                              /*Initializer=*/0, // has initializer, specified below
                                              /*Name=*/name);
    // store in the symbol table
    symbols[name] = gvar;
    
    return true;
}

LocalScope::LocalScope(llvm::Function *func, Scope *parent) : TheFunction(func), BasicScope(parent) {
    
}

Scope *LocalScope::createChildScope() {
    return new LocalScope(TheFunction,this);
}

bool LocalScope::declareVar(std::string name, TuringType *type) {
    if (isDeclaredThis(name)) {
        Message::error(Twine("Variable ") + name + " is already defined");
        return false;
    }
    
    IRBuilder<> TmpB(&TheFunction->getEntryBlock(),
                     TheFunction->getEntryBlock().begin());
    Value *lvar = TmpB.CreateAlloca(type->LLVMType, 0,name);
    
    
    // store in the symbol table
    symbols[name] = lvar;
    
    return true;
}