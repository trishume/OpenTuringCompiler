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
    
    //! parses a file and returns the AST
    //! \param fileName the relative file path of the file. Turing form.
    //! \param includedFrom the relative path of the file to which fileName is relative. Turing form.
    //! \returns the parsed AST or NULL on parsing error
    ASTNode *parseFile(const std::string &fileName, const std::string &includedFrom);
    //! get the full path of a library to load
    //! \param libName  the name of the dynamic library, no prefixes or suffixes
    //!                 for example "stdlib"
    //! \param includedFrom the file in the same directory as the library
    //!                     from which it was included.
    //! \returns    the full library path, with proper extension. 
    //!             Or an empty string if it can not be found.
    std::string getLibraryPath(const std::string &libName, const std::string &includedFrom);
    //! \returns the parsed AST or NULL on parsing error
    virtual ASTNode *parseString(const std::string &fileData, bool printAST = true);
    //! \param fileName the relative file path of the file. Turing form.
    //! \param includedFrom the relative path of the file to which fileName is relative. Turing form.
    //! \returns the native absolute file path to the file
    std::string includeFilePath(const std::string &fileName, const std::string &includedFrom);
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
