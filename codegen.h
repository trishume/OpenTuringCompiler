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
#include "Scope.h"
#include "ScopeManager.h"

#define MAIN_FUNC_NAME "main"

class ASTNode;

class CodeGen {
public:
	CodeGen(ASTNode *tree);

	bool execute();

private:
    void importStdLib();
    
    llvm::Function *currentFunction();
    bool isProcedure(llvm::Function *f);
    bool isMainFunction(llvm::Function *f);
    
    TuringType *getType(ASTNode *node);
    std::vector<VarDecl> getDecls(ASTNode *astDecls);
    
	bool compileBlock(ASTNode *node);
    bool compileStat(ASTNode *node);
	llvm::Value *compile(ASTNode *node);
    llvm::Value *compileLHS(ASTNode *node);

	llvm::Value *compileBinaryOp(ASTNode *node);
    llvm::Value *abstractCompileBinaryOp(llvm::Value *L, 
                                         llvm::Value *R, std::string op);
    llvm::Value *compileAssignOp(ASTNode *node);
    llvm::Value *compileLogicOp(ASTNode *node);
    
    void compilePutStat(ASTNode *node);
    void compileVarDecl(ASTNode *node);
    
	llvm::Value *compileCall(ASTNode *node, bool wantReturn = true);
    llvm::Function *compileFunctionPrototype(ASTNode *node);
    llvm::Function *compilePrototype(const std::string &name, TuringType *returnType, std::vector<VarDecl> args);
    llvm::Function *compileFunction(ASTNode *node);
    void compileReturn();
    
    void compileIfStat(ASTNode *node);
    

	ASTNode *Root;

	llvm::Module *TheModule;
    llvm::Function *MainFunction;
    
	llvm::IRBuilder<> Builder;
    
    //! the current value to be returned. NULL if not a valid place to return from.
    llvm::Value *RetVal;
    //! used for "exit" "return" and "result"
    //! signals the block compiler to not compile instructions after the early exit
    bool BlockEarlyExitFlag;

	//std::stack<CodeGenBlock *> Blocks;
    TypeManager Types;
    ScopeManager *Scopes;
};

#endif