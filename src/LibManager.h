#ifndef TuringCompiler_LibManager_H
#define TuringCompiler_LibManager_H

#include <string>
#include <set>

class LibManager {
public:
    LibManager() {}
    
    //! link a dynamic plugin library and initialize it
    //! \param libName  the name of the dynamic library, no prefixes or suffixes
    //!                 for example "stdlib"
    //! \param includedFrom the file in the same directory as the library
    //!                     from which it was included.
    //! \returns    false if an error occurs. 
    //!             Prints a Message::error with an error message.
    bool linkLibrary(const std::string &libName, const std::string &includedFrom);
protected:
    //! get the full path of a library to load
    //! \param libName  the name of the dynamic library, no prefixes or suffixes
    //!                 for example "stdlib"
    //! \param includedFrom the file in the same directory as the library
    //!                     from which it was included.
    //! \returns    the full library path, with proper extension. 
    //!             Or an empty string if it can not be found.
    std::string getLibraryPath(const std::string &libName, const std::string &includedFrom);
private:
    //! The set of linked libraries. Keep it to avoid the overhead of loading a library twice.
    std::set<std::string> Linked;
};

#endif