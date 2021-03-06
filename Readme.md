#The New Open Turing Compiler - by Tristan Hume

A compiler for Turing written using LLVM for code generation and Dparser for parsing.
10x faster than the most popular implementation *and* cross-platform.

Unfortunately abandoned because although I have the resources to debug a compiler (with all the common features) I don't have the time to implement the thousand library methods present in regular Turing (though I made an effort and this compiler implements hundreds). The other issue is that Turing is a learning language where the possibility of compiler bugs would be devastating to the confidence of newbies, and it takes years to fully debug a compiler.

Instead see [OpenTuring](http://tristan.hume.ca/openturing) my fork of Turing for Windows with some minor tune-ups.

## Screenshots
#### Test of drawing functions and computation
![drawing test](https://raw.githubusercontent.com/trishume/OpenTuringCompiler/master/screenshots/drawtest.png)
#### The Qt-based editor hooked up to the compiler
![editor](https://raw.githubusercontent.com/trishume/OpenTuringCompiler/master/screenshots/editor.png)
#### Stress-test of the OpengGL drawing and tight inner compiled loops doing 30,000 particles at 60fps
![particles](https://raw.githubusercontent.com/trishume/OpenTuringCompiler/master/screenshots/particles.png)

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
 * OpenGL Based Graphics
 * Records

####If you want to compile it yourself you have to:
 * Install the dependencies:
   * dparser from http://http://dparser.sourceforge.net/
   * LLVM from SVN head. It should work with regular 3.0 but I don't test it regularly http://llvm.org/
   * SFML 1.6 http://www.sfml-dev.org/
   * Cmake (so that you can build it)
 * Build it
   * Run 'cmake' and then 'make' (or figure out how to make it build a VC++ project or something)
 * Check the dist folder for the "compiler" executable and then run that, passing the file to compile as a parameter

Unfortunately it's very difficult to actually assemble all the dependencies, that's C++ for you.

##Sample Program

Here is an example the compiler is capable of running:

```

% The 13 printing test
% As I develop the compiler I use the new features
% to print 13 in increasingly complex ways...

% test constants
const threeConst := 3
% test parsing weird constants
const testConst := -6.90460016972063023e-05
const oneConst : real := 1

% test type declarations, records and multi-dimensional arrays
type recType :  record
                    mat : array 1..threeConst, 1..boolean of boolean
                    hovering : real
                end record

var rec,otherRec : recType

% test alternate multi-dimensional array index syntax
rec.mat(1,2) := true

% test string length and 'var' parameters
var bob : int
proc SetBob(var bob : int)
    %get bob
    bob := length("123456")
end SetBob
SetBob(bob)

% test case statements and string concatenation
var prinString := "P"
proc TestCaseStat(bob : int)
    case bob of
    label 7,9:
        prinString += "I"
    label 5:
        prinString += "R"
    label:
        prinString += "N"
    end case
end TestCaseStat
TestCaseStat(5)
TestCaseStat(9)
TestCaseStat(839)

% test complex type returns
fcn RetPrintingStr () : string
    result prinString + "TING "
end RetPrintingStr

var assignStr : string
assignStr := RetPrintingStr()

%test no parenthesis procedures
procedure SetHovering
    rec.hovering := 0.1 * threeConst / threeConst + 0.9 * oneConst
end SetHovering
SetHovering

% test modules
module Print13

    %test equality checking and code inside a module
    if bob = 6 and ~(-9 > 7) & "bob" = "bob" and "lol" ~= "hi" and rec.hovering = 1 and rec.mat(1)(2) then
        %test no newline
        put assignStr ..
        %test multi-expr
        put 13,"...\n" ..
    end if

    % test module variables
    var lolArr,lolArr2 : flexible array 1..0 of int

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

fcn Second( arr : array 1..* of int ) : int
    result arr(2)
end Second

% test weird lower bounds
var inittedArr : array 4..7 of int := init(5,6,13,9)
for i : 1..inittedArr(6)
    new Print13.lolArr, upper(Print13.lolArr) + 1
    Print13.lolArr(i) := i
end for
if upper(Print13.lolArr) ~= 13 or lower(inittedArr) ~= 4 then
    put "FAILED flexible arrays or array initialization"
end if
% lolArr = 1,2,3,4...


loop
    Print13.lolArr(2) += 1
    exit when Print13.lolArr(2) >= 4
end loop


% test implicit copy of arrays
new Print13.lolArr2, upper(Print13.lolArr)
Print13.lolArr2 := Print13.lolArr

% set bob to 5
bob := Print13.lolArr2(9) - Second(Print13.lolArr2)

% test implicit copy of records
otherRec := rec

% test passing and returning records
fcn RetAndPassRec(passRec : recType) : recType
    result passRec
end RetAndPassRec
rec := RetAndPassRec(otherRec)

%check record copy and test comparison
if rec ~= otherRec then
    put rec.hovering, " is not equal to ", otherRec.hovering, " after copy."
end if

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
