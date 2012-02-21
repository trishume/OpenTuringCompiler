#The New Open Turing Compiler
##by Tristan Hume

A compiler for Turing written using LLVM.

##Differences from the normal Turing *interpreter*
 * Fast, like C kind of fast.
 * Cross-platform
 * Compiler not dependent on an editor.
 * Code is commented C++ instead of incomprehensible C compiled from Turing
 	* This means it is easy to fiddle with the language, extend it, and use it

##Partial list of implemented features:
 * Variables & Types
 * Functions, Procedures and control structures
 * Arrays and strings
 * Modules
 * Logic and math

##To Do:
 * Records
 * Classes
 * Pointers
 * Reference (var) parameters
 * Link in Turing Standard library
 * Implement graphics

##Sample Program

Here is an example the compiler is capable of running:

```
% The 13 printing test
% As I develop the compiler I use the new features
% to print 13 in increasingly complex ways...

% test string length.
var bob := length("123456")

var lolArr,lolArr2 : array 1..13 of int

% test complex type returns
fcn RetPrintingStr () : string
    result "PRINTING "
end RetPrintingStr

var assignStr : string
assignStr := RetPrintingStr()

var hovering : real := 7.0
hovering := 0.1 * 3 / 3 + 0.9

% test modules
module Print13
    %test equality checking
    if bob = 6 and 9 ~= 7 and "bob" = "bob" and "lol" ~= "hi" and hovering = 1 then
        %test no newline
        put assignStr ..
        %test multi-expr
        put 13,"..."
    end if

    fcn CalcStuff(num1 : int, num2 : int) : int
        result num1 - num2**2
    end CalcStuff

    proc PutStuff(num : int)
        var bob : int := 2**num

        bob := CalcStuff(bob,2)
        bob div= 2

        if bob > 14 then
            return
        end if

        var ed := bob - 1    
        put ed
        return
        put 9
    end PutStuff
end Print13

fcn Second( arr : array 1..2 of int ) : int
    result arr(2)
end Second

for i : 1..upper(lolArr)
    lolArr(i) := i
end for
% lolArr = 1,2,3,4...


loop
    lolArr(2) += 1
    exit when lolArr(2) >= 4
end loop


% test implicit copy
lolArr2 := lolArr


% set bob to 5
bob := lolArr2(9) - Second(lolArr2)

if bob <= 5 or bob > 7 then
    var ed := 6
    Print13.PutStuff (ed) % this one returns early and never prints
    Print13.PutStuff (bob)
elsif 5 >= 5 and 6 div 2 < 3 then
    put 6
else
    put 5 >= 4
end if
 ```