View.Set ("offscreenonly,graphics:400;200")
var c, t := 0
const maxcolour := 255
for i : 1 .. maxcolour
    if i < 85 then
        RGB.SetColour (i, (sign (42.5 - i) + 1) / 2 + (sign (i - 42.5) + 1) / 2 * (1 - (i mod 42.5) / 42.5), (sign (42.5 - i) + 1) / 2 * (i mod 42.5 / 42.5) + (sign (i - 42.5) + 1) / 2, 0)
    elsif i < 170 then
        RGB.SetColour (i, 0, (sign (127.5 - i) + 1) / 2 + (sign (i - 127.5) + 1) / 2 * (1 - (i mod 42.5) / 42.5), (sign (127.5 - i) + 1) / 2 * ((i mod 42.5) / 42.5) + (sign (i - 127.5) + 1) / 2)
    else
        RGB.SetColour (i, (sign (212.5 - i) + 1) / 2 * ((i mod 42.5) / 42.5) + (sign (i - 212.5) + 1) / 2, 0, (sign (212.5 - i) + 1) / 2 + (sign (i - 212.5) + 1) / 2 * (1 - (i mod 42.5) / 42.5))
    end if
end for
loop
    t := (t + 20) mod 600 % adjust the 20 for speed/smoothness (factor of 400)
    for x : 1 .. 200 * 200
        c := round ((abs (300 - t) + 20) * ln (x mod 200 + 2) / ln (x div 200 + 2))
        drawdot (x mod 200, x div 200, max (1, min (c, maxcolour - 1)))
        drawdot (399 - x mod 200, x div 200, max (1, min (c, maxcolour - 1)))
    end for
    View.Update
end loop