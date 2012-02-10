{
    #include "ast.h"

    #define D_ParseNode_User ASTNode*

    #include "dparse.h"
    
    #include <string>
    #include <iostream>
    #include <set>

    ASTNode *treeRoot;
    
    void errorFunc(std::string msg) {
        std::cerr << "Parser Error: " << msg << std::endl;
        exit(0);
    }
    
    char *dup_str(const char *s, const char *e) {
        int l = e ? e-s : strlen(s);
        char *ss = new char[l+1];
        memcpy(ss, s, l);
        ss[l] = 0;
        return ss;
    }
    std::string nodeString(const D_ParseNode &node) {
        char *cstr = dup_str(node.start_loc.s, node.end);
        std::string cppstr(cstr);
        delete[] cstr;
        return cppstr;
    }
    
    //! used for node* and node+
    //! adds each child of the parse node to the AST node
    int addParseTokens(D_ParseNode *parseNode, ASTNode *treeNode) {
        int children = d_get_number_of_children(parseNode);
        for (int i = 0; i < children; ++i) {
            D_ParseNode *child = d_get_child(parseNode,i);
            treeNode->addChild(child->user); // add user data to ast node
        }
        return children;
    }
    
    //! used for (tok node)* and (tok node)+
    //! adds each child at index in a parse group to the AST node
    int addParseGroupItems(D_ParseNode *parseNode, ASTNode *treeNode, int index) {
        int children = d_get_number_of_children(parseNode);
        for (int i = 0; i < children; ++i) {
            D_ParseNode *child = d_get_child(parseNode,i);
            
            if(index >= d_get_number_of_children(child)) {
                errorFunc("Not enough children in group in addParseGroupItems");
                return 0;
            }
            
            // get subitem of group
            D_ParseNode *item = d_get_child(child,index);
            
            treeNode->addChild(item->user); // add user data to ast node
        }
        return children;
    }

    //! takes a decls node and fills in deferred types from types that come after
    //! ex. bob : int, foo, bar, stuff : string
    //! foo and bar would inherit the type of stuff
    void typeTransform(ASTNode *decls) {
        NodeList::iterator it, sofar;
        for (it = sofar = decls->children.begin();it < decls->children.end();++it) {
            ASTNode *typeNode = (**it).children[0];
            // if it has a type
            if(typeNode->root != Language::DEFERRED_TYPE) {
                // go through the previous nodes that haven't been iterated yet.
                for (;sofar < it;++sofar) {
                    ASTNode *curNode = *sofar; // get the node
                    // check if the type is deferred
                    // ->children[0] is the type of the declaration
                    if(curNode->children[0]->root == Language::DEFERRED_TYPE) {
                        // store the current deffered type node to delete it later
                        ASTNode *old = curNode->children[0];
                        // deep copy the proper type node
                        curNode->children[0] = new ASTNode((*it)->children[0]);
                        // free the deferred one
                        delete old;
                    }
                }
            }
        }
    }

    //! checks if an identifier is a keyword so that put (5)
    //! doesn't interperet put as a function. Etcetera...
    bool isKeyword(std::string ident) {
        std::set<std::string> keywords;
        keywords.insert("else");
        keywords.insert("put");
        keywords.insert("if");
        keywords.insert("for");
        keywords.insert("loop");
        keywords.insert("class");
        keywords.insert("var");
        keywords.insert("return");
        keywords.insert("result");
        keywords.insert("label");

        return keywords.find(ident) != keywords.end();
    }
}

// first rule is root
program     
    :   LT* instructions LT* 
    {
        if($1 != NULL) {
            treeRoot = $1;
            std::cout << $1->stringTree() << std::endl;
        } else {
          std::cerr << "null ast!" << std::endl;
        }
    }
    ;

