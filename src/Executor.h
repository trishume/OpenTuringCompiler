//
//  Executor.h
//  Turing Compiler
//
//  Created by Tristan Hume on 12-02-05.
//  Copyright 2012 15 Norwich Way. All rights reserved.
//

#ifndef Turing_Compiler_Executor_H
#define Turing_Compiler_Executor_H

#include <llvm/PassManager.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/JIT.h>
#include <llvm/Module.h>
#include <llvm/Function.h>

class Executor {
public:
    Executor(llvm::Module *mod);
    
    void optimize();
    bool run(bool timeRun = false);
private:
    llvm::Module *TheModule;
    llvm::ExecutionEngine *TheExecutionEngine;
};

#endif
