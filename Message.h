//
//  Error.h
//  Turing Compiler
//
//  Created by Tristan Hume on 12-02-01.
//  Copyright 2012 15 Norwich Way. All rights reserved.
//

#ifndef Turing_Compiler_Error__H
#define Turing_Compiler_Error__H

#include <string>
#include <llvm/ADT/Twine.h>

namespace Message {
    //! returns false on fatality (false for Error true for others)
    bool error(const llvm::Twine &message);
    bool warning(const llvm::Twine &message);
    bool log(const llvm::Twine &message);
    bool ifNull(void *cond, const llvm::Twine &message);
}

#endif
