#include "codegen.h"

#include <llvm/Instruction.h>
#include <llvm/Constants.h>

#include "language.h"
#include "ast.h"

#include "Message.h"
#include "Executor.h"

#define ITERATE_CHILDREN(node,var) \
for(std::vector<ASTNode*>::iterator var = (node)->children.begin(), e = (node)->children.end();var < e;++var)

using namespace llvm;

#pragma mark Construction

CodeGen::CodeGen(ASTNode *tree) : Builder(llvm::getGlobalContext()), RetVal(NULL), RetBlock(NULL) {
	Root = tree;
    Types.addDefaultTypes(getGlobalContext());
    TheModule = new Module("turing JIT module", getGlobalContext());
    Scopes = new ScopeManager(TheModule);
}

void CodeGen::importStdLib() {
    std::vector<VarDecl> *params;
    
    params = new std::vector<VarDecl>(1,VarDecl("num",Types.getType("int")));
    compilePrototype("TuringPrintInt",Types.getType("void"),*params);
    delete params;
    
    params = new std::vector<VarDecl>(1,VarDecl("val",Types.getType("boolean")));
    compilePrototype("TuringPrintBool",Types.getType("void"),*params);
    delete params;
    
    params = new std::vector<VarDecl>(1,VarDecl("val",Types.getType("string")));
    compilePrototype("TuringPrintString",Types.getType("void"),*params);
    delete params;
    
    params = new std::vector<VarDecl>();
    compilePrototype("TuringPrintNewline",Types.getType("void"),*params);
    delete params;
    
    params = new std::vector<VarDecl>(2,VarDecl("val",Types.getType("int")));
    compilePrototype("TuringPower",Types.getType("int"),*params);
    delete params;
}

#pragma mark Execution

bool CodeGen::execute() {
	LLVMContext &context = getGlobalContext();
    
    Message::log("Generating code...");
    
	
    
	/* Create the top level interpreter function to call as entry */
	std::vector<Type*> argTypes;
	FunctionType *mainFuntionType = FunctionType::get(Type::getVoidTy(getGlobalContext()), argTypes, false);
	MainFunction = Function::Create(mainFuntionType, GlobalValue::InternalLinkage, MAIN_FUNC_NAME, TheModule);
	BasicBlock *mainBlock = BasicBlock::Create(getGlobalContext(), "entry", MainFunction, 0);
    BasicBlock *endBB = BasicBlock::Create(getGlobalContext(), "returnblock", MainFunction);
    
	Builder.SetInsertPoint(mainBlock);
    bool good = false;
    try {
        importStdLib();
        good = compileBlock(Root);
    } catch (Message::Exception e) {
        Message::error(e.Message);
        
    }
    
    Builder.CreateBr(endBB);
    Builder.SetInsertPoint(endBB);
    if (!isCurBlockTerminated()) {
        Builder.CreateRetVoid();
    }
    
	TheModule->dump();
    
    if (!good) { // code gen failed
        return false;
    }
    
    Message::log("JIT compiling and optimizing...");
    Executor jit(TheModule);
    jit.optimize();
    Message::log("RUNNING...");
    jit.run();
    
	return true; // success
}

#pragma mark Utilities

Function *CodeGen::currentFunction() {
    BasicBlock *startBlock = Builder.GetInsertBlock();
    Function *theFunction = startBlock->getParent();
    return theFunction;
}

bool CodeGen::isProcedure(Function *f) {
    return f->getReturnType()->isVoidTy();
}

bool CodeGen::isMainFunction(Function *f) {
    return f == MainFunction;
}

bool CodeGen::isCurBlockTerminated() {
    return Builder.GetInsertBlock()->getTerminator() != NULL;
}


#pragma mark Transformation

//! transforms a declarations node into a vector of declarations. Throws exceptions.
//! \param node A ASTNode that can come from the "type" rule.
//! \return the TuringType for a node.
TuringType *CodeGen::getType(ASTNode *node) {
    switch(node->root) {
		case Language::NAMED_TYPE:
			return Types.getType(node->str); // can't be NULL
        case Language::DEFERRED_TYPE:
            return Types.getType("auto");
        case Language::VOID_TYPE:
            return Types.getType("void");
        default:
            throw Message::Exception(Twine("AST type ") + Language::getTokName(node->root) + " can't be compiled into a type.");
            // TODO other type nodes
	}    
    return NULL; // never gets here
}

