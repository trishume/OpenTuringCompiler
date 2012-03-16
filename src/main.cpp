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
#define DEFAULT_POST_INCLUDE "lib/postdefs.t"

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

bool compileIfExists(const std::string &fileName,CodeGen &gen) {
    std::ifstream includes_file(fileName.c_str());
    if (includes_file.good())
    {
        if(!gen.compileFile(fileName)) {
            std::cerr << "Failed to compile default include. This shouldn't happen and is not your fault, probably." << std::endl;
            return false;
        }
    }
    return true;
}

int main(int argc, char *argv[]) {
    //d_verbose_level = 1;
    
    if (argc < 2) {
        std::cerr << "no file given to execute. Pass it as the first parameter.";
        return 1; // no file passed
    }
    
    // TODO proper base dir
    FileSource *source = new FileSource("");
    LibManager *plugins = new LibManager();
    CodeGen gen(source,plugins);
    
    if(compileIfExists(DEFAULT_INCLUDE,gen) && 
       gen.compileFile(argv[1]) && 
       compileIfExists(DEFAULT_POST_INCLUDE, gen)) {
        gen.execute(true);
    }
    delete plugins;
    delete source;
}