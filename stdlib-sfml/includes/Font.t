module Font
    external "New" fcn Turing_StdlibSFML_Font_New(desc : string) : int
    external "Draw" proc Turing_StdlibSFML_Font_Draw(txtStr : string, x, y, fontID, clr : int)
    external "Free" proc Turing_StdlibSFML_Font_Free(fontId : int)
end Font

const defFontID := 0
const defFontId := 0