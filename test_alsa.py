import pyck
import pyck.ugens.osc as osc
from random import uniform
import struct

pyck.srate = 8000

x = osc.SinOsc(freq=1000)
pyck.dac.addSource(x)

def randomfreq(osc):
    while True:
        osc.freq = uniform(100,1000)
        yield 1000

pyck.spork(randomfreq(x))

pipe = open('fifo','wb')

def tick():
    pyck.tick()
    v = pyck.dac.input[0]
    v = (v+1)/2 * 256
    v = min(v,255)
    v = max(v,0)
    pipe.write(struct.pack('@B',v))

for _ in range(pyck.srate*5):
    tick()
