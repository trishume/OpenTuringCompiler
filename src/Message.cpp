//
//  Error.cpp
//  Turing Compiler
//
//  Created by Tristan Hume on 12-02-01.
//  Copyright 2012 15 Norwich Way. All rights reserved.
//
#include "Message.h"

#include <iostream>

namespace Message {
    static int curLine = 0;
    static std::string curFile = "";
    
    void setCurLine(int line,std::string fileName) {
        curLine = line;
        curFile = fileName;
    }
    
    bool error(const llvm::Twine &message,bool warning) {
        std::string messageType(warning ? "WARNING" : "ERROR");
        if (curLine < 1 || curFile.empty()) {
            std::cerr << messageType << ": ";
        } else {
            std::cerr << messageType << " on line " << curLine << " in file " << curFile << ": ";
        }
        std::cerr << message.str() << std::endl;
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