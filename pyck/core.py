import pyck
import numpy as np
from weakref import WeakKeyDictionary

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
    
    def shredule(self,time,shred):
        """ shredule the given shred at the given time """
        if time in self._queue:
            self._queue[time].append(shred)
        else:
            self._queue[time] = [shred]


    def runShred(self,shred,*args):
        """ run a shred and handle the yielded value """
        try:
            # run the shred
            if args == ():
                y = shred.next()
            else:
                y = shred.send(*args)
            
            # yield returns a duration : reshredule later
            if type(y) == int: self.shredule(pyck.now+y,shred)
            
            # yield is an event object : make s wait for the event
            elif isinstance(y, Event): y.shredule(shred)
            
            # yield returns none : reshredule s now
            elif y == None: self.shredule(pyck.now,shred)
            
            # yield is something else : invalid value
            else: pass
            
        except StopIteration:
            # the shred ended, nothing to do
            return

    def tick(self):
        """run what is shreduled for the current step"""
        # get the list of shreds shreduled now
        while pyck.now in self._queue:
            for s in self._queue.pop(pyck.now,[]):
                self.runShred(s)


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
    
    def shredule(self,shred):
        """add s at the end of the list of shreds currently listening to this
        event"""
        self._queue.append(shred)
    
    def signal(self,*args):
        """send a message (args) to the first shred of the list and remove it"""
        try:
            shred = self._queue.pop(0)
            pyck.shreduler.runShred(shred,*args)
        except IndexError:
            # no one is listening
            pass
        except StopIteration:
            # the shred terminated
            pass
        
    def broadcast(self,*args):
        """send a message (args) to all the shreds listening and remove them"""
        for s in self._queue:
            s.send(*args)
        self._queue = []


def spork(func,*args):
    """turn a generator function into a shred and shredule it now"""
    pyck.shreduler.shredule(pyck.now,func(*args))

def second(dur):
    return dur * pyck.srate

def minute(dur):
    return 60 * second(dur)

def hour(dur):
    return 60 * minute(dur)

def day(dur):
    return 24 * hour(dur)




class UGen(object):
    """ parent class of all ugens. Ugens should implement the following methods:
    * compute, to do the actual computation
    * __delete__ to destroy the inlets/outlets attached to the ugen
    """
    
    def __init__(self,inputs=1,outputs=1):
        self._last = pyck.now
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

    @property
    def sources(self):
        return self._sources
    
    def addSource(self,source,route):
        self._sources[source] = route
        
    def removeSource(self,source):
        self._sources.pop(source,None)
        
    def tick(self):
        if self._last < pyck.now:
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



class Dac(UGen):
    def __init__(self,channels=2):
        UGen.__init__(self,inputs=channels,outputs=0)


class Adc(UGen):
    def __init__(self,channels=2):
        UGen.__init__(self,inputs=0,outputs=channels)

def init(inputs=2,outputs=2,srate=44100):
    pyck.srate = srate
    pyck.dac = Dac(outputs)
    pyck.adc = Adc(inputs)
    pyck.shreduler = Shreduler()
