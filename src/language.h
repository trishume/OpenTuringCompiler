#ifndef _LANGUAGE_H_
#define _LANGUAGE_H_

namespace Language {
  // if modifying tokens remember to modify language.cpp as well
  enum Token {
    UNDEFINED,
#define AST_TOKEN(tok) tok,
#include "tokens.def"
#undef AST_TOKEN
    TEMP_TOKEN // mostly so commas work. But can be used for temporary ASTs
  };
  const char *getTokName(Token tok);
}

#endif