//
//  Executor.cpp
//  Turing Compiler
//
//  Created by Tristan Hume on 12-02-05.
//  Copyright 2012 15 Norwich Way. All rights reserved.
//

#include "Executor.h"

#include <iostream>

#include "Message.h"
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetData.h>
#include <llvm/Analysis/Verifier.h>
#include <llvm/Analysis/Passes.h>
#include <llvm/Transforms/Scalar.h>

using namespace llvm;

Executor::Executor(Module *mod) : TheModule(mod) {
    InitializeNativeTarget();
    std::string errStr;
    TheExecutionEngine = ExecutionEngine::create(TheModule,false,&errStr);
    
    if (!TheExecutionEngine) {
        fprintf(stderr, "Could not create ExecutionEngine: %s\n", errStr.c_str());
        exit(1);
    }
}

void Executor::optimize() {
    PassManager p;
    
    
    p.add(new TargetData(TheModule));
    p.add(createVerifierPass());
    
    // Provide basic AliasAnalysis support for GVN.
    p.add(createBasicAliasAnalysisPass());
    // Promote allocas to registers.
    p.add(createPromoteMemoryToRegisterPass());
    // Do simple "peephole" optimizations and bit-twiddling optzns.
    p.add(createInstructionCombiningPass());
    // Reassociate expressions.
    p.add(createReassociatePass());
    // Eliminate Common SubExpressions.
    p.add(createGVNPass());
    // Simplify the control flow graph (deleting unreachable blocks, etc).
    p.add(createCFGSimplificationPass());
    
    // all the important stuff
    //StandardPass::AddPassesFromSet(&p,StandardPass::Module);
    
    // Run these optimizations on our Module
    bool changed = p.run(*TheModule);
    
    if (changed) {
        Message::log("Optimized code:");
        TheModule->dump();
    }
    
}

bool Executor::run() {
    Function *mainFunc = TheModule->getFunction("main");
    
    if (!mainFunc) {
        Message::error("No main function found for execution");
        return false;
    }
    
    void *funcPtr = TheExecutionEngine->getPointerToFunction(mainFunc);
    void (*programMain)() = (void (*)())(intptr_t)funcPtr; // cast it into a function
    
    programMain();
    
    return true;
}