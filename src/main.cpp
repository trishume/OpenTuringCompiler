#include <string>
#include <iostream>
#include <istream>
#include <ostream>
#include <fstream>
#include <iterator>

#include <dparse.h>

#include "ast.h"
#include "codegen.h"

#define DEFAULT_INCLUDE "lib/predefs.t"

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
    //d_verbose_level = 1;
    
    if (argc < 2) {
        std::cerr << "no file given to execute. Pass it as the first parameter.";
        return 1; // no file passed
    }
    
    // TODO proper base dir
    FileSource *source = new FileSource("");
    
    CodeGen gen(source);
    
    // if the default include file exists then try to compile it.
    std::ifstream includes_file(DEFAULT_INCLUDE);
    if (includes_file.good())
    {
        if(!gen.compileFile(DEFAULT_INCLUDE)) {
            std::cerr << "Failed to compile default includes. This shouldn't happen and is not your fault, probably." << std::endl;
            return 1;
        }
    }
    
    if(gen.compileFile(argv[1])) {
        gen.execute(true);
    }
}