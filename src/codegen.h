#ifndef _CODEGEN_H_
#define _CODEGEN_H_

#include <llvm/DerivedTypes.h>
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/Function.h>
#include <llvm/BasicBlock.h>
#include <llvm/Analysis/Verifier.h>
#include <llvm/Support/IRBuilder.h>
#include <llvm/Target/TargetData.h>
#include <llvm/ADT/Twine.h>

#include <iostream>
#include <stack>

#include "FileSource.h"
#include "LibManager.h"
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
	CodeGen(FileSource *source, LibManager *plugins);
    
    //! finalizes code generation and returns a module
    //! no more files can be compiled after this function is called
    //! \returns    an LLVM module with a main function taking a pointer
    //!             to a stream manager as a parameter. Or NULL on fail.
    llvm::Module *getFinalizedModule();
    
    bool compileFile(std::string fileName);
    
    //! sets the frequency of window event updates, line updates, etc...
    //! higher numbers = lower speed but more accurate line numbers
    //! for runtime errors
    void setPeriodicCallbackFrequency(unsigned int freq);

protected:
    bool compileRootNode(ASTNode *fileRoot, std::string fileName);
    llvm::Function *currentFunction();
    bool isProcedure(llvm::Function *f);
    bool isFlexibleArray(TuringType *type);
    bool isMainFunction(llvm::Function *f);
    bool isCurBlockTerminated();
    
    Symbol *createEntryAlloca(TuringType *type);
    TuringValue *getConstantInt(int index);
    void addPeriodicCallback(llvm::Function *func);
    void checkForPeriodicCallbacks();
    void compilePeriodicCallback(int line, const std::string &file);
    llvm::Value *compileByteSize(TuringType *type);
    llvm::Value *compileArrayByteSize(llvm::Value *arrayRef);
    llvm::Value *compileArrayByteSize(llvm::Type *arrayType,
                                      llvm::Value *arrayLength);
    llvm::Value *compileArrayLength(llvm::Value *arrayRef);
    TuringValue *compileArrayLower(TuringType *type);
    std::pair<TuringValue*,TuringValue*> compileRange(ASTNode *node);
    void compileAllocateFlexibleArray(Symbol *arr, bool allocateNew, 
                                      llvm::Value *newSize = NULL);
    void compileInitializeArrayElements(Symbol *arr, llvm::Value *from,
                                        llvm::Value *to);
    void compileInitializeComplex(Symbol *declared);
    
    TuringType *getType(ASTNode *node);
    llvm::Constant *getConstantValue(llvm::Value *constant);
    int getConstantIntValue(llvm::Value *constant);
    TuringType *getArrayType(ASTNode *node);
    TuringType *getRecordType(ASTNode *node);
    std::vector<VarDecl> getDecls(ASTNode *astDecls,bool allowAutoTypes = true);
    TuringValue *promoteType(TuringValue *val, TuringType *destType,
                             llvm::Twine inStr = "");
    
	bool compileBlock(ASTNode *node);
    bool compileStat(ASTNode *node);
	TuringValue *compile(ASTNode *node);
    Symbol *compileLHS(ASTNode *node, bool autoDeref = true);
    
    TuringValue *abstractCompileArrayLiteral(TuringType *arrType,const std::vector<llvm::Constant*> &vals);
    TuringValue *compileStringLiteral(const std::string &str);
    TuringValue *compileArrayLiteral(ASTNode *node);

	TuringValue *compileBinaryOp(ASTNode *node);
    TuringValue *abstractCompileBinaryOp(TuringValue *L, 
                                         TuringValue *R, std::string op);
    TuringValue *compileAssignOp(ASTNode *node);
    void abstractCompileAssign(TuringValue *val, Symbol *assignSym);
    TuringValue *compileUnaryOp(ASTNode *node);
    void compileArrayCopy(TuringValue *from, Symbol *to);
    void compileRecordCopy(TuringValue *from, Symbol *to);
    TuringValue *compileLogicOp(ASTNode *node);
    TuringValue *compileEqualityOp(ASTNode *node);
    TuringValue *abstractCompileEqualityOp(TuringValue *L,TuringValue *R,bool isNotEquals = false);
    
    void compilePutStat(ASTNode *node);
    void compileGetStat(ASTNode *node);
    void compileAssertStat(ASTNode *node);
    void compileResizeStat(ASTNode *node);
    void compileVarDecl(ASTNode *node);
    void compileConstDecl(ASTNode *node);
    
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
    void compileCaseStat(ASTNode *node);
    void compileCaseLabelStat(ASTNode *node,TuringValue *expr, llvm::BasicBlock *mergeBB);
    void compileLoopStat(ASTNode *node);
    void compileForStat(ASTNode *node);
    
    LibManager *PluginManager;
	FileSource *TheSource; // sounds ominous ...
    TypeManager Types;
    ScopeManager *Scopes;
    std::string CurFile;
    
    //! is the module in a state to finalize and execute?
    bool CanExecute;
    //! are checking and periodic callbacks enabled at this time
    bool CheckingEnabled;
    
	llvm::Module *TheModule;
    llvm::Function *MainFunction;
	llvm::IRBuilder<> Builder;
    
    //! pointer to the stream manager global variable
    llvm::Value *StreamManagerPtr;
    //! the current value to be returned. 
    //! NULL if not a valid place to return from.
    Symbol *RetVal;
    llvm::BasicBlock *RetBlock;
    //! the block that is the end of the most recent loop
    llvm::BasicBlock *ExitBlock;
    
    FunctionSymbol *PeriodicCallbackFunction;
    unsigned int PeriodicCallbackFrequency;
    unsigned int StatsSinceLastCallback;
    //! callbacks from libraries that have already been added
    //! to the periodic callback function
    std::set<std::string> AddedCallbacks;
};

#endif