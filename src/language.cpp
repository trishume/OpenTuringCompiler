#include "language.h"

// used for name switch statement
#define TOK_CASE(a) case a: return #a

namespace Language { 
 const char *getTokName(Token tok) {
    switch(tok) {
      case UNDEFINED: return "<undefined/null>";
      case TEMP_TOKEN: return "<temporary token>";
#define AST_TOKEN(tok) case tok: return #tok;
#include "tokens.def"
#undef AST_TOKEN
    }
    return "<unknown>";
  }
}