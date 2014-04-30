/*
 * Timer.C: Timer object
 *
 * Version: $Revision: 0.7 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2003/04/09 19:50:04 $
 * MT-Level: Safe
 *
 * Copyright (c) 2002 - 2003
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: Timer.C,v $
 * Revision 0.7  2003/04/09 19:50:04  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.6  2003/01/29 01:58:45  benno
 * Process groups
 * pthread_t as pointer
 * IONULL, IOTF, xdelete, xadelete
 *
 * Revision 0.5  2002/05/02 23:00:15  toor
 * kill becomes process_kill
 * unused kill should have been pthread_kill
 *
 * Revision 0.4  2002/04/04 20:51:31  benno
 * copyright
 *
 * Revision 0.3  2002/04/03 19:35:22  benno
 * rcsify date
 *
 * Revision 0.2  2002/04/03 19:35:12  benno
 * Kill pids in addition to threads
 *
 * Revision 0.1  2002/04/03 19:34:36  benno
 * initial revision
 *
 */

#include "scheduler.H"

Timer::Timer()
{
  // Allocate and initialize a new Timer object.

  // Returns: A new Timer object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "Timer::Timer()");
  dlog->log_exit(DEBUG_MAJTRC, "Timer::Timer()");
#endif
  
  exit = false;
  ts = NULL;
}

void Timer::begin()
{
  // Instruct the Timer to begin operation.  It is intended that this method
  // is dedicated to one thread, and that this method will not return until
  // the program is ready to exit.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "Timer::begin()");
#endif

  ts = new Array<TimerUnit>();

  if(!ts)
  {
    exit = true;
    wlog->warn("Timer::begin failed to allocate TimerUnit Array");
  }
  
  while(!exit)
  {
    // Obtain a lock on the Array

    if(locks->ta_lock())
    {
      // Loop backwards through the Array, sending signals as appropriate.
      // We loop backwards because when we remove an item, we change all
      // the indicies after it.

      for(int i = ts->entries()-1;i >= 0;i--)
      {
	TimerUnit *tu = ts->retrieve(i);

	if(tu && tu->now())
	{
	  if(tu->pid() != -1)
	    killpg(tu->pid(), tu->signal());

#if defined(_PTHREADT_AS_POINTER)
	  if(tu->tid() != NULL)
#else
	  if(tu->tid() != -1)
#endif // _PTHREADT_AS_POINTER
	    pthread_kill(tu->tid(), tu->signal());

	  delete tu;
	  ts->remove(i);
	}
      }

      locks->ta_unlock();
    }

    // Wait a second before trying again
    sleep(1);
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "Timer::begin()");
#endif
}

bool Timer::end()
{
  // Tell the Timer to end operation.  This method returns when the
  // request has been sent, but the Timer may not have stopped by the
  // time this method returns.

  // Returns: true if the request is sent, false otherwise.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "Timer::end()");
  dlog->log_exit(DEBUG_MAJTRC, "Timer::end = true");
#endif

  exit = true;
  return(true);
}

bool Timer::request(int signal, int seconds)
{
  // Request that the calling thread be sent <signal> in <seconds>.

  // Returns: true if the request is queued, false otherwise.

  return(request(-1, signal, seconds));
}

bool Timer::request(pid_t pid, int signal, int seconds)
{
  // Request that the process <pid> be sent <signal> in <seconds>.

  // Returns: true if the request is queued, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Timer::request(%d,%d)", pid, seconds);
#endif

  // Obtain the lock on the TimerUnit Array before allocating the TimerUnit
  // in case we wait a long time to allocate it.  <seconds> should be from
  // when the request is queued, not when this method is invoked.
  
  if(ts)
  {
    if(locks->ta_lock())
    {
      struct timeval tv;
      
      if(gettimeofday(&tv, NULL)==0)
      {
	// Allocate a TimerUnit to be stored
	
	TimerUnit *t = NULL;

	t = new TimerUnit(pid,
#if defined(_PTHREADT_AS_POINTER)
			  (pid == -1 ? pthread_self() : NULL),
#else
			  (pid == -1 ? pthread_self() : -1),
#endif // _PTHREADT_AS_POINTER
			  signal, (tv.tv_sec + seconds));
	
	if(t)
	{
	  if(ts->add(t))
	    ret = true;
	  else
	  {
	    delete t;
	    wlog->warn("dlog->log_exitequest failed to queue request");
	  }
	}
	else
	  wlog->warn("Timer::request failed to allocate TimerUnit");
      }
      else
	wlog->warn("gettimeofday failed in Timer::request");
      
      locks->ta_unlock();
    }
    else
      wlog->warn("Timer::request failed to obtain Timer Array lock");
  }
  else
    wlog->warn("Timer Array does not exist at Timer::request");

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Timer::request = %s", IOTF(ret));
#endif
  
  return(ret);
}

bool Timer::unrequest()
{
  // Remove a pending notification to be sent to the calling thread.
  // If more than one notification is queued, only the first one found
  // will be removed.

  // Returns: true if a request is unqueued, false otherwise.

  return(unrequest(-1));
}

bool Timer::unrequest(pid_t pid)
{
  // Remove a pending notification to be sent to <pid>.  If more than
  // one notification is queued, only the first one found will be
  // removed.

  // Returns: true if a request is unqueued, false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Timer::unrequest()");
#endif

  if(ts)
  {
    if(locks->ta_lock())
    {
      for(int i = 0;i < ts->entries();i++)
      {
	TimerUnit *tu = ts->retrieve(i);

	if(tu &&
	   ((pid == -1 && tu->tid() == pthread_self())
	    || (pid != -1 && tu->pid() == pid)))
	{
	  delete tu;
	  ts->remove(i);
	  
	  ret = true;
	  break;
	}
      }
      
      locks->ta_unlock();
    }
    else
      wlog->warn("Timer::unrequest failed to get TimerUnit Array lock");
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Timer::unrequest = %s", IOTF(ret));
#endif
  
  return(ret);
}

Timer::~Timer()
{
  // Deallocate the Timer object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "Timer::~Timer()");
#endif
  
  exit = false;

  xadelete(ts, TimerUnit);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "Timer::~Timer()");
#endif
}
