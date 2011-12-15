import pyck
import pyck.ugens.osc as osc
from random import uniform
import struct

"""
To get some sound out of pyck :

$> mkfifo fifo

$> cat fifo | aplay

and then launch this script from another terminal.
"""

pyck.srate = 44100

x = osc.SinOsc(freq=1000,gain=0.5)
pyck.dac.addSource(x)

def randomfreq(osc):
    while True:
        osc.freq = uniform(500,1100)
        yield pyck.second(0.2)

pyck.spork(randomfreq(x))

pipe = open('fifo','wb')

def convert(v):
    v = int(v*32768)
    v = min(v,32767)
    v = max(v,-32768)
    return v

def tick():
    pyck.tick()
    v0 = convert(pyck.dac.input[0])
    v1 = convert(pyck.dac.input[1])
    pipe.write(struct.pack('<hh',v0,v1))

if __name__ == "__main__":
    for _ in range(pyck.srate*5):
        tick()
