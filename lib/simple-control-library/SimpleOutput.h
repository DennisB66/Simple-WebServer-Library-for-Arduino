// Copyright  : Dennis Buis (2017)
// License    : MIT
// Platform   : Arduino
// Library    : Simple Control Library for Arduino
// File       : SimpleOutput.h
// Purpose    : Controling Digital Output
// Repository : https://github.com/DennisB66/Simple-Control-Library-for-Arduino

#ifndef _SIMPLE_OUTPUT_H
#define _SIMPLE_OUTPUT_H

#include <Arduino.h>
#include "SimpleDevice.h"
#include "SimpleUtils.h"
#include "SimpleTimer.h"

#define OUTPUT_OFF    0
#define OUTPUT_ON     1

#define OUTPUT_SINGLE 0
#define OUTPUT_REPEAT 1

#define BLINK_PATTERN 0b10
#define BLINK_DELAY   1000

class SimpleOutput : public SimpleDevice
{
public:
  SimpleOutput( int, int = OUTPUT_OFF);           // constructor

  void on();
  void off();
  void toggle();
  void blink( unsigned long = 1000);

  void setPattern( unsigned long, unsigned long, int = OUTPUT_SINGLE, bool = true);

  void start();
  void stop();

protected:
  int           _pin;

  bool          _initState;
  bool          _currState;
  bool          _nextState;

  unsigned long _pattern;
  unsigned long _patternDelay;
  int           _patternMode;

  int           _patternCount;
  int           _patternIndex;
  bool          _patternStepping;

  SimpleTimer   _timer;

  void         _init();
  void         _next();

  virtual void _handleDevice();
};

#endif
