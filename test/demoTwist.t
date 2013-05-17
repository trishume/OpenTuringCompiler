setscreen ("graphics:200;200,offscreenonly")
for spacing : 1 .. 901 by 2
    cls
    for i : 1 .. 1800 by spacing
        drawline (maxx div 2 + round (800 / sqrt (i) * cosd (i)), maxy div 2 + round (800 / sqrt (i) * sind (i)), maxx div 2 + round (800 / sqrt (i + spacing) * cosd (i + spacing)), maxy div 2 +
            round (800 / sqrt (i + spacing) * sind (i + spacing)), black)
        drawline (maxx div 2 - round (800 / sqrt (i) * cosd (i)), maxy div 2 - round (800 / sqrt (i) * sind (i)), maxx div 2 - round (800 / sqrt (i + spacing) * cosd (i + spacing)), maxy div 2 -
            round (800 / sqrt (i + spacing) * sind (i + spacing)), black)
        drawline (maxx div 2 + round (800 / sqrt (i) * sind (i)), maxy div 2 - round (800 / sqrt (i) * cosd (i)), maxx div 2 + round (800 / sqrt (i + spacing) * sind (i + spacing)), maxy div 2 -
            round (800 / sqrt (i + spacing) * cosd (i + spacing)), black)
        drawline (maxx div 2 - round (800 / sqrt (i) * sind (i)), maxy div 2 + round (800 / sqrt (i) * cosd (i)), maxx div 2 - round (800 / sqrt (i + spacing) * sind (i + spacing)), maxy div 2 +
            round (800 / sqrt (i + spacing) * cosd (i + spacing)), black)
        delay (2)
    end for
    View.Update
end for 