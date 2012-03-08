% number of iterations
%const its := 50000000 % currently takes around 162.85s
const its := 2000000 % currently takes around 6.6s and 69s in open turing

type Body : record
    x, y, z,
    vx, vy, vz,
    mass : real
  end record

const pi := 3.141592653589793
var solarMass := 4 * (pi**2)
const daysPerYear := 365.24

var b : array 1..5 of Body

var sun,jupiter,saturn,uranus,neptune : Body

sun.x     := 0
sun.y     := 0
sun.z    := 0
sun.vx    := 0
sun.vy    := 0
sun.vz    := 0
sun.mass  := solarMass
b(1) := sun

jupiter.x    :=  4.84143144246472090e00
jupiter.y    := -1.16032004402742839e00
jupiter.z    := -1.03622044471123109e-01
jupiter.vx   :=  1.66007664274403694e-03 * daysPerYear
jupiter.vy   :=  7.69901118419740425e-03 * daysPerYear
jupiter.vz   := -6.90460016972063023e-05 * daysPerYear
jupiter.mass :=  9.54791938424326609e-04 * solarMass
b(2) := jupiter

saturn.x   :=  8.34336671824457987e00
saturn.y   :=  4.12479856412430479e00
saturn.z   := -4.03523417114321381e-01
saturn.vx  := -2.76742510726862411e-03 * daysPerYear
saturn.vy  :=  4.99852801234917238e-03 * daysPerYear
saturn.vz  :=  2.30417297573763929e-05 * daysPerYear
saturn.mass :=  2.85885980666130812e-04 * solarMass
b(3) := saturn

uranus.x    :=  1.28943695621391310e01
uranus.y    := -1.51111514016986312e01
uranus.z    := -2.23307578892655734e-01
uranus.vx   :=  2.96460137564761618e-03 * daysPerYear
uranus.vy   :=  2.37847173959480950e-03 * daysPerYear
uranus.vz   := -2.96589568540237556e-05 * daysPerYear
uranus.mass :=  4.36624404335156298e-05 * solarMass
b(4) := uranus

neptune.x    :=  1.53796971148509165e01
neptune.y    := -2.59193146099879641e01
neptune.z    :=  1.79258772950371181e-01
neptune.vx   :=  2.68067772490389322e-03 * daysPerYear
neptune.vy   :=  1.62824170038242295e-03 * daysPerYear
neptune.vz   := -9.51592254519715870e-05 * daysPerYear
neptune.mass :=  5.15138902046611451e-05 * solarMass
b(5) := neptune

procedure OffsetMomentum
  var px,py,pz : real := 0.0
  var i : int

  for i : 2..upper(b)
      px := px - b(i).vx * b(i).mass
      py := py - b(i).vy * b(i).mass
      pz := pz - b(i).vz * b(i).mass
  end for
  b(1).vx := px / solarMass
  b(1).vy := py / solarMass
  b(1).vz := pz / solarMass
end OffsetMomentum

function distance(i,j : int) : real
  result sqrt((b(i).x-b(j).x)**2 + ((b(i).y-b(j).y)**2) + (b(i).z-b(j).z)**2)
end distance

function energy() : real
  var i,j : int
  var res : real
  res := 0.0
  for i : 1..upper(b)
      res := res + b(i).mass * ((b(i).vx)**2 + (b(i).vy)**2 + (b(i).vz)**2) / 2
      for j : i+1..upper(b)
        res := res - b(i).mass * b(j).mass / distance(i,j)
      end for
  end for
  result res
end energy

procedure advance(dt : real)
  var i,j : int
  var dx,dy,dz,mag : real
  var bi,bj : int
  bi := 1
  for i : 1..upper(b)-1
    bj := bi
    var bbi := b(bi)
    for j : i+1..upper(b)
      bj += 1
      var bbj := b(bj)
      dx := bbi.x - bbj.x
      dy := bbi.y - bbj.y
      dz := bbi.z - bbj.z
      mag := dt / (sqrt((dx**2)+(dy**2)+(dz**2))*((dx**2)+(dy**2)+(dz**2)))
      b(bi).vx := bbi.vx - dx * bbj.mass * mag
      b(bi).vy := bbi.vy - dy * bbj.mass * mag
      b(bi).vz := bbi.vz - dz * bbj.mass * mag
      b(bj).vx := bbj.vx + dx * bbi.mass * mag
      b(bj).vy := bbj.vy + dy * bbi.mass * mag
      b(bj).vz := bbj.vz + dz * bbi.mass * mag
    end for
    bi += 1
  end for
  bi := 1
  for i : 1..upper(b)
    var bbi := b(bi)
    b(bi).x := bbi.x + dt * bbi.vx
    b(bi).y := bbi.y + dt * bbi.vy
    b(bi).z := bbi. z + dt * bbi.vz
    bi+=1
  end for
end advance

OffsetMomentum
put energy()
var start := Time.Elapsed()
for i : 1..its
  advance(0.01)
end for
put energy()
put "took ", (Time.Elapsed() - start) / 1000 , " seconds."
