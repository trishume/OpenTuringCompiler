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