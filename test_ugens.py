from pyck import *
from pyck.ugens.osc import SinOsc

o1 = SinOsc()
o1.freq = 1000

Server.dac.addSource(o1)

def run(epoch=Server.srate*10):
    for _ in range(epoch):
        Server.tick()

if __name__ == "__main__":
    run()

