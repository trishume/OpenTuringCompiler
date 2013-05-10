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

% old versions

external "drawdot" proc Turing_StdlibSFML_Draw_Dot(x,y,colour : int)
external "drawline" proc Turing_StdlibSFML_Draw_Line(x1,y1,x2,y2,colour : int)
external "drawthickline" proc Turing_StdlibSFML_Draw_ThickLine(x1,y1,x2,y2,width,colour : int)

external "drawpolygon" proc Turing_StdlibSFML_Draw_Polygon(x, y : array 1 .. * of int, n : int,
colourNum : int)
external "drawfillpolygon" proc Turing_StdlibSFML_Draw_FillPolygon(x, y : array 1 .. * of int, n : int,
colourNum : int)

external "drawbox" proc Turing_StdlibSFML_Draw_Box(x1,y1,x2,y2,colour : int)
external "drawfillbox" proc Turing_StdlibSFML_Draw_FillBox(x1,y1,x2,y2,colour : int)

external "drawoval" proc Turing_StdlibSFML_Draw_Oval(x,y,rx,ry,colour : int)
external "drawfilloval" proc Turing_StdlibSFML_Draw_FillOval(x,y,rx,ry,colour : int)

external "drawarc" proc Turing_StdlibSFML_Draw_Arc(x,y,rx,ry,initialAngle,finalAngle,colour : int)
external "drawfillarc" proc Turing_StdlibSFML_Draw_FillArc(x,y,rx,ry,initialAngle,finalAngle,colour : int)


% color constants
const white := 0
const blue := 1
const green := 2
const cyan := 3
const red := 4
const magenta := 5
const purple := magenta
const brown := 6
const black := 7
const gray := 8
const grey := gray
const brightblue := 9
const brightgreen := 10
const brightcyan := 11
const brightred := 12
const brightmagenta := 13
const brightpurple := brightmagenta
const yellow := 14
const darkgray := 15

const brightwhite := gray
const darkgrey := darkgray

const colorbg := white
const colourbg := colorbg
const colorfg := black
const colourfg := colorfg
