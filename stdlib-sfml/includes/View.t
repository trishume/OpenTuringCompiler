external "cls" proc Turing_StdlibSFML_View_Cls()
module View
	external "Update" proc Turing_StdlibSFML_View_Update()
    external "GetFPS" fcn Turing_StdlibSFML_View_GetFPS() : int
	external "SetExt" proc Turing_StdlibSFML_View_Set(format : string)
  proc Set(format : string)
    SetExt(format)
    Window.UpdateDimensions
  end Set
end View

proc setscreen(format : string)
  View.Set(format)
end setscreen

module RGB
    external "SetColor" proc Turing_StdlibSFML_RGB_SetColor(num : int, r,g,b : real)
    external "SetColour" proc Turing_StdlibSFML_RGB_SetColor(num : int, r,g,b : real)
end RGB
