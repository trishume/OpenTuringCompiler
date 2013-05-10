var maxx := 0
var maxy := 0

module Window
	external "Maxx" fcn Turing_StdlibSFML_Window_Maxx() : int
	external "Maxy" fcn Turing_StdlibSFML_Window_Maxy() : int
  proc UpdateDimensions
    maxx := Maxx()
    maxy := Maxy()
  end UpdateDimensions

	external "Update" proc Turing_StdlibSFML_Window_Update()

	external "SelectExt" proc Turing_StdlibSFML_Window_Select(id : int)
  proc Select(id : int)
    SelectExt(id)
    UpdateDimensions
  end Select
	external "CloseExt" proc Turing_StdlibSFML_Window_Close(id : int)
  proc Close(id : int)
    CloseExt(id)
    UpdateDimensions
  end Close

	external "SetExt" proc Turing_StdlibSFML_Window_Set(id : int, format : string)
  proc Set(id : int, format : string)
    SetExt(id, format)
    UpdateDimensions
  end Set
	external "OpenExt" fcn Turing_StdlibSFML_Window_New(format : string) : int
  fcn Open(format : string) : int
    var res := OpenExt(format)
    UpdateDimensions
    result res
  end Open
	external "GetActive" fcn Turing_StdlibSFML_Window_GetActive() : int
end Window

Window.UpdateDimensions
