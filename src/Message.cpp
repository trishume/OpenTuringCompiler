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
    bool error(const llvm::Twine &message) {
        std::cerr << "ERROR: " << message.str() << std::endl;
        return false;
    }
    bool warning(const llvm::Twine &message) {
        std::cerr << "WARNING: " << message.str() << std::endl;
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
}