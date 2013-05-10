#ifndef _WINMANAGER_H_
#define _WINMANAGER_H_

#include <map>
#include <string>
#include <set>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>

#include "TuringCommon/LibDefs.h"
#include "TuringCommon/IDManager.h"

#define DEFAULT_WINDOW_WIDTH 600
#define DEFAULT_WINDOW_HEIGHT 500
#define DEFAULT_WINDOW_TITLE "Open Turing Run Window"

struct TuringWindow {
    TuringWindow() :  Title(DEFAULT_WINDOW_TITLE), Width(DEFAULT_WINDOW_WIDTH),
    Height(DEFAULT_WINDOW_HEIGHT),PutLine(1),OffScreenOnly(false),
    MouseX(0), MouseY(0), KeysDown() {}
    
    ~TuringWindow() {
        Win.Close();
    }
    
    void buttonEvent(sf::Mouse::Button b, bool state) {
        switch (b) {
            case sf::Mouse::Left:
                Left = state;
                break;
            case sf::Mouse::Middle:
                Middle = state;
                break;
            case sf::Mouse::Right:
                Right = state;
                break;
            default:
                break;
        }
    }
    
    void keyEvent(sf::Key::Code k, bool state) {
        if (state) {
            KeysDown.insert(k);
        } else {
            KeysDown.erase(k);
        }
    }
    
    
    sf::RenderWindow Win;
    
    std::string Title;
    TInt Width,Height;
    int PutLine;
    
    bool OffScreenOnly;
    // Event attributes
    TInt MouseX, MouseY;
    bool Left,Middle,Right;
    std::set<sf::Key::Code> KeysDown;
};

class WindowManager {
public:
    WindowManager();
    ~WindowManager();
    TuringWindow *curWin();
    TuringWindow *getWin(TInt winId);
    void setCurWin(TInt winId);
    TInt curWinID();
    
    // life cycle
    TInt newWin(const std::string &params);
    void closeWin(TInt winId);
    
    void setWinParams(TInt winId, const std::string &params);
    void setupOpenGL(TuringWindow *win);
    
    //! this is called after every draw function with the force parameter false.
    //! \param force    wether to draw the buffer even if the window is not set to offscreenonly.
    //!                 force makes it update the buffer even if offscreenonly is set. Used for View.Update
    void updateWindow(TInt winId, bool force);
    //! convenience method to call after every draw.
    //! equivelant to updateWindow(curWinID(),false); but faster
    void updateCurWin();
    void clearWin(TInt winId);
    
    //! this can be called to check events and stop the system
    //! from thinking the app is frozen.
    void surface();
    
    //! the id of the main window
    TInt MainWin;
protected:
    void doWinUpdate(TuringWindow *win);
    
    TuringCommon::IDManager<TuringWindow> Windows;
    TInt CurWin;
    sf::WindowSettings Settings;
    sf::Sprite BufferSprite;
    
private:
    static void split(std::vector<std::string>& lst, const std::string& input, const std::string& separators, bool remove_empty = true);
};

void Turing_StdlibSFML_Window_Init();
void Turing_StdlibSFML_Window_Cleanup();
extern WindowManager *WinMan; // main global instance

#endif