instructions
    :   instruction ((LT+) instruction)*
    {
        $$ = new ASTNode(Language::BLOCK);
        if ($0 != NULL) $$->addChild($0); // first instruction
        addParseGroupItems(&$n1,$$,1); // rest of them
    }
    | LT*
    {
        $$ = new ASTNode(Language::BLOCK);
    }
    ;

// TODO calling procs without call brackets. As in View.Update
instruction 
    :   vardecl {$$ = $0; /* pass up */}
    |   externdecl {$$ = $0; /* pass up */}
    |   typedecl {$$ = $0; /* pass up */}
    |   put {$$ = $0; /* pass up */}
    |   get {$$ = $0; /* pass up */}
    |   constdecl {$$ = $0; /* pass up */}
    |   funcdef {$$ = $0; /* pass up */}
    |   ifstat {$$ = $0; /* pass up */}
    |   forstat {$$ = $0; /* pass up */}
    |   loopstat {$$ = $0; /* pass up */}
    |   casestat {$$ = $0; /* pass up */}
    |   exitstat {$$ = $0; /* pass up */}
    |   resultstat {$$ = $0; /* pass up */}
    |   expr {$$ = $0; /* pass up */}
    ;

type
    :   ID
    { 
        $$ = new ASTNode(Language::NAMED_TYPE,$n0.start_loc.line);
        $$->str = nodeString($n0); // type name
    }
    |   'string' '(' INT ')'
    { 
        $$ = new ASTNode(Language::SIZED_STRING_TYPE,$n0.start_loc.line);
        $$->str = nodeString($n2); // string size name
    }
    |   'array' range (','range)* 'of' type
    { 
        $$ = new ASTNode(Language::ARRAY_TYPE,$n0.start_loc.line);
        $$->addChild($4); // type
        $$->addChild($1); // first range
        addParseGroupItems(&$n2,$$,1); // rest of them
    }
    |   'record' LT* decls ((LT+) decls) LT* 'end' 'record'
    { 
        $$ = new ASTNode(Language::RECORD_TYPE,$n0.start_loc.line);
        $$->addChild($2); // first decls
        addParseGroupItems(&$n3,$$,1); // rest of them
    }
    ;

put : 'put' (':' assignableExpression ',')? expr (',' expr)* ('.' '.')? 
  {
        // TODO streams
        $$ = new ASTNode(Language::PUT_STAT);
        if(d_get_number_of_children(&$n4) > 0) {
            $$->str = ".."; // string = ".." if not to print newline
        }
        $$->addChild($2); // first expr
        addParseGroupItems(&$n3,$$,1); // rest of them
  }
  ;
get :   'get' (':' assignableExpression ',')? assignableExpression (':' '*')?
    {
        // TODO streams
        $$ = new ASTNode(Language::GET_STAT);
        if(d_get_number_of_children(&$n3) > 0) {
            $$->str = "*"; // string = "*" if whole line
        }
        $$->addChild($2); // first expr
    }
    ;

//variables
vardecl 
    :   'var' decls ':=' expr
    { 
        $$ = new ASTNode(Language::VAR_DECL,$n0.start_loc.line);
        $$->addChild($1); // decls
        $$->addChild($3); // value
    }
    |   'var' decls
    { 
        $$ = new ASTNode(Language::VAR_DECL,$n0.start_loc.line);
        $$->addChild($1); // decls
    }
    ;

constdecl
    :   'const' decl ':=' expr
    { 
        $$ = new ASTNode(Language::CONST_DECL,$n0.start_loc.line);
        $$->addChild($1); // def
        $$->addChild($3); // value
    }
    ;
typedecl
    :   'type' ID ':' LT* type
    { 
        $$ = new ASTNode(Language::TYPE_DECL,$n0.start_loc.line);
        $$->str = nodeString($n1); // new type name
        $$->addChild($4); // type to alias
    }
    ;


//sugars
range
    :   expr'.''.'expr
    { 
        $$ = new ASTNode(Language::RANGE,$n0.start_loc.line);
        $$->addChild($0); // begin
        $$->addChild($3); // end
    }
    ;

