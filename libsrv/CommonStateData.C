/*
 * CommonStateData.C: Object to manage data for State objects
 *
 * Version: $Revision: 0.2 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2006/10/17 14:03:27 $
 * MT-Level: Unsafe
 *
 * Copyright (c) 2004 - 2006
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: CommonStateData.C,v $
 * Revision 0.2  2006/10/17 14:03:27  benno
 * Add duration and since
 *
 * Revision 0.1  2004/06/11 22:14:14  benno
 * Initial revision
 *
 */

#include "survivor.H"

CommonStateData::CommonStateData(int instances, int returncode,
				 char *summary, time_t time,
				 time_t since, int duration)
{
  // Allocate and initialize a new CommonStateData object, holding
  // <instance>, <returncode>, <summary>, <time>, <since>, and <duration>.

  // Returns: A new CommonStateData object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC,
		  "CommonStateData::CommonStateData(%d,%d,%s,%d,%d,%d)",
		  instances, returncode, IONULL(summary), time, since,
		  duration);
#endif

  dur = duration;
  s = xstrdup(summary);
  insts = instances;
  rc = returncode;
  st = since;
  t = time;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CommonStateData::CommonStateData()");
#endif
}

int CommonStateData::duration()
{
  // Obtain the execution duration.

  // Returns: The execution duration, in milliseconds.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CommonStateData::duration()");
  dlog->log_exit(DEBUG_MINTRC, "CommonStateData::duration = %d", dur);
#endif
  
  return(dur);
}

int CommonStateData::instances()
{
  // Obtain the number of instances for the state data.

  // Returns: The number of instances.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CommonStateData::instances()");
  dlog->log_exit(DEBUG_MINTRC, "CommonStateData::instances = %d", insts);
#endif
  
  return(insts);
}

int CommonStateData::returncode()
{
  // Obtain the return code for the state data.

  // Returns: The return code.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CommonStateData::returncode()");
  dlog->log_exit(DEBUG_MINTRC, "CommonStateData::returncode = %d", rc);
#endif
  
  return(rc);
}

time_t CommonStateData::since()
{
  // Obtain the time of the first instance of this return.

  // Returns: The time, in seconds since the epoch.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CommonStateData::since()");
  dlog->log_exit(DEBUG_MINTRC, "CommonStateData::since = %d", st);
#endif
  
  return(st);
}

char *CommonStateData::summary()
{
  // Obtain the summary for the state data.

  // Returns: The summary, or NULL on error.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CommonStateData::summary()");
  dlog->log_exit(DEBUG_MINTRC, "CommonStateData::summary = %s", IONULL(s));
#endif
  
  return(s);
}

time_t CommonStateData::time()
{
  // Obtain the time for the state data.

  // Returns: The time, in seconds since the epoch.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CommonStateData::time()");
  dlog->log_exit(DEBUG_MINTRC, "CommonStateData::time = %d", t);
#endif
  
  return(t);
}

CommonStateData::~CommonStateData()
{
  // Deallocate the CommonStateData object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CommonStateData::~CommonStateData()");
#endif

  xdelete(s);
  dur = 0;
  insts = 0;
  rc = 0;
  st = 0;
  t = 0;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CommonStateData::~CommonStateData()");
#endif
}
