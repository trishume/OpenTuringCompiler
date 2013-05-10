#include <string>
#include <iostream>
#include <istream>
#include <ostream>
#include <fstream>
#include <iterator>

#include <dparse.h>

#include "ast.h"
#include "codegen.h"
#include "Executor.h"
#include "TuringCommon/StreamManager.h"
#include "TuringCommon/FileSystem.h"


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

void run(CodeGen &gen, LibManager *libManager, const std::string &execDir) {
    // set up the stream manager
    TuringCommon::StreamManager streamManager;
    streamManager.initWithStandardStreams();
    // if everything get the finalized module
    llvm::Module *mainModule = gen.getFinalizedModule();
#ifdef DEBUG_PRINT_BYTECODE
    std::cout << "Final Module (unoptimized)";
    mainModule->dump();
#endif
    // run it!
    std::cout << "JIT compiling and optimizing...\n";
    Executor jit(mainModule,&streamManager, libManager, execDir);
    jit.StallOnEnd = true;
    jit.optimize();
    std::cout << "RUNNING...\n";
    jit.run();
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
    std::string execFile = argv[1];
    CodeGen gen(source,plugins);
    
    gen.setPeriodicCallbackFrequency(10); // every 10 stats (lines)
    if(compileIfExists(DEFAULT_INCLUDE,gen) && 
       gen.compileFile(execFile)) {
        std::string execDir = TuringCommon::folderFromFilePath(execFile);
        run(gen,plugins,execDir); // call the run function
    }
    delete plugins;
    delete source;
}