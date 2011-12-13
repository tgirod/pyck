from core import *

now = 0
srate = 44100
dac = Dac()
adc = Adc()
shreduler = Shreduler()

def tick():
    pyck.shreduler.tick()
    pyck.dac.tick()
    pyck.now += 1
