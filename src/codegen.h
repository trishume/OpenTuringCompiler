#ifndef _CODEGEN_H_
#define _CODEGEN_H_

#include <llvm/DerivedTypes.h>
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/Function.h>
#include <llvm/Analysis/Verifier.h>
#include <llvm/Support/IRBuilder.h>
#include <llvm/Target/TargetData.h>
#include <llvm/ADT/Twine.h>

#include <iostream>
#include <stack>

#include "ASTSource.h"
#include "TuringType.h"
#include "TuringValue.h"
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
    
    TuringValue *getConstantInt(int index);
    llvm::Value *compileByteSize(TuringType *type);
    llvm::Value *compileArrayByteSize(llvm::Value *arrayRef);
    llvm::Value *compileArrayLength(llvm::Value *arrayRef);
    std::pair<TuringValue*,TuringValue*> compileRange(ASTNode *node);
    void compileInitializeComplex(Symbol *declared);
    
    TuringType *getType(ASTNode *node);
    TuringType *getArrayType(ASTNode *node);
    TuringType *getRecordType(ASTNode *node);
    std::vector<VarDecl> getDecls(ASTNode *astDecls,bool allowAutoTypes = true);
    TuringValue *promoteType(TuringValue *val, TuringType *destType,
                             llvm::Twine inStr = "");
    
	bool compileBlock(ASTNode *node);
    bool compileStat(ASTNode *node);
	TuringValue *compile(ASTNode *node);
    Symbol *compileLHS(ASTNode *node);
    
    TuringValue *compileStringLiteral(const std::string &str);

	TuringValue *compileBinaryOp(ASTNode *node);
    TuringValue *abstractCompileBinaryOp(TuringValue *L, 
                                         TuringValue *R, std::string op);
    TuringValue *compileAssignOp(ASTNode *node);
    void abstractCompileAssign(TuringValue *val, Symbol *assignSym);
    void compileArrayCopy(TuringValue *from, Symbol *to);
    void compileRecordCopy(TuringValue *from, Symbol *to);
    TuringValue *compileLogicOp(ASTNode *node);
    TuringValue *compileEqualityOp(ASTNode *node);
    
    void compilePutStat(ASTNode *node);
    void compileGetStat(ASTNode *node);
    void compileVarDecl(ASTNode *node);
    
    TuringValue *abstractCompileVarReference(Symbol *var,const std::string &name = "");
    Symbol *compileIndex(Symbol *indexed,ASTNode *node);
    Symbol *compileRecordFieldRef(Symbol *record, std::string fieldName);
    Symbol *abstractCompileIndex(Symbol *indexed,TuringValue *index);
    
    TuringValue *compileCallSyntax(ASTNode *node);
	TuringValue *compileCall(ASTNode *node, bool wantReturn = true);
    TuringValue *compileCall(Symbol *callee,ASTNode *node, bool wantReturn);
    TuringValue *abstractCompileCall(Symbol *callee, const std::vector<TuringValue*> &params, bool wantReturn);
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