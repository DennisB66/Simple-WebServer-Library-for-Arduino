// Copyright  : Dennis Buis (2017)
// License    : MIT
// Platform   : Arduino
// Library    : Simple Control Library for Arduino
// File       : SimpleRotary.cpp
// Purpose    : handle input from a rotary encoder
// Repository : https://github.com/DennisB66/Simple-Control-Library-for-Arduino

#include <Arduino.h>
#include "SimpleRotary.h"
#include "SimplePrint.h"

SimpleRotary::SimpleRotary( int pinD0, int pinD1) : SimpleDevice()
{
  pinMode( _pinD0 = pinD0, INPUT_PULLUP); // line D0 = encoder rotate (-)
  pinMode( _pinD1 = pinD1, INPUT_PULLUP); // line D1 = encoder rotate (+)

  _bits   = 0;
  _posInc = 1;
  // noInterrupts();
  // attachInterrupt( digitalPinToInterrupt( _pinD0), _pulseDetect, RISING);
  // attachInterrupt( digitalPinToInterrupt( _pinD1), _pulseDetect, RISING);
  // interrupts();
}

// SimpleRotary::~SimpleRotary()
// {
//   // noInterrupts();
//   // detachInterrupt( digitalPinToInterrupt( _pinD0));
//   // detachInterrupt( digitalPinToInterrupt( _pinD1));
//   // interrupts();
// }


bool SimpleRotary::changed()
{
  if ( _prv != _pos) {                 // if  new value
    _prv = _pos;                       // set new value

    return true;                          // success:    new value available
  } else {
    return false;                         // failure: no new value available
  }
}

int SimpleRotary::position()
{
  return _pos;
}

void SimpleRotary::setPosition( int pos)
{
  _pos = pos;

  if ( _pos < _posMin) {
    _pos = ( _loop) ? _posMax : _posMin;
  } else
  if ( _pos > _posMax) {
    _pos = ( _loop) ? _posMin : _posMax;
  }

  _prv = _pos;

  changed();
}

void SimpleRotary::setMinMax( int min, int max, bool loop)
{
  setMinMax( min, max, 1, loop);
}

void SimpleRotary::setMinMax( int min, int max, int inc, bool loop)
{
  if ( min < max) {
    _posMin =  min;
    _posMax =  max;
    _posInc =  inc;
    _loop   =  loop;
  }  else {
    _posMin =  max;
    _posMax =  min;
    _posInc = -inc;
    _loop   =  loop;
  }

  setPosition( _pos);
}

void SimpleRotary::_handleDevice()
{
 _bits |= ( digitalRead( _pinD0) << 3) | ( digitalRead( _pinD1) << 2);

  //if (( _bits == 0b1101) || ( _bits == 0b0100) || ( _bits == 0b0010) || ( _bits == 0b1011)) {}
  //if (( _bits == 0b1110) || ( _bits == 0b0111) || ( _bits == 0b0001) || ( _bits == 0b1000)) {}

  _pos += ( _posInc * ( _bits == 0b1101));
  _pos -= ( _posInc * ( _bits == 0b1110));

  _bits >>= 2;                                              // move current values for pinD0/pinD1 to bits 1/2

  if ( _pos < _posMin) {
    _pos = ( _loop) ? _posMax : _posMin;
  } else
  if ( _pos > _posMax) {
    _pos = ( _loop) ? _posMin : _posMax;
  }

  //interrupts();                           // allow new interrupts
}
