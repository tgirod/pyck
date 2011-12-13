import pyck
import pyck.ugens.osc as osc

x = osc.SinOsc(gain=0.1)
pyck.dac.addSource(x)

y = osc.SinOsc(freq=986)
pyck.dac.addSource(y)

for _ in range(pyck.srate*10):
    pyck.tick()
