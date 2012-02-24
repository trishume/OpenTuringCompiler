#include "codegen.h"

// for std::pair
#include <utility>

#include <llvm/Instruction.h>
#include <llvm/Constants.h>
#include <llvm/InstrTypes.h>
#include <llvm/Attributes.h>

#include "language.h"
#include "ast.h"

#include "Message.h"
#include "Executor.h"

#include "Symbol.h"

#define ITERATE_CHILDREN(node,var) \
for(std::vector<ASTNode*>::iterator var = (node)->children.begin(), e = (node)->children.end();var < e;++var)

static const std::string defaultIncludes =
    "extern proc TuringPrintInt(val : int)\n"
    "extern proc TuringPrintReal(val : real)\n"
    "extern proc TuringPrintBool(val : boolean)\n"
    "extern proc TuringPrintString(val : string)\n"
    "extern proc TuringGetString(val : string)\n"
    "extern \"length\" fcn TuringStringLength(val : string) : int\n"
    "extern fcn TuringStringConcat(lhs,rhs : string) : string\n"
    "extern proc TuringPrintNewline()\n"
    "extern fcn TuringPower(val : int, power : int) : int\n"
    "extern fcn TuringIndexArray(index : int, length : int) : int\n"
    "extern proc TuringCopyArray(to : voidptr, from : voidptr, fromLength : int, toLength : int)\n"
    "extern fcn TuringCompareArray(to : voidptr, from : voidptr, fromLength : int, toLength : int) : boolean\n"
;

using namespace llvm;

#pragma mark Construction

CodeGen::CodeGen(ASTSource *source) :   TheSource(source), CurFile(""), CanExecute(true),
Builder(llvm::getGlobalContext()), RetVal(NULL), RetBlock(NULL) {
    Types.addDefaultTypes(getGlobalContext());
    TheModule = new Module("turing JIT module", getGlobalContext());
    Scopes = new ScopeManager(TheModule);
    
    // Get the module ready to start compiling
    
    /* Create the top level interpreter function to call as entry */
	std::vector<Type*> argTypes;
	FunctionType *mainFuntionType = FunctionType::get(Type::getVoidTy(getGlobalContext()), argTypes, false);
	MainFunction = Function::Create(mainFuntionType, GlobalValue::InternalLinkage, MAIN_FUNC_NAME, TheModule);
	BasicBlock *mainBlock = BasicBlock::Create(getGlobalContext(), "entry", MainFunction, 0);
    
	Builder.SetInsertPoint(mainBlock);
    
    // Add all the functions that the language needs to function
    ASTNode *includesRoot = TheSource->parseString(defaultIncludes,false);
    if (includesRoot == NULL) {
        throw Message::Exception("Failed to parse default includes. This shouldn't happen ever.");
    }
    compileRootNode(includesRoot, "<default includes>");
}

bool CodeGen::compileFile(std::string fileName) {
    Message::log(Twine("Compiling file \"") + fileName + "\".");
    ASTNode *fileRoot = TheSource->parseFile(fileName, CurFile);
    
    if (fileRoot == NULL) {
        throw Message::Exception(Twine("Failed to parse file \"") + fileName + "\".");
    }
    return compileRootNode(fileRoot,fileName);
}

bool CodeGen::compileRootNode(ASTNode *fileRoot, std::string fileName) {    
    std::string oldCurFile = CurFile;
    CurFile = fileName;
    bool good = false;
    try {
        good = compileBlock(fileRoot);
    } catch (Message::Exception e) {
        good = false;
        Message::error(e.Message);
        
    }
    CurFile = oldCurFile;
    if (!good) {
        // can't execute it if it failed.
        CanExecute = false;
    }
    
    return good;
}

#pragma mark Execution

