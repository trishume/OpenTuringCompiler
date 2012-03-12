#include <math.h>
#include <SFML/Window.hpp>

#include "openTuringLibDefs.h"
#include "WindowManager.h"
#include "RGB.h"

extern "C" {
    void Turing_StdlibSFML_Draw_Dot(TInt x, TInt y, TInt colour) {
        glColor3bv((GLbyte *)getRGBColourFromNum(colour));
        
        glBegin(GL_POINTS);
        glVertex2i(x, y);
        glEnd();
        
        WinMan->updateCurWin();
	}
    void Turing_StdlibSFML_Draw_Line(TInt x1, TInt y1, TInt x2, TInt y2, TInt colour) {
        glColor3bv((GLbyte *)getRGBColourFromNum(colour));
        
        glBegin(GL_LINES);
        glVertex2i(x1, y1);
        glVertex2i(x2, y2);
        glEnd();
        
        WinMan->updateCurWin();
	}
    void Turing_StdlibSFML_Draw_Box(TInt x1, TInt y1, TInt x2, TInt y2, TInt colour) {
        glColor3bv((GLbyte *)getRGBColourFromNum(colour));
        
        glBegin(GL_LINE_LOOP);
        glVertex2i(x1, y1); glVertex2i(x2, y1); 
        glVertex2i(x2, y2); glVertex2i(x1, y2);
        glEnd();
        
        WinMan->updateCurWin();
	}    
    void Turing_StdlibSFML_Draw_FillBox(TInt x1, TInt y1, TInt x2, TInt y2, TInt colour) {
        glColor3bv((GLbyte *)getRGBColourFromNum(colour));
        
        glBegin(GL_QUADS);
        glVertex2i(x1, y1); glVertex2i(x2, y1); 
        glVertex2i(x2, y2); glVertex2i(x1, y2);
        glEnd();
        
        WinMan->updateCurWin();
	}
}