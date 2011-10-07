class Server(object):
    """ Server is the central class taking care of the shreduling. """
    
    def __init__(self,samplerate=44100,ins=2,outs=2):
        self.samplerate = samplerate
        self.ins = ins
        self.outs = outs
        self.now = 0
        self.q = {}
        
    def shredule(self,time,shred):
        """ shredule the given shred at the given time """
        if time in self.q:
            self.q[time].append(shred)
        else:
            self.q[time] = [shred]

    def spork(self,func,*args):
        """turn a generator function into a shred and shredule it now"""
        self.shredule(self.now,func(*args))
    
    def run(self):
        while self.now < 20:
            print "#", self.now
            self.runShreds()
            self.now += 1

    def handleYield(self,s,y):
        if type(y) == int: self.shredule(self.now+y,s)
        # if yield returned none, reshredule s now
        elif y == None: self.shredule(self.now,s)
        # if y is an event, make s wait for the event
        elif isinstance(y, Event): y.shredule(s)
        
    def runShreds(self):
        """run what is shreduled now."""
        # get the list of shreds shreduled now
        while self.now in self.q:
            # extract the list of shreds to execute now. If a shred shredule
            # something "now", it won't appear in this list, thus the enclosing
            # while loop
            for s in self.q.pop(self.now,[]):
                try:
                    y = s.next()
                    self.handleYield(s,y)
                except StopIteration:
                    # the shred terminates
                    pass


class Event(object):
    """ The Event class allows cross shred communication. Example :
    
    def myShred(myEvent):
        #do something
        data = yield myEvent
        print data

    Here, the myShred will stop and wait until myEvent sends some data.
    """
    def __init__(self,server):
        # queue of shreds currently listening to this event
        self.server = server
        self.q = []

    def shredule(self,s):
        """add s at the end of the list of shreds currently listening to this
        event"""
        self.q.append(s)
    
    def signal(self,*args):
        """send a message (args) to the first shred of the list and remove it"""
        if len(self.q) == 0:
            return
        
        s = self.q.pop(0)
        
        try:
            y = s.send(*args)
            self.server.handleYield(s,y)
        except StopIteration:
            # the shred terminates
            pass
        
    def broadcast(self,*args):
        """send a message (args) to all the shreds listening and remove them"""
        for s in self.q:
            s.send(*args)
        self.shreds = []




if __name__ == "__main__":
    from random import random
    s = Server()
    
    e = Event(s)
    
    def printShred(event):
        while True:
            text = yield event
            print "waking up!", text

    s.spork(printShred,e)

    def genShred(event):
        while True:
            x = random()
            print "sending", x
            event.signal(x)
            yield 3
    s.spork(genShred,e)
    
    s.run()
    
    
