__all__ = ['shred','ugen','config']

import shred
import ugen
import config

def init(inputs=2,outputs=2,srate=44100):
    config.srate = srate
    config.dac = ugen.Dac(outputs)
    config.adc = ugen.Adc(inputs)
    config.shreduler = shred.Shreduler()
