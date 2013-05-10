//
//  FileSource.h
//  TuringCompiler
//
//  Created by Tristan Hume on 12-02-20.
//  Copyright 2012 15 Norwich Way. All rights reserved.
//

#ifndef TuringCompiler_FileSource_H
#define TuringCompiler_FileSource_H

#include <string>

#include <dparse.h>

#include "ast.h"

class FileSource {
public:
    //! \param baseDir the absolute path of the base directory that all paths are relative to. OS native With trailing slash.
    FileSource(std::string baseDir);
    virtual ~FileSource() {}
    
    //! parses a file and returns the AST
    //! \param fileName the absolute path to the file, os native.
    ASTNode *parseFile(const std::string &filePath);

    //! \returns the parsed AST or NULL on parsing error
    virtual ASTNode *parseString(const std::string &fileData, bool printAST = false);
protected:
    //! get the contents of a file.
    //! \param filePath absolute, os native path to the file.
    virtual std::string getFileContents(const std::string &filePath);
private:
    std::string BasePath;
    D_Parser *Parser;
};

#endif
