pyck -- python chuck

Pyck is an implementation of ChucK's strongly timed approach to audio
programming. At the present time, it is a proof-of-concept.

Why doing this ?
----------------

ChucK is a wonderfull piece of software and a very sound approach to audio
programming. Unfortunately, it is crippled by an incomplete, poor language. 

By incomplete, I mean that many parts of the language are missing - imports,
basic OO stuff and whatnot.

By poor, I mean that even if the language in itself is complete, you won't do
much if you don't have massive libraries to go with it - namely user
interfaces, file management etc.

In short, creating a programming language from scratch is a tremendous task,
and perhaps ChucK would be better of piggybacking on an existing, largely
established language rather than reinventing the wheel.

How does Pyck work ?
--------------------

Pyck's approach to shreduling and events is based on coroutines. Through its
[PEP 342](http://www.python.org/dev/peps/pep-0342/), python offers a powerful
tool to do precisely the kind of interleaved, tightly controled threads of
execution.

Even better, you get a pretty clean syntax for free, directly integrated into
the language.

But python is slow ! what about sound synthesis ?
-------------------------------------------------

Well, there are many ways to optimize python, such as writing parts directly in
C. libraries like boost::python even allows you to write complete classes in
C++ and expose them to python seamlessly.

But right now, I prefer to focus on design rather than optimization.