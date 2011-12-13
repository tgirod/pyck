from random import randint
import pyck

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



pyck.spork(delayShred(5))

ev = pyck.Event()

pyck.spork(producerShred(ev))
pyck.spork(consumerShred(ev))
