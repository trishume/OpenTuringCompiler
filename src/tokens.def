AST_TOKEN(BLOCK) /* children: statement* string: unchecked? */
AST_TOKEN(NAMED_TYPE) /* children: string: ID */
AST_TOKEN(SIZED_STRING_TYPE) /* children: string: size */
AST_TOKEN(ARRAY_TYPE) /* children: type range+ string: */
AST_TOKEN(RECORD_TYPE) /* children: decls* string: */
AST_TOKEN(DEFERRED_TYPE) /* children: string: */
AST_TOKEN(VOID_TYPE) /* children: */
AST_TOKEN(PUT_STAT) /* children: streamNumber expr+ string: (.. if no newline) */
AST_TOKEN(GET_STAT) /* children: streamNumber assignableExpression+ string: (* if read whole line) */
AST_TOKEN(QUIT_STAT) /* children: errCode string: */
AST_TOKEN(ASSERT_STAT) /* children: expr string: exprStr */
AST_TOKEN(RESIZE_STAT) /* children: flexibleArrRef newSize string: */
AST_TOKEN(INCLUDE_STAT) /* children: string: file name to include */
AST_TOKEN(VAR_DECL) /* children: decls value? string: */
AST_TOKEN(CONST_DECL) /* children: decl value string: */
AST_TOKEN(EXTERN_DECL) /* children: prototype string: alias? */
AST_TOKEN(LIBRARY_DECL) /* children: string: libname */
AST_TOKEN(TYPE_DECL) /* children: type string: typeID */
AST_TOKEN(RANGE) /* children: begin end string: */
AST_TOKEN(RANGE_SPECIAL_END) /* children: string: "*" or "char" */
AST_TOKEN(IF_STAT) /* children: cond thenBlock elseBlock? string: */
AST_TOKEN(FOR_STAT) /* children: decreasing(bool literal) range increment block string: counterVar */
AST_TOKEN(LOOP_STAT) /* children: block string: */
AST_TOKEN(CASE_LABEL) /* children: block label? expr* string: */
AST_TOKEN(CASE_STAT) /* children: expr label string: */
AST_TOKEN(EXIT_STAT) /* children: string: */
AST_TOKEN(RESULT_STAT) /* children: expr string: */
AST_TOKEN(RETURN_STAT) /* children: string: */
AST_TOKEN(DECLARATIONS) /* children: decl* string: */
AST_TOKEN(DECLARATION) /* children: type string: var? name */
AST_TOKEN(FUNC_PROTO) /* children: returnType block string: name */
AST_TOKEN(FUNC_DEF) /* children: proto block string: */
AST_TOKEN(MODULE_DEF) /* children: block string: name */
AST_TOKEN(BIN_OP) /* children: lhs rhs string: op */
AST_TOKEN(EQUALITY_OP) /* children: lhs rhs string: op */
AST_TOKEN(ASSIGN_OP) /* children: lhs rhs string: op */
AST_TOKEN(UNARY_OP) /* children: operand string: operator */
AST_TOKEN(INT_LITERAL) /* children: string: int */
AST_TOKEN(REAL_LITERAL) /* children: string: real */
AST_TOKEN(STRING_LITERAL) /* children: string: string without quotes */
AST_TOKEN(CHAR_LITERAL) /* children: string: char without quotes */
AST_TOKEN(ARRAY_UPPER) /* children: expr string: */
AST_TOKEN(ARRAY_LOWER) /* children: expr string: */
AST_TOKEN(ARRAY_INIT) /* children: expr+ string: */
AST_TOKEN(BOOL_LITERAL) /* children: string: true or false */
AST_TOKEN(CALL) /* children: assignableExpr arg* string: */
AST_TOKEN(PTRDEREF) /* children: assignableExpr string: */
AST_TOKEN(FIELD_REF_OP) /* children: lhs string: rhs */
AST_TOKEN(VAR_REFERENCE) /* children: string: ID */