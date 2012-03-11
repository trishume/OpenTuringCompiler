#include "WindowManager.h"

#include <sstream>
#include <algorithm>
#include <iterator>

#include "openturingRuntimeError.h"

#define DEF_WIN_PARAMS "graphics:600;500,title:Open Turing Run Window"


extern "C" {
    void Turing_StdlibSFML_Window_Init() {
        if (WinMan == NULL) {
            WinMan = new WindowManager();
        } else {
            turingRuntimeError("Window init called when there is already a window.");
        }
    }
    void Turing_StdlibSFML_Window_Cleanup() {
        if (WinMan == NULL) {
            delete WinMan;
            WinMan = NULL;
        } else {
            turingRuntimeError("Window cleanup when there is no window to clean up.");
        }
    }
}

WindowManager::WindowManager() : CurWin(0){
    TInt mainWin = newWin(DEF_WIN_PARAMS);
    setCurWin(mainWin);
}

WindowManager::~WindowManager() {
    for (unsigned int i = 0; i < Windows.size(); ++i) {
        TuringWindow *curWindow = Windows[i];
        if (curWindow != NULL) {
            curWindow->Win.Close();
            delete curWindow;
        }
    }
}

TuringWindow *WindowManager::curWin() {
    return Windows[CurWin];
}

TuringWindow *WindowManager::getWin(TInt winId) {
    assertWinExists(winId);
    return Windows[winId];
}

void WindowManager::setCurWin(TInt winId) {
    TuringWindow *win = getWin(winId);
    win->Win.SetActive(true);
    CurWin = winId;
}

TInt WindowManager::curWinID() {
    return CurWin;
}

bool WindowManager::winExists(TInt winId) {
    return winId >= 0 && winId < Windows.size() && Windows[winId] != NULL;
}

TInt WindowManager::newWin(const std::string &params) {
    TuringWindow *newWin = new TuringWindow();
    Windows.push_back(newWin);
    
    TInt id = Windows.size()-1;
    setWinParams(id, params);
    newWin->Win.UseVerticalSync(true);
    
    return id;
}

void WindowManager::closeWin(TInt winId) {
    if (winId == 0) {
        turingRuntimeError("Can't close the main window.");
    }
    TuringWindow *win = getWin(winId);
    Windows[winId] = NULL;
    delete win;
    
    // set the active window to the main one if we just closed it
    if (CurWin == winId) {
        CurWin = 0;
    }
}

// this uses strings and vectors heavily so it is OK to use 'using'
void WindowManager::setWinParams(TInt winId, const std::string &params) {
    TuringWindow* win = getWin(winId);
    std::vector<std::string> items;
    WindowManager::split(items, params, ",");
    
    std::vector<std::string>::const_iterator item = items.begin();
    for (; item != items.end(); ++item) {
        std::vector<std::string> parts;
        split(parts,*item, ":;"); // split by either
        if (parts.size() == 0) turingRuntimeError("Malformed window format specifier component.");
        
        std::string tagname = parts[0];
        
        if ((tagname.compare("graphics") == 0 || tagname.compare("screen") == 0) &&
            parts.size() == 3) { // graphics:x;y
            int x = atoi(parts[1].c_str());
            int y = atoi(parts[2].c_str());
            if (x < 1 || y < 1) {
                turingRuntimeError("Tried to create a window with negative or zero size");
            }
            win->Win.Create(sf::VideoMode(x,y), win->Title,0);
            win->Win.SetSize(x,y);
        } else if (tagname.compare("title") == 0 && parts.size() == 2) { // title:name
            std::string title = parts[1];
            win->Title = title;
            win->Win.Create(sf::VideoMode(win->Win.GetWidth(),win->Win.GetHeight()), title,0);
        } else if (tagname.compare("offscreenonly") == 0) {
            win->OffScreenOnly = true;
        } else if (tagname.compare("nooffscreenonly") == 0) {
            win->OffScreenOnly = false;
        } else {
            turingRuntimeError("Don't recognize window option. Might just not be implemented.",true); // true = warn
        }
    }
    
}

void WindowManager::updateWindow(TInt winId, bool force) {
    TuringWindow* win = getWin(winId);
    if (force || !win->OffScreenOnly) {
        win->Win.Show(true);
        win->Win.Display();
    }
}

#pragma mark Protected and Private Methods

void WindowManager::assertWinExists(TInt winId) {
    if (!winExists(winId)) {
        turingRuntimeError("Window ID does not exist.");
    }
}

void WindowManager::split(std::vector<std::string>& lst, const std::string& input, const std::string& separators, bool remove_empty)
{
    std::ostringstream word;
    for (size_t n = 0; n < input.size(); ++n)
    {
        if (std::string::npos == separators.find(input[n]))
            word << input[n];
        else
        {
            if (!word.str().empty() || !remove_empty)
                lst.push_back(word.str());
            word.str("");
        }
    }
    if (!word.str().empty() || !remove_empty)
        lst.push_back(word.str());
}
