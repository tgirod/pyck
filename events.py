from pyck.core import *
from time import sleep
from random import randint

# just to slow down
def clockShred():
    while True:
        print "TICK !"
        sleep(0.5)
        yield 1


def pokeShred(event):
    while True:
        print "poking !"
        event.signal(randint(0,9))
        yield randint(1,5)

def pokedShred(event):
    while True:
        print "waiting for poke"
        data = yield event
        print "poked", data, "times"

if __name__ == "__main__":
    s = Server()
    e = Event(s)

    s.spork(clockShred)
    s.spork(pokedShred,e)
    s.spork(pokeShred,e)    

    s.run()
