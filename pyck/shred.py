class Shreduler(object):
    """ In pyck, shreds are python generators.
    
    shreduling is based on a dictionnary associating a list of generators to a
    timestamp.
    
    when calling tick(), the shreduler gets the current time and pop the
    associated list of generators. Then, it runs each of them.
    
    A generator's execution will stop when encountering yield (pause execution)
    or return (end shred). The shreduler handles what the shred returned and
    reshredule if necessary.

    A shred decided to reshredule itself now. So when the list of shreds has
    been executed, the shreduler will check if there are no new shreds added at
    the current time.
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
            if type(y) == int: self.shredule(now+y,shred)
            
            # yield is an event object : make s wait for the event
            elif isinstance(y, Event): y.shredule(shred)
            
            # yield returns none : reshredule s now
            elif y == None: self.shredule(now,shred)
            
            # yield is something else : invalid value
            else: pass
            
        except StopIteration:
            # the shred ended, nothing to do
            return

    def tick(self):
        """run what is shreduled for the current step"""
        global now
        # get the list of shreds shreduled now
        while now in self._queue:
            for s in self._queue.pop(now,[]):
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
        global shreduler
        try:
            shred = self._queue.pop(0)
            shreduler.runShred(shred,*args)
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
    global now
    global shreduler
    shreduler.shredule(now,func(*args))


if __name__ == "__main__":
    from random import randint
    
    now = 0
    shreduler = Shreduler()
    
    def delayShred(delay):
        while True:
            print "waiting for", delay
            yield delay

    spork(delayShred,5)

    ev = Event()
    
    def consumerShred(event):
        while True:
            print "consumer: waiting for data"
            data = yield event
            print "consumer: received", data

    def producerShred(event):
        while True:
            print "producer : sending data"
            event.signal(randint(0,10))
            yield 3

    spork(producerShred,ev)
    spork(consumerShred,ev)

    def tick():
        global now, shreduler
        print now, shreduler.queue
        shreduler.tick()
        now+=1

        
