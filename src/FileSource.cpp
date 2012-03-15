//
//  FileSource.cpp
//  TuringCompiler
//
//  Created by Tristan Hume on 12-02-20.
//  Copyright 2012 15 Norwich Way. All rights reserved.
//

#include "FileSource.h"

#include <istream>
#include <iostream>
#include <ostream>
#include <fstream>
#include <iterator>

#include <llvm/ADT/Twine.h>

#include "TuringCommon/FileSystem.h"

extern D_ParserTables parser_tables_gram;
//! from parser
extern ASTNode *treeRoot;

FileSource::FileSource(std::string baseDir) : BasePath(baseDir) {
    Parser = new_D_Parser(&parser_tables_gram, sizeof(ASTNode*));
}

ASTNode *FileSource::parseFile(const std::string &path) {
    std::string parseFile = getFileContents(path);
    if (parseFile.empty()) {
        return NULL;
    }
    return parseString(parseFile);
}

std::string FileSource::getLibraryPath(const std::string &libName, const std::string &includedFrom) {
    std::string path;
    
    // library path is OS dependent. Find the right one.
    // TODO use a preprocessor define to stop .so includes on windows and vice-versa
    
    path = TuringCommon::includeFilePath((llvm::Twine(libName) + ".dll").str(),
                                       includedFrom,BasePath);
    std::ifstream dll_file(path.c_str());
    if (dll_file.good())
    {
        return path;
    }
    
    path = TuringCommon::includeFilePath((llvm::Twine("lib") + libName + ".so").str(),
                                       includedFrom,BasePath);
    std::ifstream so_file(path.c_str());
    if (so_file.good())
    {
        return path;
    }
    
    return ""; // FAIL
}

ASTNode *FileSource::parseString(const std::string &fileData, bool printAST) {
    char *s = new char[fileData.length()];
    strcpy(s,fileData.c_str());
    
    if (dparse(Parser, s, strlen(s)) && !Parser->syntax_errors) {
        if (printAST)
            std::cout << treeRoot->stringTree() << std::endl;
        return treeRoot;
    }
    
    return NULL;
}

std::string FileSource::getFileContents(const std::string &filePath) {
    std::ifstream in(filePath.c_str(), std::ios::in | std::ios::binary);
    if (in)
    {
        return(std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>()));
    }
    return "";
}