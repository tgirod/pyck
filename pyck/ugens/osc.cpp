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
    gain = 0.5;
    this->init();
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
        this->init();
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
        this->init();
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
        this->init();
    }
}

void Osc::init()
{
    w = freq * 2 * M_PI / Server::singleton->srate;
}

void Osc::compute()
{
    phase += w;
    if (M_PI < phase) {
        phase -= 2 * M_PI;
    }
}


///////////////////////////////////////////////////////////////////////////////
// class Sin

Sin::Sin() : Osc::Osc()
{
    this->init();
}

Sin::~Sin()
{}

void Sin::init()
{
    Osc::init();
    p = 2 * cos(w);
    y[0] = sin(-2 * w + phase);
    y[1] = sin(-1 * w + phase);
    y[2] = 0;
}

void Sin::compute()
{
    y[2] = p * y[1] - y[0];
    y[0] = y[1];
    y[1] = y[2];
    output[0] = y[2] * gain;
    Osc::compute();
}


///////////////////////////////////////////////////////////////////////////////
// class Square

Square::Square() : Osc::Osc()
{}

Square::~Square()
{}

void Square::compute()
{
    if (0 < phase) {
        output[0] = gain;
    } else {
        output[0] = -gain;
    }

    Osc::compute();
}

///////////////////////////////////////////////////////////////////////////////
// class Saw

Saw::Saw() : Osc::Osc()
{}

Saw::~Saw()
{}

void Saw::compute()
{
    output[0] = phase/M_PI * gain;
    Osc::compute();
}

///////////////////////////////////////////////////////////////////////////////
// class Pulse

Pulse::Pulse() : Osc::Osc()
{
    width = 0.5;
}

Pulse::~Pulse()
{}

void Pulse::compute()
{
    if(M_PI*(width-0.5) < phase){
        output[0] = gain;
    } else {
        output[0] = -gain;
    }
    Osc::compute();
}

float Pulse::getWidth()
{
    return width;
}

void Pulse::setWidth(float width)
{
    if (0 <= width && width <= 1) {
        this->width = width;
    }
}

///////////////////////////////////////////////////////////////////////////////
// class Tri

Tri::Tri() : Osc::Osc()
{
    width = 0.5;
}

Tri::~Tri()
{}

void Tri::compute()
{
    float pw = M_PI * width;

    if (phase < -pw) {
        output[0] = (-M_PI - phase) / (M_PI - pw);
    } else if (phase == -pw) {
        output[0] = -1;
    } else if (phase < 0) {
        output[0] = phase / pw;
    } else if (phase == 0) {
        output[0] = 0;
    } else if (phase < pw) {
        output[0] = phase / pw;
    } else if (phase == pw) {
        output[0] = 1;
    } else if (pw < phase) {
        output[0] = (M_PI - phase) / (M_PI - pw);
    }

    Osc::compute();
}

float Tri::getWidth()
{
    return width;
}

void Tri::setWidth(float width)
{
    if (0 <= width && width <= 1) {
        this->width = width;
    }
}


///////////////////////////////////////////////////////////////////////////////
// boost export

BOOST_PYTHON_MODULE (libosc)
{
    class_<Osc, bases<UGen>, OscPtr>("Osc")
        .add_property("freq", &Osc::getFreq, &Osc::setFreq)
        .add_property("phase", &Osc::getPhase, &Osc::setPhase)
        .add_property("gain", &Osc::getGain, &Osc::setGain);

    class_<Sin, bases<Osc>, SinPtr>("Sin");

    class_<Square, bases<Osc>, SquarePtr>("Square");

    class_<Saw, bases<Osc>, SawPtr>("Saw");

    class_<Pulse, bases<Osc>, PulsePtr>("Pulse")
        .add_property("width", &Pulse::getWidth, &Pulse::setWidth);

    class_<Tri, bases<Osc>, TriPtr>("Tri")
        .add_property("width", &Tri::getWidth, &Tri::setWidth);

}
