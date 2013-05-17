View.Set ("offscreenonly")
var x, y, z, rs : array 1 .. 100 of real
proc rotate (OriginX, OriginY : real, var secondpartX, var secondpartY : real, Rotaion : real)
secondpartY := OriginY - (((OriginY - secondpartY) * cosd (Rotaion)) - ((OriginX - secondpartX) * sind (Rotaion)))
secondpartX := OriginX - (((OriginX - secondpartX) * cosd (Rotaion)) + ((OriginY - secondpartY) * sind (Rotaion)))
end rotate
for i : 1 .. 100
x (i) := Rand.Int (-100, 100)
y (i) := Rand.Int (-100, 100)
z (i) := Rand.Int (101, 301)
end for
loop
for i : 2 .. 100
drawline (round (x (i - 1) / (z (i - 1) / 100)) + 320, round (y (i - 1) / (z (i - 1) / 100)) + 200, round (x (i) / (z (i) / 100)) + 320, round (y (i) / (z (i) / 100)) + 200, black)
rotate (0, 200.5, x (i), z (i), 1)
end for
View.Update
cls
end loop