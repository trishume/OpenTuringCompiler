#include "TuringCommon/FileSystem.h"
#include <sstream>

namespace TuringCommon {
    std::string includeFilePath(const std::string &fileName, 
        const std::string &includedFrom, const std::string &basePath) {
        std::string includedFolder = folderFromFilePath(includedFrom);
        
        std::ostringstream os;
        os << basePath << includedFolder << fileName;
        return os.str();
    }

    std::string folderFromFilePath(const std::string &fileName) {
        size_t found = fileName.find_last_of("/\\");
        // if it is npos then there was no separator. Just a name. in wich case the folder is blank.
        return (found == std::string::npos ? "" : fileName.substr(0,found+1));
    }
}
