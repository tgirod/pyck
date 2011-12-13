import pyck
from math import sin, cos, pi
from array import array

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
        if -pi < phase <= pi:
            self._phase = phase

    @property
    def gain(self):
        return self._gain

    @gain.setter
    def setGain(self,gain):
        if gain >= 0:
            self._gain = gain
    
class SinOsc(Osc):

    def __init__(self,freq=44.0,phase=0.0,gain=1.0):
        Osc.__init__(self,freq,phase,gain)
        self._y = array('f',[0,0,0]) # previous values
        self._w = 0.0 # phase increment
        self._p = 0.0 # iteration constant
        self.update()

    def update(self):
        self._w = self._freq * 2 * pi / pyck.srate
        self._p = 2 * cos(self._w)
        self._y[0] = sin(-2 * self._w + self._phase)
        self._y[1] = sin(-1 * self._w + self._phase)
        self._y[2] = 0
        
    def compute(self):
        self._y[2] = self._p * self._y[1] - self._y[0]
        self._y[0] = self._y[1]
        self._y[1] = self._y[2]
        self._output[0] = self._y[2] * self._gain
        self._phase += self._w
        if (self._phase > pi):
            self._phase -= 2*pi
        
    def setFreq(self,freq):
        if freq > 0:
            self._freq = freq
            self.update()

    def setPhase(self,phase):
        if -pi < phase <= pi:
            self._phase = phase
            self.update()
    
    freq = property(Osc.freq,setFreq)
    phase = property(Osc.phase,setPhase)