//! transforms a declarations node into a vector of declarations. 
//! \param decls A DECLARATIONS ASTNode.
//! \return vector of declarations.
std::vector<VarDecl> CodeGen::getDecls(ASTNode *astDecls) {
    if(astDecls->root != Language::DECLARATIONS) throw Message::Exception("Expected DECLARATIONS node in getDecls");
    
    std::vector<VarDecl> decls;
    ITERATE_CHILDREN(astDecls,decl) {
        TuringType *type = getType((*decl)->children[0]);
        decls.push_back(VarDecl((*decl)->str,type));
    }
    
    return decls;
}

#pragma mark Compilation

//! Compiles a block of instructions
//! Returns success
bool CodeGen::compileBlock(ASTNode *node) {
	if(node->root == Language::BLOCK) {
		ITERATE_CHILDREN(node,child) {
			if(!compileStat(*child)) {
				return false; // failure
			}
            // used for returns and exits
            // instructions past this point will be unreachable so quit early
            if (isCurBlockTerminated()) break;
		}
		return true;
	} else {
        Message::error("Node is not a block");
		return false;
	}
}

//! dispatcher for statements. Checks the type of the node and calls the correct compile function.
//! treats as an expression if it can.
bool CodeGen::compileStat(ASTNode *node) {
	if(node == NULL) {
		Message::error("Can not compile null node.");
        return false;
	}
    switch(node->root) {
        case Language::FUNC_PROTO: // extern declaration
            return compileFunctionPrototype(node) != NULL;
        case Language::FUNC_DEF:
            return compileFunction(node);
        case Language::CALL:
            compileCall(node,false); // special case allowing procedure calls
            return true; // throws error on fail
        case Language::VAR_DECL:
            compileVarDecl(node);
            return true; // throws error on fail
        case Language::IF_STAT:
            compileIfStat(node);
            return true; // throws error on fail
        case Language::PUT_STAT:
            compilePutStat(node);
            return true;
        case Language::RETURN_STAT:
            compileReturn();
            return true;
        case Language::RESULT_STAT:
            if (RetVal == NULL) {
                throw Message::Exception("Result can only be used inside a function.");
            }
            Builder.CreateStore(compile(node->children[0]),RetVal);
            compileReturn();
            return true;
        default:
            return compile(node) != NULL; // treat as an expression
    }
	
	return false; // should never reach here
}

//! dispatcher. Checks the type of the node and calls the correct compile function.
Value *CodeGen::compile(ASTNode *node) {
	if(node == NULL) {
		throw Message::Exception("Can not compile null node.");
	}
    switch(node->root) {
        case Language::BIN_OP:
            return compileBinaryOp(node);
        case Language::ASSIGN_OP:            
            return compileAssignOp(node);
        case Language::CALL:
            return compileCall(node);
        case Language::VAR_REFERENCE:
        {
            // TODO maybe do as clang does and alloca and assign all params instead of this
            Value *var = compileLHS(node);
            if (!(var->getType()->isPointerTy())) {
                // if it is not a reference return it straight up. Used for arguments.
                return var;
            }
            return Builder.CreateLoad(compileLHS(node),Twine(node->str) + "val");
        }
        case Language::STRING_LITERAL:
            return Builder.CreateGlobalStringPtr(node->str);
        case Language::INT_LITERAL:
            // apint can convert a string
            return ConstantInt::get(getGlobalContext(), APInt(32,node->str,10));
        case Language::BOOL_LITERAL:
            // apint is used because booleans are one bit ints
            return ConstantInt::get(getGlobalContext(), APInt(1,(node->str.compare("true") == 0) ? 1 : 0));
        default:
            throw Message::Exception(Twine("AST type ") + Language::getTokName(node->root) + " not recognized");
    }
}

// Compiles binary operators.
Value *CodeGen::compileBinaryOp(ASTNode *node) {
    if (node->str.compare("and") == 0 || node->str.compare("or") == 0) {
        return compileLogicOp(node);
    } else {
        Value *L = compile(node->children[0]);
        Value *R = compile(node->children[1]);
        return abstractCompileBinaryOp(L,R,node->str);
    }
}

