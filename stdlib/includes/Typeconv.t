% Type conversion functions

%
% Convert the given integer value to a real.
%
function intreal (i : int) : real
    result i % type promotion does the rest
end intreal
%
% Determine the nearest integer to the given value.
%
external "round" function Turing_Stdlib_Typeconv_Round (r : real) : int

%
% Determine the largest integer less than or equal to the given real value.
%
external "floor" function Turing_Stdlib_Typeconv_Floor (r : real) : int

%
% Determine the smallest integer greater than or equal to the given real value.
%
external "ceil" function Turing_Stdlib_Typeconv_Ceil (r : real) : int

%
% Convert the given real value to a string in the format specified.
%
external "realstr" function Turing_Stdlib_Typeconv_Realstr (r : real, width : int) : string

external "intstr" function Turing_Stdlib_Typeconv_Intstr (v : int) : string

%
% Convert the given real value to a string in the format specified.
%
%external "typeconv_erealstr" function erealstr (r : real, width, fractionWidth, exponentWidth : int) : string

%
% Convert the given real value to a string in the format specified.
%
external "frealstr" fcn Turing_Stdlib_Typeconv_FRealstr (r : real, width, fracWidth : int) : string

%
% Convert the given string to a real value.
%
external "strreal" function Turing_Stdlib_Typeconv_Strreal (s : string) : real

external "strint" function Turing_Stdlib_Typeconv_Strint (str : string) : int

/* Type conversion tests */

%
% Return true if strint would succeed on string
%
external "strintok" function Turing_Stdlib_Typeconv_Strintok (str: string): boolean

%
% Return true if strreal would succeed on string
%
external "strrealok" function Turing_Stdlib_Typeconv_Strrealok (str: string): boolean

/* ASCII conversion */
external "ord" fcn Turing_Stdlib_Typeconv_Ord(c : char) : int
external "chr" fcn Turing_Stdlib_Typeconv_Chr(c : int) : char
