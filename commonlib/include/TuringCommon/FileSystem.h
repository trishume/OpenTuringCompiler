//! various path manipulation routines

#ifndef _FILESYSTEM_H_
#define _FILESYSTEM_H_

#include <string>
namespace TuringCommon {
    //! \param fileName the relative file path of the file. Turing form.
    //! \param includedFrom the relative path of the file to which fileName is relative. Turing form.
    //! \returns the native absolute file path to the file
    std::string includeFilePath(const std::string &fileName, 
        const std::string &includedFrom, const std::string &basePath = "");
    //! returns the folder part of a file path, with trailing slash
    std::string folderFromFilePath(const std::string &fileName);
    
}

#endif