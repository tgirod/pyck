from pyck.core import *
from time import sleep
from random import randint

# just to slow down
def clockShred():
    while True:
        print "TICK !"
        sleep(0.5)
        yield 1

def myShred(name):
    while True:
        r = randint(1,5)
        print name, "will sleep for", r, "cycles"
        yield r

if __name__ == "__main__":
    s = Server()
    s.spork(clockShred)
    s.spork(myShred,"foo")
    s.spork(myShred,"bar")
    s.run()
