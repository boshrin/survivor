/*
 * AlertSchedule.C: An object to hold an alertplan schedule stanza.
 *
 * Version: $Revision: 0.10 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2003/10/06 22:51:52 $
 * MT-Level: Safe
 *
 * Copyright (c) 2002 - 2003
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: AlertSchedule.C,v $
 * Revision 0.10  2003/10/06 22:51:52  benno
 * Update constructor target/replacement documentation
 * Add fix if defined support
 * Fix several typos
 *
 * Revision 0.9  2003/04/09 20:23:44  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.8  2003/04/04 03:46:32  benno
 * Don't use string as buffer for debugging
 *
 * Revision 0.7  2003/04/03 18:49:35  benno
 * Use Debugger
 *
 * Revision 0.6  2003/03/06 02:41:52  benno
 * Fix type declaration in escalate_next
 *
 * Revision 0.5  2003/03/04 17:54:53  benno
 * Bump copyright
 *
 * Revision 0.4  2003/01/23 20:11:39  benno
 * Use IONULL, IOTF, xdelete, and xadelete
 *
 * Revision 0.3  2002/12/16 00:35:18  benno
 * Remove notify on clear
 *
 * Revision 0.2  2002/12/10 18:10:23  benno
 * Add lots of functionality
 *
 * Revision 0.1  2002/12/06 21:37:43  benno
 * Initial revision
 *
 */

#include "survivor.H"

AlertSchedule::AlertSchedule(Array<Schedule> *schedule, Array<AlertTry> *tries)
{
  // Allocate and initialize a new AlertSchedule object,  <schedule>
  // is the schedule during which this object applies.  <tries>
  // is an Array of the AlertTrys that apply.  The AlertSchedule object
  // will take over maintenance of <tries>.
  
  // Returns: A new AlertSchedule object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertSchedule::AlertSchedule(%d,%d)",
		   schedule, tries);
#endif

  ats = tries;
  sched = schedule;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertSchedule::AlertSchedule()");
#endif
}

AlertSchedule::AlertSchedule(AlertSchedule *as, CallList *target,
			     CallList *replacement)
{
  // Allocate and initialize a new AlertSchedule object, based on the
  // contents of <as>, but replacing <target> with <replacement> in
  // any AlertTry objects stored within.  If <target> is NULL,
  // <replacement> will be appended instead.

  // Returns: A new AlertSchedule object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertSchedule::AlertSchedule(%d,%d,%d)",
		   as, target, replacement);
#endif

  // Set up safe defaults in case we encounter an error

  ats = NULL;
  sched = NULL;

  if(as)
  {
    // We allocate and maintain the AlertTrys, but sched is just a pointer
    // to the cf.

    Array<AlertTry> *srctrys = as->_alltrys();
    ats = new Array<AlertTry>();

    if(ats && srctrys)
    {
      for(int i = 0;i < srctrys->entries();i++)
      {
	if(srctrys->retrieve(i))
	{
	  AlertTry *t = new AlertTry(srctrys->retrieve(i), target,
				     replacement);

	  if(t)
	  {
	    if(!ats->add(t))
	    {
	      delete t;
	      wlog->warn("AlertSchedule::AlertSchedule unable to insert AlertTry");
	    }
	  }
	  else
	    wlog->warn("AlertSchedule::AlertSchedule unable to allocate AlertTry");
	}
      }
      
      sched = as->schedule();
    }
    else
      wlog->warn("AlertSchedule::AlertSchedule unable to allocate Array");
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertSchedule::AlertSchedule()");
#endif
}

Array<AlertTry> *AlertSchedule::_alltrys()
{
  // This method is for use by the configuration parser only and is otherwise
  // not to be used.

  // Returns: Something or nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertSchedule::_alltrys()");
  dlog->log_exit(DEBUG_MINTRC, "AlertSchedule::_alltrys = %d", ats);
#endif
  
  return(ats);
}

