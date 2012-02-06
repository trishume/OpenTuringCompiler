#include "codegen.h"

#include "language.h"
#include "ast.h"

#include "Message.h"
#include "Executor.h"

#define ITERATE_CHILDREN(node,var) \
for(std::vector<ASTNode*>::iterator var = (node)->children.begin(), e = (node)->children.end();var < e;++var)

using namespace llvm;

CodeGen::CodeGen(ASTNode *tree) : Builder(llvm::getGlobalContext()) {
	Root = tree;
    Types.addDefaultTypes(getGlobalContext());
}
/*
void CodeGen::pushBlock(BasicBlock *block) {
    Blocks.push(new CodeGenBlock());
    Blocks.top()->block = block;
    Builder.SetInsertPoint(block);
}
void CodeGen::popBlock() {
	CodeGenBlock *top = Blocks.top(); 
	Blocks.pop(); 
	delete top;
	if(!Blocks.empty()) {
		Builder.SetInsertPoint(Blocks.top()->block);
	}
}*/

bool CodeGen::execute() {
	LLVMContext &context = getGlobalContext();
    
    Message::log("Generating code...");
    
	TheModule = new Module("turing JIT module", context);
    
	/* Create the top level interpreter function to call as entry */
	std::vector<Type*> argTypes;
	FunctionType *mainFuntionType = FunctionType::get(Type::getVoidTy(getGlobalContext()), argTypes, false);
	MainFunction = Function::Create(mainFuntionType, GlobalValue::InternalLinkage, "main", TheModule);
	BasicBlock *mainBlock = BasicBlock::Create(getGlobalContext(), "entry", MainFunction, 0);
    
	Builder.SetInsertPoint(mainBlock);
	bool good = compileBlock(Root);
	Builder.CreateRetVoid();
    
	TheModule->dump();
    
    Message::log("JIT compiling and running...");
    Executor jit(TheModule);
    jit.optimize();
    jit.run();
    
	return good; // success
}

//! transforms a declarations node into a vector of declarations. Throws exceptions.
//! \param node A ASTNode that can come from the "type" rule.
//! \return the TuringType for a node.
TuringType *CodeGen::getType(ASTNode *node) {
    switch(node->root) {
		case Language::NAMED_TYPE:
			return Types.getType(node->str); // can't be NULL
            // TODO other type nodes
	}
    throw Message::Exception(Twine("AST type ") + Language::getTokName(node->root) + " can't be compiled into a type.");
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

//! Compiles a block of instructions
//! Returns success
bool CodeGen::compileBlock(ASTNode *node) {
	if(node->root == Language::BLOCK) {
		ITERATE_CHILDREN(node,child) {
			if(!compileStat(*child)) {
				return false; // failure
			}
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
    try {
        switch(node->root) {
            case Language::PROC_PROTO:
                return compileProcPrototype(node) != NULL;
            case Language::CALL:
                compileCall(node,false); // special case allowing procedure calls
                return true; // throws error on fail
            default:
                return compile(node) != NULL; // treat as an expression
        }
    } catch (Message::Exception e) {
        Message::error(e.Message);
    }
	
	return false; // should never reach here
}

//! dispatcher. Checks the type of the node and calls the correct compile function.
Value *CodeGen::compile(ASTNode *node) {
	if(node == NULL) {
		Message::error("Can not compile null node.");
        return NULL;
	}
    try {
        switch(node->root) {
            case Language::MATH_OP:
                return compileMathOp(node);
            case Language::CALL:
                return compileCall(node);
            case Language::INT_LITERAL:
                // apint can convert a string
                return ConstantInt::get(getGlobalContext(), APInt(64,node->str,10));
            default:
                Message::error(Twine("AST type ") + Language::getTokName(node->root) + " not recognized");
                return NULL;
        }
    } catch (Message::Exception e) {
        Message::error(e.Message);
    }
	
	
}

// Compiles binary math operators. Anything with a signature num op num : num
Value *CodeGen::compileMathOp(ASTNode *node) {
	Value *L = compile(node->children[0]);
	Value *R = compile(node->children[1]);
	if (L == 0 || R == 0) {
        Message::error("Invalid operand for binary operator.");
        return NULL;
    }
	
	switch (node->str[0]) {
		case '+': return Builder.CreateAdd(L, R, "addtmp");
		case '-': return Builder.CreateSub(L, R, "subtmp");
		case '*': return Builder.CreateMul(L, R, "multmp");
	}
    Message::error("Invalid math operator.");
    return NULL;
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
        throw Message::Exception("Unknown function referenced.");
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

Function *CodeGen::compileProcPrototype(ASTNode *node) {
    return compilePrototype(node->str,Types.getType("void"),node->children[0]);
}

/*! creates an llvm function with no implementation.
 
 Used for extern declarations and as part of the function/procedure creation process.
    
 \param name The name of the function
 \param returnType The type the function returns. the void type if it is a procedure.
 \param params A DECLARATIONS ast node containing the formal parameters.
 
 */
Function *CodeGen::compilePrototype(const std::string &name, TuringType *returnType, ASTNode *params) {
    
    std::vector<VarDecl> args = getDecls(params);
    
    // Make the function type:  double(double,double) etc.
    std::vector<Type*> argTypes;
    // extract argument types
    for (int i = 0; i < args.size(); ++i) {
        argTypes.push_back(args[i].Type->LLVMType);
    }
    
    FunctionType *FT = FunctionType::get(returnType->LLVMType,
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
        
        // TODO Add arguments to variable symbol table.
    }
    
    return f;
}