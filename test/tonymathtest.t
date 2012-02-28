var testnum : int := 1
proc test (expr : boolean)
	if not expr then
		put "Test ",testnum," failed."
	end if
	testnum += 1
end test

test (1 + 1 = 2)
test (2 + 1 * 3 = 5)
test (9001 - 1 = 9000)
test (1 ~= 2)
test (1 not= 2)
test (3 > 2)
test (-1 < 1)
test (1 / -2 * -2 = 1)
test (1.0 = 1)
test (42 / 13.37 ~= 3.141592)
test (9001 mod 1 = 0)
test (9001 mod 500 = 1)
test (9002 mod 3 = 9002 mod 4 & 9002 mod 5 = 9002 mod 6)
test (true)
test ( ~false)
test (28901374209834720.0 / 28901374209834720.0 - 1 = 0)
test (0 = -0)
test (0.0 = -0.0)
var a : int
a := 5
test (a = 5)
test (a ** 2 = 25)
var b : real := 3.14
test (b ** 2 < 10)