AlertTry *AlertSchedule::alerttry(int attempt, bool fixdefined)
{
  // Find the AlertTry for attempt number <attempt>.  If <fixdefined>
  // is false, then skip any if_defined_fix trys.

  // Returns: A pointer to the AlertTry, if found, or NULL otherwise,
  // including if there is no appropriately defined value.

  AlertTry *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertSchedule::alerttry(%d)", attempt);
#endif
  
  if(ats)
  {
    // Keep count of the number of attempts we have passed through as
    // we iterate through the AlertTry Array.  If we find a -1 entry,
    // we stop and return that.

    int count = 0;
    
    for(int i = 0;i < ats->entries();i++)
    {
      AlertTry *at = ats->retrieve(i);

      if(at && ((at->fix() != if_defined_fix) || fixdefined))
      {
	if(at->attempts() == -1)
	  ret = at;
	else
	{
	  count += at->attempts();

	  if(attempt <= count)
	    ret = at;
	}

	if(ret)
	{
#if defined(DEBUG)
	  if(at->attempts() == -1)
	    dlog->log_progress(DEBUG_SCHEDS,
	 "-- Found matching Try for %d attempts with default match at line %d",
				attempt, i);
	  else
	    dlog->log_progress(DEBUG_SCHEDS,
	 "-- Found matching Try for %d attempts, upper bound is %d at line %d",
				attempt, count, i);
#endif
	  
	  break;
	}
      }
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertSchedule::alerttry = %d", ret);
#endif
  
  return(ret);
}

int AlertSchedule::escalate_next(int from)
{
  // Determine the next level of escalation, starting at <from>.

  // Returns: The next number of tries needed to reach the next
  // escalation level, or 0 on error.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertSchedule::escalate_next(%d)", from);
#endif
  
  int ret = 0;

  if(ats && from > -1)
  {
    // Keep count of the number of attempts we have passed through as
    // we iterate through the AlertTry Array.  If we find a -1 entry,
    // there are no more levels of escalation.

    int count = 0;

    for(int i = 0;i < ats->entries();i++)
    {
      AlertTry *at = ats->retrieve(i);

      if(at)
      {
	// default entries add 1 to the count and then interrupt the loop

	if(at->attempts() == -1)
	  count++;
	else
	  count += at->attempts();

	if(count > from)
	  ret = count;

	if(ret > 0 || at->attempts() == -1)
	  break;
      }
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertSchedule::escalate_next = %d", ret);
#endif
  
  return(ret);
}

bool AlertSchedule::escalated(AlertTry *t)
{
  // Determine if the AlertTry <t> is considered escalated according to
  // this AlertSchedule.

  // Returns: true if <t> should be considered escalated, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertSchedule::escalated(%d)", t);
#endif

  if(t && ats)
  {
    int escat = 1;  // The index where escalation starts (counting from 0)
    int tat = -1;   // The index where <t> is found

    for(int i = 0;i < ats->entries();i++)
    {
      AlertTry *xt = ats->retrieve(i);

      if(xt)
      {
	if(xt == t)
	  tat = i;

	if(xt->escalated())
	  escat = i;
      }
    }

    // Now that we've swept through, compare the results.

    if(tat >= escat)
      ret = true;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertSchedule::escalated = %s", IOTF(ret));
#endif
  
  return(ret);
}

bool AlertSchedule::now()
{
  // Determine if this AlertSchedule is currently in effect.  The
  // alertshift value, if set, is taken into account.

  // Returns: true if this AlertSchedule is currently in effect, false
  // otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertSchedule::now()");
#endif

  if(sched)
  {
    for(int i = 0;i < sched->entries();i++)
    {
      Schedule *s = sched->retrieve(i);

      if(s && s->now(-1 * cf->alert_shift()))
      {
	ret = true;
	break;
      }
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertSchedule::now = %s", IOTF(ret));
#endif

  return(ret);
}

bool AlertSchedule::now(time_t last)
{
  // Determine if this AlertSchedule indicates an alert should be
  // schedule for execution.  The alertshift value, if set, is taken
  // into account.

  // Returns: true if this Schedule should be executed, false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertSchedule::now(%d)", last);
#endif

  if(sched)
  {
    int ashift = -1 * cf->alert_shift();
    
    for(int i = 0;i < sched->entries();i++)
    {
      Schedule *s = sched->retrieve(i);

      if(s && s->now(ashift))
      {
	if(s->now(last, ashift))
	  ret = true;

	// We stop on the first schedule found to be in effect, unless
	// we didn't run and s is an "at" schedule.  (This is the same
	// logic that schedule_check uses.)

	if(ret || !s->at())
	  break;
      }
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertSchedule::now = %s", IOTF(ret));
#endif

  return(ret);
}

Array<Schedule> *AlertSchedule::schedule()
{
  // Obtain the Schedule for this AlertSchedule.

  // Returns: A pointer to the Array of Schedules, or NULL on error.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertSchedule::schedule()");
  dlog->log_exit(DEBUG_MINTRC, "AlertSchedule::schedule = %d", sched);
#endif

  return(sched);
}

AlertSchedule::~AlertSchedule()
{
  // Deallocate the AlertSchedule object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertSchedule::~AlertSchedule()");
#endif

  xadelete(ats, AlertTry);
  sched = NULL;
    
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertSchedule::~AlertSchedule()");
#endif
}
