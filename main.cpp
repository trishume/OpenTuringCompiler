#include <string>
#include <iostream>
#include <istream>
#include <ostream>
#include <fstream>
#include <iterator>

#include <dparse.h>

#include "ast.h"
#include "codegen.h"

extern D_ParserTables parser_tables_gram;
//! from parser
extern ASTNode *treeRoot;
extern int d_verbose_level;

std::string get_file_contents(const char *filename)
{
    std::ifstream in(filename, std::ios::in | std::ios::binary);
    if (in)
    {
        return(std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>()));
    }
    return "";
}

int main(int argc, char *argv[]) {
    d_verbose_level = 1;
    
    if (argc < 2) {
        std::cerr << "no file given to execute. Pass it as the first parameter.";
        return 1; // no file passed
    }
    std::cout << "reading file\n";
    std::string input = get_file_contents(argv[1]);
    
    std::cout << input << std::endl;
    std::cout << "--- parsing ---" << std::endl;
    
    char *s,*ss;
    
    // kinda hackish, because new_D_parser does not accept const strings
    // we have to copy it
    s = new char[input.length()];
    strcpy(s,input.c_str());
    
    D_Parser *p = new_D_Parser(&parser_tables_gram, sizeof(ASTNode*));
    if (dparse(p, s, strlen(s)) && !p->syntax_errors) {
        std::cout << "parsing success" << std::endl;
        
        CodeGen gen(treeRoot);
        
        if(!gen.execute()) {
            std::cout << "code gen failed" << std::endl;
        }
    } else {
        std::cout << "fail" << std::endl;
    }
    
    delete[] s;
}