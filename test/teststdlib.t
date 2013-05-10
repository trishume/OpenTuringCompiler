var stdLibTestNum : int := 1
proc TestStdlibFail(name : string)
	put "Stdlib test #",stdLibTestNum,": '",name,"' failed."
end TestStdlibFail
proc TestStdlib (expr : boolean, name : string)
	if not expr then
		TestStdlibFail(name)
	end if
	stdLibTestNum += 1
end TestStdlib
proc TestStdlibIntEqual (val, correct : int, name : string)
	if val ~= correct then
		TestStdlibFail(name)
		put "GIVEN:",val," CORRECT:",correct
	end if
	stdLibTestNum += 1
end TestStdlibIntEqual

% Time
var t := Time.Sec()
% TestStdlib(Time.Date() = Time.SecDate(t),"Time.Date and Time.SecDate")
TestStdlibIntEqual(Time.DateSec(Time.SecDate(t)),t, "Time.DateSec")

var year, month, day, weekday, hour, minute, second : int
var secTime := Time.PartsSec (1989, 12, 25, 9, 27, 0)
Time.SecParts(secTime, year, month, day, weekday, hour, minute, second)
TestStdlib(secTime ~= 1,"Time.PartsSec no error")
TestStdlibIntEqual(year,1989, "Time.PartsSec and Time.SecParts year")
TestStdlibIntEqual(month,12, "Time.PartsSec and Time.SecParts month")
TestStdlibIntEqual(day,25, "Time.PartsSec and Time.SecParts day")
TestStdlibIntEqual(hour,9, "Time.PartsSec and Time.SecParts hour")
TestStdlibIntEqual(minute,27, "Time.PartsSec and Time.SecParts minute")
TestStdlibIntEqual(second,0, "Time.PartsSec and Time.SecParts second")
TestStdlib(Time.SecStr(secTime,"%B %d, %Y at %I:%M") = "December 25, 1989 at 09:27", "Time.PartsSec and Time.SecParts date string")

% Typeconv

assert ceil(0.25) = 1
assert floor(1.2) = 1
assert round(5.5) = 6
assert floor (-8.43) = -9
assert ceil (-8.43) = -8
assert round (-8.43) = -8

assert intstr(-43) = "-43"
assert realstr(25.0,4) = "  25"
assert realstr(12.2,4) = "12.2"
assert frealstr (25.0, 5, 1) = " 25.0"

assert strint(intstr(5)) = 5
assert strreal("0.25 ") = 0.25
assert strreal(" -55.55") = -55.55
assert strreal("5") = 5.0
assert not strrealok("saoethu")
assert strrealok(" 55.0 ")
assert strrealok("55")

assert strint("-123") = -123
assert strint("  1  ") = 1
assert not strintok("asoethusao")
assert strintok(" 5 ")

assert ord(chr(5)) = 5

%assert min(5,2) = 2
%assert min(3,9) = 3
%assert max(10,3) = 10
%assert max(20, 4) = 20
