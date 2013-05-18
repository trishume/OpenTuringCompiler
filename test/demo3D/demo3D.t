include "3dlib.t"

type missile :
    record
	c : cube
	v : point3D % motion vector
	col : int %colour!!!
    end record

fcn InitMissile (o, v : point3D) : missile
    var m : missile
    m.c.p := o
    m.c.s := 0.5
    m.v := v
    m.col := Rand.Int (5, 200)
    result m
end InitMissile

fcn RandomMissile () : missile
    var r1, r2, r : int
    var v1, v2, v3 : real
    r := Rand.Int (1, 6)

    r1 := Rand.Int (0, 10) - 5
    r2 := Rand.Int (0, 10) - 5

    v1 := Rand.Real * 0.2 + 0.01
    v2 := (Rand.Real - 0.5) * 0.02
    v3 := (Rand.Real - 0.5) * 0.02

    if r = 1 then
	result InitMissile (initP3D (r1, r2, 9.9), initP3D (v3, v2, v1 * -1))
    elsif r = 2 then
	result InitMissile (initP3D (r1, 9.9, r2), initP3D (v3, v1 * -1, v2))
    elsif r = 3 then
	result InitMissile (initP3D (9.9, r1, r2), initP3D (v1 * -1, v3, v2))
    elsif r = 4 then
	result InitMissile (initP3D (r1, r2, -9.9), initP3D (v3, v2, v1))
    elsif r = 5 then
	result InitMissile (initP3D (r1, -9.9, r2), initP3D (v3, v1, v2))
    else
	result InitMissile (initP3D (-9.9, r1, r2), initP3D (v1, v3, v2))
    end if
end RandomMissile

proc MoveProjectile (var m : missile)
    m.c.p.x += m.v.x
    m.c.p.y += m.v.y
    m.c.p.z += m.v.z
end MoveProjectile

var pro : array 1 .. 40 of missile
for i : 1 .. upper (pro)
    pro (i) := RandomMissile ()
end for

%bounds
var bounds : cube := initCube3D(initP3D(0,0,0),10)
var boundingbox : box := BoxForCube(bounds)
% random box
var c : cube := initCube3D(initP3D(0,0,0),0.5)

var chars : array 1..char of int
var rx,ry : real
rx := 0
ry := 0
%world := GenWorld()
camera.cz -= 20.0
loop
    % handle input
    Input.KeyDown (chars)
    if chars ('a')=1 or chars ('s')=1 then
	    if chars ('a')=1 then
		camera.cz -= 0.3
	    else
		camera.cz += 0.3
	    end if
	end if
    if chars (KEY_RIGHT_ARROW) = 1 or chars (KEY_LEFT_ARROW) = 1 then
        if chars (KEY_LEFT_ARROW) = 1 then
            rx -= 0.6
        else % right arrow key
            rx += 0.6           
        end if
        viewMat1 := rotateXZ(rx)
    end if
    if chars (KEY_UP_ARROW) = 1 or chars (KEY_DOWN_ARROW) = 1 then
        if chars (KEY_UP_ARROW) = 1 then
            ry -= 0.6
        else % right arrow key
            ry += 0.6           
        end if
        viewMat2 := rotateYZ(ry)
    end if
    
    % iterate projectiles
	for i : 1 .. upper (pro)

	    pro (i).c.p.x += pro (i).v.x
	    pro (i).c.p.y += pro (i).v.y
	    pro (i).c.p.z += pro (i).v.z

	    if PtInBox3D (pro (i).c.p, boundingbox) then
		SetColour (pro (i).col)
		DrawFastCube3D (pro (i).c)
		SetColour (red)
	    else
		pro (i) := RandomMissile ()
	    end if
	end for
    
    DrawFastCube3D(bounds)
    
    Draw.FillBox(0,0,10,10,Rand.Int(2,30))
    View.Update
    Time.DelaySinceLast (10)
    cls
    exit when chars('q')=1
end loop
