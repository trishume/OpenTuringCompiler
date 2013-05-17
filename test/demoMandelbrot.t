var s, a, b, c, d, e, f, g, h, j : real := 300
fcn sa (var A : real, C : real) : boolean
    A := C
    result false
end sa
View.Set("offscreenonly,graphics:300;300")
for x : 1 .. round (s * s)
    exit when sa (b, x div s) or sa (a, x - b * s) or sa (d, 3.5 / s * b - 1.75) or sa (c, 3.5 / s * a - 1.75) or sa (g, d) or sa (h, c)
    for i : 1 .. 75
        exit when sa (j, i) or (c - s / 2) ** 2 + (d - s / 2) ** 2 > 230 ** 2 or i > 75 or sa (e, d * d - c * c + g) or sa (f, d * c + c * d + h) or sa (d, e) or sa (c, f)
    end for
    drawdot (round (b), round (a), 15 + round (j / 75 * 15))
end for 
View.Update