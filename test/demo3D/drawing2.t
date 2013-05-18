%stores current colour
var cc : int := red
proc SetColour(c : int)
    cc := c
end SetColour

function SetArray(p1,p2 : int) : array 1..2 of int
    var a : array 1..2 of int
    a (1) := p1
    a (2) := p2
    result a
end SetArray
proc DrawFastCube3D(c : cube)
    if c.p.z > camera.cz then
    var pt1 : point2D := project(initP3D(c.p.x - c.s,c.p.y + c.s,c.p.z - c.s),256)
    var pt2 : point2D := project(initP3D(c.p.x + c.s,c.p.y + c.s,c.p.z - c.s),256)
    var pt3 : point2D := project(initP3D(c.p.x + c.s,c.p.y - c.s,c.p.z - c.s),256)
    var pt4 : point2D := project(initP3D(c.p.x - c.s,c.p.y - c.s,c.p.z - c.s),256)

    var pt5 : point2D := project(initP3D(c.p.x - c.s,c.p.y + c.s,c.p.z + c.s),256)
    var pt6 : point2D := project(initP3D(c.p.x + c.s,c.p.y + c.s,c.p.z + c.s),256)
    var pt7 : point2D := project(initP3D(c.p.x + c.s,c.p.y - c.s,c.p.z + c.s),256)
    var pt8 : point2D := project(initP3D(c.p.x - c.s,c.p.y - c.s,c.p.z + c.s),256)
    %round coords
    var pt1c : array 1..2 of int := SetArray(round(pt1.x),round(pt1.y))
    var pt2c : array 1..2 of int := SetArray(round(pt2.x),round(pt2.y))
    var pt3c : array 1..2 of int := SetArray(round(pt3.x),round(pt3.y))
    var pt4c : array 1..2 of int := SetArray(round(pt4.x),round(pt4.y))

    var pt5c : array 1..2 of int := SetArray(round(pt5.x),round(pt5.y))
    var pt6c : array 1..2 of int := SetArray(round(pt6.x),round(pt6.y))
    var pt7c : array 1..2 of int := SetArray(round(pt7.x),round(pt7.y))
    var pt8c : array 1..2 of int := SetArray(round(pt8.x),round(pt8.y))
    
    Draw.Line(pt1c (1),pt1c (2),pt2c (1),pt2c (2),cc)
    Draw.Line(pt2c (1),pt2c (2),pt3c (1),pt3c (2),cc)
    Draw.Line(pt3c (1),pt3c (2),pt4c (1),pt4c (2),cc)
    Draw.Line(pt4c (1),pt4c (2),pt1c (1),pt1c (2),cc)
    
    Draw.Line(pt5c (1),pt5c (2),pt6c (1),pt6c (2),cc)
    Draw.Line(pt6c (1),pt6c (2),pt7c (1),pt7c (2),cc)
    Draw.Line(pt7c (1),pt7c (2),pt8c (1),pt8c (2),cc)
    Draw.Line(pt8c (1),pt8c (2),pt5c (1),pt5c (2),cc)
    
    Draw.Line(pt1c (1),pt1c (2),pt5c (1),pt5c (2),cc)
    Draw.Line(pt2c (1),pt2c (2),pt6c (1),pt6c (2),cc)
    Draw.Line(pt3c (1),pt3c (2),pt7c (1),pt7c (2),cc)
    Draw.Line(pt4c (1),pt4c (2),pt8c (1),pt8c (2),cc)
    end if
end DrawFastCube3D
proc drawLine3D(p1, p2: point3D)
    var dp1 : point2D := project(p1,256)
    var dp2 : point2D := project(p2,256)
    Draw.Line(round(dp1.x),round(dp1.y),round(dp2.x),round(dp2.y),cc)
end drawLine3D
proc DrawQuad3D(p1, p2, p3, p4: point3D)
    %if (p1.z > camera.cz) and (p2.z > camera.cz) and (p3.z > camera.cz) and (p4.z > camera.cz) then
    var dp1 : point2D := project(p1,256)
    var dp2 : point2D := project(p2,256)
    var dp3 : point2D := project(p3,256)
    var dp4 : point2D := project(p4,256)
    Draw.Line(round(dp1.x),round(dp1.y),round(dp2.x),round(dp2.y),cc)
    Draw.Line(round(dp2.x),round(dp2.y),round(dp3.x),round(dp3.y),cc)
    Draw.Line(round(dp3.x),round(dp3.y),round(dp4.x),round(dp4.y),cc)
    Draw.Line(round(dp4.x),round(dp4.y),round(dp1.x),round(dp1.y),cc)
    %end if
end DrawQuad3D
proc DrawOctolateral3D(p1, p2, p3, p4, p5, p6, p7, p8: point3D)
    if (p1.z > camera.cz) and (p2.z > camera.cz) and (p3.z > camera.cz) and (p4.z > camera.cz) and (p5.z > camera.cz) and (p6.z > camera.cz) and (p7.z > camera.cz) and (p8.z > camera.cz) then
	DrawQuad3D(p1,p2,p3,p4)
	DrawQuad3D(p1,p2,p6,p5)
	DrawQuad3D(p3,p4,p8,p7)
	DrawQuad3D(p2,p3,p7,p6)
	DrawQuad3D(p1,p4,p8,p5)
	DrawQuad3D(p5,p6,p7,p8)
    end if
end DrawOctolateral3D
proc DrawBox3D(b : box)
    var pt1 : point3D := initP3D(b.p1.x,b.p2.y,b.p1.z)
    var pt2 : point3D := initP3D(b.p2.x,b.p2.y,b.p1.z)
    var pt3 : point3D := initP3D(b.p1.x,b.p1.y,b.p1.z)
    var pt4 : point3D := initP3D(b.p2.x,b.p1.y,b.p1.z)

    var pt5 : point3D := initP3D(b.p1.x,b.p2.y,b.p2.z)
    var pt6 : point3D := initP3D(b.p2.x,b.p2.y,b.p2.z)
    var pt7 : point3D := initP3D(b.p1.x,b.p1.y,b.p2.z)
    var pt8 : point3D := initP3D(b.p2.x,b.p1.y,b.p2.z)
    
    DrawOctolateral3D(pt1,pt2,pt3,pt4,pt5,pt6,pt7,pt8)
end DrawBox3D
proc DrawCube3D(c : cube)
    var pt1 : point3D := initP3D(c.p.x - c.s,c.p.y + c.s,c.p.z - c.s)
    var pt2 : point3D := initP3D(c.p.x + c.s,c.p.y + c.s,c.p.z - c.s)
    var pt3 : point3D := initP3D(c.p.x + c.s,c.p.y - c.s,c.p.z - c.s)
    var pt4 : point3D := initP3D(c.p.x - c.s,c.p.y - c.s,c.p.z - c.s)

    var pt5 : point3D := initP3D(c.p.x - c.s,c.p.y + c.s,c.p.z + c.s)
    var pt6 : point3D := initP3D(c.p.x + c.s,c.p.y + c.s,c.p.z + c.s)
    var pt7 : point3D := initP3D(c.p.x + c.s,c.p.y - c.s,c.p.z + c.s)
    var pt8 : point3D := initP3D(c.p.x - c.s,c.p.y - c.s,c.p.z + c.s)
    
    DrawOctolateral3D(pt1,pt2,pt3,pt4,pt5,pt6,pt7,pt8)
end DrawCube3D
