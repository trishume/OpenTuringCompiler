//
//  ScopeManager.cpp
//  Turing Compiler
//
//  Created by Tristan Hume on 12-02-07.
//  Copyright 2012 15 Norwich Way. All rights reserved.
//

#include "ScopeManager.h"

#include <llvm/ADT/Twine.h>
#include <cassert>

#include "Message.h"

ScopeManager::ScopeManager(llvm::Module *mod) {
    assert(mod != NULL);
    
    CurrentScope = new GlobalScope(mod,NULL);
}

Scope *ScopeManager::curScope() {
    return CurrentScope;
}

Scope *ScopeManager::getNamedScope(std::string name) {
    if (NamedScopes.find(name) == NamedScopes.end()) {
        throw Message::Exception(llvm::Twine("Module ") + name + " does not exist.");
    }  
    return NamedScopes[name];
}

bool ScopeManager::namedScopeExists(std::string name) {
    return NamedScopes.find(name) != NamedScopes.end();
}

void ScopeManager::pushScope() {
    CurrentScope = CurrentScope->createChildScope();
}

void ScopeManager::pushLocalScope(llvm::Function *func) {
    assert(func != NULL);
    
    CurrentScope = new LocalScope(func,CurrentScope);
}

void ScopeManager::pushNamedScope(std::string name) {
    if (NamedScopes.find(name) != NamedScopes.end()) {
        Scope *existingScope = NamedScopes[name];
        // if there is no parent conflicts we can add stuff to a module
        if (existingScope->Parent == CurrentScope) {
            //Message::error("Redefining module.",true); // true = warning
            CurrentScope = existingScope;
            return;
        } else {
            throw Message::Exception(llvm::Twine("Tried to redefine module ") + name + ".");
        }
    } else {
        pushScope();
        CurrentScope->setScopeName(name);
    }
    NamedScopes[name] = CurrentScope;
    AllNamed.insert(CurrentScope);
}

void ScopeManager::popScope() {
    Scope *oldScope = CurrentScope;
    CurrentScope = CurrentScope->Parent;
    // don't delete something that is referenced in the NamedScopes map
    if (AllNamed.find(oldScope) == AllNamed.end()) {
        delete oldScope;
    }
}