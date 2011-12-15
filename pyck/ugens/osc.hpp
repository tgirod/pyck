#ifndef OSC_HPP
#define OSC_HPP

#include "../core.hpp"

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <cmath>

// structs
struct Osc;
struct SinOsc;

// shared pointers
typedef boost::shared_ptr<Osc> OscPtr;
typedef boost::shared_ptr<SinOsc> SinOscPtr;

struct Osc : UGen, public boost::enable_shared_from_this<Osc>
{
    float freq;
    float phase;
    float gain;
    
    Osc();
    ~Osc();
    
    float getFreq();
    virtual void setFreq(float freq);
    
    float getPhase();
    virtual void setPhase(float phase);
    
    float getGain();
    virtual void setGain(float gain);
    
};

struct SinOsc : Osc, public boost::enable_shared_from_this<SinOsc>
{
    float y[3]; // previous values
    float w; // angular speed in radians/sample
    float p; // iteration constant
    
    SinOsc();
    ~SinOsc();
    
    void setFreq(float freq);
    void setPhase(float phase);

    void update();
    void compute();    
};

#endif
