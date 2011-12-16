from pyck import *
from pyck.ugens.osc import SinOsc

o1 = SinOsc()
o1.freq = 1000

Config.dac.addSource(o1)

def run(epoch=Config.srate*10):
    for _ in range(epoch):
        Config.tick()

if __name__ == "__main__":
    run()

