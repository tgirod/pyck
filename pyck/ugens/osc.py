from pyck.core import *
from math import sin,pi

class Osc(UGen):
    
    def __init__(self,server,freq=440.0,gain=1.0):
        UGen.__init__(self,server)
        self._outlet = Outlet(self)
        self._freq = freq
        self._gain = gain

    def __delete__(self):
        del self._outlet

    @property
    def outlet(self):
        return self._outlet
    
    @property
    def freq(self):
        return self._freq
    
    @freq.setter
    def freq(self,freq):
        self._freq = freq

    @property
    def gain(self):
        return self._gain

    @gain.setter
    def gain(self,gain):
        self._gain = gain


class Sin(Osc):
    
    def __init__(self,server):
        Osc.__init__(self,server,angle=0.0)
        self._angle = angle

    @property
    def angle(self):
        return self._angle
        
    def compute(self):
        self._outlet.value = sin(self._angle)*self._gain
        self._angle += self._freq * 2 * pi / self.server.samplerate
        if self._angle > pi: self._angle -= 2*pi
