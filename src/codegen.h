#ifndef _CODEGEN_H_
#define _CODEGEN_H_

#include <llvm/DerivedTypes.h>
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/Function.h>
#include <llvm/Analysis/Verifier.h>
#include <llvm/Support/IRBuilder.h>
#include <llvm/Target/TargetData.h>

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
    bool isCurBlockTerminated();
    
    llvm::Value *compileArrayByteSize(llvm::Value *arrayRef);
    llvm::Value *compileArrayLength(llvm::Value *arrayRef);
    std::pair<llvm::Value*,llvm::Value*> compileRange(ASTNode *node);
    
    TuringType *getType(ASTNode *node);
    TuringType *getArrayType(ASTNode *node);
    std::vector<VarDecl> getDecls(ASTNode *astDecls);
    
	bool compileBlock(ASTNode *node);
    bool compileStat(ASTNode *node);
	llvm::Value *compile(ASTNode *node);
    Symbol compileLHS(ASTNode *node);
    
    llvm::Value *compileStringLiteral(const std::string &str);

	llvm::Value *compileBinaryOp(ASTNode *node);
    llvm::Value *abstractCompileBinaryOp(llvm::Value *L, 
                                         llvm::Value *R, std::string op);
    llvm::Value *compileAssignOp(ASTNode *node);
    void compileArrayCopy(llvm::Value *from, Symbol to);
    llvm::Value *compileLogicOp(ASTNode *node);
    llvm::Value *compileEqualityOp(ASTNode *node);
    
    void compilePutStat(ASTNode *node);
    void compileVarDecl(ASTNode *node);
    
    llvm::Value *compileVarReference(ASTNode *node);
    llvm::Value *compileIndex(llvm::Value *indexed,ASTNode *node);
    
    llvm::Value *compileCallSyntax(ASTNode *node);
	llvm::Value *compileCall(ASTNode *node, bool wantReturn = true);
    llvm::Value *compileCall(llvm:: Value *callee,ASTNode *node, bool wantReturn);
    llvm::Function *compileFunctionPrototype(ASTNode *node);
    llvm::Function *compilePrototype(const std::string &name, TuringType *returnType, std::vector<VarDecl> args);
    llvm::Function *compileFunction(ASTNode *node);
    void compileReturn();
    
    void compileIfStat(ASTNode *node);
    void compileLoopStat(ASTNode *node);
    void compileForStat(ASTNode *node);
    

	ASTNode *Root;

	llvm::Module *TheModule;
    llvm::Function *MainFunction;
    
	llvm::IRBuilder<> Builder;
    
    //! the current value to be returned. NULL if not a valid place to return from.
    llvm::Value *RetVal;
    llvm::BasicBlock *RetBlock;
    //! the block that is the end of the most recent loop
    llvm::BasicBlock *ExitBlock;

	//std::stack<CodeGenBlock *> Blocks;
    TypeManager Types;
    ScopeManager *Scopes;
};

#endif