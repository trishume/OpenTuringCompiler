module Draw
    external "Cls" proc Turing_StdlibSFML_Draw_Cls()

	external "Dot" proc Turing_StdlibSFML_Draw_Dot(x,y,colour : int)
    external "Line" proc Turing_StdlibSFML_Draw_Line(x1,y1,x2,y2,colour : int)
    external "ThickLine" proc Turing_StdlibSFML_Draw_ThickLine(x1,y1,x2,y2,width,colour : int)

    external "Polygon" proc Turing_StdlibSFML_Draw_Polygon(x, y : array 1 .. * of int, n : int,
colourNum : int)
    external "FillPolygon" proc Turing_StdlibSFML_Draw_FillPolygon(x, y : array 1 .. * of int, n : int,
colourNum : int)

    external "Box" proc Turing_StdlibSFML_Draw_Box(x1,y1,x2,y2,colour : int)
    external "FillBox" proc Turing_StdlibSFML_Draw_FillBox(x1,y1,x2,y2,colour : int)

    external "Oval" proc Turing_StdlibSFML_Draw_Oval(x,y,rx,ry,colour : int)
    external "FillOval" proc Turing_StdlibSFML_Draw_FillOval(x,y,rx,ry,colour : int)

    external "Arc" proc Turing_StdlibSFML_Draw_Arc(x,y,rx,ry,initialAngle,finalAngle,colour : int)
    external "FillArc" proc Turing_StdlibSFML_Draw_FillArc(x,y,rx,ry,initialAngle,finalAngle,colour : int)
end Draw