// control structures
ifstat
    :   'if' expr 'then' LT instructions LT* (elsifstat?) 'end' 'if'
    { 
        $$ = new ASTNode(Language::IF_STAT,$n0.start_loc.line);
        $$->addChild($1); // cond
        $$->addChild($4); // block
        addParseTokens(&$n6,$$); // elsif
    }
    ;
// semantically equivelant to (if then (else (if cond ...
elsifstat   
    :   'elsif' expr 'then' LT* instructions LT* (elsifstat?)
    { 
        ASTNode *innerIf = new ASTNode(Language::IF_STAT);
        innerIf->addChild($1); // cond
        innerIf->addChild($4); // block
        addParseTokens(&$n6,innerIf); // elsif

        $$ = new ASTNode(Language::BLOCK,$n0.start_loc.line);
        $$->addChild(innerIf);
    }
    |   'else' LT* instructions LT*
    { 
        $$ = $2;
    }
    ;

forstat 
    :   'for' ID ':' range LT+ instructions LT* 'end' 'for'
    { 
        $$ = new ASTNode(Language::FOR_STAT,$n0.start_loc.line);
        $$->str = nodeString($n1); // loop variable
        $$->addChild($3); // range
        $$->addChild($5); // block
    }
    ;
loopstat
    :   'loop' LT* instructions LT* 'end' 'loop'
    { 
        $$ = new ASTNode(Language::LOOP_STAT,$n0.start_loc.line);
        $$->addChild($2); // block
    }
    ;

caselabel
    :   'label' expr (',' (LT*) expr)* ':' LT* instructions LT*
    { 
        $$ = new ASTNode(Language::CASE_LABEL,$n0.start_loc.line);
        $$->addChild($5); // block
        $$->addChild($1);
        addParseGroupItems(&$n2,$$,2); // rest of args
    }
    |   'label' ':'  LT* instructions LT*
    { 
        $$ = new ASTNode(Language::CASE_LABEL,$n0.start_loc.line);
        $$->addChild($3); // block
    }
    ;

casestat
    :   'case' expr 'of' LT* caselabel+ LT* 'end' 'case'
    { 
        $$ = new ASTNode(Language::CASE_STAT,$n0.start_loc.line);
        $$->addChild($1);
        addParseTokens(&$n4,$$); // labels
    }
    ;
exitstat
    :   'exit'
    { 
        $$ = new ASTNode(Language::EXIT_STAT,$n0.start_loc.line);
    }
    |   'exit' 'when' expr
    { 
        /* 
        cheat and code it as

        if expr then
            exit
        end if
        */
        $$ = new ASTNode(Language::IF_STAT);
        $$->addChild($2); // condition

        ASTNode *thenStat = new ASTNode(Language::BLOCK);
        thenStat->addChild(new ASTNode(Language::EXIT_STAT,$n0.start_loc.line));

        $$->addChild(thenStat); // then part
    }
    ;
resultstat
    :   'result' expr
    { 
        $$ = new ASTNode(Language::RESULT_STAT,$n0.start_loc.line);
        $$->addChild($1); // expr
    }
    |   'return'
    { 
        $$ = new ASTNode(Language::RETURN_STAT,$n0.start_loc.line);
    }
    ;

//functions and procedures
formalargs
    :   decls { $$ = $0; /* pass up */}
    |   
    {
        $$ = new ASTNode(Language::DECLARATIONS); // no arguments
    }
    ;

decls  
    :   decl (',' decl)* 
    {
        $$ = new ASTNode(Language::DECLARATIONS);
        $$->addChild($0); // first instruction
        addParseGroupItems(&$n1,$$,1); // rest of them

        // TYPE TRANSFORM
        typeTransform($$);
    }
    ;

