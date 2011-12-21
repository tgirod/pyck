#ifndef OSC_HPP
#define OSC_HPP

#include "../core.hpp"

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <cmath>

// structs
struct Osc;
struct Sin;
struct Square;
struct Saw;
struct Pulse;

// shared pointers
typedef boost::shared_ptr<Osc> OscPtr;
typedef boost::shared_ptr<Sin> SinPtr;
typedef boost::shared_ptr<Square> SquarePtr;
typedef boost::shared_ptr<Saw> SawPtr;
typedef boost::shared_ptr<Pulse> PulsePtr;

struct Osc : UGen
{
    float freq;
    float phase;
    float gain;
    
    float w; // angular speed in radians/sample

    Osc();
    ~Osc();

    virtual void init();
    virtual void compute();

    float getFreq();
    virtual void setFreq(float freq);
    
    float getPhase();
    virtual void setPhase(float phase);
    
    float getGain();
    virtual void setGain(float gain);
    
};

struct Sin : Osc
{
    float y[3]; // previous values
    float p; // iteration constant
    
    void init();
    void compute();    
};

struct Square : Osc
{
  void compute();
};

struct Saw : Osc
{
  void compute();
};

struct Pulse : Osc
{
  float pwm;

  Pulse();
  ~Pulse();

  void compute();

  float getPwm();
  void setPwm(float pwm);
};

#endif
