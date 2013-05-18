%My name is Tristan Hume
%February 3, 2011
% --------------------
include "types.t"
include "matrices.t"

View.Set("graphics:800;800,offscreenonly")

    
var camera : cam
camera.cx := 0
camera.cy := 0
camera.cz := -21
camera.axy := 0
camera.axz := 0
camera.ayz := 0
camera.zoom := 1000

var viewMat1, viewMat2 : matrix
viewMat1 := translate(0,0,0)
viewMat2 := translate(0,0,0)

% returns if one 3d box is in another
fcn PtInBox3D(p : point3D, b : box) : boolean
    var inside : boolean := true
    
    % double check box format (optional)
    if b.p1.z > b.p2.z or b.p1.x > b.p2.x or b.p1.y > b.p2.y then
	put "bad format for box in PtInBox3D"
    end if
    
    %check front and back
    if p.z < b.p1.z or p.z > b.p2.z then
	inside := false
    end if
    %check top and bottom
    if p.y < b.p1.y or p.y > b.p2.y then
	inside := false
    end if
    %check sides
    if p.x < b.p1.x or p.x > b.p2.x then
	inside := false
    end if
    
    result inside
end PtInBox3D
fcn BoxForCube(c : cube) : box
    var b : box
    
    b.p1.x := c.p.x - c.s
    b.p1.y := c.p.y - c.s
    b.p1.z := c.p.z - c.s
    
    b.p2.x := c.p.x + c.s
    b.p2.y := c.p.y + c.s
    b.p2.z := c.p.z + c.s
    
    result b
end BoxForCube
function project( pi : point3D, z : real) : point2D
    var p := applyMatrix(pi,viewMat1)
    p := applyMatrix(p,viewMat2)
    var rp :point2D
    rp.x := (p.x - camera.cx) / ((p.z + 2 - camera.cz) / camera.zoom) + (maxx div 2)  
    rp.y := (p.y - camera.cy) / ((p.z + 2 - camera.cz) / camera.zoom) + (maxy div 2)
    
    result rp
end project

include "drawing2.t"

