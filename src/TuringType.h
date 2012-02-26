//
//  Type.h
//  Turing Compiler
//
//  Created by Tristan Hume on 12-02-01.
//  Copyright 2012 15 Norwich Way. All rights reserved.
//

#ifndef Turing_Compiler_Type_H
#define Turing_Compiler_Type_H

#include <string>
#include <vector>
#include <map>

#include <llvm/Type.h>

#include "VarDecl.h"

class TuringType {
public:
    virtual ~TuringType() {}
    //! \param isReference  wether this is being passed by reference
    //!                     some types like arrays have a different type when 
    //!                     passed as parameters.
    virtual llvm::Type *getLLVMType(bool isReference = false) = 0;
    virtual std::string getName() = 0;
    virtual void setName(const std::string &newName) = 0;
    
    //! returns true if the types are equal
    virtual bool compare(TuringType *otherType);
    
    virtual bool isArrayTy() = 0;
    virtual bool isRecordTy() = 0;
    //! true if the type can not be stored in a register. I.E arrays and structs
    virtual bool isComplexTy() = 0;
};

class BasicTuringType : public TuringType {
public:
    BasicTuringType(std::string n, llvm::Type *type) : LLVMType(type), Name(n) {}
    //! \param isReference has no effect for basic types
    virtual llvm::Type *getLLVMType(bool isReference = true);
    virtual std::string getName();
    virtual void setName(const std::string &newName);
    
    virtual bool isArrayTy() {return false;}
    virtual bool isRecordTy() {return false;}
    virtual bool isComplexTy() {return false;}
protected:
    llvm::Type *LLVMType;
    std::string Name;
};

class TuringArrayType : public TuringType {
public:
    //! \param upper the size of the array. 0 for any size (used for parameters)
    TuringArrayType(TuringType *elementType, unsigned int upper);
    //! \param isReference returns {i32, [Size x Type]} normally, *{i32, [0 x Type]} by reference
    virtual llvm::Type *getLLVMType(bool isReference = false);
    TuringType *getElementType() {return ElementType;};
    unsigned int getSize() {return Size;}
    
    virtual bool compare(TuringType *other);
    
    virtual std::string getName();
    //! set a nickname for this array type, used for strings
    virtual void setName(const std::string &newName) {
        Name = newName;
    }
    
    virtual bool isArrayTy() {return true;}
    virtual bool isRecordTy() {return false;}
    virtual bool isComplexTy() {return true;}
protected:
    TuringType *ElementType;
    unsigned int Size;
    std::string Name;
};

class TuringRecordType : public TuringType {
public:
    TuringRecordType(std::vector<VarDecl> elements);
    //! \param isReference returns {elems} normally, *{elems} by reference
    virtual llvm::Type *getLLVMType(bool isReference = false);
    VarDecl getDecl(unsigned int index) {return Elems[index];};
    unsigned int getIndex(std::string field);
    unsigned int getSize() {return Elems.size();}
    
    virtual std::string getName() {return Name;}
    //! set a nickname for this array type, used for strings
    virtual void setName(const std::string &newName) {
        Name = newName;
    }
    
    virtual bool isArrayTy() {return false;}
    virtual bool isRecordTy() {return true;}
    virtual bool isComplexTy() {return true;}
protected:
    std::vector<VarDecl> Elems;
    //! Maps field names to an index in Elems.
    std::map<std::string,unsigned int> NameToIndex;
    std::string Name;
};


#endif
