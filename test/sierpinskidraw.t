View.Set("offscreenonly")

type point : record
				x,y : int
			end record

var verts : array 1..3 of point
verts(1).x := 10
verts(1).y := 10
verts(2).x := 250
verts(2).y := 400
verts(3).x := 390
verts(3).y := 10

var p : point
p.x := 210
p.y := 110
var start := Time.Elapsed
for i : 1..1000000
	unchecked
	var randomvert := Rand.Int(1,3)
	var v : point := verts(randomvert)
	p.x := (p.x + v.x) div 2
	p.y := (p.y + v.y) div 2
	Draw.Dot(p.x,p.y,7) % has to be reversed. Probably a bug in the program.
end for

% test drawing functions
Draw.Box(10,70,80,20,44)

Draw.FillBox(300,200,320,370,52)
Draw.Line(10,70,320,370,48)
Draw.ThickLine(100,100,10,450,6,9)

Draw.Oval(200,300,20,30,3)
Draw.FillOval(400,300,100,50,3)

Draw.Arc(200,300,40,40,50,100,5)
Draw.FillArc(400,400,100,50,0,180,5)

% test with an octagon with a concave side
var x := init (100, 100, 135, 185, 220, 220, 185, 135)
var y := init (100, 150, 185, 150, 150, 100, 65, 65)
Draw.FillPolygon (x, y, 8, 48)
Draw.Polygon (x, y, 8, 3)

for i : 1..300
	Draw.Dot(Rand.Int(400,450),Rand.Int(50,70),32)
end for

% draw open turing logo
%Pic.ScreenLoad("icon.jpg", 30, 450, picMerge)
% take a picture of part of the octagon
var picId := Pic.New(70,70,150,150)
Pic.Draw(picId,400,50,picMerge)
Pic.Free(picId)

View.Update
% takes 4395ms in open turing 1.1.0 alpha (on an amazing computer)
put "Done drawing. Took ", Time.Elapsed - start, "ms"
delay(5000)