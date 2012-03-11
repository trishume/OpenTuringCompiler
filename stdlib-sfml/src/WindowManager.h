#ifndef _CODEGEN_H_
#define _CODEGEN_H_

#include <vector>
#include <string>
#include <SFML/Graphics/RenderWindow.hpp>

#include "openTuringLibDefs.h"

struct TuringWindow {
    TuringWindow() : OffScreenOnly(false) {}
    ~TuringWindow() {
        Win.Close();
    }
    sf::RenderWindow Win;
    std::string Title;
    bool OffScreenOnly;
};

class WindowManager {
public:
    WindowManager();
    ~WindowManager();
    TuringWindow *curWin();
    TuringWindow *getWin(TInt winId);
    void setCurWin(TInt winId);
    TInt curWinID();
    bool winExists(TInt winId);
    
    // life cycle
    TInt newWin(const std::string &params);
    void closeWin(TInt winId);
    
    void setWinParams(TInt winId, const std::string &params);
    
    //! this is called after every draw function with the force parameter false.
    //! \param force    wether to draw the buffer even if the window is not set to offscreenonly.
    //!                 force makes it update the buffer even if offscreenonly is set. Used for View.Update
    void updateWindow(TInt winId, bool force);
    
protected:
    void assertWinExists(TInt winId);
    
    std::vector<TuringWindow *> Windows;
    TInt CurWin;
    
private:
    static void split(std::vector<std::string>& lst, const std::string& input, const std::string& separators, bool remove_empty = true);
};

WindowManager *WinMan = NULL; // main global instance

#endif