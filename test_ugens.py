import pyck
import pyck.ugens.osc as osc
import pyck.ugens.filter as filter

pyck.init()

o1 = osc.SinOsc(freq=30)
o2 = osc.SinOsc(freq=33,gain=0.2)

pyck.connect(o1,pyck.dac)
pyck.connect(o2,pyck.dac) 

def tick():
    pyck.now += 1
    pyck.dac.tick()

for _ in xrange(pyck.srate*10):
    tick()
