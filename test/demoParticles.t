type vector2D : record
    x : real
    y : real
   end record

type particle : record
    pos : vector2D
    vel : vector2D
   end record
   
View.Set("offscreenonly")

const maxParticles := 30000 % maximum number of particles
const darkBack := true

var numParticles := 0 % number of particles on screen
var p : array 1..maxParticles of particle % array of particles

proc Update(mx,my,btn : int)
  for i : 1..numParticles
    % attract to mouse
    if btn > 0 then
      var factor := -1.0
      if btn = 100 then
        factor := 3.0
      elsif btn = 101 then
        factor := -6.3
      end if
      var mvx := (p(i).pos.x - mx)
      var mvy := (p(i).pos.y - my)
      var mag := sqrt(mvx**2 + mvy**2) + 0.7
      p(i).vel.x += factor * mvx * (0.1/mag)
      p(i).vel.y += factor * mvy * (0.1/mag)
    end if

    % move it
    p(i).pos.x += p(i).vel.x
    p(i).pos.y += p(i).vel.y
    % drag
    p(i).vel.x *= (1.0 - 0.005)
    p(i).vel.y *= (1.0 - 0.005)
  end for
end Update

proc DrawParticles
  for i : 1..numParticles
      var mag := sqrt(p(i).vel.x**2 + p(i).vel.y**2) + 0.7
      var r := 0.10*mag + 0.2
      if r > 1.0 then
        r := 1.0
      end if
      RGB.SetColor(250,r,0.0,1-r)
      %RGB.SetColor(250,r,-r**2.0+r*2.0,1-r)
      Draw.Dot(round(p(i).pos.x), round(p(i).pos.y), 250)
  end for
end DrawParticles

fcn RandVector(min,max:real) : vector2D
  var v : vector2D

  v.x := (Rand.Real() * (max-min)) + min % get random real number between ends
  v.y := (Rand.Real() * (max-min)) + min % get random real number between ends

  result v
end RandVector

proc AddParticle(x:int,y:int)
  var n := numParticles + 1 % index of new particle

  if n > maxParticles then
    n := n mod maxParticles
  end if


  p(n).pos.x := x
  p(n).pos.y := y

  p(n).vel := RandVector(-1,1) % random velocity

  numParticles += 1
end AddParticle

% add all the particles
for i : 1..maxParticles
  AddParticle(maxx div 2+Rand.Int(-50,50),maxy div 2+Rand.Int(-50,50))
end for

loop
    var x,y,button : int
    Mouse.Where(x,y,button)
    
    % comment out for full trails:
    cls
    % draw a massive dark rectangle if necessary
    var fpsColor := black
    if darkBack then
      Draw.FillBox(0,0,maxx,maxy,black)
      fpsColor := white
    end if
    % fps counter
    var fps := View.GetFPS()
    Font.Draw(intstr(fps),10,10,defFontId,fpsColor)
    
    Update(x,y,button)
    DrawParticles
    View.Update
    Time.DelaySinceLast(15)
end loop
