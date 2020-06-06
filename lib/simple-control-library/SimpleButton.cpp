// Copyright  : Dennis Buis (2017)
// License    : MIT
// Platform   : Arduino
// Library    : Simple Control Library for Arduino
// File       : SimpleButton.cpp
// Purpose    : Receiving input from buttons (incuding buffering and auto-repeat)
// Repository : https://github.com/DennisB66/Simple-Control-Library-for-Arduino

#include "SimpleButton.h"
#include "SimplePrint.h"

SimpleButton::SimpleButton( int pinD, byte mode) : SimpleDevice()
{
  _pinD = pinD;
  _mode = mode;

  _state = BUTTON_WAIT_FOR_CLICK;       // state for state engine
  _ticks = millis();                    // stopwatch for current  click
  _count = 0;                           // number of unprocessed clicks

  pinMode( _pinD, INPUT_PULLUP);
}

#define buffered (( BUTTON_BUFFER_LENGTH + _next - _last) % BUTTON_BUFFER_LENGTH)
                                                            // calculate ring buffer population
byte SimpleButton::available()
{
  return buffered;                                          // return availabe clicks in buffer
}

byte SimpleButton::read()
{
  return _curr = _getNextClick();                           // read click from buffer
}

byte SimpleButton::lastValue()
{
  return _curr;                                             // return last read click
}

void SimpleButton::_handleDevice()                          // simple state engine
{
  if ( digitalRead( _pinD) == LOW) {                        // if button is active
    switch ( _state) {
    case BUTTON_WAIT_FOR_CLICK:
      _ticks = millis();                                     // reset stopwatch for current click
      //count = 1;                                              // keep number of unprocessed clicks
      _count++;                                              // keep number of unprocessed clicks
      _state = BUTTON_CHECK_BOUNCING;                        // goto next state (bouncing check)
      break;

    case BUTTON_CHECK_BOUNCING:
      if ( millis() - _ticks > BUTTON_BOUNCE_DELAY) {        // check if button is still down after 25 msec
        _state = BUTTON_PREP_RETRIGGER;                      // goto next state (repeat mode preparation)
      }
      break;

     case BUTTON_PREP_RETRIGGER :
      if ( millis() - _ticks > BUTTON_DOUBLE_DELAY) {        // extra wait to allow for double clicks
        switch ( _mode) {
        case BUTTON_REPEAT:
          _addNextClick( BUTTON_NORMAL);                    // insert (normal) click in buffer
          _count = 0;
          _state = BUTTON_WAIT_RETRIGGER;                    // goto next state (repeat mode check)
          break;
        default:
          _addNextClick( BUTTON_HOLD);                      // insert (normal) click in buffer
          _count = 0;
          _state = BUTTON_LOOP_ENDLESSLY;                    // goto next state (all done)
          break;
        }
      }
      break;

    case BUTTON_WAIT_RETRIGGER:
      if ( millis() - _ticks > BUTTON_REPEAT_DELAY) {        // check if button is held long enough
        _state = BUTTON_LOOP_RETRIGGER;                      // goto next state (repeat mode active)
      }
      break;

    case BUTTON_LOOP_RETRIGGER:
      if ( millis() - _ticks > BUTTON_REPEAT_SPEED) {        // check if ready to repeat click
        _addNextClick( BUTTON_NORMAL);                      // insert (normal) click in buffer
        _ticks = millis();                                   // reset stopwatch for current click
      }
      break;

    case BUTTON_LOOP_ENDLESSLY:                             // loop until buton is released
      break;
    }
  } else {                                                  // button is released (potential double click)
    switch ( _state) {
    case BUTTON_CHECK_BOUNCING:
      break;
    default:
      if ( _count  && ( millis() - _ticks > BUTTON_DOUBLE_DELAY)) {
                                                            // provide extra time to check double clicks
        _addNextClick(( _count == 1) ? BUTTON_NORMAL : BUTTON_DOUBLE);
                                                            // insert click in buffer
        _count = 0;                                            // reset unprocessed clicks
      };
      _state = BUTTON_WAIT_FOR_CLICK;                          // reset state engine
      break;
    }
  }
}

void SimpleButton::_addNextClick( byte c)
{
  if ( buffered < BUTTON_BUFFER_LENGTH) {                   // if free slots available
    _buffer[ _next++] = c;                                  // populate first free slot
    _next %= BUTTON_BUFFER_LENGTH;                          // keep pointer within boundaries
  }
}

byte SimpleButton::_getNextClick()
{
  if ( buffered) {                                          // if clicks available
    byte c = _buffer[ _last++];                             // read oldest click

    _last %= BUTTON_BUFFER_LENGTH;                          // keep pointer within boundaries

    return c;                                               // success: return click value
  } else {
    return 0;                                               // failure: return NOTHING
  }
}
