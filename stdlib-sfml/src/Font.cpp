#include <SFML/Graphics.hpp>
#include <SFML/Graphics/String.hpp>
#include <cstring>
//#include <iostream>

#include "Font.h"
#include "TuringCommon/IDManager.h"
#include "WindowManager.h"
#include "RGB.h"

struct TuringFont {
    TuringFont() : font(), style(sf::String::Regular), size(12) {}
    sf::Font font;
    sf::String::Style style;
    int size;
};

static TuringCommon::IDManager<TuringFont> Fonts("Font");

static const int defFontID = 0;
static const int putPadding = 2;
static const int putLineHeight = 15;

extern "C" {
    TInt Turing_StdlibSFML_Font_New(TString *description) {
        TuringFont *newFont = new TuringFont();
        TInt id = Fonts.insertNew(newFont);
        
        // parse font string
        size_t length = strlen(description->strdata);
        char *buffer = new char[length+1];
        strcpy(buffer, description->strdata);
        char *fontName = strtok(buffer,":");
        char *sizeStr = strtok(NULL, ":");
        char *styleStr = strtok(NULL, ":");
        
        // convert size
        int size = 12;
        if(sizeStr != NULL) size = atoi(sizeStr);
        newFont->size = size;
        
        // convert font name
        std::string fontFile("lib/fonts/");
        if (fontName == NULL || strcmp("mono", fontName) == 0) {
            fontFile += "Courier New.ttf";
        } else if(strcmp("sans serif", fontName) == 0) {
            fontFile += "Arial.ttf";
        } else if(strcmp("serif", fontName) == 0) {
            fontFile += "Times New Roman.ttf";
        } else {
            fontFile += fontName;
            fontFile += ".ttf";
        }
        
        // convert font style
        if (styleStr != NULL) {
            // lower case
            for(int i = 0; styleStr[i]; i++)
                styleStr[i] = tolower(styleStr[i]);
            // find all attributes
            int style = 0;
            if(strstr(styleStr, "italic") != NULL) {
                style |= sf::String::Italic;
            }
            if(strstr(styleStr, "bold") != NULL) {
                style |= sf::String::Bold;
            }
            if(strstr(styleStr, "underline") != NULL) {
                style |= sf::String::Underlined;
            }
            newFont->style = (sf::String::Style)style;
        }
        
        
        // initialize font
        newFont->font.LoadFromFile(fontFile,size*2);
        return id;
    }
    
    void Turing_StdlibSFML_Font_Draw(TString *str, TInt x, TInt y, TInt fontId, TInt colour) {
        TuringFont *font = Fonts.get(fontId);
        TuringWindow *win = WinMan->curWin();
        sf::String text(str->strdata,font->font,font->size + 4.0);
        
        text.SetStyle(font->style);
        const char *rgb = getRGBColourFromNum(colour);
        text.SetColor(sf::Color(rgb[0],rgb[1],rgb[2]));
        
        // calculate position
        sf::FloatRect curRect = text.GetRect();
        text.Move(x + 0.5, win->Height - curRect.Bottom - y - 0.5);
        
        // draw it
        win->Win.Draw(text);
        WinMan->updateCurWin();
    }
    
    void Turing_StdlibSFML_Font_Free(TInt id) {
        Fonts.remove(id);
    }
}

void Turing_StdlibSFML_Font_Init() {
    TString descript;
    strcpy(descript.strdata, "mono:10");
    Turing_StdlibSFML_Font_New(&descript);
}

void Turing_StdlibSFML_Put_Line(const std::string &line) {
    TuringWindow *cur = WinMan->curWin();
    int y = cur->Height - putPadding - (cur->PutLine * putLineHeight);
    
    // reset to top on overflow
    if(y < 0) {
        WinMan->clearWin(WinMan->curWinID());
        y = cur->Height - putPadding - putLineHeight; // recalculate
    }
    
    TString draw;
    strcpy(draw.strdata, line.c_str());
    Turing_StdlibSFML_Font_Draw(&draw, putPadding, y, defFontID, 7); // 7 is black
    
    //std::cout << "puts: " << draw.strdata << " y: " << y << std::endl;
    
    cur->PutLine += 1;
}