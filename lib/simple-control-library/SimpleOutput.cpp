// Copyright  : Dennis Buis (2017)
// License    : MIT
// Platform   : Arduino
// Library    : Simple Control Library for Arduino
// File       : SimpleOutput.cpp
// Purpose    : Controling Digital Output
// Repository : https://github.com/DennisB66/Simple-Control-Library-for-Arduino

#include "SimpleOutput.h"
#include "SimpleUtils.h"

SimpleOutput::SimpleOutput( int pin, int state) : SimpleDevice()
{
  pinMode     ( _pin = pin, OUTPUT);
  digitalWrite( _pin      , _initState = state);    // sets the LED off

  _patternMode = OUTPUT_SINGLE;
}

void SimpleOutput::on()
{
  stop();

  _nextState = ( _initState == OUTPUT_OFF) ? OUTPUT_ON : OUTPUT_OFF;
}

void SimpleOutput::off()
{
  stop();

  _nextState = ( _initState == OUTPUT_OFF) ? OUTPUT_OFF : OUTPUT_ON;
}

void SimpleOutput::toggle()
{
  stop();

  _nextState = ( _nextState == OUTPUT_OFF) ? OUTPUT_ON : OUTPUT_OFF;
}

void SimpleOutput::blink( unsigned long delay)
{
  setPattern( BLINK_PATTERN, delay, OUTPUT_REPEAT);     // sets the LED off
}

void SimpleOutput::setPattern( unsigned long pattern, unsigned long delay, int mode, bool activate)
{
  _pattern      = pattern;
  _patternDelay = delay;
  _patternMode  = mode;
  _patternIndex = 0;
  _patternCount = 0;

  for ( int i = 0; i < 32; i++) {
    if ( pattern) _patternCount++;
    pattern >>= 1;
  }

  if ( activate) start();
}

void SimpleOutput::start()
{
  _patternIndex    = 0;
  _patternStepping = true;

  _timer.lapse( _patternDelay);
}

void SimpleOutput::stop()
{
  _patternIndex    = 0;
  _patternStepping = false;
}

void SimpleOutput::_handleDevice()
{
  if ( _patternStepping && _timer.check()) {
    if ( _pattern & ((unsigned long) 1 << (_patternCount - _patternIndex - 1))) {
      _nextState = ( _initState == OUTPUT_OFF) ? OUTPUT_ON : OUTPUT_OFF;
    } else {
      _nextState = ( _initState == OUTPUT_OFF) ? OUTPUT_OFF : OUTPUT_ON;
    }
    _patternIndex    = ( _patternIndex + 1) % _patternCount;
    _patternStepping = ( _patternIndex > 0) | _patternMode;
  }

  if ( _currState != _nextState) {
    digitalWrite( _pin, _currState = _nextState);             // sets the LED off
  }
}
