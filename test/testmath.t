% tests expressions
assert 1 + 1 = 2
assert 2 + 1 * 3 = 5
assert 9001 - 1 = 9000
assert 1 ~= 2
assert 1 not= 2
assert 3 > 2
assert -1 < 1
assert 1 / -2 * -2 = 1
assert 1.0 = 1
assert 42 / 13.37 ~= 3.141592
assert 9001 mod 1 = 0
assert 9001 mod 500 = 1
assert 9002 mod 3 = 9002 mod 4 & 9002 mod 5 = 9002 mod 6
assert true
assert  ~false
assert 28901374209834720.0 / 28901374209834720.0 - 1 = 0
assert 0 = -0
assert 0.0 = -0.0
var a : int
a := 5
assert a = 5
assert a ** 2 = 25
var b : real := 3.14
assert b ** 2 < 10