decl // declaration of a variable, argument or field
    :   ID 
    { 
        $$ = new ASTNode(Language::DECLARATION,$n0.start_loc.line);
        $$->str = nodeString($n0); // var name
        $$->addChild(new ASTNode(Language::DEFERRED_TYPE)); // no type
    }
    |   ID ':' type
    { 
        $$ = new ASTNode(Language::DECLARATION,$n0.start_loc.line);
        $$->str = nodeString($n0); // var name
        $$->addChild($2); // type
    }
    // TODO impliment 'var' arguments
    ;
    
prototype
    : ('fcn'|'function') ID '('formalargs')'':'type
    {
        $$ = new ASTNode(Language::FUNC_PROTO,$n0.start_loc.line);
        $$->str = nodeString($n1); // func name
        $$->addChild($6);
        $$->addChild($3);
    }
    | ('proc'|'procedure') ID '('? formalargs ')'?
    {
        $$ = new ASTNode(Language::FUNC_PROTO,$n0.start_loc.line);
        $$->str = nodeString($n1); // func name
        $$->addChild(new ASTNode(Language::VOID_TYPE));
        $$->addChild($3);
    }
    ;
    
externdecl
    : 'extern' prototype {$$ = $1; /* pass up */}
    ;
    
funcdef 
    :   prototype LT+ instructions LT+ 'end' ID
    {
        $$ = new ASTNode(Language::FUNC_DEF,$n0.start_loc.line);
        $$->addChild($0);
        $$->addChild($2);
    }
    ;

//lexer
ID  :   "[a-zA-Z_][a-zA-Z0-9_]*"
    [
        // otherwise else is parsed as a function call
        // TODO make it reject all keywords
        std::string ident = nodeString($n0);
        if (isKeyword(ident)) {
            ${reject};
        } 
    ]
    ;

INT :   "[0-9]+"
    ;

REAL:   "[0-9]*\.[0-9]+"
    ;

whitespace: ( "[ \t]+" | COMMENT )*;

COMMENT
    :   '%' "[^\n]*"
    |   '/*' ( "[^*]" | '*'+ "[^*\/]" )* '*'+ '/'
    ;


LT
    :   '\n'      // Line feed.
    |   '\r' '\n'     // Carriage return.
    |   '\u2028'  // Line separator.
    |   '\u2029'  // Paragraph separator.
    ;

STRING
    :   "([^\"\\]|\\[^])*"
    ;

CHAR:   "([^'\\]|\\[^])"
    ;

ESC_SEQ
    :   '\\' ('b'|'t'|'n'|'f'|'r'|'\"'|'\''|'\\')
    ;

BIN_OP 
    :   '**' $binary_op_left 70
    |   '*' $binary_op_left 60
    |   '/' $binary_op_left 60
    |   'div' $binary_op_left 60
    |   'mod' $binary_op_left 60
    |   '+' $binary_op_left 50
    |   '-' $binary_op_left 50
    |   '<=' $binary_op_left 40
    |   '>=' $binary_op_left 40
    |   '<' $binary_op_left 40
    |   '>' $binary_op_left 40
    |   'or' $binary_op_left 20
    |   'and' $binary_op_left 20
    ;

EQUALITY_OP
    :   '=' $binary_op_left 30
    |   '~=' $binary_op_left 30
    |   'not=' $binary_op_left 30
    ;

ASSIGN_OP   
    :   ':=' $binary_op_right 10
    |   '+=' $binary_op_right 10
    |   '-=' $binary_op_right 10
    |   '/=' $binary_op_right 10
    |   '*=' $binary_op_right 10
    |   '**=' $binary_op_right 10
    |   'div=' $binary_op_right 10
    |   'mod=' $binary_op_right 10
    ;
    
UNARY_OPERATOR
    : '-' $unary_op_right 100
    | 'not' $unary_op_right 90
    ;
POINTER_FIELD_REF_OPERATOR
    : '->' $binary_op_left 80
    ;
