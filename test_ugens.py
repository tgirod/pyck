from pyck import *
from pyck.ugens.osc import SinOsc

s = Server.start(1,1,44100)

o1 = SinOsc()
o1.freq = 1000

s.dac.addSource(o1)

def run(epoch=s.srate*10):
    for _ in range(epoch):
        s.tick()

if __name__ == "__main__":
    run()