Value *CodeGen::abstractCompileBinaryOp(Value *L, Value *R, std::string op) {
    if (L == 0 || R == 0) {
        throw Message::Exception("Invalid operand for binary operator.");
    }
	
	if (op.compare(">") == 0) {
        return Builder.CreateICmpUGT(L, R, "GTtmp");
    } else if (op.compare("<") == 0) {
        return Builder.CreateICmpULT(L, R, "LTtmp");
    } else if (op.compare(">=") == 0) {
        return Builder.CreateICmpUGE(L, R, "GEtmp");
    } else if (op.compare("<=") == 0) {
        return Builder.CreateICmpULE(L, R, "LEtmp");
    } else if (op.compare("+") == 0) {
        return Builder.CreateAdd(L, R, "addtmp");
    } else if (op.compare("-") == 0) {
        return Builder.CreateSub(L, R, "subtmp");
    } else if (op.compare("*") == 0) {
        return Builder.CreateMul(L, R, "multmp");
    } else if (op.compare("div") == 0) {
        return Builder.CreateSDiv(L, R, "divtmp");
    } else if (op.compare("mod") == 0) {
        return Builder.CreateSRem(L, R, "modtmp");
    } else if (op.compare("**") == 0) {
        std::vector<Value*> argVals;
        argVals.push_back(L);
        argVals.push_back(R);
        return Builder.CreateCall(TheModule->getFunction("TuringPower"),argVals,"powertmp");
    }
    
    throw Message::Exception("Invalid binary operator.");
    return NULL; // never reaches here
}

//! compiles a properly short-circuiting logic operator
//! \param isAnd false for 'or' true for 'and'
Value *CodeGen::compileLogicOp(ASTNode *node) {
    Value *cond1 = compile(node->children[0]);
    
    BasicBlock *startBlock = Builder.GetInsertBlock();
    Function *theFunction = startBlock->getParent();
    
    // Create blocks for the then and else cases.  Insert the 'then' block at the
    // end of the function.
    BasicBlock *secondBB = BasicBlock::Create(getGlobalContext(), "cond2", theFunction);
    BasicBlock *mergeBB = BasicBlock::Create(getGlobalContext(), "andmerge");
    
    //! and continues if the first is true and or short-circuits
    if (node->str.compare("and") == 0) {
        Builder.CreateCondBr(cond1, secondBB, mergeBB);
    } else { // or
        Builder.CreateCondBr(cond1, mergeBB, secondBB);
    }
    
    
    
    // Emit second condition
    Builder.SetInsertPoint(secondBB);
    
    Value *cond2 = compile(node->children[1]);
    
    Builder.CreateBr(mergeBB);
    
    
    // Emit merge block.
    theFunction->getBasicBlockList().push_back(mergeBB);
    Builder.SetInsertPoint(mergeBB);
    
    PHINode *resPhi = Builder.CreatePHI(Types.getType("boolean")->getLLVMType(),2,"andresult");
    resPhi->addIncoming(cond1,startBlock);
    resPhi->addIncoming(cond2,secondBB);
    
    return resPhi;
}

Value *CodeGen::compileLHS(ASTNode *node) {
    switch (node->root) {
        case Language::VAR_REFERENCE:
            return Scopes->curScope()->resolve(node->str);
        default:
            throw Message::Exception(Twine("LHS AST type ") + Language::getTokName(node->root) + 
                                     " not recognized");
    }
    return NULL; // never reaches here
}

Value *CodeGen::compileAssignOp(ASTNode *node) {
    Value *val = compile(node->children[1]);
    
    std::string op = node->str.substr(0,node->str.size() - 1);
    // check for += div= etc...
    // convert to form "var := var op val
    if ((op.compare("+") == 0) ||
        (op.compare("-") == 0) ||
        (op.compare("*") == 0) ||
        (op.compare("/") == 0) ||
        (op.compare("**") == 0) ||
        (op.compare("mod") == 0) ||
        (op.compare("div") == 0)) {
        val = abstractCompileBinaryOp(compile(node->children[0]),val,op);
    } else if (op.compare(":") != 0) {
        throw Message::Exception(Twine("Can't find operator ") + op);
    }
    Value *assignVar = compileLHS(node->children[0]);
    
    if (isa<Argument>(assignVar)) {
        throw Message::Exception("Can't assign to an argument.");
    }
    
    // store needs a pointer
    if (!(assignVar->getType()->isPointerTy())) {
        throw Message::Exception("Can't assign to something that is not a variable");
    }
    
    Builder.CreateStore(val,assignVar);
    return val;
}

