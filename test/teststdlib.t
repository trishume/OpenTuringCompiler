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
var time := Time.Sec()
% TestStdlib(Time.Date() = Time.SecDate(time),"Time.Date and Time.SecDate")
TestStdlibIntEqual(Time.DateSec(Time.SecDate(time)),time, "Time.DateSec")

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