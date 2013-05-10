#include <math.h>
#include <SFML/Window.hpp>

#include "TuringCommon/LibDefs.h"
#include "WindowManager.h"

extern "C" {
    void Turing_StdlibSFML_Window_Close(TInt id){
    	WinMan->closeWin(id);
    }
    void Turing_StdlibSFML_Window_Select(TInt id){
    	WinMan->setCurWin(id);
    }
    int Turing_StdlibSFML_Window_GetActive(){
    	return WinMan->curWinID();
    }
    int Turing_StdlibSFML_Window_Maxx(){
    	return WinMan->curWin()->Width;
    }
    int Turing_StdlibSFML_Window_Maxy(){
    	return WinMan->curWin()->Height;
    }
    void Turing_StdlibSFML_Window_Update(TInt id) {
        WinMan->updateWindow(id,true);
    }
    void Turing_StdlibSFML_Window_Set(TInt id, TString *format) {
        WinMan->setWinParams(id,format->strdata);
    }
    int Turing_StdlibSFML_Window_Open(TString *format) {
        return WinMan->newWin(format->strdata);
    }
}