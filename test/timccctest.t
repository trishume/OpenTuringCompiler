% problem 1

var limit, carSpeed : int
put "Enter the speed limit: "..
get limit
put "Enter the recorded speed of the car: "..
get carSpeed
if ((carSpeed - limit) < 1) then
put "Congratulations, you are within the speed limit!"
elsif ((carSpeed - limit) >= 1 and (carSpeed - limit) <= 20) then 
put "You are speeding and your fine is $100."
elsif ((carSpeed - limit) >= 21 and (carSpeed - limit) <= 30) then 
put "You are speeding and your fine is $270."
elsif ((carSpeed - limit) >= 31) then 
put "You are speeding and your fine is $500."
end if

% problem 2


var reading1, reading2, reading3, reading4 : int
get reading1 
get reading2 
get reading3 
get reading4 
if ((reading1 < reading2) and (reading2 < reading3) and (reading3 < reading4)) then
put "Fish Rising"
elsif ((reading1 > reading2) and (reading2 > reading3) and (reading3 > reading4)) then
put "Fish Diving"
elsif ((reading1 = reading2) and (reading2 = reading3) and (reading3 = reading4)) then
put "Constant Depth"
else 
put "No Fish"
end if

% problem 3
var input : int
get input

for i : 1..input
for x : 1..input
put "*"..
end for
for x : 1..input
put "X"..
end for
for x : 1..input
put "*"..
end for
put ""
end for

for i : 1..input
for x : 1..input
put " "..
end for
for x : 1..input
put "X"..
end for
for x : 1..input
put "X"..
end for
put ""
end for

for i : 1..input
for x : 1..input
put "*"..
end for
for x : 1..input
put " "..
end for
for x : 1..input
put "*"..
end for
put ""
end for

% problem 4
var decodedWord : string
var position : int
var K : int
var encodedWord : string
get K
get encodedWord 

function encodeLetter(letter : char) : char
var resultLetter : char
var shift : int
shift := 3 * position + K
if ((ord(letter)-64) + shift <= 26) then
resultLetter := chr(ord(letter) + shift)
elsif ((ord(letter)-64) + shift > 26) then
resultLetter := chr(ord(letter) + shift - 26)
end if
result resultLetter
end encodeLetter

function decode (encoded : char) : char
    var testLetter : char
    for i : 65..90
        if (encodeLetter(chr(i))=encoded) then
            testLetter := (chr(i))
        end if    
    end for
    result testLetter
end decode

decodedWord := ""
for i : 1..length(encodedWord)
position := i
decodedWord += decode(encodedWord(i))
end for
put decodedWord