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

// structs
struct UGen;
struct Route;
struct Server;
struct Shreduler;
struct Shred;
struct Event;

struct UGenComparator;
struct ShredComparator;

// shared pointers
typedef boost::shared_ptr<UGen> UGenPtr;
typedef boost::shared_ptr<Route> RoutePtr;
typedef boost::shared_ptr<Server> ServerPtr;
typedef boost::shared_ptr<Shreduler> ShredulerPtr;
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
    void init();
    ~UGen();
    
    Time getLast();
    
    Sample getInput(int channel);
    void setInput(int channel, Sample value);
    void resetInput();
    
    Sample getOutput(int channel);
    void setOutput(int channel, Sample value);
    void resetOutput();
    
    void addSource(UGenPtr source, RoutePtr route);
    void addSourceList(UGenPtr source, boost::python::list route);
    void addSourceGuess(UGenPtr source);

    void removeSource(UGenPtr source);
    
    void tick();
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

struct Shreduler: public boost::enable_shared_from_this<Shreduler>
{
    ShredQueue queue;
    
    Shreduler();
    ~Shreduler();
    
    ShredPtr spork(boost::python::object gen);
    void addShred(ShredPtr shred);
    void tick();
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
    static Time now;
    static Samplerate srate;
    static UGenPtr dac;
    static UGenPtr adc;    
    static ShredulerPtr shreduler;
    
    static void init(int inputs, int outputs, Samplerate srate);
    static void tick();
    
    static Time getNow() { return Server::now; }
    static Samplerate getSrate() { return Server::srate; }
    static UGenPtr getDac() { return Server::dac; }
    static UGenPtr getAdc() { return Server::adc; }
    static ShredulerPtr getShreduler() { return Server::shreduler; }
};

#endif
