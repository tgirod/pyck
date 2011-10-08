from core import *
from time import sleep
import thread

def clockShred():
    while True:
        sleep(0.1)
        yield 1

def printShred(name,event):
    while True:
        text = yield event
        print "my name is %s and i'm printing %s" %(name,text)


if __name__ == "__main__":
    s = Server()
    e = Event(s)
    
    s.spork(clockShred)
    s.spork(printShred,"bobby",e)
    s.spork(printShred,"steve",e)
    thread.start_new_thread(s.run,())
    
    
