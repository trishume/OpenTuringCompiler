#include "codegen.h"

// for std::pair
#include <utility>

#include <llvm/Instruction.h>
#include <llvm/Constants.h>
#include <llvm/InstrTypes.h>
#include <llvm/Attributes.h>
#include <llvm/Support/DynamicLibrary.h>

#include "language.h"
#include "ast.h"

#include "Message.h"
#include "Executor.h"

#include "Symbol.h"

#define ITERATE_CHILDREN(node,var) \
for(std::vector<ASTNode*>::iterator var = (node)->children.begin(), e = (node)->children.end();var < e;++var)

static const std::string defaultIncludes =
    "external proc TuringQuitWithCode(code : int)\n"
    "external proc TuringPrintInt(val : int)\n"
    "external proc TuringPrintReal(val : real)\n"
    "external proc TuringPrintBool(val : boolean)\n"
    "external proc TuringPrintString(val : string)\n"
    "external proc TuringGetString(val : string)\n"
    "external proc TuringGetInt(var val : int)\n"
    "external \"length\" fcn TuringStringLength(val : string) : int\n"
    "external fcn TuringStringConcat(lhs,rhs : string) : string\n"
    "external fcn TuringStringCompare(lhs,rhs : string) : boolean\n"
    "external proc TuringPrintNewline()\n"
    "external fcn TuringPower(val : int, power : int) : int\n"
    "external \"TuringPowerReal\" fcn pow(val : real, power : real) : real\n" // use the C 'pow' function directly
    "external fcn TuringIndexArray(index : int, length : int) : int\n"
    "external proc TuringCopyArray(to : voidptr, from : voidptr, fromLength : int, toLength : int)\n"
    "external fcn TuringCompareRecord(to : voidptr, from : voidptr, fromLength : int) : boolean\n"
    "external fcn TuringCompareArray(to : voidptr, from : voidptr, fromLength : int, toLength : int) : boolean\n"
    "external fcn TuringAllocateFlexibleArray(arr : voidptr, byteSize,length : int) : voidptr\n"
;

using namespace llvm;

#pragma mark Construction

CodeGen::CodeGen(FileSource *source) :   TheSource(source), CurFile(""), CanExecute(true),
Builder(llvm::getGlobalContext()), RetVal(NULL), RetBlock(NULL) {
    Types.addDefaultTypes(getGlobalContext());
    TheModule = new Module("turing JIT module", getGlobalContext());
    Scopes = new ScopeManager(TheModule);
    
    // Get the module ready to start compiling
    
    /* Create the top level interpreter function to call as entry */
	std::vector<Type*> argTypes;
	FunctionType *mainFuntionType = FunctionType::get(Type::getVoidTy(getGlobalContext()), argTypes, false);
	MainFunction = Function::Create(mainFuntionType, GlobalValue::ExternalLinkage, MAIN_FUNC_NAME, TheModule);
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
    std::string path = TheSource->includeFilePath(fileName, CurFile);
    ASTNode *fileRoot = TheSource->parseFile(path);
    
    if (fileRoot == NULL) {
        Message::error(Twine("Failed to parse file \"") + fileName + "\". Either there was a syntax error or the file does not exist.");
        return false;
    }
    return compileRootNode(fileRoot,path);
}

bool CodeGen::linkLibrary(std::string libName) {
    std::string libPath = TheSource->getLibraryPath(libName, CurFile);
    if (libPath.empty()) {
        Message::error(Twine("Can't find library ") + libName);
        return false;
    }
    std::string errMsg;
    bool fail = llvm::sys::DynamicLibrary::LoadLibraryPermanently (libPath.c_str(), &errMsg);
    if (fail) {
        Message::error(Twine("Failed to load library ") + libName + ": " + errMsg);
        return false;
    }
    return true;
}

