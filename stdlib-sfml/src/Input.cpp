#include <SFML/Window.hpp>

#include "TuringCommon/LibDefs.h"
#include "WindowManager.h"

extern "C" {
    void Turing_StdlibSFML_Input_MouseWhere(int *x, int *y, int *button) {
        WinMan->surface();
    	TuringWindow *win = WinMan->curWin();
        *x = win->MouseX;
        *y = win->Height - win->MouseY;
        
        *button = 0;
        if (win->Left) {
            *button += 1;
        }
        if (win->Middle) {
            *button += 10;
        }
        if (win->Right) {
            *button += 100;
        }
    }
}