View.Set ("graphics:500,500,offscreenonly")
var xm := 200
var ym := 100
fcn round(num : real) : int
	result num div 1
end round
cls
for x : -18 .. 18
    for y : -18 .. 18
        Draw.FillOval(round(200*cosd(x*10)*sind(y*10)*cosd(xm)+200*cosd(y*10)*sind(xm))+500 div 2,round(200*sind(x*10)*sind(y*10)*cosd(ym)+(200*cosd(y*10)*cosd(xm)-200*cosd(x*10)*sind(y*10)*sind(xm))*sind(ym))+500 div 2,1,1,7)
    end for
end for
View.Update
delay(5000)
