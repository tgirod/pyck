#include <map>
#include <iostream>
#include <string>

#include <boost/python.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/enable_shared_from_this.hpp>

// BUGFIX for boost's weird behaviour when comparing weak_ptr
template <typename T>
struct SmartComparator
{
    bool operator()(boost::weak_ptr<T> const& lhs, boost::weak_ptr<T> const& rhs) {
	boost::shared_ptr<T> shared_lhs = lhs.lock();
	boost::shared_ptr<T> shared_rhs = rhs.lock();
        return shared_lhs.get() < shared_rhs.get();
    }
};

struct UGen;
typedef boost::shared_ptr<UGen> UGenPtr;

struct Config;
typedef boost::shared_ptr<Config> ConfigPtr;

struct Route;
typedef boost::shared_ptr<Route> RoutePtr;

typedef unsigned long int Time;
typedef unsigned long int Duration;
typedef float Sample;
typedef unsigned long int Samplerate;

typedef std::map< boost::weak_ptr<UGen>, RoutePtr, SmartComparator<UGen> > SourceList;

struct UGen: public boost::enable_shared_from_this<UGen>
{
    Time last;
    
    int inputSize;
    int outputSize;
    
    boost::shared_array<Sample> input;
    boost::shared_array<Sample> output;
    
    SourceList sources;
    
    UGen(int inputs, int outputs);
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

// global config

struct Config
{
    static Time now;
    static Samplerate srate;
    static UGenPtr dac;
    static UGenPtr adc;    
    
    static Time getNow() { return Config::now; }
    static Samplerate getSrate() { return Config::srate; }
    static UGenPtr getDac() { return Config::dac; }
    static UGenPtr getAdc() { return Config::adc; }
};

struct Route
{
    int sourceSize;
    int targetSize;
    boost::shared_array<int> weights;
    
    Route(int sourceSize, int targetSize);
    Route(UGenPtr source, UGenPtr target);
    Route(boost::python::list weights);
    ~Route();
    
    void fetch(UGenPtr source, UGenPtr target);
};


void initialize(int inputs, int outputs, Samplerate srate);
