//
//  BasicScopes.cpp
//  Turing Compiler
//
//  Created by Tristan Hume on 12-02-07.
//  Copyright 2012 15 Norwich Way. All rights reserved.
//

#include "BasicScopes.h"

#include <llvm/Support/IRBuilder.h>
#include <llvm/Constant.h>

#include "Message.h"

using namespace llvm;

Symbol BasicScope::resolveVarThis(std::string name) {
    if (symbols.find(name) == symbols.end()) {
        throw Message::Exception(Twine("Could not find variable or function \"") + name + "\".");
    }
    return symbols[name];
}

//! sets a variable name to reference a specific value
void BasicScope::setVar(std::string name, llvm::Value *val) {
    // TODO check if it already exists?
    symbols[name] = Symbol(val,NULL);
}

bool BasicScope::isDeclaredThis(std::string name) {
    return symbols.find(name) != symbols.end();
}

GlobalScope::GlobalScope(llvm::Module *mod, Scope *parent) : TheModule(mod), BasicScope(parent) {
    
}

Scope *GlobalScope::createChildScope() {
    return new GlobalScope(TheModule,this);
}

Symbol GlobalScope::declareVar(std::string name, TuringType *type) {
    if (isDeclaredThis(name)) {
        throw Message::Exception(Twine("Variable ") + name + " is already defined.");
    }
    
    GlobalVariable* gvar = new GlobalVariable(/*Module=*/*TheModule, 
                                              /*Type=*/type->getLLVMType(),
                                              /*isConstant=*/false,
                                              /*Linkage=*/GlobalValue::CommonLinkage,
                                              /*Initializer=*/Constant::getNullValue(type->getLLVMType()), // has initializer, specified below
                                              /*Name=*/name);
    Symbol sym(gvar,type);
    
    // store in the symbol table
    symbols[name] = sym;
    
    return sym;
}

LocalScope::LocalScope(llvm::Function *func, Scope *parent) : TheFunction(func), BasicScope(parent) {
    
}

Scope *LocalScope::createChildScope() {
    return new LocalScope(TheFunction,this);
}

Symbol LocalScope::declareVar(std::string name, TuringType *type) {
    if (isDeclaredThis(name)) {
        throw Message::Exception(Twine("Variable ") + name + " is already defined.");
    }
    
    IRBuilder<> TmpB(&TheFunction->getEntryBlock(),
                     TheFunction->getEntryBlock().begin());
    Value *lvar = TmpB.CreateAlloca(type->getLLVMType(), 0,name);
    
    Symbol sym(lvar,type);
    
    // store in the symbol table
    symbols[name] = sym;
    
    return sym;
}