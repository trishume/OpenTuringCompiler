//
//  Scope.cpp
//  Turing Compiler
//
//  Created by Tristan Hume on 12-02-07.
//  Copyright 2012 15 Norwich Way. All rights reserved.
//

#include "Scope.h"

#include "Message.h"

using namespace llvm;

Scope::Scope(Scope *parent) : Parent(parent) {
    
}

Symbol Scope::resolve(std::string name) {
    if (isDeclaredThis(name)){
        return resolveVarThis(name);
    } else if(Parent != NULL) {
        return Parent->resolve(name);
    } else {
        throw Message::Exception(Twine("could not find variable ") + name);
    }
}

bool Scope::isDeclared(std::string name) {
    if (isDeclaredThis(name)){
        return true;
    } else if(Parent != NULL) {
        return Parent->isDeclared(name);
    } else {
        return false;
    }
}
