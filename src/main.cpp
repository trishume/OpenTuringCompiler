#include <string>
#include <iostream>
#include <istream>
#include <ostream>
#include <fstream>
#include <iterator>

#include <dparse.h>

#include "ast.h"
#include "codegen.h"

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
    
    // TODO proper base dir
    ASTSource *source = new ASTSource("");
    
    CodeGen gen(source);
    
    if(gen.compileFile(argv[1])) {
        gen.execute(true);
    }
}