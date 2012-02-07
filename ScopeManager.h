//
//  ScopeManager.h
//  Turing Compiler
//
//  Created by Tristan Hume on 12-02-07.
//  Copyright 2012 15 Norwich Way. All rights reserved.
//

#ifndef Turing_Compiler_ScopeManager_H
#define Turing_Compiler_ScopeManager_H

#include <llvm/Function.h>
#include <llvm/Module.h>

#include "Scope.h"
#include "BasicScopes.h"

class ScopeManager {
public:
    ScopeManager(llvm::Module *mod);
    
    Scope *curScope();
    
    void pushScope();
    void pushLocalScope(llvm::Function *func);
    void popScope();
    
protected:
    Scope *currentScope;
};

#endif
