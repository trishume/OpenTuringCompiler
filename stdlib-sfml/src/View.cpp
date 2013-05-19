#include <math.h>
#include <SFML/Window.hpp>

#include "TuringCommon/LibDefs.h"
#include "WindowManager.h"
#include "RGB.h"

extern "C" {
    void Turing_StdlibSFML_View_Cls() {
    	WinMan->clearWin(WinMan->curWinID());
    }
    void Turing_StdlibSFML_View_Update() {
        WinMan->updateWindow(WinMan->curWinID(),true);
    }
    int Turing_StdlibSFML_View_GetFPS() {
        return WinMan->curWin()->Fps;
    }
    void Turing_StdlibSFML_View_Set(TString *format) {
        WinMan->setWinParams(WinMan->curWinID(),format->strdata);
    }
}