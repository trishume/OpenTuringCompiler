external "delay" proc Turing_StdlibSFML_Time_Delay(time : int)

module Time
	external "Elapsed" fcn Turing_StdlibSFML_Time_Elapsed() : int
	external "ElapsedCPU" fcn Turing_StdlibSFML_Time_Elapsed() : int
	external "DelaySinceLast" proc Turing_StdlibSFML_Time_DelaySinceLast(time : int)
	proc Delay(time : int)
		delay(time)
	end Delay
end Time

proc clock(var t : int)
	t := Time.Elapsed
end clock

proc sysclock(var t : int)
	t := Time.Elapsed
end sysclock