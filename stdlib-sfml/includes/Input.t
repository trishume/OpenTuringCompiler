
module Mouse
    external "Where" proc Turing_StdlibSFML_Input_MouseWhere(var x, var y, var btn : int)
end Mouse

external "mousewhere" proc Turing_StdlibSFML_Input_MouseWhere(var x, var y, var btn : int)

module Input
    external "KeyDown" proc Turing_StdlibSFML_Input_KeyDown(var chars : array 1..char of int)
end Input

const KEY_UP_ARROW := 200
const KEY_DOWN_ARROW := 201
const KEY_LEFT_ARROW := 202
const KEY_RIGHT_ARROW := 203