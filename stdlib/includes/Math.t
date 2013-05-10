% math includes. 

% Many things are included here but not implemented in the library.
% They actually reference math.h functions that are automatically linked in.

%
% Determine the arc tangent of the given value, with result in radians.
%
external "arctan" function atan (r : real) : real

%
% Determine the arc tangent of the given value, with result in degrees.
%
external "arctand" function Turing_Stdlib_Math_Arctand (r : real) : real

%
% Determine the cosine of the given angle in radians.
%
external "cos" function Turing_Stdlib_Math_Cos (r : real) : real

%
% Determine the cosine of the given angle in degrees.
%
external "cosd" function Turing_Stdlib_Math_Cosd (r : real) : real

%
% Determine the natural exponent (e ** r) of the given value.
%
external "exp" function Turing_Stdlib_Math_Exp (r : real) : real

%
% Determine the natural logarithm of the given value.
%
external "ln" function Turing_Stdlib_Math_Ln (r : real) : real

%
% Determine the sine of the given angle in radians.
%
external "sin" function Turing_Stdlib_Math_Sin (r : real) : real

%
% Determine the sine of the given angle in degrees.
%
external "sind" function Turing_Stdlib_Math_Sind (r : real) : real

%
% Determine the square root of the given value.
%
external "sqrt" function sqrt (r : real) : real

module Math
    % Constants
    const E : real := 2.71828182845904523536
    const PI : real := 3.14159265358979323846

    %
    % Calculate the distance between two points
    function Distance (x1, y1, x2, y2 : real) : real
	var dx : real := x1 - x2
	var dy : real := y1 - y2
	result sqrt (dx * dx + dy * dy)
    end Distance

    %
    % Calculate the distance between a point and a line segment
    %
    function DistancePointLine (px, py, x1, y1, x2, y2 : real) : real
    	var lineSize : real := Distance (x1, y1, x2, y2)
    	if lineSize = 0 then
    	    result Distance (px, py, x1, y1)
    	end if

    	var u : real := ((px - x1) * (x2 - x1) +
    	    (py - y1) * (y2 - y1)) / (lineSize * lineSize)

    	if u < 0.0 then
    	    result Distance (px, py, x1, y1)
    	elsif u > 1.0 then
    	    result Distance (px, py, x2, y2)
    	else
    	    var ix : real := x1 + u * (x2 - x1)
    	    var iy : real := y1 + u * (y2 - y1)
    	    result Distance (px, py, ix, iy)
    	end if
    end DistancePointLine
end Math

module Rand
    %
    % Generate a pseudo-random number from zero to one.
    %
    external "Real" function Turing_Stdlib_Rand_Real() : real

    %
    % Generate a pseudo-random integer in the given range.
    %
    external "Int" function Turing_Stdlib_Rand_Int (low, high : int) : int

    %
    % Generate a pseudo-random number from zero to one in the given sequence.
    %
    /*function Next (seq : 1 .. 10) : real
    external procedure rand_next (var r : real, seq : 1 .. 10)
    var r : real

    rand_next (r, seq)
    result r
    end Next*/

    %
    % Reset the pseudo-random number sequences to a pseudo-random state.
    %
    external "Randomize" procedure Turing_Stdlib_Rand_Randomize()

    %
    % Reset the specified pseudo-random number sequences to the given state.
    %
    % external "rand_seed" procedure Seed (seed : int, seq : 1 .. 10)

    % Sets the random seed
    external "Set" procedure Turing_Stdlib_Rand_Set (seed : int)

    procedure Reset
        Set (1337)
    end Reset

    Randomize
end Rand

proc rand(var r : real)
    r := Rand.Real
end rand
proc randint (var i : int, low, high : int)
    i := Rand.Int(low,high)
end randint
external "randomize" procedure Turing_Stdlib_Rand_Randomize()

/* Math function exceptions */
const excpRealOverflow := 51
const excpTrigArgumentTooLarge := 100

%
% Determine the arc sine of the given value, with result in radians.
%
function arcsin (r : real) : real
    if r = 1 then
        result Math.PI / 2
    elsif r = 0 then
        result 0
    elsif r = -1 then
        result - Math.PI / 2
    elsif r < -1 or r > 1 then
        quit < : excpTrigArgumentTooLarge
    elsif r < 0 then
        result - arctan (sqrt ((r * r) / (1 - r * r)))
    else
        result arctan (sqrt ((r * r) / (1 - r * r)))
    end if
end arcsin

%
% Determine the arc sine of the given value, with result in degrees.
%
function arcsind (r : real) : real
    if r = 1 then
        result 90
    elsif r = 0 then
        result 0
    elsif r = -1 then
        result - 90
    elsif r < -1 or r > 1 then
        quit < : excpTrigArgumentTooLarge
    elsif r < 0 then
        result - arctand (sqrt ((r * r) / (1 - r * r)))
    else
        result arctand (sqrt ((r * r) / (1 - r * r)))
    end if
end arcsind

%
% Determine the arc cosine of the given value, with result in radians.
%
function arccos (r : real) : real
    if r = 1 then
        result 0
    elsif r = 0 then
        result Math.PI / 2
    elsif r = -1 then
        result Math.PI
    elsif r < -1 or r > 1 then
        quit < : excpTrigArgumentTooLarge
    elsif r > 0 then
        result arctan (sqrt ((1 - r * r) / (r * r)))
    else
        result Math.PI - arctan (sqrt ((1 - r * r) / (r * r)))
    end if
end arccos

%
% Determine the arc cosine of the given value, with result in degrees.
%
function arccosd (r : real) : real
    if r = 1 then
        result 0
    elsif r = 0 then
        result 90
    elsif r = -1 then
        result 180
    elsif r < -1 or r > 1 then
        quit < : excpTrigArgumentTooLarge
    elsif r > 0 then
        result arctand (sqrt ((1 - r * r) / (r * r)))
    else
        result 180 - arctand (sqrt ((1 - r * r) / (r * r)))
    end if
end arccosd

%
% Determine whether the given value is positive, zero, or negative.
%
function sign (n : real) : int
    if n > 0 then
        result 1
    elsif n = 0 then
        result 0
    else
        result -1
    end if
end sign

%
% Determine the tangent of a given angle in radians
%
function tan (r : real) : real
    if cos (r) = 0 then
        quit < : excpRealOverflow
    else
        result sin (r) / cos (r)
    end if
end tan

%
% Determine the tangent of a given angle in degrees
%
function tand (r : real) : real
    if r mod 180 = 90 then
        quit < : excpRealOverflow
    else
        result sind (r) / cosd (r)
    end if
end tand