#ifndef _CODEGEN_H_
#define _CODEGEN_H_

#include <llvm/DerivedTypes.h>
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/Function.h>
#include <llvm/Analysis/Verifier.h>
#include <llvm/Support/IRBuilder.h>

#include <iostream>
#include <stack>

#include "TuringType.h"
#include "TypeManager.h"
#include "VarDecl.h"

class ASTNode;

class CodeGen {
public:
	CodeGen(ASTNode *tree);

	bool execute();

private:
    TuringType *getType(ASTNode *node);
    std::vector<VarDecl> getDecls(ASTNode *astDecls);
    
	bool compileBlock(ASTNode *node);
    bool compileStat(ASTNode *node);
	llvm::Value *compile(ASTNode *node);

	llvm::Value *compileMathOp(ASTNode *node);
    
	llvm::Value *compileCall(ASTNode *node, bool wantReturn = true);
    llvm::Function *compileProcPrototype(ASTNode *node);
    llvm::Function *compilePrototype(const std::string &name, TuringType *returnType, ASTNode *params);
    

	//! block stack manipulation
    //void pushBlock(llvm::BasicBlock *block);
    //void popBlock();

	ASTNode *Root;

	llvm::Module *TheModule;
    llvm::Function *MainFunction;
    
	llvm::IRBuilder<> Builder;

	//std::stack<CodeGenBlock *> Blocks;
    TypeManager Types;
};

#endif