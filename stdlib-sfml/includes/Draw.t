module Draw
	external "Dot" proc Turing_StdlibSFML_Draw_Dot(x,y,colour : int)
    external "Line" proc Turing_StdlibSFML_Draw_Line(x1,y1,x2,y2,colour : int)
    external "Box" proc Turing_StdlibSFML_Draw_Box(x1,y1,x2,y2,colour : int)
    external "FillBox" proc Turing_StdlibSFML_Draw_FillBox(x1,y1,x2,y2,colour : int)
end Draw