void CodeGen::compilePutStat(ASTNode *node) {
    Value *val = compile(node->children[0]);
    TuringType *type = Types.getTypeLLVM(val->getType());
    
    Function *calleeFunc;
    std::vector<Value*> argVals;
    argVals.push_back(val);
    
    if (type->getName().compare("int") == 0) {
        calleeFunc = TheModule->getFunction("TuringPrintInt");
        
    } else if (type->getName().compare("boolean") == 0) {
        calleeFunc = TheModule->getFunction("TuringPrintBool");
    } else if (type->getName().compare("string") == 0) {
        calleeFunc = TheModule->getFunction("TuringPrintString");
    } else {
        throw Message::Exception(Twine("Can't 'put' type ") + type->getName());
    }
        
    Builder.CreateCall(calleeFunc,argVals);
    
    // if the string is not ".." print a newline
    if (node->str.compare("..") != 0) {
        Builder.CreateCall(TheModule->getFunction("TuringPrintNewline"),std::vector<Value*>());
    }
}

void CodeGen::compileVarDecl(ASTNode *node) {
    std::vector<VarDecl> args = getDecls(node->children[0]);
    
    for (int i = 0; i < args.size();++i) {
        bool hasInitial = node->children.size() > 1;
        
        TuringType *type = args[i].Type;
        Value *initializer = NULL;
        
        if (hasInitial) {
            initializer = compile(node->children[1]);
        }
        
        
        if (type->getName() == "auto") {
            if (!hasInitial) {
                throw Message::Exception("Can't infer the type of a declaration with no initial value.");
            }
            type = Types.getTypeLLVM(initializer->getType());
        }
        
        Value *declared = Scopes->curScope()->declareVar(args[i].Name,type);
        if (hasInitial) {
            Builder.CreateStore(initializer,declared);
        }
    }
}

//! Compile a function call
//! \param wantReturn  Wether the return value is ignored. 
//!                    Should always be true for procedures.
//! \return the return value of the function
//!         unless the wantReturn parameter is null
//!         in wich case NULL is returned
Value *CodeGen::compileCall(ASTNode *node, bool wantReturn) {
    // Look up the name in the global module table.
    // TODO rewrite to get function from scope once implimented
    Function *calleeFunc = TheModule->getFunction(node->str);
    if (calleeFunc == NULL) {
        throw Message::Exception(Twine("Unknown function '") + node->str + "' referenced.");
    }
    
    // If argument mismatch error.
    if (calleeFunc->arg_size() != node->children.size()) {
        throw Message::Exception(Twine(node->children.size()) + " arguments passed to a function that takes " + Twine(calleeFunc->arg_size()));
    }
    
    std::vector<Value*> argVals;
    for (unsigned i = 0, e = node->children.size(); i < e; ++i) {
        argVals.push_back(compile(node->children[i]));
    }
    
    if (calleeFunc->getReturnType()->isVoidTy() && wantReturn) {
        throw Message::Exception(Twine("Procedure ") + calleeFunc->getName() + " can not return a value.");
    } else if(!wantReturn) {
        Builder.CreateCall(calleeFunc, argVals);
        return NULL;
    }
    
    return Builder.CreateCall(calleeFunc, argVals, "calltmp");
}

Function *CodeGen::compileFunctionPrototype(ASTNode *node) {
    return compilePrototype(node->str,getType(node->children[0]),getDecls(node->children[1]));
}

/*! creates an llvm function with no implementation.
 
 Used for extern declarations and as part of the function/procedure creation process.
 
 \param name The name of the function
 \param returnType The type the function returns. the void type if it is a procedure.
 \param params A DECLARATIONS ast node containing the formal parameters.
 
 */
Function *CodeGen::compilePrototype(const std::string &name, TuringType *returnType, std::vector<VarDecl> args) {
    
    // Make the function type:  double(double,double) etc.
    std::vector<Type*> argTypes;
    // extract argument types
    for (int i = 0; i < args.size(); ++i) {
        argTypes.push_back(args[i].Type->getLLVMType());
    }
    
    FunctionType *FT = FunctionType::get(returnType->getLLVMType(),
                                         argTypes, false);
    
    Function *f = Function::Create(FT, Function::ExternalLinkage, name, TheModule);
    
    // If F conflicted, there was already something named 'name'.  If it has a
    // body, don't allow redefinition or reextern.
    if (f->getName() != name) {
        // Delete the one we just made and get the existing one.
        f->eraseFromParent();
        f = TheModule->getFunction(name);
        
        // If F already has a body, reject this.
        if (!f->empty()) {
            throw Message::Exception("Redefinition of function.");
        }
        
        // If F took a different number of args, reject.
        if (f->arg_size() != args.size()) {
            throw Message::Exception("Redefinition of function with different # args.");
        }
    }
    
    // Set names for all arguments.
    unsigned idx = 0;
    for (Function::arg_iterator ai = f->arg_begin(); idx != args.size();
         ++ai, ++idx) {
        ai->setName(args[idx].Name);
    }
    
    return f;
}

