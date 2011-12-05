#include "core.hpp"

using namespace boost;
using namespace boost::python;
using namespace std;

UGen::UGen()
{
    this->inputSize = 1;
    this->outputSize = 1;
    init();
}

UGen::UGen(int inputs, int outputs)
{
    this->inputSize = inputs;
    this->outputSize = outputs;
    init();
}

void UGen::init()
{
    this->last = Config::now;
    
    this->input = shared_array<Sample>(new Sample[inputSize]);
    resetInput();
    
    this->output = shared_array<Sample>(new Sample[outputSize]);
    resetOutput();
}

UGen::~UGen()
{}

Time UGen::getLast()
{
    return this->last;
}

Sample UGen::getInput(int channel)
{
    return this->input[channel];
}

void UGen::setInput(int channel, Sample value)
{
    this->input[channel] = value;
}

void UGen::resetInput()
{
    for (int i=0; i<inputSize; i++) {
	input[i] = 0.0f;
    }
}

Sample UGen::getOutput(int channel)
{
    return this->output[channel];
}

void UGen::setOutput(int channel, Sample value)
{
    this->output[channel] = value;
}

void UGen::resetOutput()
{
    for (int o=0; o<outputSize; o++) {
	output[o] = 0.0f;
    }    
}

void UGen::addSource(UGenPtr source, RoutePtr route)
{
    UGenPtr target = shared_from_this();
    if (source->outputSize == route->sourceSize && 
	target->inputSize == route->targetSize) {
	
	// BUGFIX boost::python doing nasty things with shared_ptr
	weak_ptr<UGen> u(source->shared_from_this());
	this->sources[u] = route;
    } else {
	cerr << "route shape does not match" << endl;
    }
}

void UGen::addSourceList(UGenPtr source, list route)
{
    // FIXME build a route from a python list
}

void UGen::addSourceGuess(UGenPtr source)
{
    RoutePtr route(new Route(source,shared_from_this()));
    addSource(source,route);
}

void UGen::removeSource(UGenPtr source)
{
    // BUGFIX boost::python doing nasty things with shared_ptr
    weak_ptr<UGen> u(source->shared_from_this());
    this->sources.erase(u);
}

void UGen::tick()
{
    if (this->last < Config::now) {
	this->fetch();
	this->compute();
	this->last += 1;
    }
}

void UGen::fetch()
{
    resetInput();
    
    // for each source
    for (SourceList::iterator it = sources.begin(); it != sources.end(); ++it) {
	UGenPtr source = it->first.lock();
	RoutePtr route = it->second;
	UGenPtr target = shared_from_this();
	if (source) {
	    // propagate evaluation
	    source->tick();
	    route->fetch(source,target);
	} else {
	    // source does not exist, remove
	    sources.erase(it);
	}
    }
}

void UGen::compute()
{
    // doing nothing, should be overridden in subclass
}

// Default config

Time Config::now = 0;
Samplerate Config::srate = 44100;
UGenPtr Config::dac = UGenPtr(new UGen(1,0));
UGenPtr Config::adc = UGenPtr(new UGen(0,1));

// Route 

Route::Route(int sourceSize, int targetSize)
{
    this->sourceSize = sourceSize;
    this->targetSize = targetSize;
    init();
}

Route::Route(UGenPtr source, UGenPtr target)
{
    sourceSize = source->outputSize;
    targetSize = target->inputSize;
    init();
}

Route::Route(list weights)
{
    sourceSize = len(weights);
    targetSize = len(weights[0]);
    
    this->weights = shared_array<int>(new int[sourceSize * targetSize]);
    
    for (int i=0; i<sourceSize; i++) {
	for (int j=0; j<targetSize; j++) {
	    this->weights[i*targetSize + j] = extract<int>(weights[i][j]);
	}
    }
}

void Route::init()
{
    weights = shared_array<int>(new int[sourceSize * targetSize]);
    
    if (sourceSize == targetSize) {
	// 1->1
	for (int i=0; i<sourceSize; i++) {
	    for (int j=0; j<targetSize; j++) {
		weights[i*targetSize + j] = (i==j);
	    }
	}
	
    } else {
	// 1->n, n->1, n->n
	for (int i=0; i<sourceSize*targetSize; i++) {
	    weights[i] = 1;
	}
    }
}

Route::~Route()
{}

void Route::fetch(UGenPtr source, UGenPtr target)
{
    for (int i=0; i<sourceSize; i++) {
	for (int j=0; j<targetSize; j++) {
	    target->input[j] += source->output[i] * weights[i*targetSize+j];
	}
    }
}

void initialize(int inputs, int outputs, Samplerate srate)
{
    Config::now = 0;
    Config::srate = srate;
    Config::dac = UGenPtr(new UGen(inputs,0));
    Config::adc = UGenPtr(new UGen(0,outputs));
}

BOOST_PYTHON_MODULE(_core)
{
    class_<UGen, UGenPtr>("UGen", init<int,int>())
	.def(init<>())
	.def_readonly("inputSize",&UGen::inputSize)
	.def_readonly("outputSize",&UGen::outputSize)
    	.def("input",&UGen::getInput)
	.def("setInput",&UGen::setInput)
    	.def("output",&UGen::getOutput)
	.def("setOutput",&UGen::setOutput)
    	.def("addSource",&UGen::addSourceGuess)
	.def("addSource",&UGen::addSource)
    	.def("removeSource",&UGen::removeSource)
	.def("tick", &UGen::tick)
	.def("fetch",&UGen::fetch)
	.def("compute",&UGen::compute);
    
    // def("initialize",&initialize);
    class_<Config, ConfigPtr>("Config")
	.add_static_property("now",&Config::getNow)
	.add_static_property("srate",&Config::getSrate)
	.add_static_property("dac",&Config::getDac)
	.add_static_property("adc",&Config::getAdc);

    class_<Route, RoutePtr>("Route", init<int, int>())
	.def(init<UGenPtr, UGenPtr>())
	.def(init<list>())
	.def_readonly("sourceSize", &Route::sourceSize)
	.def_readonly("targetSize", &Route::targetSize);
    
    def("initialize",&initialize);
}
