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

#include "ASTSource.h"
#include "TuringType.h"
#include "TypeManager.h"
#include "VarDecl.h"
#include "Scope.h"
#include "ScopeManager.h"

#define MAIN_FUNC_NAME "main"

class ASTNode;

class CodeGen {
public:
	CodeGen(ASTSource *source);

	bool execute(bool dumpModule = false);
    
    bool compileFile(std::string fileName);

protected:
    bool compileRootNode(ASTNode *fileRoot, std::string fileName);
    llvm::Function *currentFunction();
    bool isProcedure(llvm::Function *f);
    bool isMainFunction(llvm::Function *f);
    bool isCurBlockTerminated();
    
    llvm::Value *getConstantInt(int index);
    llvm::Value *compileByteSize(llvm::Type *type);
    llvm::Value *compileArrayByteSize(llvm::Value *arrayRef);
    llvm::Value *compileArrayLength(llvm::Value *arrayRef);
    std::pair<llvm::Value*,llvm::Value*> compileRange(ASTNode *node);
    void compileInitializeComplex(llvm::Value *declared, TuringType *type);
    
    TuringType *getType(ASTNode *node);
    TuringType *getArrayType(ASTNode *node);
    TuringType *getRecordType(ASTNode *node);
    std::vector<VarDecl> getDecls(ASTNode *astDecls,bool allowAutoTypes = true);
    llvm::Value *promoteType(llvm::Value *val, TuringType *destType);
    
	bool compileBlock(ASTNode *node);
    bool compileStat(ASTNode *node);
	llvm::Value *compile(ASTNode *node);
    Symbol *compileLHS(ASTNode *node);
    
    llvm::Value *compileStringLiteral(const std::string &str);

	llvm::Value *compileBinaryOp(ASTNode *node);
    llvm::Value *abstractCompileBinaryOp(llvm::Value *L, 
                                         llvm::Value *R, std::string op);
    llvm::Value *compileAssignOp(ASTNode *node);
    void abstractCompileAssign(llvm::Value *val, Symbol *assignSym);
    void compileArrayCopy(llvm::Value *from, Symbol *to);
    void compileRecordCopy(llvm::Value *from, Symbol *to);
    llvm::Value *compileLogicOp(ASTNode *node);
    llvm::Value *compileEqualityOp(ASTNode *node);
    
    void compilePutStat(ASTNode *node);
    void compileGetStat(ASTNode *node);
    void compileVarDecl(ASTNode *node);
    
    llvm::Value *abstractCompileVarReference(Symbol *var,const std::string &name);
    llvm::Value *compileIndex(llvm::Value *indexed,ASTNode *node);
    Symbol *compileRecordFieldRef(Symbol *record, std::string fieldName);
    llvm::Value *abstractCompileIndex(llvm::Value *indexed,llvm::Value *index);
    
    llvm::Value *compileCallSyntax(ASTNode *node);
	llvm::Value *compileCall(ASTNode *node, bool wantReturn = true);
    llvm::Value *compileCall(Symbol *callee,ASTNode *node, bool wantReturn);
    llvm::Value *abstractCompileCall(Symbol *callee, const std::vector<llvm::Value*> &params, bool wantReturn);
    llvm::Function *compileFunctionPrototype(ASTNode *node, const std::string &aliasName = "");
    FunctionSymbol *compilePrototype(const std::string &name, TuringType *returnType, std::vector<VarDecl> args, const std::string &aliasName = "", bool internal = true);
    llvm::Function *compileFunction(ASTNode *node);
    void compileReturn();
    
    void compileModule(ASTNode *node);
    
    void compileIfStat(ASTNode *node);
    void compileLoopStat(ASTNode *node);
    void compileForStat(ASTNode *node);
    

	ASTSource *TheSource; // sounds ominous ...
    std::string CurFile;
    //! is the module in a state to finalize and execute?
    bool CanExecute;

	llvm::Module *TheModule;
    llvm::Function *MainFunction;
    
	llvm::IRBuilder<> Builder;
    
    //! the current value to be returned. NULL if not a valid place to return from.
    Symbol *RetVal;
    llvm::BasicBlock *RetBlock;
    //! the block that is the end of the most recent loop
    llvm::BasicBlock *ExitBlock;

	//std::stack<CodeGenBlock *> Blocks;
    TypeManager Types;
    ScopeManager *Scopes;
};

#endif