Function *CodeGen::compileFunction(ASTNode *node) {
    Function *f = compileFunctionPrototype(node->children[0]);
    
    // TODO add separate scope for arguments so they can be redefined?
    Scopes->pushLocalScope(f);
    // add arguments to the scope
    for (Function::arg_iterator ai = f->arg_begin(); ai != f->arg_end();++ai) {
        // TODO llvm may auto-rename the arguments. The renamed version should not be the one added.
        Scopes->curScope()->setVar(ai->getName(),&(*ai));
    }
    
    // save these for later
    IRBuilderBase::InsertPoint prevPoint = Builder.saveIP();
    
    BasicBlock *entryBB = BasicBlock::Create(getGlobalContext(), "entry", f);
    BasicBlock *endBB = BasicBlock::Create(getGlobalContext(), "returnblock");
    RetBlock = endBB;
    
    Builder.SetInsertPoint(entryBB);

    if (!isProcedure(f)) {
        RetVal = Builder.CreateAlloca(f->getReturnType(), 0,"returnval");
    }
    
    compileBlock(node->children[1]);
    
    if (!isCurBlockTerminated()) {
        Builder.CreateBr(endBB);
    }
    
    f->getBasicBlockList().push_back(endBB);
    
    // procedures have an implicit "return"
    Builder.SetInsertPoint(endBB);
    if (isProcedure(f)) {        
        Builder.CreateRetVoid();
    } else { // function
        // TODO check if RetVal is ever set
        Builder.CreateRet(Builder.CreateLoad(RetVal));
    }
    
    Scopes->popScope();
    
    RetVal = NULL; // only does anything in functions
    RetBlock = NULL;
    // back to normal programming...
    Builder.restoreIP(prevPoint);
    
    //verifyFunction(*f);
    
    return f;
}
// compiles the "return" and "result" statements.
// the last block in a function contains the return statement
void CodeGen::compileReturn() {
    Builder.CreateBr(RetBlock);
}

//! compiles an if statement as a series of blocks
//! works on IF_STAT or ELSIF_STAT nodes
void CodeGen::compileIfStat(ASTNode *node) {
    Value *cond = compile(node->children[0]);
    
    Function *theFunction = Builder.GetInsertBlock()->getParent();
    
    bool hasElse = node->children.size() > 2;
    
    // Create blocks for the then and else cases.  Insert the 'then' block at the
    // end of the function.
    BasicBlock *thenBB = BasicBlock::Create(getGlobalContext(), "then", theFunction);
    BasicBlock *elseBB = NULL;
    BasicBlock *mergeBB = BasicBlock::Create(getGlobalContext(), "ifcont");
    
    // if it has an else then branch to it if the condition fails
    // otherwise skip over it
    if (hasElse) {
        elseBB = BasicBlock::Create(getGlobalContext(), "else");
        Builder.CreateCondBr(cond, thenBB, elseBB);
    } else {
        Builder.CreateCondBr(cond, thenBB, mergeBB);
    }
    
    
    // Emit then value.
    Builder.SetInsertPoint(thenBB);
    
    Scopes->pushScope();
    compileBlock(node->children[1]);
    Scopes->popScope();
    
    if (!isCurBlockTerminated()) {
        Builder.CreateBr(mergeBB);
    }    
    
    if (hasElse) {
        // Emit else block.
        theFunction->getBasicBlockList().push_back(elseBB);
        Builder.SetInsertPoint(elseBB);
        
        Scopes->pushScope();
        compileBlock(node->children[2]);
        Scopes->popScope();
        
        if (!isCurBlockTerminated()) {
            Builder.CreateBr(mergeBB);
        }
    }
    
    
    // Emit merge block.
    theFunction->getBasicBlockList().push_back(mergeBB);
    Builder.SetInsertPoint(mergeBB);
}