// Copyright  : Dennis Buis (2017)
// License    : MIT
// Platform   : Arduino
// Library    : Simple Control Library for Arduino
// File       : SimpleScheluder.h
// Purpose    : Receiving input from buttons (incuding auto-repeat)
// Repository : https://github.com/DennisB66/Simple-Control-Library-for-Arduino

#ifndef _SIMPLE_SCHEDULER_H
#define _SIMPLE_SCHEDULER_H

#include "SimpleTask.h"

class SimpleSchedulerTask : public SimpleTask
{
public:
  SimpleSchedulerTask( TaskFunc);
};

class SimpleScheduler : public SimpleTaskList
{
public:
  SimpleScheduler( unsigned long = 1);

  virtual void attachHandler( TaskFunc);

  void start();
  void stop();

protected:
  unsigned long _msec;
};

#endif
