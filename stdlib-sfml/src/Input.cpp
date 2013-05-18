#include <SFML/Window.hpp>

#include "TuringCommon/LibDefs.h"
#include "TuringCommon/RuntimeError.h"
#include "WindowManager.h"

#define KEYDOWN_ARR_SIZE 250

static int MyGetSpecialCode(sf::Key::Code code) {
    switch (code) {
        case sf::Key::Up:
            return 200;
        case sf::Key::Down:
            return 201;
        case sf::Key::Left:
            return 202;
        case sf::Key::Right:
            return 203;
            
        default:
            break;
    }
    return 0;
}

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
    void Turing_StdlibSFML_Input_KeyDown(TIntArray *arr) {
        // check array
        if (arr->length < KEYDOWN_ARR_SIZE) {
            TuringCommon::runtimeError("Array passed to Input.Keydown is too small");
        }
        // get keys
        WinMan->surface();
    	TuringWindow *win = WinMan->curWin();
        // fill with zeros
        for (int i=0; i < KEYDOWN_ARR_SIZE; ++i) {
            arr->data[i] = 0;
        }
        //parse and set ones
        std::set<sf::Key::Code>::iterator it;
        for (it = win->KeysDown.begin(); it != win->KeysDown.end(); ++it) {
            sf::Key::Code code = *it;
            if (code < KEYDOWN_ARR_SIZE) {
                arr->data[code-1] = 1; // code-1 because turing 1-indexing
            } else {
                int special = MyGetSpecialCode(code);
                if (special) {
                    arr->data[special-1] = 1;
                }
            }
        }
    }
}