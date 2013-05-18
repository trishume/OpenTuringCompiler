type point2D:
    record
	x,y : real
    end record
type point3D:
    record
	x,y,z : real
    end record
type cube:
    record
	p : point3D
	s : real
    end record
type box:
    record
	p1 : point3D
	p2 : point3D
    end record
type cam :
    record
	axy, axz, ayz, cx, cy, cz, zoom : real
    end record
type matrix : array 1 .. 3, 1 .. 4 of real

%-----------------------
function initP3D(x,y,z : real) : point3D
    var p : point3D
    p.x := x
    p.y := y
    p.z := z
    result p
end initP3D
function initCube3D(p : point3D,s : real) : cube
    var c : cube
    c.p := p
    c.s := s
    result c
end initCube3D
