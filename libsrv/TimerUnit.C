/*
 * TimerUnit.C: survivor TimerUnit object
 *
 * Version: $Revision: 0.7 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2003/04/09 20:23:50 $
 * MT-Level: Safe
 *
 * Copyright (c) 2002 - 2003
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: TimerUnit.C,v $
 * Revision 0.7  2003/04/09 20:23:50  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.6  2003/04/06 13:45:38  benno
 * Use Debugger
 *
 * Revision 0.5  2003/01/24 18:18:41  benno
 * pthread_t may be a pointer
 * Use IONULL
 *
 * Revision 0.4  2002/04/04 20:11:53  benno
 * copyright
 *
 * Revision 0.3  2002/04/03 18:53:25  benno
 * rcsify date
 *
 * Revision 0.2  2002/04/03 18:53:14  benno
 * add pid and signal number
 *
 * Revision 0.1  2002/04/03 18:52:38  benno
 * initial revision
 *
 */

#include "survivor.H"

TimerUnit::TimerUnit(pid_t pid, pthread_t tid, int signal, time_t when)
{
  // Allocate and initialize a new TimerUnit object, used to note that
  // <signal> should be sent to the <pid> or <tid> at <when>.  To
  // specify no <pid> or no <tid>, pass -1 for the appropriate argument.

  // Returns: A new TimerUnit object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "TimerUnit::TimerUnit(%d,%d,%d,%d)",
		   pid, tid, signal, when);
  dlog->log_exit(DEBUG_MINTRC, "TimerUnit::TimerUnit()");
#endif

  p = pid;
  s = signal;
  t = tid;
  w = when;
}

bool TimerUnit::now()
{
  // Determine if it is time to notify the thread.

  // Returns: true if the requested time is now or has passed, false
  // otherwise.

  bool r = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "TimerUnit::now()");
#endif

  struct timeval n;

  if(gettimeofday(&n, NULL)==0)
  {
    if(n.tv_sec >= w)
      r = true;
  }
  else
    wlog->warn("gettimeofday failed in TimerUnit::now");

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "TimerUnit::now = %s", IOTF(r));
#endif
  
  return(r);
}

pid_t TimerUnit::pid()
{
  // Obtain the pid for this TimerUnit.

  // Returns: The pid, or -1 if no pid was specified.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "TimerUnit::pid()");
  dlog->log_exit(DEBUG_MINTRC, "TimerUnit::pid = %d", p);
#endif
  
  return(p);
}

int TimerUnit::signal()
{
  // Obtain the signal for this TimerUnit.

  // Returns: The signal.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "TimerUnit::signal()");
  dlog->log_exit(DEBUG_MINTRC, "TimerUnit::signal = %d", s);
#endif
  
  return(s);
}

pthread_t TimerUnit::tid()
{
  // Obtain the tid for this TimerUnit.

  // Returns: The tid, or -1 if no tid was specified.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "TimerUnit::tid()");
  dlog->log_exit(DEBUG_MINTRC, "TimerUnit::tid = %d", t);
#endif
  
  return(t);
}

TimerUnit::~TimerUnit()
{
  // Delete the TimerUnit object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "TimerUnit::~TimerUnit()");
#endif
  
  p = -1;
  s = 0;
#if defined(_PTHREADT_AS_POINTER)
  t = NULL;
#else
  t = -1;
#endif // _PTHREADT_AS_POINTER
  w = 0;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "TimerUnit::~TimerUnit()");
#endif
}
