from random import randint
import pyck as p
    
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

p.init()

p.spork(delayShred,5)
ev = p.Event()

p.spork(producerShred,ev)
p.spork(consumerShred,ev)

def tick():
    print p.now, p.shreduler.queue
    p.shreduler.tick()
    p.now+=1
