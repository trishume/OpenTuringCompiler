#include "codegen.h"

#include "language.h"
#include "ast.h"

#include "Message.h"

#define ITERATE_CHILDREN(node,var) \
for(std::vector<ASTNode*>::iterator var = (node)->children.begin(), e = (node)->children.end();var < e;++var)

using namespace llvm;

CodeGen::CodeGen(ASTNode *tree) : Builder(llvm::getGlobalContext()), Blocks() {
	Root = tree;
}

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
}

bool CodeGen::execute() {
	LLVMContext &context = getGlobalContext();
    
    Message::log("Generating code...");
    
	MainModule = new Module("turing JIT module", context);
    
	/* Create the top level interpreter function to call as entry */
	std::vector<Type*> argTypes;
	FunctionType *mainFuntionType = FunctionType::get(Type::getVoidTy(getGlobalContext()), argTypes, false);
	MainFunction = Function::Create(mainFuntionType, GlobalValue::InternalLinkage, "main", MainModule);
	BasicBlock *mainBlock = BasicBlock::Create(getGlobalContext(), "entry", MainFunction, 0);
    
	/* Push a new variable/block context */
	pushBlock(mainBlock);
	bool good = compileBlock(Root);
	Builder.CreateRetVoid();
	popBlock();
    
	MainModule->dump();
    
	return good; // success
}

//! Compiles a block of instructions
//! Returns success
bool CodeGen::compileBlock(ASTNode *node) {
	if(node->root == Language::BLOCK) {
		Value *result = NULL;
		ITERATE_CHILDREN(node,child) {
			result = compile(*child);
			if(result == NULL) {
				return false; // failure
			}
		}
		return true;
	} else {
        Message::error("Node is not a block");
		return false;
	}
}

//! dispatcher. Checks the type of the node and calls the correct compile function.
Value *CodeGen::compile(ASTNode *node) {
	if(node == NULL) {
		Message::error("Can not compile null node.");
        return NULL;
	}
	switch(node->root) {
		case Language::MATH_OP:
			return compileMathOp(node);
		case Language::CALL:
			return compileCall(node);
		case Language::INT_LITERAL:
			// apint can convert a string
			return ConstantInt::get(getGlobalContext(), APInt(64,node->str,10));
	}
	Message::error(Twine("AST type ") + Language::getTokName(node->root) + " not recognized");
    return NULL;
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
Value *CodeGen::compileCall(ASTNode *node) {
    // Look up the name in the global module table.
    Function *calleeFunc = MainModule->getFunction(node->str);
    if (calleeFunc == NULL) {
        Message::error("Unknown function referenced");
        return NULL;
    }
    
    // If argument mismatch error.
    if (calleeFunc->arg_size() != node->children.size()) {
        Message::error(Twine(node->children.size()) + " arguments passed to a function that takes " + Twine(calleeFunc->arg_size()));
        return NULL;
    }
    
    std::vector<Value*> argVals;
    for (unsigned i = 0, e = node->children.size(); i < e; ++i) {
        argVals.push_back(compile(node->children[i]));
    }
    
    return Builder.CreateCall(calleeFunc, argVals, "calltmp");
}