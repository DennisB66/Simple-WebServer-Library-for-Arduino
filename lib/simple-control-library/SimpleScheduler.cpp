// Copyright  : Dennis Buis (2017)
// License    : MIT
// Platform   : Arduino
// Library    : Simple Control Library for Arduino
// File       : SimpleScheluder.cpp
// Purpose    : Receiving input from buttons (incuding auto-repeat)
// Repository : https://github.com/DennisB66/Simple-Control-Library-for-Arduino

#include <Arduino.h>
#include "SimpleScheduler.h"
#include "SimpleDevice.h"
#include "SimpleUtils.h"

#if defined(ESP8266)
extern "C" {
#include "user_interface.h"
}
os_timer_t myTimer;
#else
#include "TimerOne.h"
#endif

SimpleSchedulerTask::SimpleSchedulerTask( TaskFunc func)
: SimpleTask( func)
{
}

SimpleScheduler::SimpleScheduler( unsigned long msec)
: SimpleTaskList()
{
  _msec = _max( 1, msec);

#if defined(ESP8266)
  os_timer_disarm( &myTimer);
  os_timer_setfn ( &myTimer, _handle, NULL);
  //os_timer_arm   ( &Timer1, _msec  , true);
#else
  Timer1.initialize( _msec * 1000);
#endif

  attachHandler( SimpleDevice::handle);
  //start();
}

void SimpleScheduler::attachHandler( TaskFunc func)
{
  SimpleSchedulerTask* task = new SimpleSchedulerTask( func);

  _attach( task);
}

void SimpleScheduler::start()
{
#if defined(ESP8266)
  os_timer_arm( &myTimer, _msec, true);
#else
  Timer1.attachInterrupt( _handle);
#endif
}

void SimpleScheduler::stop()
{
#if defined(ESP8266)
  os_timer_disarm( &myTimer);
#else
  Timer1.detachInterrupt();
#endif
}
