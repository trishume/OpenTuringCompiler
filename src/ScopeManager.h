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

#include <map>
#include <set>

#include "Scope.h"
#include "BasicScopes.h"

class ScopeManager {
public:
    ScopeManager(llvm::Module *mod);
    
    Scope *curScope();
    Scope *getNamedScope(std::string name);
    
    bool namedScopeExists(std::string name);
    
    void pushScope();
    void pushLocalScope(llvm::Function *func);
    void pushNamedScope(std::string name);
    void popScope();
    
protected:
    Scope *CurrentScope;
    //! pointers to all the named module scopes
    std::map<std::string, Scope*> NamedScopes;
    //! stores named sets so they don't get deleted
    std::set<Scope*> AllNamed;
};

#endif
