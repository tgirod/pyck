from random import random
from core import *

def printShred(name,event):
    while True:
        text = yield event
        print "my name is %s and i'm printing %s" %(name,text)

def genShred(event):
    while True:
        x = random()
        print "sending", x
        event.signal(x)
        yield 3


if __name__ == "__main__":
    s = Server()
    e = Event(s)
    
    s.spork(printShred,"steve",e)
    s.spork(printShred,"bob",e)
    s.spork(genShred,e)
    
    s.run()
    
    
