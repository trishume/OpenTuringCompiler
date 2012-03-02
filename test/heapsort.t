const ar_size : int := 1000000
var a : array 1 .. ar_size of int

external proc srand(s : int)
external fcn rand() : int

var heapsize : int
proc max_heapify (i : int)
var l : int := i * 2
var r : int := l + 1
var largest : int
if (l <= heapsize & a (l) > a (i)) then
largest := l
else
largest := i
end if
if (r <= heapsize & a (r) > a (largest)) then
largest := r
end if
if (largest ~= i) then
a (largest) := a (largest) xor a (i)
a (i) := a (largest) xor a (i)
a (largest) := a (largest) xor a (i)
end if
end max_heapify

proc build_max_heap
	heapsize := ar_size
	var i := ar_size div 2
	loop
		exit when i < 1
		max_heapify (i)
		i -= 1
	end loop
end build_max_heap

proc heap_sort
	build_max_heap
	var i := ar_size
	loop
		exit when i < 2
		a (i) := a (i) xor a (1)
		a (1) := a (i) xor a (1)
		a (i) := a (i) xor a (1)
		max_heapify (1)
		i -= 1
	end loop
end heap_sort

srand(456)
for i : 1 .. ar_size
a (i) := rand()
end for

put "start"
heap_sort
put a (upper(a))