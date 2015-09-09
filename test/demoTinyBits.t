View.Set("offscreenonly")
for m : 1..250
    for x : 1..maxx
        for y : 1..maxy
            Draw.Dot(x,y,(x or y) mod m)
        end for
    end for
    View.Update
    Time.Delay(40)
end for
