/*
 * ScheduleAsOd.C: An object to sort schedules based on arbitrary times.
 *
 * Version: $Revision: 0.1 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2006/01/25 00:43:31 $
 * MT-Level: Safe
 *
 * Copyright (c) 2005
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: ScheduleAsOf.C,v $
 * Revision 0.1  2006/01/25 00:43:31  benno
 * Initial revision
 *
 */

#include "survivor.H"

ScheduleAsOf::ScheduleAsOf(Schedule *sched, time_t asof)
{
  // Allocate and initialize a new ScheduleAsOf object, pointing to <s>
  // and using a sortkey based on s->next(<asof>).

  // Returns: A newly allocated ScheduleAsOf object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "ScheduleAsOf::ScheduleAsOf(%d,%d)",
		  sched, asof);
#endif

  s = sched;

  if(sched && sched->at())
    sk = sched->next(asof);
  else
    sk = 0;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "ScheduleAsOf::ScheduleAsOf()");
#endif
}

Schedule *ScheduleAsOf::schedule()
{
  // Obtain the Schedule for this ScheduleAsOf.

  // Returns: A pointer to the Schedule, or NULL.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "ScheduleAsOf::schedule()");
  dlog->log_exit(DEBUG_MINTRC, "ScheduleAsOf::schedule = %d", s);
#endif
  
  return(s);
}

int ScheduleAsOf::sort_key()
{
  // Return a sort key for this object.

  // Returns: An integer useful for sorting.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "ScheduleAsOf::sort_key()");
  dlog->log_exit(DEBUG_MINTRC, "ScheduleAsOf::sort_key = %d", sk);
#endif
  
  return(sk);
}

ScheduleAsOf::~ScheduleAsOf()
{
  // Deallocate the ScheduleAsOf object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "ScheduleAsOf::~ScheduleAsOf()");
#endif

  s = NULL;
  sk = 0;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "ScheduleAsOf::~ScheduleAsOf()");
#endif
}
