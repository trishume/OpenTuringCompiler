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
#include <llvm/Transforms/IPO.h>
#include <llvm/Support/CodeGen.h>
#include <llvm/ExecutionEngine/JIT.h>
#include <llvm/ExecutionEngine/JITMemoryManager.h>

using namespace llvm;

Executor::Executor(Module *mod, TuringCommon::StreamManager *streamManager,
                   LibManager *libManager, const std::string &executionDir) : TheModule(mod), TheStreamManager(streamManager), TheLibManager(libManager), ExecutionDir(executionDir) {
    InitializeNativeTarget();
    std::string errStr;
    
    llvm::TargetOptions Opts;
    Opts.JITExceptionHandling = true;
    llvm::EngineBuilder factory(TheModule);
    factory.setEngineKind(llvm::EngineKind::JIT);
    factory.setAllocateGVsWithCode(false);
    factory.setTargetOptions(Opts);
    factory.setOptLevel(CodeGenOpt::Aggressive);
    TheExecutionEngine = factory.create();
                                    
    
    if (!TheExecutionEngine) {
        fprintf(stderr, "Could not create ExecutionEngine: %s\n", errStr.c_str());
        exit(1);
    }
}

void Executor::optimize() {
    PassManager p;
    
    
    p.add(new TargetData(TheModule));
    p.add(createVerifierPass());
    
    // constant folding
    p.add(createConstantMergePass());
    // Provide basic AliasAnalysis support for GVN.
    p.add(createBasicAliasAnalysisPass());
    // inline functions
    p.add(createFunctionInliningPass());
    // Promote allocas to registers.
    p.add(createPromoteMemoryToRegisterPass());
    // Do simple "peephole" optimizations and bit-twiddling optzns.
    p.add(createInstructionCombiningPass());
    // Reassociate expressions.
    p.add(createReassociatePass());
    // Eliminate Common SubExpressions.
    p.add(createGVNPass());
    // get rid of extra copying
    p.add(createMemCpyOptPass());
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
    
    for (unsigned int i = 0; i < TheLibManager->InitRunFunctions.size(); ++i) {
        LibManager::InitRunFunction initFunc = TheLibManager->InitRunFunctions[i];
        (*initFunc)(ExecutionDir.c_str()); // call function pointer
    }
    void *funcPtr = TheExecutionEngine->getPointerToFunction(mainFunc);
    void (*programMain)(TuringCommon::StreamManager*) = 
        (void (*)(TuringCommon::StreamManager*))(intptr_t)funcPtr; // cast it into a function
    
    try {
        programMain(TheStreamManager);
    } catch (int errCode) {
        Message::error(Twine("Execution failed with error code ") + Twine(errCode));
    }
    
    
    return true;
}