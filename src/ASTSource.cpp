//
//  ASTSource.cpp
//  TuringCompiler
//
//  Created by Tristan Hume on 12-02-20.
//  Copyright 2012 15 Norwich Way. All rights reserved.
//

#include "ASTSource.h"

#include <istream>
#include <iostream>
#include <ostream>
#include <fstream>
#include <iterator>

#include <llvm/ADT/Twine.h>

extern D_ParserTables parser_tables_gram;
//! from parser
extern ASTNode *treeRoot;

ASTSource::ASTSource(std::string baseDir) : BasePath(baseDir) {
    Parser = new_D_Parser(&parser_tables_gram, sizeof(ASTNode*));
}

ASTNode *ASTSource::parseFile(const std::string &fileName, const std::string &includedFrom) {
    std::string includedFolder = ASTSource::folderFromFilePath(includedFrom);
    
    std::string path = (llvm::Twine(BasePath) + includedFolder + fileName).str();
    
    return parseString(getFileContents(path));
}

std::string ASTSource::folderFromFilePath(const std::string &fileName) {
    size_t found = fileName.find_last_of("/\\");
    // if it is npos then there was no separator. Just a name. in wich case the folder is blank.
    return (found == std::string::npos ? "" : fileName.substr(0,found+1));
}

ASTNode *ASTSource::parseString(const std::string &fileData, bool printAST) {
    char *s = new char[fileData.length()];
    strcpy(s,fileData.c_str());
    
    if (dparse(Parser, s, strlen(s)) && !Parser->syntax_errors) {
        if (printAST)
            std::cout << treeRoot->stringTree() << std::endl;
        return treeRoot;
    }
    
    return NULL;
}

std::string ASTSource::getFileContents(const std::string &filePath) {
    std::ifstream in(filePath.c_str(), std::ios::in | std::ios::binary);
    if (in)
    {
        return(std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>()));
    }
    return "";
}