module Time
	external "Sec" fcn Turing_StdlibSFML_Time_Sec() : int
	external "DateSec" fcn Turing_StdlibSFML_Time_DateSec(format : string) : int
	external "PartsSec" fcn Turing_StdlibSFML_Time_PartsSec(year, month, day, hour, minute, second : int) : int
	external "SecDate" fcn Turing_StdlibSFML_Time_SecDate(seconds : int) : string
	external "SecParts" proc Turing_StdlibSFML_Time_SecParts(sec : int, var year, var month, var day,
 	 	 	var dayOfWeek, var hour, var minute, var second : int)
	external "SecStr" fcn Turing_StdlibSFML_Time_SecStr(seconds : int, format : string) : string
	external "Date" fcn Turing_StdlibSFML_Time_Date() : string
end Time