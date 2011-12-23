#ifndef CORE_HPP
#define CORE_HPP

#include <map>
#include <queue>
#include <vector>
#include <iostream>
#include <string>

#include <boost/python.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <rtaudio/RtAudio.h>

// structs
struct UGen;
struct Route;
struct Server;
struct Shred;
struct Event;

struct UGenComparator;
struct ShredComparator;

// shared pointers
typedef boost::shared_ptr<UGen> UGenPtr;
typedef boost::shared_ptr<Route> RoutePtr;
typedef boost::shared_ptr<Server> ServerPtr;
typedef boost::shared_ptr<Shred> ShredPtr;
typedef boost::shared_ptr<Event> EventPtr;

// simple aliases
typedef unsigned long int Time;
typedef unsigned long int Duration;
typedef float Sample;
typedef unsigned long int Samplerate;

// templatefull aliases
typedef std::map< boost::weak_ptr<UGen>, RoutePtr, UGenComparator > SourceList;
typedef std::priority_queue<ShredPtr, std::vector<ShredPtr>, ShredComparator> ShredQueue;

// structs complete declarations
struct UGenComparator
{
    bool operator()(boost::weak_ptr<UGen> const& lhs, boost::weak_ptr<UGen> const& rhs);
};

struct ShredComparator
{
    bool operator()(ShredPtr const& lhs, ShredPtr const& rhs);
};

struct UGen: public boost::enable_shared_from_this<UGen>
{
    Time last;
    
    int inputSize;
    int outputSize;
    
    boost::shared_array<Sample> input;
    boost::shared_array<Sample> output;
    
    SourceList sources;

    UGen();
    UGen(int inputs, int outputs);
    UGen(UGenPtr source);
    ~UGen();
    
    Time getLast();
    
    Sample getInput(int channel);
    void setInput(int channel, Sample value);
    void resetInput();
    
    Sample getOutput(int channel);
    void setOutput(int channel, Sample value);
    void resetOutput();
    
    void addSource(UGenPtr source);
    void addSourceList(UGenPtr source, boost::python::list route);
    void addSourceRoute(UGenPtr source, RoutePtr route);
    
    void removeSource(UGenPtr source);
    
    virtual void tick();
    virtual void fetch();
    virtual void compute();
};

struct Route: public boost::enable_shared_from_this<Route>
{
    int sourceSize;
    int targetSize;
    boost::shared_array<int> weights;
    
    Route(int sourceSize, int targetSize);
    Route(UGenPtr source, UGenPtr target);
    Route(boost::python::list weights);
    void init();
    ~Route();
    
    void fetch(UGenPtr source, UGenPtr target);
};

struct Shred: public boost::enable_shared_from_this<Shred>
{
    boost::python::object gen; // call this (generator)
    Time next; // at this time
    
    Shred(boost::python::object gen, Time t);
    Shred(boost::python::object gen);
    ~Shred();
    
    void run();
    void run(boost::python::object args);
    void handleYield(boost::python::object yield);
    void kill();
};

struct Event: public boost::enable_shared_from_this<Event>
{
    std::queue<ShredPtr> queue;
    
    Event();
    ~Event();
    
    void addShred(ShredPtr shred);
    void broadcast(boost::python::object args);
    void signal(boost::python::object args);
};

struct Server
{
    static ServerPtr singleton;
    
    RtAudio audio;
    RtAudio::DeviceInfo info;
    RtAudio::StreamParameters inputParams, outputParams;
    unsigned int bufferFrames; // number of frames processed at once
    PyGILState_STATE gstate;
    
    Time now;
    Samplerate srate;
    UGenPtr io;

    ShredQueue queue;
    
    Server(int channels);
    ~Server();
    
    static ServerPtr open(int channels);
    void start();
    void stop();
    void close();

    void tick();
    
    Time getNow();
    Samplerate getSrate();
    UGenPtr getIO();

    ShredPtr spork(boost::python::object gen);
    void addShred(ShredPtr shred);
};

int callback(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData );

// Useful functions

Duration ms(float t);
Duration second(float t);
Duration minute(float t);
Duration hour(float t);

#endif
