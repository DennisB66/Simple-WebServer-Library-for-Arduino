// Copyright  : Dennis Buis (2017)
// License    : MIT
// Platform   : Arduino
// Library    : Simple Control Library for Arduino
// File       : SimpleDevice.h
// Purpose    : Receiving input from buttons (incuding auto-repeat)
// Repository : https://github.com/DennisB66/Simple-Control-Library-for-Arduino

#ifndef _SIMPLE_DEVICE_H
#define _SIMPLE_DEVICE_H

#include <Arduino.h>

#define MAX_DEVICES             8

class SimpleDevice
{
public:
  SimpleDevice();                       // constructor

  static void handle();                 // handle device actions

protected:
  static SimpleDevice* _device[MAX_DEVICES];
  static int           _deviceCount;
  int                  _deviceEntry;

  virtual void _handleDevice();
};

#endif
