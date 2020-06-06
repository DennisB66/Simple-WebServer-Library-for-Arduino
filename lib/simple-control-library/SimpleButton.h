// Copyright  : Dennis Buis (2017)
// License    : MIT
// Platform   : Arduino
// Library    : Simple Control Library for Arduino
// File       : SimpleButton.h
// Purpose    : Receiving input from buttons (incuding auto-repeat)
// Repository : https://github.com/DennisB66/Simple-Control-Library-for-Arduino

#ifndef _SIMPLE_BUTTON_H
#define _SIMPLE_BUTTON_H

#include <Arduino.h>
#include "SimpleDevice.h"

#define BUTTON_MAX              4

#define BUTTON_BOUNCE_DELAY    25       // min. time to filter contact bouncing (msec)
#define BUTTON_DOUBLE_DELAY   300       // max. time between double clicks      (msec)
#define BUTTON_REPEAT_DELAY  1000       // min. time before repeat activation   (msec)
#define BUTTON_REPEAT_SPEED   100       // min. time between repeated clicks    (msec)

#define BUTTON_WAIT_FOR_CLICK   0       // state 0: check for button down
#define BUTTON_CHECK_BOUNCING   1       // state 1: check for steady down
#define BUTTON_PREP_RETRIGGER   2       // state 2: prepare for button hold
#define BUTTON_WAIT_RETRIGGER   3       // state 3: check for button hold
#define BUTTON_LOOP_RETRIGGER   4       // state 4: repeat mode activated
#define BUTTON_LOOP_ENDLESSLY   5       // state 5: loop during button hold

#define BUTTON_BUFFER_LENGTH   32       // max buffered clicks

#define BUTTON_FAIL             0       // button error
#define BUTTON_NORMAL           1       // normal click
#define BUTTON_DOUBLE           2       // double click
#define BUTTON_HOLD             3       // button click & hold
#define BUTTON_REPEAT           4       // button repeat

class SimpleButton : public SimpleDevice
{
public:
  SimpleButton( int, byte = BUTTON_HOLD);// constructor

  byte available();                     // return available clicks in buffering
  byte read();                          // read oldest click from buffer
  byte lastValue();                     // return last read click value

private:
  byte _pinD;                           // connected pin
  byte _mode;                           // true = activates click repeat
  byte _curr;                           // last read click value
  byte _last;                           // first available click value in buffer
  byte _next;                           // first free position in buffer
  byte _buffer[ BUTTON_BUFFER_LENGTH];  // click value (ring) buffer

  byte           _state;                // state for state engine
  unsigned long _ticks;                 // timer for current  click
  unsigned long _count;                 // number of unprocessed clicks

  virtual void _handleDevice();
  void         _addNextClick( byte);    // add (newest) click value to buffer
  byte         _getNextClick();         // get (oldest) click value from buffer
};

#endif