bool CodeGen::compileRootNode(ASTNode *fileRoot, std::string fileName) {    
    std::string oldCurFile = CurFile;
    CurFile = std::string(fileName);
    bool good = false;
    try {
        good = compileBlock(fileRoot);
    } catch (Message::Exception e) {
        good = false;
        if(!e.Message.empty()) Message::error(e.Message);
        
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
    jit.run(false); // true = time run
    
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

bool CodeGen::isFlexibleArray(TuringType *type) {
    if (!type->isArrayTy()) {
        return false;
    }
    
    return static_cast<TuringArrayType*>(type)->isFlexible();
}

bool CodeGen::isMainFunction(Function *f) {
    return f == MainFunction;
}

bool CodeGen::isCurBlockTerminated() {
    return Builder.GetInsertBlock()->getTerminator() != NULL;
}

TuringValue *CodeGen::getConstantInt(int index) {
    return new TuringValue(ConstantInt::get(getGlobalContext(), APInt(32,index)),
                           Types.getType("int"));
}

//! allocates a local variable but does not put it in the symbol table
Symbol *CodeGen::createEntryAlloca(TuringType *type) {
    BasicBlock *entryBlock = &currentFunction()->getEntryBlock();
    IRBuilder<> TmpB(entryBlock,entryBlock->begin());
    return new VarSymbol(TmpB.CreateAlloca(type->getLLVMType(false),0,"internalVar"),type);
}

//! gets the size in bytes of a type. Used for memcmp and memcpy operations
//! \param type the pointer type of an allocated buffer of that type. 
//!             I.E {i32,i32}* for a record of two ints. Or i32* for an int.
Value *CodeGen::compileByteSize(TuringType *turType) {
    Type *type = turType->getLLVMType(true);
    if (!type->isPointerTy()) {
        throw Message::Exception("Can only find the size of pointer types. This shouldn't happen and is probably a bug in the compiler.");
    }
    // getting index 1 of a pointer will treat it as an array and thus get the second element
    // for a null pointer this will be the size in bytes of the type.
    Value *sizePtr = Builder.CreateConstGEP1_32(ConstantPointerNull::get(cast<PointerType>(type)), 1);
    return Builder.CreatePointerCast(sizePtr,Types.getType("int")->getLLVMType(),"typelengthint");
}

//! Arrays are a special case for getting the byte size because the type {i32,[0 x type]}
//! is larger than LLVM thinks it is. use the length value to get the actual size.
Value *CodeGen::compileArrayByteSize(Value *arrayRef) {
    return compileArrayByteSize(arrayRef->getType(),compileArrayLength(arrayRef));
}

Value *CodeGen::compileArrayByteSize(Type *arrayType,Value *arrayLength) {
    std::vector<Value*> indices;
    // depointerize
    indices.push_back(ConstantInt::get(getGlobalContext(), APInt(32,0)));
    // get the array part of the turing array struct
    indices.push_back(ConstantInt::get(getGlobalContext(), APInt(32,1)));
    // index it with the length
    indices.push_back(arrayLength);
    
    // double checking, this should never happen
    if (!arrayType->isPointerTy()) {
        throw Message::Exception("Can't find the length of a non-referenced array.");
    }
    
    Value *sizePtr = Builder.CreateGEP(ConstantPointerNull::get(cast<PointerType>(arrayType)),indices,"arraysizeptr");
    return Builder.CreatePointerCast(sizePtr,Types.getType("int")->getLLVMType(),"arrlengthint");
}

Value *CodeGen::compileArrayLength(Value *arrayRef) {
    if (!Types.isArrayRef(arrayRef->getType())) {
        throw Message::Exception("Can only find the upper limit of an array.");
    }
    return Builder.CreateLoad(Builder.CreateConstGEP2_32(arrayRef,0,0,"arrlengthptr"),"arraylengthval");
}

std::pair<TuringValue*,TuringValue*> CodeGen::compileRange(ASTNode *node) {
    TuringValue *start = compile(node->children[0]);
    
    if (node->children[1]->root == Language::RANGE_SPECIAL_END) {
        throw Message::Exception(Twine("Special range end ") + node->children[1]->str + 
                                 " is not supported here.");
    }
    
    TuringValue *end = compile(node->children[1]);
    
    if (! Types.isType(start,"int") || !Types.isType(end,"int") ) {
        throw Message::Exception("The start and end of a range must be 'int's.");
    }
    
    return std::pair<TuringValue*,TuringValue*>(start,end);
}

//! allocates or resizes a flexible array
//! \param arr the flexible array to be resized. Not dereferenced.
//! \param allocateNew are we resizing or allocating?
//! \param if we are resizing then to what size. Can be NULL to use the starting size specified in the array type.
void CodeGen::compileAllocateFlexibleArray(Symbol *arr, bool allocateNew, Value *newSize) {
    // caller should check this but just to be safe...
    if (!isFlexibleArray(arr->getType())) {
        throw Message::Exception("Can't resize something that is not a flexible array.");
    }
    TuringArrayType *arrtype = static_cast<TuringArrayType*>(arr->getType());
    
    Value *prevArr;
    if(allocateNew) {
        prevArr = ConstantPointerNull::get(cast<PointerType>(Types.getType("voidptr")->getLLVMType(true)));
    } else {
        // the symbol passed is a pointer to a buffer, load the symbol and then cast it to a voidptr
        Value *arrBuffer = Builder.CreateLoad(arr->getVal(), "flexibleArrBuffer");
        prevArr = Builder.CreatePointerCast(arrBuffer, Types.getType("voidptr")->getLLVMType(true));
    }
    
    // if newSize is not null, use it
    Value *arrLength = newSize ? newSize : getConstantInt(arrtype->getSize())->getVal();
    Value *byteSize = compileArrayByteSize(arrtype->getLLVMType(true), arrLength);
    Value *buffer = Builder.CreateCall3(TheModule->getFunction("TuringAllocateFlexibleArray"),
                                        prevArr,byteSize,arrLength);
    Value *castedBuffer = Builder.CreatePointerCast(buffer, arrtype->getLLVMType(true));
    Builder.CreateStore(castedBuffer,arr->getVal()); // store the pointer to the buffer
}

//! uses a runtime loop to initialize all the elements in an array.
//! \param from the integer starting index to initialize
//! \param to the integer ending index to initialize
void CodeGen::compileInitializeArrayElements(Symbol *arr, Value *from, Value *to) {    
    Function *theFunction = currentFunction();    
    BasicBlock *loopBB = BasicBlock::Create(getGlobalContext(), "initializeArray", theFunction);
    BasicBlock *mergeBB = BasicBlock::Create(getGlobalContext(), "initializeArrayCont");
    
    // loop induction variable. node->str is the name
    Symbol *inductionVar = createEntryAlloca(Types.getType("int"));
    
    // starting at the first number
    Builder.CreateStore(from,inductionVar->getVal());
    
    // if the first value is greater than the second then skip it
    // I.E for i : 6..5
    Value *overEnd = Builder.CreateICmpUGT(from,to);
    Builder.CreateCondBr(overEnd,mergeBB,loopBB);    
    Builder.SetInsertPoint(loopBB);
    
    // ACTUAL INITIALIZATION!
    Symbol *indexed = abstractCompileIndex(arr, abstractCompileVarReference(inductionVar));
    compileInitializeComplex(indexed);
    
    if (!isCurBlockTerminated()) {
        // increment = load -> add 1 -> store
        Value *incremented = Builder.CreateAdd(Builder.CreateLoad(inductionVar->getVal(),"inductvarval"),
                                                 getConstantInt(1)->getVal(),"incremented");
        Builder.CreateStore(incremented,inductionVar->getVal());        
        // finished yet?
        Value *done = Builder.CreateICmpUGT(incremented,to);
        Builder.CreateCondBr(done,mergeBB,loopBB);
    }
    // Emit merge block.
    theFunction->getBasicBlockList().push_back(mergeBB);
    Builder.SetInsertPoint(mergeBB);
}

//! properly initializes complex data structures. It's main duty is initializing the length of arrays.
//! \param declared A pointer to a newly allocated buffer to be initialized
void CodeGen::compileInitializeComplex(Symbol *declared) {
    // might need to initialize record fields
    if (declared->getType()->isRecordTy()) {
        TuringRecordType *recType = static_cast<TuringRecordType*>(declared->getType());
        // initialize fields if they are complex
        for (unsigned int i = 0; i < recType->getSize(); ++i) {
            VarDecl field = recType->getDecl(i);
            if (field.Type->isComplexTy()) {
                // get the field to initialize
                Symbol *fieldSym = compileRecordFieldRef(declared, field.Name);
                // initialize it
                compileInitializeComplex(fieldSym);
            }            
        }
        return;
    }
    
    // only arrays from here on
    if (!declared->getType()->isArrayTy()) {
        return;
    }
    
    TuringArrayType *arrtype = static_cast<TuringArrayType*>(declared->getType());
    
    //! flexible arrays are heap allocated with a special function
    if (arrtype->isFlexible()) {
        compileAllocateFlexibleArray(declared, true); // true = allocate new
        if (arrtype->getElementType()->isComplexTy()) {
            Symbol *derefed = new VarSymbol(Builder.CreateLoad(declared->getVal(), "derefedArr"),
                                            declared->getType());
            compileInitializeArrayElements(derefed, getConstantInt(1)->getVal(), 
                                           getConstantInt(arrtype->getSize())->getVal());
            delete derefed;
        }
        return;
    }
    
    // must initialize the length part of the array struct   
    Value *arrLengthPtr = Builder.CreateConstGEP2_32(declared->getVal(),0,0,"arrlengthptr");
    Value *length = ConstantInt::get(getGlobalContext(),APInt(32,arrtype->getSize()));
    Builder.CreateStore(length,arrLengthPtr);
    
    // initialize the subelements
    // TODO make the loop at runtime not compile time. This is inefficient and hacky.
    if (arrtype->getElementType()->isComplexTy()) {
        compileInitializeArrayElements(declared, getConstantInt(1)->getVal(), 
                                       getConstantInt(arrtype->getSize())->getVal());
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
    bool isFlexible = (node->str.compare("flexible") == 0);
    // the node can contain multiple ranges. These denote multi-dimensional arrays.
    // Since these are just arrays in arrays. We keep wrapping the previous type
    // in an array until there are no more ranges. Starting from the end so that
    // 1..5, 1..4 is [5 x [4 x type]] instead of vice-versa.
    for (int i = node->children.size() - 1; i > 0; --i) {
        ASTNode *range = node->children[i];
        
        TuringValue *upperVal;
        bool isAnySize = false;
        // arrays support special endpoints like 1..* or 1..char
        if (range->children[1]->root == Language::RANGE_SPECIAL_END) {
            ASTNode *specialEnd = range->children[1];
            if (specialEnd->str.compare("*") == 0) {
                upperVal = getConstantInt(0);
                isAnySize = true;
            } else if (specialEnd->str.compare("char") == 0) {
                upperVal = getConstantInt(255); // char array size
            } else {
                throw Message::Exception(Twine("Special array size value ") + specialEnd->str + " not recognized."); // parser prevents this
            }
        } else {
            upperVal = compile(range->children[1]);
        }
        
        Value *upperConst = upperVal->getVal();
        
        // this is fancy. It checks if the upper bound is a loaded const variable.
        // if it is, it sets upperVal to the Constant initializer instead of the load instruction
        if(isa<LoadInst>(upperConst) && 
           isa<GlobalVariable>(cast<LoadInst>(upperConst)->getPointerOperand())) {
            GlobalVariable *globalVar = cast<GlobalVariable>(cast<LoadInst>(upperConst)->getPointerOperand());
            if (globalVar->isConstant()) {
                upperConst = globalVar->getInitializer();
            }
        }
        
        // we don't want someone putting "array bob..upper(bob) of int" because
        // we have to know the size at compile time.
        if (!isa<ConstantInt>(upperConst)) {
            throw Message::Exception("Bounds of array must be int constants");
        }
        // *ConstantInt -> APInt -> uint_64
        int upper = cast<ConstantInt>(upperConst)->getValue().getLimitedValue();
        
        // 800,000,000 should be enough array elements for Turing's target audience
        // the upper limit is just so people don't declare enormous arrays just to crash the computer
        if (upper<0 || upper > 800000000) {
            throw Message::Exception("Array limit out of bounds.");
        }
        // wrap it up
        arrayType = new TuringArrayType(arrayType,upper,isAnySize,isFlexible);
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
        
        std::string name = (*decl)->str;
        bool isVarRef = false;
        if (name.substr(0,4).compare("var ") == 0) {
            isVarRef = true;
            name = name.substr(4);
        }
        
        VarDecl vardecl(name,type);
        if (isVarRef) {
            vardecl.IsVarRef = true;
        }
        decls.push_back(vardecl);
    }
    
    return decls;
}
//! \param inStr string context of the check. I.E "first parameter of AFunction" or "Left side of operator"
TuringValue *CodeGen::promoteType(TuringValue *val, TuringType *destType, Twine inStr) {
    TuringType *type = val->getType();
    Type *llvmDestType = destType->getLLVMType(true);
    
    // if they are the same, no casting needed
    if (type->compare(destType)) {
        return val;
    }
    
    // any pointer can be cast to voidptr
    if (destType->getName().compare("voidptr") == 0 && val->getType()->getLLVMType(true)->isPointerTy()) {
        Value *casted = Builder.CreatePointerCast(val->getVal(), destType->getLLVMType());
        return new TuringValue(casted,Types.getType("voidptr"));
    }
    
    // any array can be converted to an array 1..*
    if (val->getType()->isArrayTy() && destType->isArrayTy() && 
        static_cast<TuringArrayType*>(destType)->isAnySize()) {
        return new TuringValue(val->getVal(),destType);
    }
    
    if (Types.isType(val,"int") && llvmDestType->isFloatingPointTy()) {
        return new TuringValue(Builder.CreateSIToFP(val->getVal(), llvmDestType, "promotedint"),
                               Types.getType("real"));
    }
    
    // if it gets this far, it's an error
    Twine message = Twine("Can't convert expression of type \"") + 
    type->getName() + "\" to type \"" + destType->getName() + "\"";
    if (!inStr.str().empty()) {
        message.concat(Twine(" in ") + inStr);
    }
    throw Message::Exception(message + ".");
    
    return NULL; // never reaches here
    
}

#pragma mark Compilation

//! Compiles a block of instructions
//! Returns success
bool CodeGen::compileBlock(ASTNode *node) {
	if(node->root == Language::BLOCK) {
		ITERATE_CHILDREN(node,child) {
			if(!compileStat(*child)) {
				throw Message::Exception("");
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
        case Language::LIBRARY_DECL:           
            return linkLibrary(node->str);
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
        case Language::CONST_DECL:
            compileConstDecl(node);
            return true;
        case Language::TYPE_DECL:
            Types.aliasType(getType(node->children[0]), node->str);
            return true;
        case Language::IF_STAT:
            compileIfStat(node);
            return true; // throws error on fail
        case Language::CASE_STAT:
            compileCaseStat(node);
            return true;
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
        case Language::RESIZE_STAT:
            compileResizeStat(node);
            return true;
        case Language::QUIT_STAT:
        {
            TuringValue *errCode = promoteType(compile(node->children[0])
                                               , Types.getType("int"));
            Builder.CreateCall(TheModule->getFunction("TuringQuitWithCode"), errCode->getVal());
            return true;
        }
        case Language::INCLUDE_STAT:
            return compileFile(node->str);
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
            TuringValue *val = compile(node->children[0]);
            abstractCompileAssign(val,RetVal);
            compileReturn();
            return true;
        }
        default:
            return compile(node) != NULL; // treat as an expression
    }
	
	return false; // should never reach here
}

//! dispatcher. Checks the type of the node and calls the correct compile function.
TuringValue *CodeGen::compile(ASTNode *node) {
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
        case Language::UNARY_OP:
            return compileUnaryOp(node);
        case Language::CALL:
            return compileCallSyntax(node);
        case Language::ARRAY_UPPER:
            return new TuringValue(compileArrayLength(compile(node->children[0])->getVal()),
                                   Types.getType("int"));
        case Language::VAR_REFERENCE:
        case Language::FIELD_REF_OP:
            // compileLHS knows how to handle these. We just have to load them.
            return abstractCompileVarReference(compileLHS(node),node->str);
        case Language::STRING_LITERAL:
            return compileStringLiteral(node->str);
        case Language::INT_LITERAL:
            // apint can convert a string
            return new TuringValue(ConstantInt::get(getGlobalContext(), APInt(32,node->str,10)),
                                   Types.getType("int"));
        case Language::REAL_LITERAL:
            // apint can convert a string
            return new TuringValue(ConstantFP::get(Types.getType("real")->getLLVMType(), node->str),
                                   Types.getType("real"));
        case Language::BOOL_LITERAL:
            // apint is used because booleans are one bit ints
            return new TuringValue(ConstantInt::get(getGlobalContext(), 
                                                    APInt(1,(node->str.compare("true") == 0) ? 1 : 0)),
                                   Types.getType("boolean"));
        default:
            throw Message::Exception(Twine("AST type ") + Language::getTokName(node->root) + " not recognized");
    }
}
//! creates a constant struct to represent a string literal
//! \returns an array reference to the string literal
TuringValue *CodeGen::compileStringLiteral(const std::string &str) {
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
    
    return new TuringValue(Builder.CreatePointerCast(gvar,stringRefType,"castedstringconstant"),
                           Types.getType("string"));
}

// Compiles binary operators.
TuringValue *CodeGen::compileBinaryOp(ASTNode *node) {
    if (node->str.compare("and") == 0 || node->str.compare("or") == 0 || 
        node->str.compare("&") == 0 || node->str.compare("|") == 0) {
        return compileLogicOp(node);
    } else {
        TuringValue *L = compile(node->children[0]);
        TuringValue *R = compile(node->children[1]);
        return abstractCompileBinaryOp(L,R,node->str);
    }
}

TuringValue *CodeGen::abstractCompileBinaryOp(TuringValue *L, TuringValue *R, std::string op) {
    if (L == 0 || R == 0) {
        throw Message::Exception("Invalid operand for binary operator.");
    }
    
    bool fp = false;
    // if one side of the operator is floating point, it all is. 
    // Division is always floating point.
    if (Types.isType(L,"real") || Types.isType(R,"real") || op.compare("/") == 0) {
        fp = true;
        L = promoteType(L, Types.getType("real"), Twine("left side of ") + op + " operator");
        R = promoteType(R, Types.getType("real"), Twine("right side of ") + op + " operator");
    }
    TuringType *retTy = fp ? Types.getType("real") : Types.getType("int");
    
    Instruction::BinaryOps binOp;
	
    // COMPARISONS
	if (op.compare(">") == 0) {
        Value *result = fp ? Builder.CreateFCmpOGT(L->getVal(), R->getVal()) : Builder.CreateICmpSGT(L->getVal(), R->getVal());
        return new TuringValue(result,Types.getType("boolean"));
    } else if (op.compare("<") == 0) {
        Value *result = fp ? Builder.CreateFCmpOLT(L->getVal(), R->getVal()) : Builder.CreateICmpSLT(L->getVal(), R->getVal());
        return new TuringValue(result,Types.getType("boolean"));
    } else if (op.compare(">=") == 0) {
        Value *result = fp ? Builder.CreateFCmpOGE(L->getVal(), R->getVal()) : Builder.CreateICmpSGE(L->getVal(), R->getVal());
        return new TuringValue(result,Types.getType("boolean"));
    } else if (op.compare("<=") == 0) {
        Value *result = fp ? Builder.CreateFCmpOLE(L->getVal(), R->getVal()) : Builder.CreateICmpSLE(L->getVal(), R->getVal());
        return new TuringValue(result,Types.getType("boolean"));
    } else if (op.compare("+") == 0) { // MATH
        // string + string = TuringStringConcat(string)
        if (Types.isType(L, "string") && Types.isType(R, "string")) {
            Symbol *callee = Scopes->curScope()->resolve("TuringStringConcat");
            std::vector<TuringValue*> params;
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
        binOp = Instruction::FDiv;
    } else if (op.compare("div") == 0) {
        if (fp) { // floating point 'div' just truncates the division
            Value *unTruncated = Builder.CreateFDiv(L->getVal(), R->getVal());
            Value *truncated = Builder.CreateFPTrunc(unTruncated, Types.getType("int")->getLLVMType());
            return new TuringValue(truncated,Types.getType("int"));
        }
        binOp = Instruction::SDiv;
    } else if (op.compare("xor") == 0) {
        if (fp) { // TODO resolve this
            throw Message::Exception("Can't use 'xor' on real numbers.");
        }
        binOp = Instruction::Xor;
    } else if (op.compare("mod") == 0) {
        binOp = fp ? Instruction::FRem : Instruction::SRem;
    } else if (op.compare("**") == 0) {
        std::vector<Value*> argVals;
        argVals.push_back(L->getVal());
        argVals.push_back(R->getVal());
        
        Function *func;
        if (fp) {
            func = TheModule->getFunction("pow");
        } else {
            func = TheModule->getFunction("TuringPower");
        }
        
        Value *resVal = Builder.CreateCall(func,argVals,"powertmp");
        return new TuringValue(resVal,retTy);
    } else {
        throw Message::Exception("Invalid binary operator.");
    }
    
    // if it hasn't already been promoted (which type checks), do type checking
    if (!fp) {
        L = promoteType(L, Types.getType("int"), Twine("left side of ") + op + " operator");
        R = promoteType(R, Types.getType("int"), Twine("right side of ") + op + " operator");
    }
    
    // if it hasn't returned by now it must be a normal binop
    Value *resVal = BinaryOperator::Create(binOp, L->getVal(), R->getVal(),"binOpRes",Builder.GetInsertBlock());
    return new TuringValue(resVal,retTy);
}

TuringValue *CodeGen::compileUnaryOp(ASTNode *node) {
    TuringValue *val = compile(node->children[0]);
    std::string op = node->str;
    
    if (op.compare("not") == 0 || op.compare("~") == 0) {
        val = promoteType(val, Types.getType("boolean"),"boolean not operator");
        // XOR with one is boolean not
        Value *oneConst = ConstantInt::get(getGlobalContext(),APInt(1,1));
        return new TuringValue(Builder.CreateXor(oneConst,val->getVal()),Types.getType("boolean"));
    } else if(op.compare("-") == 0) {
        if (Types.isType(val, "real")) {
            Value *zeroConst = ConstantFP::get(Types.getType("real")->getLLVMType(),0.0);
            return new TuringValue(Builder.CreateFSub(zeroConst,val->getVal()),Types.getType("real"));
        }
        
        val = promoteType(val, Types.getType("int"),"negation operator");
        return new TuringValue(Builder.CreateSub(getConstantInt(0)->getVal(),val->getVal()),Types.getType("int"));
    } else {
        throw Message::Exception("Invalid unary operator '" + op + "'.");
    }
}

//! compiles a properly short-circuiting logic operator
//! \param isAnd false for 'or' true for 'and'
TuringValue *CodeGen::compileLogicOp(ASTNode *node) {
    TuringValue *cond1 = compile(node->children[0]);
    
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
    if (node->str.compare("and") == 0 || node->str.compare("&") == 0) {
        Builder.CreateCondBr(cond1->getVal(), secondBB, mergeBB);
    } else { // or
        Builder.CreateCondBr(cond1->getVal(), mergeBB, secondBB);
    }
    
    
    
    // Emit second condition
    Builder.SetInsertPoint(secondBB);
    
    TuringValue *cond2 = compile(node->children[1]);
    
    if (!Types.isType(cond2,"boolean")) {
        throw Message::Exception(Twine("Arguments of logical ") + node->str + " must be of type boolean");
    }
    
    Builder.CreateBr(mergeBB);
    
    
    // Emit merge block.
    theFunction->getBasicBlockList().push_back(mergeBB);
    Builder.SetInsertPoint(mergeBB);
    
    PHINode *resPhi = Builder.CreatePHI(Types.getType("boolean")->getLLVMType(),2,"andresult");
    resPhi->addIncoming(cond1->getVal(),startBlock);
    resPhi->addIncoming(cond2->getVal(),secondBB);
    
    return new TuringValue(resPhi,Types.getType("boolean"));
}

TuringValue *CodeGen::compileEqualityOp(ASTNode *node) {
    TuringValue *L = compile(node->children[0]);
    TuringValue *R = compile(node->children[1]);
    return abstractCompileEqualityOp(L,R,node->str.compare("=") != 0);
}

TuringValue *CodeGen::abstractCompileEqualityOp(TuringValue *L,TuringValue *R,bool isNotEquals) {    
    bool fp = false;
    if (L->getVal()->getType()->isFloatingPointTy() || R->getVal()->getType()->isFloatingPointTy()) {
        fp = true;
        L = promoteType(L, Types.getType("real"),"left side of = operator");
        R = promoteType(R, Types.getType("real"),"right side of = operator");
    } else if (!L->getType()->compare(R->getType())) {
        throw Message::Exception(Twine("Can't compare an expression of type \"") + L->getType()->getName() +
                                 "\" to one of type \"" + R->getType()->getName() + "\".");
    }
    
    TuringType *type = L->getType();
    
    Value *ret = NULL;
    
    if (type->getName().compare("int") == 0 || type->getName().compare("boolean") == 0) {
        ret = Builder.CreateICmpEQ(L->getVal(),R->getVal(),"equal");        
    } else if (fp) {
        ret = Builder.CreateFCmpOEQ(L->getVal(), R->getVal(),"fpequal");
    } else if (type->getName().compare("string") == 0 ) { // strings
        ret = Builder.CreateCall2(TheModule->getFunction("TuringStringCompare"),
                                  L->getVal(),R->getVal());
    } else if (type->isArrayTy()) { // arrays
        Value *srcSize = compileArrayByteSize(L->getVal());
        Value *destSize = compileArrayByteSize(R->getVal());
        Value *fromPtr = Builder.CreatePointerCast(L->getVal(),Types.getType("voidptr")->getLLVMType(),"fromptr");
        Value *toPtr = Builder.CreatePointerCast(R->getVal(),Types.getType("voidptr")->getLLVMType(),"toptr");
        ret = Builder.CreateCall4(TheModule->getFunction("TuringCompareArray"),fromPtr,toPtr,srcSize,destSize);
    } else if (type->isRecordTy()) { // records
        if (!L->getType()->compare(R->getType())) {
            throw Message::Exception("Can't compare records of different types.");
        }
        Value *srcSize = compileByteSize(L->getType());
        Value *fromPtr = Builder.CreatePointerCast(L->getVal(),Types.getType("voidptr")->getLLVMType(),"fromptr");
        Value *toPtr = Builder.CreatePointerCast(R->getVal(),Types.getType("voidptr")->getLLVMType(),"toptr");
        ret = Builder.CreateCall3(TheModule->getFunction("TuringCompareRecord"),fromPtr,toPtr,srcSize);
    } else {
        throw Message::Exception(Twine("Can't compare type ") + type->getName());
    }

    // if it is not =, it is ~= so invert it
    if (isNotEquals) {
        // bool == 0 is the same as not bool
        ret = Builder.CreateICmpEQ(ret,ConstantInt::get(getGlobalContext(), APInt(1,0)));
    }
    
    return new TuringValue(ret,Types.getType("boolean"));
}

//! \param autoDeref    Wether to dereference pointers to flexible array buffers.
//!                     Normally true except when buffer location needs to be changed.
Symbol *CodeGen::compileLHS(ASTNode *node, bool autoDeref) {
    Symbol *result;
    switch (node->root) {
        case Language::VAR_REFERENCE:
            result = Scopes->curScope()->resolve(node->str);
            break;
        case Language::FIELD_REF_OP:
        {
            ASTNode *lhs = node->children[0];
            // is it a module? If so, retrieve its scope and resolve the variable without searching up.
            if (lhs->root == Language::VAR_REFERENCE &&
                Scopes->namedScopeExists(lhs->str)) {
                result = Scopes->getNamedScope(lhs->str)->resolveVarThis(node->str);
                break;
            }
            
            // is it a record?
            Symbol *rec = compileLHS(lhs,autoDeref);
            if (rec->getType()->isRecordTy()) {
                result = compileRecordFieldRef(rec, node->str);
                break;
            }
            
            // TODO other reference types
            throw Message::Exception(Twine("Can't reference element ") + node->str);
        }
        case Language::CALL:
        {
            // must be an array reference
            Symbol *callee = compileLHS(node->children[0],autoDeref);
            if (!callee->getType() || !callee->getType()->isArrayTy()) {
                throw Message::Exception("Can't index something that isn't an array.");
            }
            
            result = compileIndex(callee,node);
            break;
        }
        default:
            throw Message::Exception(Twine("LHS AST type ") + Language::getTokName(node->root) + 
                                     " not recognized. You might be using a feature that hasn't been implemented yet");
    }
    
    // dereference flexible arrays so they are the same type as normal array references
    if (isFlexibleArray(result->getType()) && autoDeref) {
        result = new VarSymbol(Builder.CreateLoad(result->getVal(), "derefedFlexibleArray"),
                               result->getType());
    }
    
    return result;
}

TuringValue *CodeGen::compileAssignOp(ASTNode *node) {
    TuringValue *val = compile(node->children[1]);
    
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

void CodeGen::abstractCompileAssign(TuringValue *val, Symbol *assignSym) {
    Value *assignVar = assignSym->getVal();
    
    // can't assign to arguments unless they are actual structure returns
    if (isa<Argument>(assignVar) && !(cast<Argument>(assignVar)->hasStructRetAttr()) &&
        !(assignVar->getType()->isPointerTy())) {
        
        throw Message::Exception("Can't assign to an argument.");
    }
    
    // store needs a pointer
    if (!(assignVar->getType()->isPointerTy())) {
        throw Message::Exception("Can't assign to something that is not a variable");
    }
    
    // assigning an array to an array copies it
    if (assignSym->getType()->isArrayTy() && val->getType()->isArrayTy()) {
        compileArrayCopy(val,assignSym);
        return;
    } else if (assignSym->getType()->isRecordTy() && val->getType()->isRecordTy()) {
        compileRecordCopy(val, assignSym);
        return;
    }
    
    if (assignSym->isFunction()) {
        throw Message::Exception("Can't assign to a function.");
    }
    
    val = promoteType(val, assignSym->getType(),"assignment or result");
    
    Builder.CreateStore(val->getVal(),assignVar);
}

void CodeGen::compileArrayCopy(TuringValue *from, Symbol *to) {
    Value *srcSize = compileArrayByteSize(from->getVal());
    Value *destSize = compileArrayByteSize(to->getVal());
    Value *fromPtr = Builder.CreatePointerCast(from->getVal(),Types.getType("voidptr")->getLLVMType(),"fromptr");
    Value *toPtr = Builder.CreatePointerCast(to->getVal(),Types.getType("voidptr")->getLLVMType(),"toptr");
    Builder.CreateCall4(TheModule->getFunction("TuringCopyArray"),fromPtr,toPtr,srcSize,destSize);
}

void CodeGen::compileRecordCopy(TuringValue *from, Symbol *to) {
    assert(from->getType()->isRecordTy());
    if (!to->getType()->compare(from->getType())) {
        throw Message::Exception(Twine("Can't assign a record of type \"") + from->getType()->getName() +
                                       "\" to one of type \"" + to->getType()->getName() + "\".");
    }
    Value *fromSize = compileByteSize(from->getType());
    // llvm intrinsic memcpy. Auto-converts and optimizes well.
    Builder.CreateMemCpy(to->getVal(), from->getVal(), fromSize, 0, true);
}

void CodeGen::compilePutStat(ASTNode *node) {
    // print out all the comma separated expressions
    ITERATE_CHILDREN(node, curNode) {
        TuringValue *val = compile(*curNode);
        TuringType *type = val->getType();
        
        Function *calleeFunc;
        std::vector<Value*> argVals;
        argVals.push_back(val->getVal());
        
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
    Value *val = var->getVal();
    
    TuringType *type = var->getType();
    
    Function *calleeFunc;
    if (type->compare(Types.getType("string"))) {
        calleeFunc = TheModule->getFunction("TuringGetString");
        val = Builder.CreatePointerCast(val, var->getType()->getLLVMType(true));
    } else if (type->compare(Types.getType("int"))) {
        calleeFunc = TheModule->getFunction("TuringGetInt");
    } else {
        throw Message::Exception("Can't 'get' type \"" + type->getName() + "\".");
    }
    
    Builder.CreateCall(calleeFunc, val);
}

void CodeGen::compileResizeStat(ASTNode *node) {
    Symbol *sym = compileLHS(node->children[0],false); // false = don't dereference the array
    if (!isFlexibleArray(sym->getType())) {
        throw Message::Exception("Can only allocate new elements for flexible arrays.");
    }
    
    TuringValue *newSize = compile(node->children[1]);
    newSize = promoteType(newSize, Types.getType("int"));
    
    bool complexElements = static_cast<TuringArrayType*>(sym->getType())->getElementType()->isComplexTy();
    
    // if we are initializing new elements we start at one mor than the original size of the array
    Value *initStart;
    if (complexElements) {
        Value *derefed = Builder.CreateLoad(sym->getVal(), "derefedFlexibleArr");
        initStart = Builder.CreateAdd(compileArrayLength(derefed),getConstantInt(1)->getVal());
    }
    
    // resize it
    compileAllocateFlexibleArray(sym, false, newSize->getVal());
    
    // initialize the new elements if they are complex
    if (complexElements) {
        // have to reload because the resize changes the buffer location
        Value *derefed = Builder.CreateLoad(sym->getVal(), "derefedFlexibleArr");
        Symbol *derefedSym = new VarSymbol(derefed,sym->getType());
        compileInitializeArrayElements(derefedSym, initStart, newSize->getVal());
        delete derefedSym;
    }    
}

void CodeGen::compileVarDecl(ASTNode *node) {
    std::vector<VarDecl> args = getDecls(node->children[0]);
    
    bool hasInitial = node->children.size() > 1;
    TuringValue *initializer = NULL;
    if (hasInitial) {
        initializer = compile(node->children[1]);
    }
    
    for (unsigned int i = 0; i < args.size();++i) {
        TuringType *type = args[i].Type;
        
        if (type->getName().compare("auto") == 0) {
            if (!hasInitial) {
                throw Message::Exception("Can't infer the type of a declaration with no initial value.");
            }
            type = initializer->getType();
        }
        Symbol *declared = Scopes->curScope()->declareVar(args[i].Name,type);
        
        // must initialize the length part of the array struct
        if (type->isComplexTy()) {
            compileInitializeComplex(declared);
        }
        
        if (hasInitial) {
            abstractCompileAssign(initializer, declared);
        }
    }
}

void CodeGen::compileConstDecl(ASTNode *node) {
    assert(node->children[0]->root == Language::DECLARATION);
    
    TuringValue *initializer = compile(node->children[1]);
    
    TuringType *type = getType(node->children[0]->children[0]);
    if (type->getName().compare("auto") == 0) {
        type = initializer->getType();
    }
    
    initializer = promoteType(initializer, type);
    
    if(!isa<Constant>(initializer->getVal())) {
        throw Message::Exception("Constant declaration does not have a constant initializer.");
    }
    
    Value *gvar = new GlobalVariable(/*Module=*/*TheModule,
                                     /*Type=*/type->getLLVMType(false),
                                     /*isConstant=*/true,
                                     /*Linkage=*/GlobalValue::InternalLinkage,
                                     /*Initializer=*/cast<Constant>(initializer->getVal()),
                                     "constDecl");
    
    Scopes->curScope()->setVar(node->children[0]->str, new VarSymbol(gvar,type));
}

TuringValue *CodeGen::abstractCompileVarReference(Symbol *var,const std::string &name) {
    // TODO maybe do as clang does and alloca and assign all params instead of this
    if (!(var->getVal()->getType()->isPointerTy())) {
        // if it is not a reference return it straight up. Used for arguments.
        return new TuringValue(var->getVal(),var->getType());
    }
    // complex types are referenced
    if (var->getType()->isComplexTy()) {
        return new TuringValue(Builder.CreateBitCast(var->getVal(),var->getType()->getLLVMType(true),"complexref"), var->getType());
    }
    
    // TODO implement function references
    if (var->isFunction()) {
        throw Message::Exception("Function references are not implemented yet");
    }
    
    return new TuringValue(Builder.CreateLoad(var->getVal(),
                                              name.empty() ? (Twine(name) + "val") : "loadedVal"),
                           var->getType());
}

//! \param node  a CALL node
//! \returns a pointer to the element
Symbol *CodeGen::compileIndex(Symbol *indexed,ASTNode *node) {
    if (node->children.size() < 2) {
        throw Message::Exception("Must have at least one array index in brackets.");
    }
    
    // support multiple indices like mat(2,3) indexing multi-dimensional arrays
    // iteratively wrap the symbol. curIndexed starts as the main array and then becomes the subarray
    // and then finally, the element.
    Symbol *curIndexed = indexed;
    for (unsigned int i = 1; i < node->children.size(); ++i) {
        curIndexed = abstractCompileIndex(curIndexed,compile(node->children[i]));
    }
    
    return curIndexed;
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


Symbol *CodeGen::abstractCompileIndex(Symbol *indexed,TuringValue *index) {
    if(!indexed->getType()->isArrayTy()) {
        throw Message::Exception(Twine("Can't index non-array type ") + indexed->getType()->getName() + "\".");
    }
    promoteType(index,Types.getType("int")); // type check
    
    std::vector<Value*> indices;
    // depointerize
    indices.push_back(ConstantInt::get(getGlobalContext(), APInt(32,0)));
    // get the array part of the turing array struct
    indices.push_back(ConstantInt::get(getGlobalContext(), APInt(32,1)));
    // take the 1-based index, bounds-check it and return the 0-based index
    Value *realIndex = Builder.CreateCall2(TheModule->getFunction("TuringIndexArray"),index->getVal(),
                                            compileArrayLength(indexed->getVal()),"realIndexVal");
    indices.push_back(realIndex);
    
    TuringArrayType *arrTy = static_cast<TuringArrayType*>(indexed->getType());
    
    return new VarSymbol(Builder.CreateInBoundsGEP(indexed->getVal(),indices,"indextemp"),
                           arrTy->getElementType());
}

//! in turing, the call syntax is used for array indexes, calls and other things
//! this function dispatches the call to the correct compile function
TuringValue *CodeGen::compileCallSyntax(ASTNode *node) {
    Symbol *callee = compileLHS(node->children[0]);
    if (callee->isFunction()) {
        return compileCall(callee,node,true);
    }
    
    if (callee->getType() == NULL) {
        // FIXME EEEEVVVVIIIIILLLL!!!!
        goto fail;
    }
    
    if (callee->getType()->isArrayTy()) {
        return abstractCompileVarReference(compileIndex(callee,node));
    }
    
fail:        
    throw Message::Exception("Only functions and procedures can be called.");
    return NULL; // never reaches here
}
TuringValue *CodeGen::compileCall(ASTNode *node, bool wantReturn) {
    return compileCall(compileLHS(node->children[0]),node,wantReturn);
}

TuringValue *CodeGen::compileCall(Symbol *callee,ASTNode *node, bool wantReturn) {
    if (!callee->isFunction()) {
        throw Message::Exception("Only functions and procedures can be called.");
    }
    FunctionSymbol *funcSym = static_cast<FunctionSymbol*>(callee);
    std::vector<TuringValue*> params;
    for (unsigned int i = 1; i < node->children.size(); ++i) {
        bool isVarRef = funcSym->getArgDecl(i-1).IsVarRef;
        TuringValue *argVal;
        //  handle 'var' parameters as symbols
        if (isVarRef) {
            Symbol *argSym = compileLHS(node->children[i]);
            argVal = new TuringValue(argSym->getVal(),argSym->getType());
        } else {
            argVal = compile(node->children[i]);
        }
        params.push_back(argVal);
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
TuringValue *CodeGen::abstractCompileCall(Symbol *callee,const std::vector<TuringValue*> &params, bool wantReturn) {
    
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
    
    Value *returnBuffer = NULL;
    if (calleeFuncSym->IsSRet) {
        // start at the entry block. Optimizers like this
        BasicBlock *entryBlock = &currentFunction()->getEntryBlock();
        IRBuilder<> TmpB(entryBlock,entryBlock->begin());
        // pass a memory buffer of the return type as the first parameter, false means not a reference
        returnBuffer = TmpB.CreateAlloca(calleeFuncSym->getType()->getLLVMType(false),0,"sretBuffer");
        // cast it to a reference
        returnBuffer = Builder.CreatePointerCast(returnBuffer, calleeFuncSym->getType()->getLLVMType(true));
        compileInitializeComplex(new VarSymbol(returnBuffer,calleeFuncSym->getType())); // initialize array lengths and stuff
        argVals.push_back(returnBuffer);
    }
    
    for (unsigned idx = 0; idx < params.size(); ++idx) {
        TuringValue *val = params[idx];
        TuringType *typ = calleeFuncSym->getArgDecl(idx).Type;
        argVals.push_back(promoteType(val, typ, 
                                      Twine("argument ") + Twine(idx + 1) + " of function " +
                                      calleeFunc->getName())->getVal());
    }
    if (calleeFuncSym->IsSRet) {        
        assert(returnBuffer != NULL);
        // return the casted reference to the return value. true = this is a reference
        Builder.CreateCall(calleeFunc, argVals);
        return new TuringValue(returnBuffer,calleeFuncSym->getType());
    } else if (calleeFunc->getReturnType()->isVoidTy() && wantReturn) {
        throw Message::Exception(Twine("Procedure ") + calleeFunc->getName() + " can not return a value.");
    } else if(!wantReturn) {
        Builder.CreateCall(calleeFunc, argVals);
        return NULL;
    }
    
    return new TuringValue(Builder.CreateCall(calleeFunc, argVals, "calltmp"),calleeFuncSym->getType());
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
        Type *argType = args[i].Type->getLLVMType(true); // true = parameters are references
        // if it is a var parameter and it is not a type normally passed by reference then pass a pointer
        if (args[i].IsVarRef && !argType->isPointerTy()) {
            argType = argType->getPointerTo();
        }
        argTypes.push_back(argType);  
    }
    // the return type is void if it is a procedure OR if the return is a complex type
    // WARNING it shouldn't matter wether the getLLVMType() is a reference since complex types are passed
    // as parameters. However, some change later on may need this to be decided.
    Type *funcRetType = structRet ? Type::getVoidTy(getGlobalContext()) : returnType->getLLVMType();
    FunctionType *FT = FunctionType::get(funcRetType,argTypes, false);
    
    Function *f = Function::Create(FT, internal ? Function::InternalLinkage : Function::ExternalLinkage, name, TheModule);
    FunctionSymbol *fSym = new FunctionSymbol(f,returnType,args);
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
        RetVal = new VarSymbol(&(*ai),fSym->getType());
        ++ai; // the next argument is the actual first one
    } else if (!isProcedure(f)) {
        RetVal = new VarSymbol(Builder.CreateAlloca(f->getReturnType(), 0,"returnval"),fSym->getType());
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
        Builder.CreateRet(Builder.CreateLoad(RetVal->getVal()));
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
    TuringValue *cond = compile(node->children[0]);
    cond = promoteType(cond,Types.getType("boolean")); // type check
    
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
        Builder.CreateCondBr(cond->getVal(), thenBB, elseBB);
    } else {
        Builder.CreateCondBr(cond->getVal(), thenBB, mergeBB);
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

void CodeGen::compileCaseStat(ASTNode *node) {
    TuringValue *expr = compile(node->children[0]);
    BasicBlock *mergeBB = BasicBlock::Create(getGlobalContext(), "caseCont");
    compileCaseLabelStat(node->children[1], expr,mergeBB);
    currentFunction()->getBasicBlockList().push_back(mergeBB);
    Builder.SetInsertPoint(mergeBB);
}

void CodeGen::compileCaseLabelStat(ASTNode *node, TuringValue *expr, BasicBlock *mergeBB) {
    Function *theFunction = currentFunction();
    
    // special case. No exprs and no next label means it is a catch-all case.
    if (node->children.size() == 1) {
        Scopes->pushLocalScope(theFunction);
        compileBlock(node->children[0]);
        Scopes->popScope();
        
        if (!isCurBlockTerminated()) {
            Builder.CreateBr(mergeBB);
        }
        return;
    }
    
    bool hasNextLabel = node->children.size() > 2 && node->children[1]->root == Language::CASE_LABEL;
    
    // if the expr is equal to one of this label's expressions, run this block
    BasicBlock *thenBB = BasicBlock::Create(getGlobalContext(), "labelBlock");
    
    // otherwise continue on in this block
    BasicBlock *elseBB = NULL;
    
    // for each possiblity expr
    for (unsigned int i = (hasNextLabel ? 2 : 1); i < node->children.size(); ++i) {
        elseBB = BasicBlock::Create(getGlobalContext(), "labelElse");
        
        TuringValue *cond = abstractCompileEqualityOp(expr, compile(node->children[i]));
        Builder.CreateCondBr(cond->getVal(), thenBB, elseBB);
        
        // if it isn't the last iteration, then tack on the merge block and set it to the insert block
        if (i != node->children.size() - 1) {
            theFunction->getBasicBlockList().push_back(elseBB);
            Builder.SetInsertPoint(elseBB);
        }
    }
    theFunction->getBasicBlockList().push_back(thenBB);
    Builder.SetInsertPoint(thenBB);
    
    Scopes->pushLocalScope(theFunction);
    compileBlock(node->children[0]);
    Scopes->popScope();
    
    if (!isCurBlockTerminated()) {
        Builder.CreateBr(mergeBB);
    }    
    
    theFunction->getBasicBlockList().push_back(elseBB);
    Builder.SetInsertPoint(elseBB);
    
    if (hasNextLabel) {        
        compileCaseLabelStat(node->children[1], expr, mergeBB);
    }
    
    if (!isCurBlockTerminated()) {
        Builder.CreateBr(mergeBB);
    }
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
    
    // first child is a bool literal of wether the loop is decreasing.
    bool isDecreasing = (node->children[0]->str.compare("true") == 0);
    // if the loop is decreasing we check if it is smaller than the end, otherwise, larger
    CmpInst::Predicate donePred = isDecreasing ? CmpInst::ICMP_SLT : CmpInst::ICMP_SGT;
    Instruction::BinaryOps loopNextOp = isDecreasing ? Instruction::Sub : Instruction::Add;
    
    Function *theFunction = currentFunction();
    
    BasicBlock *loopBB = BasicBlock::Create(getGlobalContext(), "for", theFunction);
    BasicBlock *mergeBB = BasicBlock::Create(getGlobalContext(), "forcont");
    
    Scopes->pushLocalScope(currentFunction());
    
    // loop induction variable. node->str is the name
    Symbol *inductionVar = Scopes->curScope()->declareVar(node->str,Types.getType("int"));
    TuringValue *increment = promoteType(compile(node->children[2]), Types.getType("int"), "for loop increment");
    
    std::pair<TuringValue*,TuringValue*> range = compileRange(node->children[1]);
    
    // starting at the first number
    Builder.CreateStore(range.first->getVal(),inductionVar->getVal());
    
    // if the first value is greater than the second then skip it
    // I.E for i : 6..5
    Value *overEnd = Builder.CreateICmp(donePred,range.first->getVal(),range.second->getVal());
    Builder.CreateCondBr(overEnd,mergeBB,loopBB);
    
    Builder.SetInsertPoint(loopBB);
    
    // set the block the "exit" statement should continue to
    BasicBlock *prevExitBlock = ExitBlock;
    ExitBlock = mergeBB;
    
    compileBlock(node->children[3]);
    Scopes->popScope();
    
    // stack like. return to previous one
    ExitBlock = prevExitBlock;
    
    if (!isCurBlockTerminated()) {
        // increment = load -> add/sub 1 -> store
        Value *incremented = Builder.CreateBinOp(loopNextOp, Builder.CreateLoad(inductionVar->getVal(),"inductvarval"),
                                               increment->getVal(),"incremented");
        Builder.CreateStore(incremented,inductionVar->getVal());
        
        // finished yet?
        Value *done = Builder.CreateICmp(donePred,incremented,range.second->getVal());
        Builder.CreateCondBr(done,mergeBB,loopBB);
    }
    
    
    // Emit merge block.
    theFunction->getBasicBlockList().push_back(mergeBB);
    Builder.SetInsertPoint(mergeBB);
}