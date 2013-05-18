function applyMatrix (p : point3D, m : matrix) : point3D

    var tx := (p.x * m (1, 1)) + (p.y * m (1, 2)) + (p.z * m (1, 3)) + m (1, 4)

    var ty := (p.x * m (2, 1)) + (p.y * m (2, 2)) + (p.z * m (2, 3)) + m (2, 4)

    var tz := (p.x * m (3, 1)) + (p.y * m (3, 2)) + (p.z * m (3, 3)) + m (3, 4)

    var p2 : point3D
    p2.x := tx
    p2.y := ty
    p2.z := tz
    result p2
end applymatrix
fcn translate (x, y, z : real) : matrix
    var m : matrix
    m (1, 1) := 1
    m (1, 2) := 0
    m (1, 3) := 0
    m (1, 4) := x
    m (2, 1) := 0
    m (2, 2) := 1
    m (2, 3) := 0
    m (2, 4) := y
    m (3, 1) := 0
    m (3, 2) := 0
    m (3, 3) := 1
    m (3, 4) := z
    result m
end translate

fcn scale (x, y, z : real) : matrix
    var m : matrix
    m (1, 1) := x
    m (1, 2) := 0
    m (1, 3) := 0
    m (1, 4) := 0
    m (2, 1) := 0
    m (2, 2) := y
    m (2, 3) := 0
    m (2, 4) := 0
    m (3, 1) := 0
    m (3, 2) := 0
    m (3, 3) := z
    m (3, 4) := 0
    result m
end scale


fcn rotateXY (a : real) : matrix
    var m : matrix
    m (1, 1) := cosd (a)
    m (1, 2) := -sind (a)
    m (1, 3) := 0
    m (1, 4) := 0
    m (2, 1) := sind (a)
    m (2, 2) := cosd (a)
    m (2, 3) := 0
    m (2, 4) := 0
    m (3, 1) := 0
    m (3, 2) := 0
    m (3, 3) := 1
    m (3, 4) := 0
    result m
end rotateXY



fcn rotateYZ (a : real) : matrix
    var m : matrix
    m (1, 1) := 1
    m (1, 2) := 0
    m (1, 3) := 0
    m (1, 4) := 0
    m (2, 1) := 0
    m (2, 2) := cosd (a)
    m (2, 3) := -sind (a)
    m (2, 4) := 0
    m (3, 1) := 0
    m (3, 2) := sind (a)
    m (3, 3) := cosd (a)
    m (3, 4) := 0
    result m
end rotateYZ


fcn rotateXZ (a : real) : matrix
    var m : matrix
    m (1, 1) := cosd (a)
    m (1, 2) := 0
    m (1, 3) := sind (a)
    m (1, 4) := 0
    m (2, 1) := 0
    m (2, 2) := 1
    m (2, 3) := 0
    m (2, 4) := 0
    m (3, 1) := -sind (a)
    m (3, 2) := 0
    m (3, 3) := cosd (a)
    m (3, 4) := 0
    result m
end rotateXZ
