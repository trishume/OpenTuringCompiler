//
//  ASTSource.h
//  TuringCompiler
//
//  Created by Tristan Hume on 12-02-20.
//  Copyright 2012 15 Norwich Way. All rights reserved.
//

#ifndef TuringCompiler_ASTSource_H
#define TuringCompiler_ASTSource_H

#include <string>

#include <dparse.h>

#include "ast.h"

class ASTSource {
public:
    //! \param baseDir the absolute path of the base directory that all paths are relative to. OS native With trailing slash.
    ASTSource(std::string baseDir);
    
    //! parses a file and returns the AST
    //! \param fileName the relative file path of the file. Turing form.
    //! \param includedFrom the relative path of the file to which fileName is relative. Turing form.
    //! \returns the parsed AST or NULL on parsing error
    ASTNode *parseFile(const std::string &fileName, const std::string &includedFrom);
    //! \returns the parsed AST or NULL on parsing error
    virtual ASTNode *parseString(const std::string &fileData, bool printAST = true);
    //! returns the folder part of a file path
    static std::string folderFromFilePath(const std::string &fileName);
protected:
    //! get the contents of a file.
    //! \param filePath absolute, os native path to the file.
    virtual std::string getFileContents(const std::string &filePath);
private:
    std::string BasePath;
    D_Parser *Parser;
};

#endif
