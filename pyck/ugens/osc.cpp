#include "osc.hpp"

using namespace boost;
using namespace boost::python;
using namespace std;

///////////////////////////////////////////////////////////////////////////////
// class Osc

Osc::Osc() : UGen::UGen(0,1)
{
    freq = 440.0;
    phase = 0.0;
    gain = 1.0;
}

Osc::~Osc()
{}

float Osc::getFreq()
{
    return freq;
}

void Osc::setFreq(float freq)
{
    if (0 < freq) {
	this->freq = freq;
    }
}

float Osc::getPhase()
{
    return phase;
}

void Osc::setPhase(float phase)
{
    if (-M_PI < phase && phase <= M_PI) {
	this->phase = phase;
    }
}

float Osc::getGain()
{
    return gain;
}

void Osc::setGain(float gain)
{
    if (0 <= gain) {
	this->gain = gain;
    }
}

///////////////////////////////////////////////////////////////////////////////
// class SinOsc

SinOsc::SinOsc()
{
    update();
}

SinOsc::~SinOsc()
{}

void SinOsc::setFreq(float freq)
{
    if (0 < freq) {
	this->freq = freq;
	update();
    }
}

void SinOsc::setPhase(float phase)
{
    if (-M_PI < phase && phase <= M_PI) {
	this->phase = phase;
	update();
    }
}

void SinOsc::update()
{
    w = freq * 2 * M_PI / Server::srate;
    p = 2 * cos(w);
    y[0] = sin(-2 * w + phase);
    y[1] = sin(-1 * w + phase);
    y[2] = 0;
}

void SinOsc::compute()
{
    y[2] = p * y[1] - y[0];
    y[0] = y[1];
    y[1] = y[2];
    output[0] = y[2] * gain;
    phase += w;
    if (M_PI < phase) {
	phase -= 2 * M_PI;
    }
}

BOOST_PYTHON_MODULE (libosc)
{
    class_<Osc, bases<UGen>, OscPtr>("Osc")
	.add_property("freq", &Osc::getFreq, &Osc::setFreq)
	.add_property("phase", &Osc::getPhase, &Osc::setPhase)
	.add_property("gain", &Osc::getGain, &Osc::setGain);
    
    class_<SinOsc, bases<Osc>, SinOscPtr>("SinOsc")
	.add_property("freq", &Osc::getFreq, &SinOsc::setFreq)
	.add_property("phase", &Osc::getPhase, &SinOsc::setPhase);
}
