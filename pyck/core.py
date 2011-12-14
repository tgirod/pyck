import pyck
from weakref import WeakKeyDictionary
from itertools import repeat
from array import array

class Shred(object):
    def __init__(self,gen):
        self._gen = gen

    def run(self,args=None):

        try:
            if args == None:
                y = self._gen.next()
            else:
                y = self._gen.send(args)
            
            if y == None:
                # re-shredule now
                pyck.shreduler.addShred(pyck.now, self)
            elif type(y) == int:
                # re-shredule later
                pyck.shreduler.addShred(pyck.now+y, self)
            elif isinstance(y,Event):
                # re-shredule in event queue
                y.addShred(self)

        except StopIteration:
            # the shred ended, nothing to do
            return

        except GeneratorExit:
            # the shred has been killed
            return
        
    def kill(self):
        self._gen.close()


class Shreduler(object):
    """ In pyck, shreds are python generators.
    
    shreduling is based on a dictionnary associating a list of generators to a
    timestamp.
    
    when calling tick(), the shreduler gets the current time and pop the
    associated list of generators. Then, it runs each of them.
    
    A generator's execution will stop when encountering yield (pause execution)
    or return (end shred). The shreduler handles what the shred returned and
    reshredule as necessary.

    A shred might reshredule itself now. So when the list of shreds has been
    consumed, the shreduler will check if there are new shreds added at the
    current time.
    """
    
    def __init__(self):
        self._queue = {}
        
    @property
    def queue(self):
        return self._queue
    
    def addShred(self,time,shred):
        """ shredule the given shred at the given time """
        if time in self._queue:
            self._queue[time].append(shred)
        else:
            self._queue[time] = [shred]

    def tick(self):
        """run what is shreduled for the current step"""
        # get the list of shreds shreduled now
        while pyck.now in self._queue:
            for s in self._queue.pop(pyck.now,[]):
                s.run()


class Event(object):
    """ The Event class allows communication between various parts of code (not
    necessarily shreds).

    by calling "x = yield myEvent", a shred will wait for a message from
    "myEvent" and store it in x.
    
    by calling "myEvent.signal(myData)", one will wake up one shred waiting
    for myEvent.

    by calling "myEvent.broadcast(myData)", one will wake up all shreds waiting
    for myEvent.
    """
    
    def __init__(self):
        # queue of shreds currently listening to this event
        self._queue = []
    
    def addShred(self,shred):
        """add s at the end of the list of shreds currently listening to this
        event"""
        assert isinstance(shred,Shred)
        
        self._queue.append(shred)
    
    def signal(self,args):
        """send a message (args) to the first shred of the list and remove it"""
        try:
            shred = self._queue.pop(0)
            shred.run(args)
        except IndexError:
            # no one is listening
            pass
        except StopIteration:
            # the shred terminated
            pass
        
    def broadcast(self,*args):
        """send a message (args) to all the shreds listening and remove them"""
        for s in self._queue:
            s.run(args)
        self._queue = []


def spork(gen):
    """turn a generator function into a shred and shredule it now"""
    shred = Shred(gen)
    pyck.shreduler.addShred(pyck.now,shred)
    return shred

def ms(dur):
    return int(dur * pyck.srate / 1000)

def second(dur):
    return int(dur * pyck.srate)

def minute(dur):
    return int(60 * dur * pyck.srate)

def hour(dur):
    return int(3600 * dur * pyck.srate)

def day(dur):
    return int(86400 * dur * pyck.srate)



class UGen(object):
    """ parent class of all ugens. Ugens should implement the following methods:
    * compute, to do the actual computation
    * __delete__ to destroy the inlets/outlets attached to the ugen
    """
    
    def __init__(self,inputs=1,outputs=1):
        self._last = pyck.now
        self._input = array('f',repeat(0.0, inputs))
        self._output = array('f',repeat(0.0, outputs))
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

    @property
    def sources(self):
        return self._sources
    
    def addSource(self,source,route=None):
        assert isinstance(source,UGen)
        assert route == None or isinstance(route,Route)
        
        if route == None:
            route = Route(source,self)
            
        assert len(source.output) == route.sourceSize
        assert len(self.input) == route.targetSize
        
        self._sources[source] = route
        
    def removeSource(self,source):
        self._sources.pop(source,None)
        
    def tick(self):
        if self._last < pyck.now:
            self.fetch()
            self.compute()
            self._last += 1

    def fetch(self):
        """ clean input, call tick() on each source and fetch results """
        for i,_ in enumerate(self._input):
            self._input[i] = 0.0
            
        for source,route in self._sources.iteritems():
            source.tick()
            route.fetch(source,self)

    def compute(self):
        """ 
        This method should be implemented in derived classes its purpose is to:
        * call each inlet to get new values
        * perform actual computation
        * store the results in outlets
        """
        pass

class Route(object):

    def __init__(self,source,target):
        assert isinstance(source,UGen)
        assert isinstance(target,UGen)
        
        self._sourceSize = len(source._output)
        self._targetSize = len(target._input)
        
        self._weights = array('f',repeat(0,self._sourceSize * self._targetSize))
        self.initializeWeights()

    def initializeWeights(self):
        if self._sourceSize == self._targetSize:
            for w in range(self._sourceSize):
                self._weights[w + w*self._targetSize] = 1.0
        else:
            for w in range(self._sourceSize * self._targetSize):
                self._weights[w] = 1.0
    
    def fetch(self,source,target):
        assert len(source.output) == self._sourceSize
        assert len(target.input) == self._targetSize
        
        for s in range(self._sourceSize):
            for t in range(self._targetSize):
                target.input[t] += source.output[s] * self._weights[t + s * self._targetSize]
                
    @property
    def sourceSize(self):
        return self._sourceSize

    @property
    def targetSize(self):
        return self._targetSize

    @property
    def weights(self):
        return self._weights

class Dac(UGen):
    def __init__(self,channels=2):
        UGen.__init__(self,inputs=channels,outputs=0)


class Adc(UGen):
    def __init__(self,channels=2):
        UGen.__init__(self,inputs=0,outputs=channels)
