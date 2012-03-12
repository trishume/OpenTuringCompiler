View.Set("offscreenonly")

type point : record
				x,y : int
			end record

var verts : array 1..3 of point
verts(1).x := 10
verts(1).y := 10
verts(2).x := 200
verts(2).y := 400
verts(3).x := 390
verts(3).y := 10

var p : point
p.x := 210
p.y := 110
var start := Time.Elapsed()
for i : 1..1000000
	var randomvert := Rand.Int(1,3)
	var v := verts(randomvert)
	p.x := (p.x + v.x) div 2
	p.y := (p.y + v.y) div 2
	Draw.Dot(p.x,p.y,7)
end for
View.Update()
% takes 4395ms in turing 1.1
put "Done drawing. Took ", Time.Elapsed() - start, "ms"
delay(6000)