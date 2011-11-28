import pyck
from math import sin, pi

class Osc(pyck.UGen):
    def __init__(self,freq=440.0,phase=0.0,gain=1.0):
        pyck.UGen.__init__(self,inputs=0,outputs=1)
        self._freq = freq
        self._phase = phase
        self._gain = gain

    @property
    def freq(self):
        return self._freq

    @freq.setter
    def setFreq(self,freq):
        if freq > 0: 
            self._freq = freq
    
    @property
    def phase(self):
        return self._phase

    @phase.setter
    def setPhase(self,phase):
        self._phase = phase

    @property
    def gain(self):
        return self._gain

    @gain.setter
    def setGain(self,gain):
        if gain >= 0:
            self._gain = gain
    
class SinOsc(Osc):

    def compute(self):
        self._output[0] = sin(self._phase) * self._gain
        self._phase += 2*pi * self._freq / pyck.srate
        if self._phase > pi: self._phase -= 2*pi
        
