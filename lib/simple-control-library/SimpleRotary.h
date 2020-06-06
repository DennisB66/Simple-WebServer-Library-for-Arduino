// Copyright  : Dennis Buis (2017)
// License    : MIT
// Platform   : Arduino
// Library    : Simple Control Library for Arduino
// File       : SimpleRotary.h
// Purpose    : handle input from a rotary encoder
// Repository : https://github.com/DennisB66/Simple-Control-Library-for-Arduino

#ifndef _SIMPLE_ROTARY_H
#define _SIMPLE_ROTARY_H

#include "SimpleDevice.h"

class SimpleRotary : public SimpleDevice
{
public:
  SimpleRotary( int, int);

  bool changed();
  int  position();
  void setPosition( int);

  void setMinMax( int, int, bool = false);
  void setMinMax( int, int, int, bool = false);

private:
  int _pinD0;                  // pin for rotate line D0 (must be 2 on Arduino Uno)
  int _pinD1;                  // pin for rotate line D1 (must be 3 on Arduino Uno)

  int  _pos;         // curr. encoder count value
  int  _prv;         // prev. encoder count value
  int  _posMin;      // encoder count minimal value
  int  _posMax;      // encoder count maxiaml value
  int  _posInc;
  bool _loop;        // encoder count loop indicator

  byte _bits;

  virtual void _handleDevice();
};

#endif