bool CodeGen::execute(bool dumpModule) {
    
    if(dumpModule) TheModule->dump();
    
    if (!CanExecute) {
        Message::error("Code generation failed. Can not execute.");
        return false; // fail
    }
    
    // Finalize the main function
    BasicBlock *endBB = BasicBlock::Create(getGlobalContext(), "returnblock");
    Builder.CreateBr(endBB);
    MainFunction->getBasicBlockList().push_back(endBB);
    Builder.SetInsertPoint(endBB);
    if (!isCurBlockTerminated()) {
        Builder.CreateRetVoid();
    }
    
    // we have it finalized. No more!
    CanExecute = false;
    
    // run it!
    Message::setCurLine(0, "");
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

Value *CodeGen::getConstantInt(int index) {
    return ConstantInt::get(getGlobalContext(), APInt(32,index));
}

Value *CodeGen::compileArrayByteSize(Value *arrayRef) {
    std::vector<Value*> indices;
    // depointerize
    indices.push_back(ConstantInt::get(getGlobalContext(), APInt(32,0)));
    // get the array part of the turing array struct
    indices.push_back(ConstantInt::get(getGlobalContext(), APInt(32,1)));
    // index it with the length
    indices.push_back(compileArrayLength(arrayRef));
    
    // double checking, this should never happen
    if (!arrayRef->getType()->isPointerTy()) {
        throw Message::Exception("Can't find the length of a non-referenced array.");
    }
    
    Value *sizePtr = Builder.CreateGEP(ConstantPointerNull::get(cast<PointerType>(arrayRef->getType())),indices,"arraysizeptr");
    return Builder.CreatePointerCast(sizePtr,Types.getType("int")->getLLVMType(),"arrlengthint");
}

Value *CodeGen::compileArrayLength(Value *arrayRef) {
    if (!Types.isArrayRef(arrayRef->getType())) {
        throw Message::Exception("Can only find the upper limit of an array.");
    }
    return Builder.CreateLoad(Builder.CreateConstGEP2_32(arrayRef,0,0,"arrlengthptr"),"arraylengthval");
}

std::pair<Value*,Value*> CodeGen::compileRange(ASTNode *node) {
    Value *start = compile(node->children[0]);
    Value *end = compile(node->children[1]);
    
    if (! start->getType()->isIntegerTy() && !end->getType()->isIntegerTy() ) {
        throw Message::Exception("The start and end of a range must be 'int's.");
    }
    
    return std::pair<Value*,Value*>(start,end);
}

//! properly initializes complex data structures. It's main duty is initializing the length of arrays.
//! \param declared A pointer to a newly allocated buffer to be initialized
void CodeGen::compileInitializeComplex(Value *declared, TuringType *type) {
    // must initialize the length part of the array struct
    if (type->isArrayTy()) {
        TuringArrayType *arrtype = static_cast<TuringArrayType*>(type);
        Value *arrLengthPtr = Builder.CreateConstGEP2_32(declared,0,0,"arrlengthptr");
        Value *length = ConstantInt::get(getGlobalContext(),APInt(32,arrtype->getSize()));
        Builder.CreateStore(length,arrLengthPtr);
        
        // initialize the subelements
        // TODO make the loop at runtime not compile time. This is inefficient and hacky.
        if (arrtype->getElementType()->isComplexTy()) {
            // use 1 and <= because we are using Turing's one based indices
            for (unsigned int i = 1; i <= arrtype->getSize(); ++i) {
                Value *indexed = abstractCompileIndex(declared, getConstantInt(i));
                compileInitializeComplex(indexed,arrtype->getElementType());
            }
        }
    }
    if (type->isRecordTy()) {
        TuringRecordType *recType = static_cast<TuringRecordType*>(type);
        // initialize fields if they are complex
        for (unsigned int i = 0; i < recType->getSize(); ++i) {
            VarDecl field = recType->getDecl(i);
            if (field.Type->isComplexTy()) {
                // get the field to initialize
                Symbol *recordSym = new VarSymbol(declared,type);
                Symbol *fieldSym = compileRecordFieldRef(recordSym, field.Name);
                delete recordSym;
                // initialize it
                compileInitializeComplex(fieldSym->getVal(),fieldSym->getType());
            }            
        }
    }
}

#pragma mark Transformation

//! transforms a declarations node into a vector of declarations. Throws exceptions.
//! \param node A ASTNode that can come from the "type" rule.
//! \return the TuringType for a node.
TuringType *CodeGen::getType(ASTNode *node) {
    switch(node->root) {
		case Language::NAMED_TYPE:
			return Types.getType(node->str); // can't be NULL
        case Language::ARRAY_TYPE:
            return getArrayType(node);
        case Language::RECORD_TYPE:
            return getRecordType(node);
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

TuringType *CodeGen::getRecordType(ASTNode *node) {
    std::vector<VarDecl> decls;
    
    ITERATE_CHILDREN(node, it) {
        std::vector<VarDecl> subDecls = getDecls(*it,false); // don't allow auto types
        // tack them on to the end of the decls list
        decls.insert(decls.end(), subDecls.begin(), subDecls.end());
    }
    return new TuringRecordType(decls);
}

// TODO TEST fancy logic. Test this.
TuringType *CodeGen::getArrayType(ASTNode *node) {
    TuringType *arrayType = getType(node->children[0]);
    // the node can contain multiple ranges. These denote multi-dimensional arrays.
    // Since these are just arrays in arrays. We keep wrapping the previous type
    // in an array until there are no more ranges. Starting from the end so that
    // 1..5, 1..4 is [5 x [4 x type]] instead of vice-versa.
    for (int i = node->children.size() - 1; i > 0; --i) {
        ASTNode *range = node->children[i];
        Value *upperVal = compile(range->children[1]);
        
        // we don't want someone putting "array bob..upper(bob) of int" because
        // we have to know the size at compile time.
        if (!isa<ConstantInt>(upperVal)) {
            throw Message::Exception("Bounds of array must be int constants");
        }
        // *ConstantInt -> APInt -> uint_64
        int upper = cast<ConstantInt>(upperVal)->getValue().getLimitedValue();
        // wrap it up
        arrayType = Types.getArrayType(arrayType,upper);
    }
    return arrayType;
}

//! transforms a declarations node into a vector of declarations. 
//! \param decls A DECLARATIONS ASTNode.
//! \return vector of declarations.
std::vector<VarDecl> CodeGen::getDecls(ASTNode *astDecls,bool allowAutoTypes) {
    if(astDecls->root != Language::DECLARATIONS) throw Message::Exception("Expected DECLARATIONS node in getDecls");
    
    std::vector<VarDecl> decls;
    ITERATE_CHILDREN(astDecls,decl) {
        TuringType *type = getType((*decl)->children[0]);
        if(!allowAutoTypes && (type == Types.getType("auto") || type == Types.getType("void"))) {
            throw Message::Exception("Can't infer type for this declaration.");
        }
        decls.push_back(VarDecl((*decl)->str,type));
    }
    
    return decls;
}

//! Creates a symbol out of an allocated buffer pointer
Symbol *CodeGen::getSymbolForVal(Value *val) {
    if (!val->getType()->isPointerTy()) {
        throw Message::Exception("Symbols must be a pointer type");
    }
    
    return new VarSymbol(val,Types.getTypeLLVM(cast<PointerType>(val->getType())->getElementType()));
}

Value *CodeGen::promoteType(Value *val, TuringType *destType) {
    Type *type = val->getType();
    Type *llvmDestType = destType->getLLVMType(true);
    
    // if they are the same, no casting needed
    if (type == llvmDestType) {
        return val;
    }
    
    if (type->isIntegerTy() && llvmDestType->isFloatingPointTy()) {
        return Builder.CreateSIToFP(val, llvmDestType, "promotedint");
    }
    
    // if it gets this far, it's an error
    throw Message::Exception(Twine("Can't convert expression to type ") + destType->getName());
    
    return NULL; // never reaches here
    
}

#pragma mark Compilation

//! Compiles a block of instructions
//! Returns success
bool CodeGen::compileBlock(ASTNode *node) {
	if(node->root == Language::BLOCK) {
		ITERATE_CHILDREN(node,child) {
			if(!compileStat(*child)) {
				throw Message::Exception("Failed to compile statement in block.");
			}
            // used for returns and exits
            // instructions past this point will be unreachable so quit early
            if (isCurBlockTerminated()) break;
		}
	} else {
        throw Message::Exception("Node is not a block");
	}
    return true;
}

//! dispatcher for statements. Checks the type of the node and calls the correct compile function.
//! treats as an expression if it can.
bool CodeGen::compileStat(ASTNode *node) {
	if(node == NULL) {
		Message::error("Can not compile null node.");
        return false;
	}
    
    Message::setCurLine(node->getLine(),CurFile);
    
    switch(node->root) {
        case Language::EXTERN_DECL: // extern declaration
            return compileFunctionPrototype(node->children[0],node->str) != NULL;
        case Language::FUNC_DEF:
            return compileFunction(node);
        case Language::MODULE_DEF:
            compileModule(node);
            return true;
        case Language::CALL:
            compileCall(node,false); // special case allowing procedure calls
            return true; // throws error on fail
        case Language::VAR_DECL:
            compileVarDecl(node);
            return true; // throws error on fail
        case Language::IF_STAT:
            compileIfStat(node);
            return true; // throws error on fail
        case Language::LOOP_STAT:
            compileLoopStat(node);
            return true;
        case Language::FOR_STAT:
            compileForStat(node);
            return true;
        case Language::PUT_STAT:
            compilePutStat(node);
            return true;
        case Language::GET_STAT:
            compileGetStat(node);
            return true;
        case Language::INCLUDE_STAT:
            compileFile(node->str);
            return true;
        case Language::RETURN_STAT:
            compileReturn();
            return true;
        case Language::EXIT_STAT:
            Builder.CreateBr(ExitBlock);
            return true;
        case Language::RESULT_STAT:
        {
            if (RetVal == NULL) {
                throw Message::Exception("Result can only be used inside a function.");
            }
            Value *val = compile(node->children[0]);
            abstractCompileAssign(val,getSymbolForVal(RetVal));
            compileReturn();
            return true;
        }
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
        case Language::EQUALITY_OP:
            return compileEqualityOp(node);
        case Language::CALL:
            return compileCallSyntax(node);
        case Language::ARRAY_UPPER:
            return compileArrayLength(compile(node->children[0]));
        case Language::VAR_REFERENCE:
        case Language::FIELD_REF_OP:
            // compileLHS knows how to handle these. We just have to load them.
            return abstractCompileVarReference(compileLHS(node),node->str);
        case Language::STRING_LITERAL:
            return compileStringLiteral(node->str);
        case Language::INT_LITERAL:
            // apint can convert a string
            return ConstantInt::get(getGlobalContext(), APInt(32,node->str,10));
        case Language::REAL_LITERAL:
            // apint can convert a string
            return ConstantFP::get(Types.getType("real")->getLLVMType(), node->str);
        case Language::BOOL_LITERAL:
            // apint is used because booleans are one bit ints
            return ConstantInt::get(getGlobalContext(), APInt(1,(node->str.compare("true") == 0) ? 1 : 0));
        default:
            throw Message::Exception(Twine("AST type ") + Language::getTokName(node->root) + " not recognized");
    }
}
//! creates a constant struct to represent a string literal
//! \returns an array reference to the string literal
Value *CodeGen::compileStringLiteral(const std::string &str) {
    std::vector<Constant*> arrayStructVals,arrayVals;
    
    // string length
    arrayStructVals.push_back(ConstantInt::get(getGlobalContext(),APInt(32,str.size()+1)));
    
    // create string array
    
    // iterate and add the characters, C STYLE!
    // TODO maybe make this not use C style pointer iteration
    const char *cstr = str.c_str();
    while (*cstr != 0) {
        arrayVals.push_back(ConstantInt::get(getGlobalContext(),APInt(8,*cstr)));
        ++cstr;
    }    
    
    // add null terminator
    arrayVals.push_back(ConstantInt::get(getGlobalContext(),APInt(8,0)));
    
    // add the string to the struct
    ArrayType *arrTy = ArrayType::get(Types.getType("int8")->getLLVMType(),str.size()+1);
    arrayStructVals.push_back(ConstantArray::get(arrTy,arrayVals));
    
    std::vector<Type *> structTypes;
    structTypes.push_back(Types.getType("int")->getLLVMType());
    structTypes.push_back(arrTy);
    StructType *structTy = StructType::get(getGlobalContext(),structTypes);
    
    Constant *structConst = ConstantStruct::get(structTy,arrayStructVals);
    
    Value *gvar = new GlobalVariable(/*Module=*/*TheModule,
                                     /*Type=*/structTy,
                                     /*isConstant=*/true,
                                     /*Linkage=*/GlobalValue::InternalLinkage,
                                     /*Initializer=*/structConst,
                                     "stringConst");
    Type *stringRefType = Types.getType("string")->getLLVMType(true);
    
    return Builder.CreatePointerCast(gvar,stringRefType,"castedstringconstant");
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
    
    bool fp = false;
    if (L->getType()->isFloatingPointTy() || R->getType()->isFloatingPointTy()) {
        fp = true;
        L = promoteType(L, Types.getType("real"));
        R = promoteType(R, Types.getType("real"));
    }
    
    Instruction::BinaryOps binOp;
	
    // COMPARISONS
	if (op.compare(">") == 0) {
        return fp ? Builder.CreateFCmpOGT(L, R) : Builder.CreateICmpSGT(L, R);
    } else if (op.compare("<") == 0) {
        return fp ? Builder.CreateFCmpOLT(L, R) : Builder.CreateICmpSLT(L, R);
    } else if (op.compare(">=") == 0) {
        return fp ? Builder.CreateFCmpOGE(L, R) : Builder.CreateICmpSGE(L, R);
    } else if (op.compare("<=") == 0) {
        return fp ? Builder.CreateFCmpOLE(L, R) : Builder.CreateICmpSLE(L, R);
    } else if (op.compare("+") == 0) { // MATH
        // string + string = TuringStringConcat(string)
        if (Types.isType(L, "string") && Types.isType(R, "string")) {
            Symbol *callee = Scopes->curScope()->resolve("TuringStringConcat");
            std::vector<Value*> params;
            params.push_back(L);
            params.push_back(R);
            return abstractCompileCall(callee, params, true);
        }
        binOp = fp ? Instruction::FAdd : Instruction::Add;
    } else if (op.compare("-") == 0) {
        binOp = fp ? Instruction::FSub : Instruction::Sub;
    } else if (op.compare("*") == 0) {
        binOp = fp ? Instruction::FMul : Instruction::Mul;
    } else if (op.compare("/") == 0) {
        if (!fp) { // this one is always floating point
            L = promoteType(L, Types.getType("real")); R = promoteType(R, Types.getType("real"));
        }
        binOp = Instruction::FDiv;
    } else if (op.compare("div") == 0) {
        if (fp) { // TODO resolve this
            throw Message::Exception("Can't use 'div' on real numbers yet. Try using '/' and then rounding");
        }
        binOp = Instruction::SDiv;
    } else if (op.compare("mod") == 0) {
        binOp = fp ? Instruction::FRem : Instruction::SRem;
    } else if (op.compare("**") == 0) {
        if (fp) throw Message::Exception("Can't use '**' on real numbers.");
        std::vector<Value*> argVals;
        argVals.push_back(L);
        argVals.push_back(R);
        return Builder.CreateCall(TheModule->getFunction("TuringPower"),argVals,"powertmp");
    } else {
        throw Message::Exception("Invalid binary operator.");
    }
    
    // if it hasn't already been promoted (which type checks), do type checking
    if (!fp) {
        L = promoteType(L, Types.getType("int"));
        R = promoteType(R, Types.getType("int"));
    }
    
    // if it hasn't returned by now it must be a normal binop
    return BinaryOperator::Create(binOp, L, R,"binOpRes",Builder.GetInsertBlock());
}

//! compiles a properly short-circuiting logic operator
//! \param isAnd false for 'or' true for 'and'
Value *CodeGen::compileLogicOp(ASTNode *node) {
    Value *cond1 = compile(node->children[0]);
    
    if (!Types.isType(cond1,"boolean")) {
        throw Message::Exception(Twine("Arguments of logical ") + node->str + " must be of type boolean");
    }
    
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
    
    if (!Types.isType(cond2,"boolean")) {
        throw Message::Exception(Twine("Arguments of logical ") + node->str + " must be of type boolean");
    }
    
    Builder.CreateBr(mergeBB);
    
    
    // Emit merge block.
    theFunction->getBasicBlockList().push_back(mergeBB);
    Builder.SetInsertPoint(mergeBB);
    
    PHINode *resPhi = Builder.CreatePHI(Types.getType("boolean")->getLLVMType(),2,"andresult");
    resPhi->addIncoming(cond1,startBlock);
    resPhi->addIncoming(cond2,secondBB);
    
    return resPhi;
}

Value *CodeGen::compileEqualityOp(ASTNode *node) {
    Value *L = compile(node->children[0]);
    Value *R = compile(node->children[1]);
    
    bool fp = false;
    if (L->getType()->isFloatingPointTy() || R->getType()->isFloatingPointTy()) {
        fp = true;
        L = promoteType(L, Types.getType("real"));
        R = promoteType(R, Types.getType("real"));
    } else if (L->getType() != R->getType()) {
        throw Message::Exception("Arguments of comparison must be the same type.");
    }
    
    TuringType *type = Types.getTypeLLVM(L->getType(),true);
    
    Value *ret = NULL;
    
    if (type->getName().compare("int") == 0 || type->getName().compare("boolean") == 0) {
        ret = Builder.CreateICmpEQ(L,R,"equal");        
    } else if (fp) {
        ret = Builder.CreateFCmpOEQ(L, R,"fpequal");
    } else if (type->isArrayTy()) { // strings and arrays
        Value *srcSize = compileArrayByteSize(L);
        Value *destSize = compileArrayByteSize(R);
        Value *fromPtr = Builder.CreatePointerCast(L,Types.getType("voidptr")->getLLVMType(),"fromptr");
        Value *toPtr = Builder.CreatePointerCast(R,Types.getType("voidptr")->getLLVMType(),"toptr");
        ret = Builder.CreateCall4(TheModule->getFunction("TuringCompareArray"),fromPtr,toPtr,srcSize,destSize);
    } else {
        throw Message::Exception(Twine("Can't compare type ") + type->getName());
    }

    // if it is not =, it is ~= so invert it
    if (node->str.compare("=") != 0) {
        // bool == 0 is the same as not bool
        ret = Builder.CreateICmpEQ(ret,ConstantInt::get(getGlobalContext(), APInt(1,0)));
    }
    
    return ret;
}

Symbol *CodeGen::compileLHS(ASTNode *node) {
    switch (node->root) {
        case Language::VAR_REFERENCE:
            return Scopes->curScope()->resolve(node->str);
        case Language::FIELD_REF_OP:
        {
            ASTNode *lhs = node->children[0];
            // is it a module? If so, retrieve its scope and resolve the variable without searching up.
            if (lhs->root == Language::VAR_REFERENCE &&
                Scopes->namedScopeExists(lhs->str)) {
                return Scopes->getNamedScope(lhs->str)->resolveVarThis(node->str);
            }
            
            // is it a record?
            Symbol *rec = compileLHS(lhs);
            if (rec->getType()->isRecordTy()) {
                return compileRecordFieldRef(rec, node->str);
            }
            
            // TODO other reference types
            throw Message::Exception(Twine("Can't reference element ") + node->str);
        }
        case Language::CALL:
        {
            // must be an array reference
            Symbol *callee = compileLHS(node->children[0]);
            if (!callee->getType() || !callee->getType()->isArrayTy()) {
                throw Message::Exception("Can't index something that isn't an array.");
            }
            
            TuringArrayType *arr = static_cast<TuringArrayType*>(callee->getType());
            
            return new VarSymbol(compileIndex(callee->getVal(),node),arr->getElementType());
        }
        default:
            throw Message::Exception(Twine("LHS AST type ") + Language::getTokName(node->root) + 
                                     " not recognized. You might be using a feature that hasn't been implemented yet");
    }
    return new VarSymbol(); // never reaches here
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
    Symbol *assignSym = compileLHS(node->children[0]);
    
    abstractCompileAssign(val, assignSym);
    
    return val;
}

void CodeGen::abstractCompileAssign(Value *val, Symbol *assignSym) {
    Value *assignVar = assignSym->getVal();
    
    // can't assign to arguments unless they are actual structure returns
    if (isa<Argument>(assignVar) && !(cast<Argument>(assignVar)->hasStructRetAttr())) {
        
        throw Message::Exception("Can't assign to an argument.");
    }
    
    // store needs a pointer
    if (!(assignVar->getType()->isPointerTy())) {
        throw Message::Exception("Can't assign to something that is not a variable");
    }
    
    // assigning an array to an array copies it
    if (assignSym->getType()->isArrayTy()) {
        compileArrayCopy(val,assignSym);
        return;
    }
    
    // assert asignee is a pointer to the type being assigned
    if (cast<PointerType>(assignVar->getType())->getElementType() != val->getType()) {
        throw Message::Exception(Twine("Only expressions of type \"") + 
                                 assignSym->getType()->getName() + 
                                 "\" can be assigned to this variable.");
    }
    
    Builder.CreateStore(val,assignVar);
}

void CodeGen::compileArrayCopy(Value *from, Symbol *to) {
    Value *srcSize = compileArrayByteSize(from);
    Value *destSize = compileArrayByteSize(to->getVal());
    Value *fromPtr = Builder.CreatePointerCast(from,Types.getType("voidptr")->getLLVMType(),"fromptr");
    Value *toPtr = Builder.CreatePointerCast(to->getVal(),Types.getType("voidptr")->getLLVMType(),"toptr");
    Builder.CreateCall4(TheModule->getFunction("TuringCopyArray"),fromPtr,toPtr,srcSize,destSize);
}

void CodeGen::compilePutStat(ASTNode *node) {
    // print out all the comma separated expressions
    ITERATE_CHILDREN(node, curNode) {
        Value *val = compile(*curNode);
        TuringType *type = Types.getTypeLLVM(val->getType(),true);
        
        Function *calleeFunc;
        std::vector<Value*> argVals;
        argVals.push_back(val);
        
        if (type->getName().compare("int") == 0) {
            calleeFunc = TheModule->getFunction("TuringPrintInt");
        } else if (type->getName().compare("real") == 0) {
            calleeFunc = TheModule->getFunction("TuringPrintReal");
        } else if (type->getName().compare("boolean") == 0) {
            calleeFunc = TheModule->getFunction("TuringPrintBool");
        } else if (type->getName().compare("string") == 0) {
            calleeFunc = TheModule->getFunction("TuringPrintString");
        } else {
            throw Message::Exception(Twine("Can't 'put' type ") + type->getName());
        }
            
        Builder.CreateCall(calleeFunc,argVals);        
    }
    
    // if the string is not ".." print a newline
    if (node->str.compare("..") != 0) {
        Builder.CreateCall(TheModule->getFunction("TuringPrintNewline"),std::vector<Value*>());
    }
}

void CodeGen::compileGetStat(ASTNode *node) {
    Symbol *var = compileLHS(node->children[0]);
    
    if (var->getType()->getName().compare("string") != 0) {
        throw Message::Exception("This version of the Open Turing Compiler can only 'get' strings.");
    }
    
    Function *calleeFunc = TheModule->getFunction("TuringGetString");
    Builder.CreateCall(calleeFunc, Builder.CreatePointerCast(var->getVal(), var->getType()->getLLVMType(true)));
}

void CodeGen::compileVarDecl(ASTNode *node) {
    std::vector<VarDecl> args = getDecls(node->children[0]);
    
    for (unsigned int i = 0; i < args.size();++i) {
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
        
        Symbol *declared = Scopes->curScope()->declareVar(args[i].Name,type);
        // must initialize the length part of the array struct
        if (type->isComplexTy()) {
            compileInitializeComplex(declared->getVal(), type);
        }
        
        if (hasInitial) {
            abstractCompileAssign(initializer, declared);
        }
    }
}

Value *CodeGen::abstractCompileVarReference(Symbol *var,const std::string &name) {
    // TODO maybe do as clang does and alloca and assign all params instead of this
    if (!(var->getVal()->getType()->isPointerTy())) {
        // if it is not a reference return it straight up. Used for arguments.
        return var->getVal();
    }
    // arrays are referenced
    if (var->getType()->isArrayTy()) {
        return Builder.CreateBitCast(var->getVal(),var->getType()->getLLVMType(true),"arrayref");
    }
    
    // TODO implement function references
    if (var->isFunction()) {
        throw Message::Exception("Function references are not implemented yet");
    }
    
    return Builder.CreateLoad(var->getVal(),Twine(name) + "val");
}

Value *CodeGen::compileIndex(llvm::Value *indexed,ASTNode *node) {
    if (node->children.size() == 0) {
        throw Message::Exception("Must have at least one array index in brackets.");
    }
    return abstractCompileIndex(indexed,compile(node->children[1]));
}

Symbol *CodeGen::compileRecordFieldRef(Symbol *record, std::string fieldName) {
    if (!record->getType()->isRecordTy()) {
        throw Message::Exception(Twine("Can't access field of non-record type \"") + record->getType()->getName() + "\".");
    }
    TuringRecordType *recType = static_cast<TuringRecordType*>(record->getType());
    
    // in LLVM, structs don't have named fields so turn the name into an index
    unsigned int fieldIndex = recType->getIndex(fieldName);
    Value *fieldPtr = Builder.CreateConstGEP2_32(record->getVal(), 0, recType->getIndex(fieldName),
                                                 Twine("recordField")+fieldName);
    TuringType *fieldType = recType->getDecl(fieldIndex).Type;
    return new VarSymbol(fieldPtr,fieldType);
}

//! \param node  a CALL node
//! \returns a pointer to the element
Value *CodeGen::abstractCompileIndex(Value *indexed,Value *index) {        
    std::vector<Value*> indices;
    // depointerize
    indices.push_back(ConstantInt::get(getGlobalContext(), APInt(32,0)));
    // get the array part of the turing array struct
    indices.push_back(ConstantInt::get(getGlobalContext(), APInt(32,1)));
    // take the 1-based index, bounds-check it and return the 0-based index
    Value *realIndex = Builder.CreateCall2(TheModule->getFunction("TuringIndexArray"),index,
                                            compileArrayLength(indexed),"realIndexVal");
    indices.push_back(realIndex);
    return Builder.CreateInBoundsGEP(indexed,indices,"indextemp");
}

//! in turing, the call syntax is used for array indexes, calls and other things
//! this function dispatches the call to the correct compile function
Value *CodeGen::compileCallSyntax(ASTNode *node) {
    Symbol *callee = compileLHS(node->children[0]);
    if (callee->isFunction()) {
        return compileCall(callee,node,true);
    }
    
    if (callee->getType() == NULL) {
        // FIXME EEEEVVVVIIIIILLLL!!!!
        goto fail;
    }
    
    if (callee->getType()->isArrayTy()) {
        return Builder.CreateLoad(compileIndex(callee->getVal(),node),"indexloadtemp");
    }
    
fail:        
    throw Message::Exception("Only functions and procedures can be called.");
    return NULL; // never reaches here
}
Value *CodeGen::compileCall(ASTNode *node, bool wantReturn) {
    return compileCall(compileLHS(node->children[0]),node,wantReturn);
}

Value *CodeGen::compileCall(Symbol *callee,ASTNode *node, bool wantReturn) {
    std::vector<Value*> params;
    for (unsigned int i = 1; i < node->children.size(); ++i) {
        params.push_back(compile(node->children[i]));
    }
    return abstractCompileCall(callee, params, wantReturn);
}

//! Compile a function call
//! \param wantReturn  Wether the return value is ignored. 
//!                    Should always be true for procedures.
//! \return the return value of the function
//!         unless the wantReturn parameter is null
//!         in wich case NULL is returned.
//!         defaults to true.
Value *CodeGen::abstractCompileCall(Symbol *callee,const std::vector<Value*> &params, bool wantReturn) {
    
    if (!callee->isFunction()) {
        throw Message::Exception("Only functions and procedures can be called.");
    }
    
    // must be a function
    FunctionSymbol *calleeFuncSym = static_cast<FunctionSymbol*>(callee);
    Function *calleeFunc = cast<Function>(calleeFuncSym->getVal());
    
    // If argument mismatch error.
    unsigned int numArgsPassed = params.size();
    unsigned int numArgsNeeded = calleeFunc->arg_size();
    if (calleeFuncSym->IsSRet) {
        numArgsNeeded -= 1; // first argument is automatically passed
    }
    if (numArgsNeeded != numArgsPassed) {
        throw Message::Exception(Twine(numArgsPassed) + " arguments passed to a function that takes " + 
                                 Twine(numArgsNeeded));
    }
    
    std::vector<Value*> argVals;
    // args start at second child    
    Function::arg_iterator ai = calleeFunc->arg_begin(), e = calleeFunc->arg_end();
    
    Value *returnBuffer = NULL;
    if (calleeFuncSym->IsSRet) {
        // start at the entry block. Optimizers like this
        BasicBlock *entryBlock = &currentFunction()->getEntryBlock();
        IRBuilder<> TmpB(entryBlock,entryBlock->begin());
        // pass a memory buffer of the return type as the first parameter, false means not a reference
        returnBuffer = TmpB.CreateAlloca(calleeFuncSym->getType()->getLLVMType(false),0,"sretBuffer");
        // cast it to a reference
        returnBuffer = Builder.CreatePointerCast(returnBuffer, calleeFuncSym->getType()->getLLVMType(true));
        compileInitializeComplex(returnBuffer, calleeFuncSym->getType()); // initialize array lengths and stuff
        argVals.push_back(returnBuffer);
        // move on to the next argument
        ai++;
    }
    for (unsigned idx = 0; ai != e;++ai, ++idx) {
        Value *val = params[idx];
        TuringType *typ = Types.getTypeLLVM(ai->getType(),true);
        argVals.push_back(promoteType(val, typ));
    }
    if (calleeFuncSym->IsSRet) {        
        assert(returnBuffer != NULL);
        // return the casted reference to the return value. true = this is a reference
        Builder.CreateCall(calleeFunc, argVals);
        return returnBuffer;
    } else if (calleeFunc->getReturnType()->isVoidTy() && wantReturn) {
        throw Message::Exception(Twine("Procedure ") + calleeFunc->getName() + " can not return a value.");
    } else if(!wantReturn) {
        Builder.CreateCall(calleeFunc, argVals);
        return NULL;
    }
    
    return Builder.CreateCall(calleeFunc, argVals, "calltmp");
}

Function *CodeGen::compileFunctionPrototype(ASTNode *node, const std::string &aliasName) {
    // false = external function
    return compilePrototype(node->str,getType(node->children[0]),getDecls(node->children[1]),aliasName,false)->getFunc();
}

/*! creates an llvm function with no implementation.
 
 Used for extern declarations and as part of the function/procedure creation process.
 
 \param name The name of the function
 \param returnType The type the function returns. the void type if it is a procedure.
 \param params A DECLARATIONS ast node containing the formal parameters.
 \param aliasName the name to put in the symbol table. Blank if same as LLVM func name.
 \param internal wether it is an internal function with a body or an extern declaration
 
 */
FunctionSymbol *CodeGen::compilePrototype(const std::string &name, TuringType *returnType, std::vector<VarDecl> args, const std::string &aliasName, bool internal) {
    
    bool structRet = returnType->isComplexTy();
    
    // Make the function type:  double(double,double) etc.
    std::vector<Type*> argTypes;
    // complex types are returned by passing a pointer to a return value as the first parameter
    if (structRet) {
        argTypes.push_back(returnType->getLLVMType(true));
    }
    
    // extract argument types
    for (unsigned int i = 0; i < args.size(); ++i) {
        argTypes.push_back(args[i].Type->getLLVMType(true));  // true = parameters are references
    }
    // the return type is void if it is a procedure OR if the return is a complex type
    // WARNING it shouldn't matter wether the getLLVMType() is a reference since complex types are passed
    // as parameters. However, some change later on may need this to be decided.
    Type *funcRetType = structRet ? Type::getVoidTy(getGlobalContext()) : returnType->getLLVMType();
    FunctionType *FT = FunctionType::get(funcRetType,argTypes, false);
    
    Function *f = Function::Create(FT, internal ? Function::InternalLinkage : Function::ExternalLinkage, name, TheModule);
    FunctionSymbol *fSym = new FunctionSymbol(f,returnType);
    fSym->IsSRet = structRet;
    
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
    Function::arg_iterator ai = f->arg_begin();
    // add the right attributes
    if (structRet) {
        ai->setName("returnVal");
        ai->addAttr(Attribute::StructRet);
        ++ai;
    }
    for (unsigned idx = 0;idx != args.size();
         ++ai, ++idx) {
        ai->setName(args[idx].Name);
    }
    
    // add it to the LOCAL scope so that modules work
    // the parser prevents them from being defined
    // in places they shouldn't be
    std::string symName = aliasName.empty() ? name : aliasName;
    Scopes->curScope()->setVar(symName,fSym);
    
    return fSym;
}

Function *CodeGen::compileFunction(ASTNode *node) {
    ASTNode *proto = node->children[0];
    std::vector<VarDecl> args = getDecls(proto->children[1]);
    // true = internal, "" = don't alias
    FunctionSymbol *fSym = compilePrototype(proto->str,getType(proto->children[0]),args,"",true);
    Function *f = fSym->getFunc();
    
    /* using allocas, allows modification of parameters. Saving the code in case it is needed
    // add arguments to the scope
    unsigned idx = 0;
    for (Function::arg_iterator ai = f->arg_begin(); idx != args.size();
         ++ai, ++idx) {
        Symbol arg = Scopes->curScope()->declareVar(args[idx].Name,args[idx].Type);
    }*/
    
    // save these for later
    IRBuilderBase::InsertPoint prevPoint = Builder.saveIP();
    
    BasicBlock *entryBB = BasicBlock::Create(getGlobalContext(), "entry", f);
    BasicBlock *endBB = BasicBlock::Create(getGlobalContext(), "returnblock");
    RetBlock = endBB;
    
    Builder.SetInsertPoint(entryBB);
    
    // declare this early so it can be read as the return value for srets
    Function::arg_iterator ai = f->arg_begin();
    
    if (fSym->IsSRet) {
        // RetVal is the memory pointed to by the first argument
        RetVal = &(*ai);
        ++ai; // the next argument is the actual first one
    } else if (!isProcedure(f)) {
        RetVal = Builder.CreateAlloca(f->getReturnType(), 0,"returnval");
    }
    
    // TODO add separate scope for arguments so they can be redefined?
    Scopes->pushLocalScope(f);
    // add NORMAL arguments to the scope
    for (unsigned idx = 0; idx != args.size();
         ++ai, ++idx) {
        Scopes->curScope()->setVar(args[idx].Name,&(*ai),args[idx].Type);
    }
    
    compileBlock(node->children[1]);
    
    if (!isCurBlockTerminated()) {
        Builder.CreateBr(endBB);
    }
    
    f->getBasicBlockList().push_back(endBB);
    
    // procedures have an implicit "return"
    Builder.SetInsertPoint(endBB);
    // complex returns and procedures have void return types
    if (isProcedure(f) || fSym->IsSRet) {        
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
    
    // verifies early. Disadvantage is that it does not print the module first. Makes debugging harder.
    //verifyFunction(*f);
    
    return f;
}
// compiles the "return" and "result" statements.
// the last block in a function contains the return statement
void CodeGen::compileReturn() {
    if (RetBlock == NULL) {
        throw Message::Exception("Can't use 'return' or 'result' outside of a function.");
    }
    Builder.CreateBr(RetBlock);
}

void CodeGen::compileModule(ASTNode *node) {
    Scopes->pushNamedScope(node->str);
    compileBlock(node->children[0]);
    Scopes->popScope();
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
    
    Builder.SetInsertPoint(thenBB);
    
    Scopes->pushLocalScope(currentFunction());
    compileBlock(node->children[1]);
    Scopes->popScope();
    
    if (!isCurBlockTerminated()) {
        Builder.CreateBr(mergeBB);
    }    
    
    if (hasElse) {
        // Emit else block.
        theFunction->getBasicBlockList().push_back(elseBB);
        Builder.SetInsertPoint(elseBB);
        
        Scopes->pushLocalScope(currentFunction());
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

//! compiles an if statement as a series of blocks
//! works on LOOP_STAT nodes
void CodeGen::compileLoopStat(ASTNode *node) {
    
    Function *theFunction = Builder.GetInsertBlock()->getParent();
    
    BasicBlock *loopBB = BasicBlock::Create(getGlobalContext(), "loop", theFunction);
    BasicBlock *mergeBB = BasicBlock::Create(getGlobalContext(), "loopcont");
    
    Builder.CreateBr(loopBB);
    
    Builder.SetInsertPoint(loopBB);
    
    // set the block the "exit" statement should continue to
    BasicBlock *prevExitBlock = ExitBlock;
    ExitBlock = mergeBB;
    
    Scopes->pushLocalScope(currentFunction());
    compileBlock(node->children[0]);
    Scopes->popScope();
    
    // stack like. return to previous one
    ExitBlock = prevExitBlock;
    
    if (!isCurBlockTerminated()) {
        // not conditional, only exit can escape!
        Builder.CreateBr(loopBB);
    }
    
    
    // Emit merge block.
    theFunction->getBasicBlockList().push_back(mergeBB);
    Builder.SetInsertPoint(mergeBB);
}

//! compiles an if statement as a series of blocks
//! works on LOOP_STAT nodes
void CodeGen::compileForStat(ASTNode *node) {
    
    Function *theFunction = currentFunction();
    
    BasicBlock *loopBB = BasicBlock::Create(getGlobalContext(), "for", theFunction);
    BasicBlock *mergeBB = BasicBlock::Create(getGlobalContext(), "forcont");
    
    Scopes->pushLocalScope(currentFunction());
    
    // loop induction variable. node->str is the name
    Symbol *inductionVar = Scopes->curScope()->declareVar(node->str,Types.getType("int"));
    
    std::pair<Value*,Value*> range = compileRange(node->children[0]);
    
    // starting at the first number
    Builder.CreateStore(range.first,inductionVar->getVal());
    
    Builder.CreateBr(loopBB);
    
    Builder.SetInsertPoint(loopBB);
    
    // set the block the "exit" statement should continue to
    BasicBlock *prevExitBlock = ExitBlock;
    ExitBlock = mergeBB;
    
    compileBlock(node->children[1]);
    Scopes->popScope();
    
    // stack like. return to previous one
    ExitBlock = prevExitBlock;
    
    if (!isCurBlockTerminated()) {
        // increment = load -> add 1 -> store
        Value *oneConst = ConstantInt::get(getGlobalContext(), APInt(32,1));
        Value *incremented = Builder.CreateAdd(Builder.CreateLoad(inductionVar->getVal(),"inductvarval"),
                                               oneConst,"incremented");
        Builder.CreateStore(incremented,inductionVar->getVal());
        
        // finished yet?
        Value *done = Builder.CreateICmpUGT(incremented,range.second);
        Builder.CreateCondBr(done,mergeBB,loopBB);
    }
    
    
    // Emit merge block.
    theFunction->getBasicBlockList().push_back(mergeBB);
    Builder.SetInsertPoint(mergeBB);
}