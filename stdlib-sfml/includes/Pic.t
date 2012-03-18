const picCopy := 0
const picMerge := 1
module Pic
    external "New" fcn Turing_StdlibSFML_Pic_New(x1, y1, x2, y2 : int) : int
    external "FileNew" fcn Turing_StdlibSFML_Pic_FileNew(fileName : string) : int
    external "Draw" proc Turing_StdlibSFML_Pic_Draw(picId, x, y, mode : int)
    external "Free" proc Turing_StdlibSFML_Pic_Free(picId : int)

    proc ScreenLoad(fileName : string, x, y, mode : int)
    	var pic := FileNew(fileName)
    	Draw(pic,x,y,mode)
    	Free(pic)
    end ScreenLoad
end Pic