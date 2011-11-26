import numpy as np
from weakref import WeakKeyDictionary

class UGen(object):
    """ parent class of all ugens. Ugens should implement the following methods:
    * compute, to do the actual computation
    * __delete__ to destroy the inlets/outlets attached to the ugen
    """
    
    def __init__(self,inputs=1,outputs=1):
        global now
        self._last = now
        self._input = np.zeros(inputs,np.float32)
        self._output = np.zeros(outputs,np.float32)
        self._sources = WeakKeyDictionary()
        
    @property
    def last(self):
        return self._last

    @property
    def input(self):
        return self._input
    
    @property
    def output(self):
        return self._output

    def addSource(self,source,route):
        self._sources[source] = route
        
    def removeSource(self,source):
        self._sources.pop(source,None)
        
    def tick(self):
        global now
        if self._last < now:
            self.fetch()
            self.compute()
            self._last += 1
    
    def fetch(self):
        """ call tick() on each source, and fetch results """
        self._input[:] = 0
        for source,route in self._sources.iteritems():
            source.tick()
            self._input += np.dot(source._output, route)

    def compute(self):
        """ 
        This method should be implemented in derived classes its purpose is to:
        * call each inlet to get new values
        * perform actual computation
        * store the results in outlets
        """
        pass


def connect(source,target,route=None):
    if route == None:
        if source.output.size == target.input.size:
            route = np.identity(source.output.size,np.bool)
        elif source.output.size == 1:
            route = np.ones((1,target.input.size),np.bool)
        elif target.input.size == 1:
            route = np.ones((source.output.size,1),np.bool)
        else:
            raise Exception('cannot guess a default route')

    target.addSource(source,route)
    
def disconnect(source,target):
    target.removeSource(source)


if __name__ == "__main__":
    now = 0

    u1 = UGen(0,1)
    u2 = UGen(3,1)
    
    connect(u1,u2)
    
    u1.output[:] = 1
    
