import pyck
from math import sin, pi

class Osc(pyck.UGen):
    def __init__(self):
        pyck.UGen.__init__(self,inputs=0,outputs=1)
        self._freq = 440.0
        self._phase = 0.0
        self._gain = 0.0

    @property
    def freq(self):
        return self._freq

    @property
    def phase(self):
        return self._phase

    @property
    def gain(self):
        return self._gain

class SinOsc(Osc):
    
    def compute(self):
        self._output[0] = sin(self._phase)
        self._phase += 2*pi * self._freq / pyck.srate
