#include "LibManager.h"

#include <fstream>
#include <llvm/ADT/Twine.h>
#include <llvm/Support/DynamicLibrary.h>

#include "TuringCommon/FileSystem.h"
#include "Message.h"

using namespace llvm;

bool LibManager::linkLibrary(const std::string &libName, const std::string &includedFrom) {
    // get the path
    std::string libPath = getLibraryPath(libName, includedFrom);
    if (libPath.empty()) {
        Message::error(Twine("Can't find library ") + libName);
        return false;
    }
    // dynamically link it
    std::string errMsg;
    bool fail = llvm::sys::DynamicLibrary::LoadLibraryPermanently (libPath.c_str(), &errMsg);
    if (fail) {
        Message::error(Twine("Failed to load library ") + libName + ": " + errMsg);
        return false;
    }
    // check for callbacks
    checkForFunctions(libName);
    return true;
}

std::string LibManager::getLibraryPath(const std::string &libName, const std::string &includedFrom) {
    std::string path;
    
#ifdef OS_WINDOWS
    path = TuringCommon::includeFilePath((llvm::Twine(libName) + ".dll").str(),
                                         includedFrom);
#else
    path = TuringCommon::includeFilePath((llvm::Twine("lib") + libName + ".so").str(),
                                         includedFrom);
#endif
    std::ifstream lib_file(path.c_str());
    if (lib_file.good())
    {
        return path;
    }
    
    return ""; // FAIL
}

void LibManager::checkForFunctions(const std::string &libName) {
    std::string funcName; 
    void *funcPtr;
    
    funcName = (Twine("Turing_") + libName + "_IntermittentCallback").str();
    funcPtr = llvm::sys::DynamicLibrary::SearchForAddressOfSymbol(funcName);
    if (funcPtr != NULL) {
        IntermittentCallbacks.push_back((IntermittentCallbackFunction)funcPtr);
    }
    
    funcName = (Twine("Turing_") + libName + "_InitRun").str();
    funcPtr = llvm::sys::DynamicLibrary::SearchForAddressOfSymbol(funcName);
    if (funcPtr != NULL) {
        InitRunFunctions.push_back((InitRunFunction)funcPtr);
    }
}