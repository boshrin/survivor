/*
 * AlertTry.C: An object to hold alertplan try information.
 *
 * Version: $Revision: 0.8 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2003/10/06 22:52:57 $
 * MT-Level: Safe
 *
 * Copyright (c) 2002 - 2003
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: AlertTry.C,v $
 * Revision 0.8  2003/10/06 22:52:57  benno
 * Add support for appending a calllist
 *
 * Revision 0.7  2003/09/16 01:32:05  benno
 * Use fix_attempt_t
 *
 * Revision 0.6  2003/04/09 20:23:44  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.5  2003/04/03 22:47:20  benno
 * Use Debugger
 *
 * Revision 0.4  2003/03/04 17:56:07  benno
 * Bump copyright
 *
 * Revision 0.3  2003/01/23 22:29:30  benno
 * Use IONULL, IOTF, xdelete, and xadelete
 *
 * Revision 0.2  2002/12/16 00:40:25  benno
 * Remove notify on clear and notify on fix
 * AlertTry(AlertTry ...)
 *
 * Revision 0.1  2002/12/06 21:40:01  benno
 * Initial revision
 *
 */

#include "survivor.H"

AlertTry::AlertTry(int attempts, fix_attempt_t fix, Array<CallList> *notify,
		   int degrade, Array<Schedule> *degrade_schedule,
		   bool escalated)
{
  // Allocate and initialize a new AlertTry object.  <attempts> is the
  // number of times this try should be attempted (-1 for infinite),
  // <fix> indicates whether the predefined fix should be called,
  // <notify> is the set of call lists to notify (the Array will be
  // maintained by this object, but not the CallLists within),
  // <degrade> indicates the number of failed hosts that are
  // permitted, <degrade_schedule> is the schedule during which
  // <degrade> applies, and <escalated> indicates this try should be
  // considered the escalated state (default is the second try
  // stanza).

  // Returns: A new AlertTry object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertTry::AlertTry(%d,%d,%d,%d,%d,%s)",
		   attempts, fix, notify, degrade, degrade_schedule,
		   IOTF(escalated));
#endif

  cl = notify;
  ds = degrade_schedule;
  esc = escalated;
  fx = fix;
  atmp = attempts;
  deg = degrade;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertTry::AlertTry()");
#endif
}

AlertTry::AlertTry(AlertTry *at, CallList *target, CallList *replacement)
{
  // Allocate and initialize a new AlertTry object, based on the contents
  // of <at>, but replacing references to the CallList <target> with
  // references to the CallList <replacement>.  If <target> is NULL,
  // <replacement> will be added to the existing set.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertTry::AlertTry(%d,%d,%d)",
		  at, target, replacement);
#endif

  // Set up safe defaults in case we encounter an error
  
  cl = NULL;
  ds = NULL;
  esc = false;
  fx = no_fix;
  atmp = 0;
  deg = 0;
  
  // We allocate a new CallList Array, but everything else remains
  // as pointers to configuration data.

  if(at && at->notify() && replacement)
  {
    cl = new Array<CallList>();
    
    if(cl)
    {
      Array<CallList> *source = at->notify();

      for(int i = 0;i < source->entries();i++)
      {
	CallList *srccl = source->retrieve(i);

	if(srccl == target)
	  cl->add(replacement);
	else
	  cl->add(srccl);
      }

      // If target is NULL append replacement.
      
      if(!target)
	cl->add(replacement);
      
      ds = at->degrade_schedule();
      esc = at->escalated();
      fx = at->fix();
      atmp = at->attempts();
      deg = at->degrade();
    }
    else
      wlog->warn("AlertTry::AlertTry unable to allocate Array");
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertTry::AlertTry()");
#endif
}

int AlertTry::attempts()
{
  // Obtain the number of times this try should be attempted, with -1
  // indicating infinite.

  // Returns: The number of times.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertTry::attempts()");
  dlog->log_exit(DEBUG_MINTRC, "AlertTry::attempts = %d", atmp);
#endif

  return(atmp);
}

int AlertTry::degrade()
{
  // Obtain the number of failed hosts permitted by this try.

  // Returns: The number of permitted failed hosts.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertTry::degrade()");
  dlog->log_exit(DEBUG_MINTRC, "AlertTry::degrade = %d", deg);
#endif

  return(deg);
}

Array<Schedule> *AlertTry::degrade_schedule()
{
  // Obtain the schedule during which degrade() applies.

  // Returns: The degrade schedule, if set, or NULL..

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertTry::degrade_schedule()");
  dlog->log_exit(DEBUG_MINTRC, "AlertTry::degrade_schedule = %d", ds);
#endif

  return(ds);
}

bool AlertTry::escalated()
{
  // Determine if this try should be considered escalated.

  // Returns: true if this try is considered escalated, false otherwise.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertTry::escalated()");
  dlog->log_exit(DEBUG_MINTRC, "AlertTry::escalated = %s", IOTF(esc));
#endif

  return(esc);
}

fix_attempt_t AlertTry::fix()
{
  // Determine if this try indicates that the predefined fix should be
  // attempted.

  // Returns: A fix_attempt_t value, indicating how to behave.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertTry::fix()");
  dlog->log_exit(DEBUG_MINTRC, "AlertTry::fix = %d", fx);
#endif

  return(fx);
}

Array<CallList> *AlertTry::notify()
{
  // Obtain the Array of CallLists to be notified by this try.

  // Returns: The Array of CallLists if set, or NULL otherwise.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertTry::notify()");
  dlog->log_exit(DEBUG_MINTRC, "AlertTry::notify = %d", cl);
#endif

  return(cl);
}

AlertTry::~AlertTry()
{
  // Deallocate the AlertTry object.

  // Returns: Nothing.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertTry::~AlertTry()");
#endif

  // We delete the actual CallList Array since that was created just for
  // us, but not the CallLists (or the Schedule Array) which are pointers
  // to objects maintained by the Configuration file.

  xdelete(cl);
  ds = NULL;
  esc = false;
  fx = no_fix;
  atmp = 0;
  deg = 0;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertTry::~AlertTry()");
#endif
}
