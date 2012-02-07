//
//  ScopeManager.cpp
//  Turing Compiler
//
//  Created by Tristan Hume on 12-02-07.
//  Copyright 2012 15 Norwich Way. All rights reserved.
//

#include "ScopeManager.h"

#include <cassert>

ScopeManager::ScopeManager(llvm::Module *mod) {
    assert(mod != NULL);
    
    currentScope = new GlobalScope(mod,NULL);
}

Scope *ScopeManager::curScope() {
    return currentScope;
}

void ScopeManager::pushScope() {
    currentScope = currentScope->createChildScope();
}

void ScopeManager::pushLocalScope(llvm::Function *func) {
    assert(func != NULL);
    
    currentScope = new LocalScope(func,currentScope);
}

void ScopeManager::popScope() {
    Scope *oldScope = currentScope;
    currentScope = currentScope->Parent;
    delete oldScope;
}