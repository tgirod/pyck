import pyck
import pyck.ugens.osc as osc
from random import uniform

x = osc.SinOsc()
pyck.dac.addSource(x)

def randomfreq(osc):
    while True:
        osc.freq = uniform(10,10000)
        yield 100

pyck.spork(randomfreq(x))

for _ in range(pyck.srate*10):
    pyck.tick()
