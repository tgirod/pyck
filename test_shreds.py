from random import randint
from pyck.shred import *
    
def delayShred(delay):
    while True:
        print "waiting for", delay
        yield delay

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

config.now = 0
config.shreduler = Shreduler()

spork(delayShred,5)
ev = Event()

spork(producerShred,ev)
spork(consumerShred,ev)

def tick():
    print config.now, config.shreduler.queue
    config.shreduler.tick()
    config.now+=1
