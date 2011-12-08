#include "core.hpp"

using namespace boost;
using namespace boost::python;
using namespace std;

// UGen class
///////////////////////////////////////////////////////////////////////////////

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

// Route class
///////////////////////////////////////////////////////////////////////////////

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

// Shreduler class
///////////////////////////////////////////////////////////////////////////////

Shreduler::Shreduler()
{}

Shreduler::~Shreduler()
{}

void Shreduler::spork(boost::python::object gen)
{
    ShredPtr shred(new Shred(gen));
    addShred(shred);
}

void Shreduler::addShred(ShredPtr shred)
{
    queue.push(shred);
}

void Shreduler::tick()
{
    ShredPtr shred = queue.top();
    while (!queue.empty() && shred->time <= Config::now) {
	queue.pop();
	shred->run();
	shred = queue.top();
    }
    Config::now++;
}

// Shred class
///////////////////////////////////////////////////////////////////////////////

Shred::Shred(object gen, Time t)
{
    this->time = t;
    this->gen = gen;
}

Shred::Shred(object gen)
{
    this->time = Config::now;
    this->gen = gen;
}

Shred::~Shred()
{}

void Shred::run()
{
    // resume the shred and store the result sent by yield
    try {
	object yield = gen.attr("next")();
	handleYield(yield);
    } catch (const error_already_set& e) {
	// StopIteration error means the shred is finished, so we don't have to
	// reshredule it later.
    }
}

void Shred::run(object args)
{
    try {
	object yield = gen.attr("send")(args);
	handleYield(yield);
    } catch (const error_already_set& e) {
	// StopIteration error means the shred is finished, so we don't have to
	// reshredule it later.	
    }
}

void Shred::handleYield(object yield)
{
    // yield returned None -> reshredule now
    if (yield.is_none()) {
	time = Config::now;
	Config::shreduler->addShred(shared_from_this());
	return;
    }
    
    // yield returned a duration -> reshredule now+duration
    extract<Duration> get_dur(yield);
    if (get_dur.check()) {
	time = Config::now + get_dur();
	Config::shreduler->addShred(shared_from_this());
	return;
    }

    // yield returned an Event object -> reshredule in event queue
    extract<EventPtr> get_event(yield);
    if (get_event.check()) {
	time = Config::now;
	get_event()->addShred(shared_from_this());
	return;
    }
}

// Event class
///////////////////////////////////////////////////////////////////////////////

Event::Event()
{}

Event::~Event()
{}

void Event::addShred(ShredPtr shred)
{
    queue.push(shred);
}

void Event::broadcast(object args)
{
    // wake up all shreds that are waiting for this event. if a shred finish by
    // waiting for this event again, it will create an endless loop. that is
    // why we add a counter so we wake up the shreds only once.
    ShredPtr shred;
    int i = queue.size();
    while (!queue.empty() && i > 0) {
	shred = queue.front();
	queue.pop();
	shred->run(args);
	i--;
    }
}

void Event::signal(object args)
{
    // if at least a shred is waiting for this event get the first shred from
    // the queue and shredule it now
    
    if (queue.empty()) {
	return;
    }
    
    ShredPtr shred = queue.front();
    queue.pop();
    shred->run(args);
}

bool UGenComparator::operator()(weak_ptr<UGen> const& lhs, weak_ptr<UGen> const& rhs) {
    UGenPtr shared_lhs = lhs.lock();
    UGenPtr shared_rhs = rhs.lock();
    return shared_lhs.get() < shared_rhs.get();
}

bool ShredComparator::operator()(ShredPtr const& lhs, ShredPtr const& rhs) {
    return lhs->time > rhs->time;
}

// Config class
///////////////////////////////////////////////////////////////////////////////

Time		Config::now = 0;
Samplerate	Config::srate = 44100;
UGenPtr		Config::dac = UGenPtr(new UGen(1,0));
UGenPtr		Config::adc = UGenPtr(new UGen(0,1));
ShredulerPtr	Config::shreduler = ShredulerPtr(new Shreduler());

void Config::init(int inputs, int outputs, Samplerate srate)
{
    Config::now = 0;
    Config::srate = srate;
    Config::dac = UGenPtr(new UGen(inputs,0));
    Config::adc = UGenPtr(new UGen(0,outputs));    
}

// Boost python export
///////////////////////////////////////////////////////////////////////////////

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
    
    class_<Route, RoutePtr>("Route", init<int, int>())
	.def(init<UGenPtr, UGenPtr>())
	.def(init<list>())
	.def_readonly("sourceSize", &Route::sourceSize)
	.def_readonly("targetSize", &Route::targetSize);

    class_<Config, ConfigPtr>("Config")
	.add_static_property("now",&Config::getNow)
	.add_static_property("srate",&Config::getSrate)
	.add_static_property("dac",&Config::getDac)
	.add_static_property("adc",&Config::getAdc)
	.add_static_property("shreduler",&Config::getShreduler)
	.def("init",&Config::init)
	.staticmethod("init");
    
    class_<Shreduler, ShredulerPtr>("Shreduler")
	.def("spork",&Shreduler::spork)
	.def("tick",&Shreduler::tick);
    
    class_<Event, EventPtr>("Event")
	.def("signal",&Event::signal)
	.def("broadcast",&Event::broadcast);
}
