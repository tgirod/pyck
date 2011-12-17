pyck -- python chuck

Pyck is an implementation of ChucK's strongly timed approach to audio
programming. At the present time, it is a proof-of-concept.

News
----

**2011.12.17**

Thanks to cmake, I got back on track with boost::python, and it's giving
results ! I'm almost there. PycK is now using RtAudio to communicate with Jack,
and sound synthesis works !

There's still one problem to solve, though. In their current state, Shreds are
segfaulting. This problem seems to be caused by RtAudio's use of threads. When
I call Server.start(), pyck's execution is drived by jack, through the callback
API. It works fine with sound synthesis because all the code used there is pure
C++. It's more problematic with shreds because the shreduler has to run some
python code: the shreds themselves. This callback from an independant C++
thread to Python is causing quite a mess.

**2011.12.14**

Finally, some noise is coming out of PycK ! As I don't have an audio interface
coded, I did it by piping the samples to aplay - and it worked ! But before I
go any further down the audio interface road, there is a lot of optimization to
do.

**2011.12.12**

I got bored of the boost::python branch. All this compiling and linking between
modules is such a mess, I'm getting back to pure python for now. I'll probably
have a shot at cython and see where it gets me.

Why doing this ?
----------------

[ChucK](http://chuck.cs.princeton.edu/) is a wonderful piece of software and a
very sound approach to audio programming. Unfortunately, it is crippled by an
incomplete, poor language.

By incomplete, I mean that many parts of the language are missing - imports,
basic OO stuff and whatnot.

By poor, I mean that even if the language in itself were complete, you wouldn't
do much without massive libraries to go with it - namely user interfaces, file
management etc.

In short, creating a programming language from scratch is a tremendous task,
and perhaps ChucK would be better off piggybacking on an existing, largely
established language rather than reinventing the wheel.

How does Pyck work ?
--------------------

Pyck's approach to shreduling and events is based on coroutines. Through its
[PEP 342](http://www.python.org/dev/peps/pep-0342/), python offers a powerful
tool to do precisely the kind of interleaved, tightly controled threads of
execution we need to implement Chuck's shreduling.

Even better, you get a pretty clean syntax for free, directly integrated into
the language.

But python is slow ! what about sound synthesis ?
-------------------------------------------------

Well, there are many ways to optimize python, such as :

 - writing parts directly in C 
 - using tools like [cython](http://cython.org)
 - running pyck with the excellent [pypy](http://pypy.org) interpreter
 - using [boost::python](http://www.boost.org/doc/libs/1_48_0/libs/python/doc/)
   to write classes in C++ and expose them to python seamlessly

But right now, I prefer to focus on design rather than optimization.