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
    
    void setCurLine(int line) {
        curLine = line;
    }
    
    bool error(const llvm::Twine &message) {
        if (curLine < 1) {
            std::cerr << "ERROR: ";
        } else {
            std::cerr << "ERROR on line " << curLine << ": ";
        }
        std::cerr << message.str() << std::endl;
        return true;
    }
    bool warning(const llvm::Twine &message) {
        if (curLine < 1) {
            std::cerr << "WARNING: ";
        } else {
            std::cerr << "WARNING on line " << curLine << ": ";
        }
        std::cerr << message.str() << std::endl;
        return false;
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
        exit(1);
    }
}