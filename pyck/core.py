class Server(object):
    """ Server is the central class taking care of the shreduling. """
    
    def __init__(self,samplerate=44100,inputs=2,outputs=2):
        self._samplerate = samplerate
        self._now = 0
        self._queue = {}
        self._adc = Adc(self,inputs)
        self._dac = Dac(self,outputs)

    @property
    def samplerate(self):
        return self._samplerate
    
    @property
    def now(self):
        return self._now

    @property
    def adc(self):
        return self._adc

    @property
    def dac(self):
        return self._dac
    
    def shredule(self,time,shred):
        """ shredule the given shred at the given time """
        if time in self._queue:
            self._queue[time].append(shred)
        else:
            self._queue[time] = [shred]

    def spork(self,func,*args):
        """turn a generator function into a shred and shredule it now"""
        self.shredule(self._now,func(*args))
    
    def step(self):
        self.runShreds()
        self.runUGens()
        self._now += 1

    def run(self):
        while True:
            self.step()

    def handleYield(self,s,y):
        if type(y) == int: self.shredule(self._now+y,s)
        # if yield returned none, reshredule s now
        elif y == None: self.shredule(self._now,s)
        # if y is an event, make s wait for the event
        elif isinstance(y, Event): y.shredule(s)
        
    def runShreds(self):
        """run what is shreduled now."""
        # get the list of shreds shreduled now
        while self._now in self._queue:
            # extract the list of shreds to execute now. If a shred shredule
            # something "now", it won't appear in this list, thus the enclosing
            # while loop
            for s in self._queue.pop(self._now,[]):
                try:
                    y = s.next()
                    self.handleYield(s,y)
                except StopIteration:
                    # the shred terminates
                    pass

    def runUGens(self):
        self._dac()

class Event(object):
    """ The Event class allows communication between various parts of code :
    
    def myShred(myEvent):
        #do something
        data = yield myEvent
        print data

    Here, the myShred will stop and wait until myEvent sends some data.
    """
    def __init__(self,server):
        # queue of shreds currently listening to this event
        self._server = server
        self._queue = []

    def shredule(self,s):
        """add s at the end of the list of shreds currently listening to this
        event"""
        self._queue.append(s)
    
    def signal(self,*args):
        """send a message (args) to the first shred of the list and remove it"""
        if len(self._queue) == 0:
            return
        
        s = self._queue.pop(0)
        
        try:
            y = s.send(*args)
            self._server.handleYield(s,y)
        except StopIteration:
            # the shred terminates
            pass
        
    def broadcast(self,*args):
        """send a message (args) to all the shreds listening and remove them"""
        for s in self._queue:
            s.send(*args)
        self._queue = []


class UGen(object):
    """ parent class of all ugens. Ugens should implement the following methods:
    * compute, to do the actual computation
    * __delete__ to destroy the inlets/outlets attached to the ugen
    """
    
    def __init__(self,server):
        self._server = server
        self._now = server.now
    
    @property
    def server(self):
        return self._server
    
    @property
    def now(self):
        return self._now
    
    def __call__(self):
        if self._now < self._server.now:
            self._now = self._server.now
            self.compute()

    def compute(self):
        """ 
        This method should be implemented in derived classes its purpose is to:
        * calls each inlet to get new values
        * perform actual computation
        * store the results in outlets
        """
        raise NotImplementedError()


class Inlet(object):
    def __init__(self,ugen):
        self._sources = []
        self._value = 0.0
        self._ugen = ugen
    
    @property
    def value(self):
        return self._value

    @value.setter
    def value(self,value):
        self._value = value

    @property
    def ugen(self):
        return self._ugen
    
    def addSource(self,source):
        if source not in self._sources:
            self._sources.append(source)

    def delSource(self,source):
        if source in self._sources:
            self._sources.remove(source)

    def connect(self,source):
        self.addSource(source)
        source.addTarget(self)

    def disconnect(self,source):
        self.delSource(source)
        source.delTarget(self)

    def disconnectAll(self):
        for s in self._sources:
            self.disconnect(s)

    def __call__(self):
        self.value = 0.0
        for s in self._sources:
            self.value += s()

    def __del__(self):
        disconnectAll()

class Outlet(object):
    def __init__(self,ugen):
        self._targets = []
        self._value = 0.0
        self._ugen = ugen

    @property
    def value(self):
        return self._value

    @value.setter
    def value(self,value):
        self._value = value

    @property
    def ugen(self):
        return self._ugen

    def addTarget(self,target):
        if target not in self._targets:
            self._targets.append(target)

    def delTarget(self,target):
        if target in self._targets:
            self._targets.remove(target)

    def connect(self,target):
        self.addTarget(target)
        target.addSource(self)

    def disconnect(self,target):
        self.delTarget(target)
        target.delSource(self)

    def disconnectAll(self):
        for t in self._targets:
            self.disconnect(t)

    def __call__(self):
        self.ugen()
        return self._value

    def __del__(self):
        disconnectAll()


class Adc(UGen):

    def __init__(self,server,inputs):
        UGen.__init__(self,server)
        self._outlet = [Outlet(self) for _ in range(inputs)]
    
    def __delete__(self):
        for o in self._outlet:
            del o

    @property
    def outlet(self):
        return self._outlet
    
    def compute(self):
        # transmit the signal from jack
        pass
    
class Dac(UGen):

    def __init__(self,server,outputs):
        UGen.__init__(self,server)
        self._inlet = [Inlet(self) for _ in range(outputs)]
    
    def __delete__(self):
        for i in self._inlet:
            del i

    @property
    def inlet(self):
        return self._inlet

    def compute(self):
        # transmit the signal to jack. right now it only prints the results
        for i in self._inlet:
            i()
