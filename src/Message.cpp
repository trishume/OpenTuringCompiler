//
//  Error.cpp
//  Turing Compiler
//
//  Created by Tristan Hume on 12-02-01.
//  Copyright 2012 15 Norwich Way. All rights reserved.
//
#include "Message.h"

#include <iostream>

static void MyDefaultErrorCallBack(std::string message, std::string file, 
                                   int line, bool isWarning, bool approximate) {
    std::string messageType(isWarning ? "WARNING" : "ERROR");
    if (line < 1 || file.empty()) {
        std::cerr << messageType << ": ";
    } else {
        std::cerr << messageType << " on line " << line << " in file " << file << ": ";
    }
    std::cerr << message << std::endl;
}

namespace Message {
    static int curLine = 0;
    static bool lineApproximate = false;
    static std::string curFile = "";
    static ErrorCallback curErrCallback = &MyDefaultErrorCallBack;
    
    void setCurLine(int line,std::string fileName, bool approximate) {
        curLine = line;
        curFile = fileName;
        lineApproximate = approximate;
    }
    
    void setErrorCallback(ErrorCallback callback) {
        curErrCallback = callback;
    }
    
    bool error(const llvm::Twine &message,bool warning) {
        curErrCallback(message.str(),curFile,curLine,warning,lineApproximate);
        return true;
    }
    bool log(const llvm::Twine &message) {
        std::cout << "LOG: " << message.str() << std::endl;
        return false;
    }
    
    bool ifNull(void *cond, const llvm::Twine &message) {
        if (!cond) error(message);
        return cond == NULL;
    }
    
    void runtimeError(const llvm::Twine &message) {
        error(message);
        throw 1; // unwind to Executor run function
    }
}