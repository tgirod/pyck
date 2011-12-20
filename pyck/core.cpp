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
    // special case if the server is not yet started
    if (Server::singleton) {
	this->last = Server::singleton->now;
    } else {
	this->last = 0;
    }
    
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
    if (this->last < Server::singleton->now) {
	this->last = Server::singleton->now;
	this->fetch();
	this->compute();
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

// Shred class
///////////////////////////////////////////////////////////////////////////////

Shred::Shred(object gen, Time t)
{
    this->next = t;
    this->gen = gen;
}

Shred::Shred(object gen)
{
    this->next = Server::singleton->now;
    this->gen = gen;
}

Shred::~Shred()
{
    kill();
}

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
    ServerPtr s = Server::singleton;
    // yield returned None -> reshredule now
    if (yield.is_none()) {
	next = s->now;
	s->addShred(shared_from_this());
	return;
    }
    
    // yield returned a duration -> reshredule now+duration
    extract<Duration> get_dur(yield);
    if (get_dur.check()) {
	next = s->now + get_dur();
	s->addShred(shared_from_this());
	return;
    }

    // yield returned an Event object -> reshredule in event queue
    extract<EventPtr> get_event(yield);
    if (get_event.check()) {
	next = s->now;
	get_event()->addShred(shared_from_this());
	return;
    }
}

void Shred::kill()
{
    gen.attr("close")();
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
    return lhs->next > rhs->next;
}

// Server class
///////////////////////////////////////////////////////////////////////////////

ServerPtr Server::singleton = ServerPtr();

Server::Server(int channels)
{
    // check if there is at least one audio interface available
    if (audio.getDeviceCount() == 0) {
	// FIXME throw exception "no audio interface available"
    }
    
    int device = audio.getDefaultOutputDevice();
    info = audio.getDeviceInfo(device);
    
    inputParams.deviceId = device;
    inputParams.nChannels = channels;
    outputParams.deviceId = device;
    outputParams.nChannels = channels;
    
    bufferFrames = 256;
    
    this->now = 0;
    this->srate = info.sampleRates[0];
    this->io = UGenPtr(new UGen(channels,channels));
    
    audio.openStream(&outputParams, &inputParams, RTAUDIO_FLOAT32, srate, 
		     &bufferFrames, &callback, NULL, NULL);
}

Server::~Server()
{
    stop();
    if (audio.isStreamOpen()) {
	audio.closeStream();
    }
}

ServerPtr Server::open(int channels)
{
    if (Server::singleton) {
	// FIXME throw exception server started
	cerr << "server already started" << endl;
    } else {
	Server::singleton = ServerPtr(new Server(channels));
    }
    return Server::singleton;
}

void Server::start()
{
    audio.startStream();
}

void Server::stop()
{
    audio.stopStream();
}

void Server::close()
{
    if (Server::singleton) {
	cout << "ending server" << endl;
	Server::singleton.reset();
    } else {
	// FIXME throw exception "no server running"
	cerr << "no server running" << endl;
    }
}

void Server::tick()
{
    // shreduling
    if (!queue.empty()){
	gstate = PyGILState_Ensure();
	while (!queue.empty() && queue.top()->next <= now ) {
	    queue.top()->run();
	    queue.pop();
	}
	PyGILState_Release(gstate);
    }
    // sound synthesis
    io->tick();
    now++;
}

ShredPtr Server::spork(boost::python::object gen)
{
    ShredPtr shred(new Shred(gen));
    addShred(shred);
    return shred;
}

void Server::addShred(ShredPtr shred)
{
    queue.push(shred);
}

Time Server::getNow() 
{ 
    return now; 
}

Samplerate Server::getSrate() 
{ 
    return srate; 
}

UGenPtr Server::getIO()
{ 
    return io;
}


int callback(void *outputBuffer, void *inputBuffer, unsigned int bufferFrames,
	     double streamTime, RtAudioStreamStatus status, void *userData )
{
    ServerPtr server = Server::singleton;
    
    // points to one value inside the inputBuffer
    Sample *input = (Sample *) inputBuffer;
    Sample *output = (Sample *) outputBuffer;
    
    for (int i=0; i<bufferFrames; i++) {
	
	// copy values from inputBuffer to io.output	
	for (int j=0; j<server->inputParams.nChannels; j++) {
	    server->io->output[j] = *input++;
	}

	// calling pyck's ugen processing
	server->tick();
	
	// retrieving results
	for (int j=0; j<server->outputParams.nChannels; j++) {
	    *output++ = server->io->input[j];
	}
    }
    
    return 0;
}

// Boost python export
///////////////////////////////////////////////////////////////////////////////

BOOST_PYTHON_MODULE(libcore)
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
    
    class_<Shred, ShredPtr>("Shred", no_init)
    	.def_readonly("next",&Shred::next)
    	.def("kill",&Shred::kill);
    
    class_<Event, EventPtr>("Event")
    	.def("signal",&Event::signal)
    	.def("broadcast",&Event::broadcast);
    
    class_<Server, ServerPtr>("Server", no_init)
	.def("open",&Server::open).staticmethod("open")
	.def("start",&Server::start)
	.def("stop",&Server::stop)	
	.def("close",&Server::close)
    	.def("spork",&Server::spork)
	.def("tick",&Server::tick)
    	.add_property("now",&Server::getNow)
    	.add_property("srate",&Server::getSrate)
    	.add_property("dac",&Server::getIO)
    	.add_property("adc",&Server::getIO);
}
