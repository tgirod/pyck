class Server(object):
    """ Server is the central class taking care of the shreduling. """
    
    def __init__(self,samplerate=44100,ins=2,outs=2):
        self._samplerate = samplerate
        self._ins = ins
        self._outs = outs
        self._now = 0
        self._queue = {}
    
    @property
    def samplerate(self):
        return self._samplerate
    
    @property
    def now(self):
        return self._now
    
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

