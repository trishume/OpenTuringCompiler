//
//  Error.cpp
//  Turing Compiler
//
//  Created by Tristan Hume on 12-02-01.
//  Copyright 2012 15 Norwich Way. All rights reserved.
//
#include "Message.h"

#include <iostream>
#include <sstream>

static void MyDefaultErrorCallBack(std::string message, std::string file,
                                   int line, bool isWarning, int lineRange) {
    std::string messageType(isWarning ? "WARNING" : "ERROR");
    if (line < 1 || file.empty()) {
        std::cerr << messageType << ": ";
    } else {
        std::cerr << messageType;
        if (lineRange > 1) {
            std::cerr << " within " << lineRange << " lines of line ";
        } else {
            std::cerr << " on line ";
        }
        std::cerr << line << " in file " << file << ": ";
    }
    std::cerr << message << std::endl;
}

static void MyEditorCompatibleErrorCallBack(std::string message, std::string file,
                                   int line, bool isWarning, int lineRange) {
    std::ostringstream output;
    output << "Error on line ";
    output << line;
    output << " [0] of ";
    output << file;
    output << ": ";
    output << message;
    std::cout << output.str() << std::endl;
}

namespace Message {
    static int curLine = 0;
    static int LineRange = 1; // line number is within n lines of curLine
    static std::string curFile = "";
    static ErrorCallback curErrCallback = &MyEditorCompatibleErrorCallBack;
    
    void setCurLine(int line,std::string fileName, int lineRange) {
        curLine = line;
        curFile = fileName;
        LineRange = lineRange;
    }
    
    void setErrorCallback(ErrorCallback callback) {
        curErrCallback = callback;
    }
    
    bool error(const llvm::Twine &message,bool warning) {
        curErrCallback(message.str(),curFile,curLine,warning,LineRange);
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