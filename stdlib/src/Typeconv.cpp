#include <math.h>
#include <stdio.h>
#include <cstdlib>
#include <string>
#include <sstream>

#include "TuringCommon/LibDefs.h"
#include "TuringCommon/RuntimeError.h"


//! C++ magic that converts a string into any type that streams support
template< typename T >
static inline T MyConvert(const char *str,const char *type)
{
    std::istringstream iss(str);
    T obj;
    
    iss >> std::ws >> obj >> std::ws;
    
    if(!iss.eof()) {
        std::ostringstream os;
        os << "Can't convert string \"" << str << "\" to type " << type << "."; 
        TuringCommon::runtimeError(os.str().c_str());
    }

    return obj; 
}
template< typename T >
static inline bool MyCanConvert(const char *str)
{
    std::istringstream iss(str);
    T obj;
    
    iss >> std::ws >> obj >> std::ws;
    
    if(!iss.eof()) {
        return false;
    }
    
    return true; 
}


extern "C" {
#pragma mark Real to Int
    TInt Turing_Stdlib_Typeconv_Floor(TReal val) {
        return floor(val);
    }
    TInt Turing_Stdlib_Typeconv_Ceil(TReal val) {
        return ceil(val);
    }
    TInt Turing_Stdlib_Typeconv_Round(TReal val) {
        return round(val);
    }
   
#pragma mark To String
    void Turing_Stdlib_Typeconv_Intstr(TString *buffer,TInt val) {
        sprintf(buffer->strdata,"%i",val);
    }
    void Turing_Stdlib_Typeconv_Realstr(TString *buffer,TReal val, TInt width) {
        sprintf(buffer->strdata,"%*g",width,val);
    }    
    void Turing_Stdlib_Typeconv_FRealstr(TString *buffer,TReal val, TInt width, TInt fracWidth) {
        sprintf(buffer->strdata,"%*.*f",width,fracWidth,val);
    }
    
#pragma mark From String
    TReal Turing_Stdlib_Typeconv_Strreal(TString *str) {
        return MyConvert<TReal>(str->strdata, "real");
    }
    bool Turing_Stdlib_Typeconv_Strrealok(TString *str) {
        return MyCanConvert<TReal>(str->strdata);
    }
    TInt Turing_Stdlib_Typeconv_Strint(TString *str) {
        return MyConvert<TInt>(str->strdata, "int");
    }
    bool Turing_Stdlib_Typeconv_Strintok(TString *str) {
        return MyCanConvert<TInt>(str->strdata);
    }
    
#pragma mark ASCII
    TInt Turing_Stdlib_Typeconv_Ord(char chr) {
        return chr;
    }
    char Turing_Stdlib_Typeconv_Chr(TInt chr) {
        if (chr > 255 || chr < 0) {
            TuringCommon::runtimeError("Character value out of range in 'chr'.");
        }
        return static_cast<char>(chr);
    }
}