FIELD_REF_OPERATOR
    : '.' $binary_op_left 80
    ;

//expressions

expr
    : primaryExpression {$$ = $0; /* pass up */}
    | UNARY_OPERATOR primaryExpression
    {
        $$ = new ASTNode(Language::UNARY_OP,$n0.start_loc.line);
        $$->str = nodeString($n1); // op string
        $$->addChild($1);
    }
    | expr LT* BIN_OP LT* expr 
    {
        $$ = new ASTNode(Language::BIN_OP,$n0.start_loc.line);
        $$->str = nodeString($n2); // op string
        $$->addChild($0);
        $$->addChild($4);
    }
    | expr LT* EQUALITY_OP LT* expr
    {
        $$ = new ASTNode(Language::EQUALITY_OP,$n0.start_loc.line);
        $$->str = nodeString($n2); // op string
        $$->addChild($0);
        $$->addChild($4);
    }
    | assignableExpression LT* ASSIGN_OP LT* expr
    {
        $$ = new ASTNode(Language::ASSIGN_OP,$n0.start_loc.line);
        $$->str = nodeString($n2); // op string
        $$->addChild($0);
        $$->addChild($4);
    }
    ;
    
primaryExpression
    :   INT
    {
        $$ = new ASTNode(Language::INT_LITERAL,$n0.start_loc.line);
        $$->str = nodeString($n0); // literal string
    }
    |   REAL
    {
        $$ = new ASTNode(Language::REAL_LITERAL,$n0.start_loc.line);
        $$->str = nodeString($n0); // literal string
    }
    |   '"' STRING '"'  { $$ = new ASTNode(Language::STRING_LITERAL,$n0.start_loc.line); $$->str = nodeString($n1);}
    |   '\'' CHAR  '\'' { $$ = new ASTNode(Language::CHAR_LITERAL,$n0.start_loc.line); $$->str = nodeString($n1);}
    |   'true' { $$ = new ASTNode(Language::BOOL_LITERAL,$n0.start_loc.line); $$->str = nodeString($n0);}
    |   'false' { $$ = new ASTNode(Language::BOOL_LITERAL,$n0.start_loc.line); $$->str = nodeString($n0);}
    |   assignableExpression '(' expr (',' (LT*) expr)* ')' // function call with args
    {
        $$ = new ASTNode(Language::CALL,$n0.start_loc.line);
        $$->str = nodeString($n0); // op string
        $$->addChild($2);
        addParseGroupItems(&$n3,$$,2); // rest of args
    }
    |   assignableExpression '(' ')' // function call with no args
    {
        $$ = new ASTNode(Language::CALL,$n0.start_loc.line);
        $$->str = nodeString($n0); // op string
    }
    |   assignableExpression {$$ = $0; /* pass up */}
    |   '(' expr')' {$$ = $1; /* pass up */}
    ;

assignableExpression
    :   '^' assignableExpression
    {
        $$ = new ASTNode(Language::PTRDEREF,$n0.start_loc.line);
        $$->addChild($1);
    }
    |   assignableExpression POINTER_FIELD_REF_OPERATOR ID // '->' operator
    {
        // cheating. -> is semantically equivelant to ^lhs.rhs
        $$ = new ASTNode(Language::FIELD_REF_OP,$n0.start_loc.line);
        $$->str = nodeString($n2); // right operand
        ASTNode *deref = new ASTNode(Language::PTRDEREF);
        deref->addChild($0);
        $$->addChild(deref);
    }
    |   assignableExpression FIELD_REF_OPERATOR ID // '.' operator
    {
        $$ = new ASTNode(Language::FIELD_REF_OP,$n0.start_loc.line);
        $$->addChild($0);
        $$->str = nodeString($n2); // right operand
    }
    |   ID
    {
        $$ = new ASTNode(Language::VAR_REFERENCE,$n0.start_loc.line);
        $$->str = nodeString($n0); // var identifier
    }
    ;