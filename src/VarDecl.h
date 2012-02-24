//
//  VarDecl.h
//  Turing Compiler
//
//  Created by Tristan Hume on 12-02-02.
//  Copyright 2012 15 Norwich Way. All rights reserved.
//

#ifndef Turing_Compiler_VarDecl_h
#define Turing_Compiler_VarDecl_h

#include <vector>

class TuringType; // forward

struct VarDecl {
    VarDecl(std::string name, TuringType *type) : Name(name), Type(type) {}
    std::string Name;
    TuringType *Type;
};

#endif
