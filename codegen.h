#ifndef _CODEGEN_H_
#define _CODEGEN_H_

#include <llvm/DerivedTypes.h>
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/Analysis/Verifier.h>
#include <llvm/Support/IRBuilder.h>

#include <iostream>
#include <stack>

class ASTNode;

class CodeGenBlock {
public:
    llvm::BasicBlock *block;
};

class CodeGen {
public:
	CodeGen(ASTNode *tree);

	bool execute();

private:
	bool compileBlock(ASTNode *node);
	llvm::Value *compile(ASTNode *node);

	llvm::Value *compileMathOp(ASTNode *node);
	llvm::Value *compileCall(ASTNode *node);

	//! block stack manipulation
    void pushBlock(llvm::BasicBlock *block);
    void popBlock();

	ASTNode *Root;

	llvm::Module *MainModule;
	llvm::IRBuilder<> Builder;

	std::stack<CodeGenBlock *> Blocks;
    llvm::Function *MainFunction;
};

#endif