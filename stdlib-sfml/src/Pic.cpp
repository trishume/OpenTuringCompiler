#include <SFML/Graphics/Image.hpp>
#include <SFML/Window.hpp>

#include "openTuringLibDefs.h"

#include "WindowManager.h"
#include "IDManager.h"

#define PIC_MODE_COPY 0
#define PIC_MODE_MERGE 1

struct TuringPic{
    TuringPic() : Image(sf::Image()), Width(0), Height(0) {}
    void reloadDimensions() {
        Width = Image.GetWidth();
        Height = Image.GetHeight();
    }
    sf::Image Image;
    TInt Width,Height;
};

static IDManager<TuringPic> Pics("Pic");
static sf::Color transparentColour(255,255,255);

extern "C" {
    /*TInt Turing_StdlibSFML_Pic_FileNew(TString *fileName) {
        TInt id = Pics.getNew();
        TuringPic *pic = Pics.get(id);
        pic->Image.LoadFromFile(fileName->strdata);
    }*/
    TInt Turing_StdlibSFML_Pic_New(TInt x1, TInt y1, TInt x2, TInt y2) {
        TInt id = Pics.getNew();
        TuringPic *pic = Pics.get(id);
        
        pic->Image.CopyScreen(WinMan->curWin()->Win,sf::IntRect(x1,y1,x2,y2));
        pic->Image.CreateMaskFromColor(transparentColour);
        pic->reloadDimensions();
        return id;
    }
    
    void Turing_StdlibSFML_Pic_Draw(TInt id, TInt x, TInt y, TInt mode) {
        TuringPic *pic = Pics.get(id);
        pic->Image.SetSmooth(false);
        pic->Image.Bind();
        
        if (mode == PIC_MODE_MERGE) {
            // enable alpha
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        }
        
        glColor4f(1.0,1.0,1.0,1.0);
        glBegin(GL_QUADS);
            // Top left
            glTexCoord2f(0.0, 0.0);
            glVertex2i(x, y);
            // Top right
            glTexCoord2f(1.0, 0.0);
            glVertex2i(x + pic->Width, y);
            // Bottom right
            glTexCoord2f(1.0, 1.0);
            glVertex2i(x + pic->Width, y + pic->Height);
            // Bottom left
            glTexCoord2f(0.0, 1.0);
            glVertex2i(x, y + pic->Height);
        glEnd();
        
        glDisable(GL_BLEND);
    }
    
    void Turing_StdlibSFML_Pic_Free(TInt id) {
        Pics.remove(id);
    }
}