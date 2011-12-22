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

    float getFreq();
    void setFreq(float freq);
    
    float getPhase();
    void setPhase(float phase);
    
    float getGain();
    void setGain(float gain);

    // (re)initialize internal values whenever a parameter is changed.
    virtual void init();
    virtual void compute();
};

struct Sin : Osc
{
    float y[3]; // previous values
    float p; // iteration constant
    
    Sin();
    ~Sin();

    virtual void init();
    virtual void compute();    
};

struct Square : Osc
{

  Square();
  ~Square();

  void compute();
};

struct Saw : Osc
{
  Saw();
  ~Saw();

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
