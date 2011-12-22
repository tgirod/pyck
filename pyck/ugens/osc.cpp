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
    cout << "Osc::init()" << endl;
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
    cout << "Sin::init()" << endl;
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
    pwm = 0.5;
}

Pulse::~Pulse()
{}

void Pulse::compute()
{
    if(M_PI*(pwm-0.5) < phase){
        output[0] = gain;
    } else {
        output[0] = -gain;
    }
    Osc::compute();
}

float Pulse::getPwm()
{
    return pwm;
}

void Pulse::setPwm(float pwm)
{
    if (0 <= pwm && pwm <= 1) {
        this->pwm = pwm;
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
        .add_property("pwm", &Pulse::getPwm, &Pulse::setPwm);

}
