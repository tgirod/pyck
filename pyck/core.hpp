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
struct Config;
struct Shreduler;
struct Shred;
struct Event;

struct UGenComparator;
struct ShredComparator;

// shared pointers
typedef boost::shared_ptr<UGen> UGenPtr;
typedef boost::shared_ptr<Route> RoutePtr;
typedef boost::shared_ptr<Config> ConfigPtr;
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

// complete declarations

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
    void fetch();
    void compute();
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
    
    void spork(boost::python::object gen, boost::python::object args);
    void addShred(ShredPtr shred);
    void tick();
};

struct Shred: public boost::enable_shared_from_this<Shred>
{
    boost::python::object gen; // call this (generator)
    boost::python::object args; // with this arguments: gen.send(args)
    Time time; // at this time
    
    Shred(boost::python::object gen, boost::python::object args, Time t);
    Shred(boost::python::object gen, Time t);
    Shred(boost::python::object gen, boost::python::object args);
    ~Shred();
    
    void run();
};

// struct Event: public boost::enable_shared_from_this<Event>
// {
//     std::queue<boost::python::object> q;
    
//     Event();
//     ~Event();
    
//     void shredule(boost::python::object gen);
//     void signal(boost::python::object args);
//     void broadcast(boost::python::object args);
// };

struct Config
{
    static Time now;
    static Samplerate srate;
    static UGenPtr dac;
    static UGenPtr adc;    
    static ShredulerPtr shreduler;
    
    static void init(int inputs, int outputs, Samplerate srate);
    
    static Time getNow() { return Config::now; }
    static Samplerate getSrate() { return Config::srate; }
    static UGenPtr getDac() { return Config::dac; }
    static UGenPtr getAdc() { return Config::adc; }
    static ShredulerPtr getShreduler() { return Config::shreduler; }
};
