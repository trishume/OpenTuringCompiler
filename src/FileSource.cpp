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
#include "Message.h"

using namespace llvm;
extern D_ParserTables parser_tables_gram;
//! from parser
extern ASTNode *treeRoot;

static std::string cur_path("");
static void SyntaxErrorFn(struct D_Parser *parser) {
    int line = parser->loc.line;
    int col = parser->loc.col;
    Message::setCurLine(line, cur_path);
    throw Message::Exception(Twine("Syntax error at column ") + Twine(col) + ".");
}

FileSource::FileSource(std::string baseDir) : BasePath(baseDir) {
    Parser = new_D_Parser(&parser_tables_gram, sizeof(ASTNode*));
    Parser->syntax_error_fn = &SyntaxErrorFn;
}

ASTNode *FileSource::parseFile(const std::string &path) {
    std::string parseFile = getFileContents(path);
    if (parseFile.empty()) {
        throw Message::Exception(Twine("Could not open file \"") + path + Twine("\"."));
    }
    cur_path = path;
    return parseString(parseFile);
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