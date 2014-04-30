/*
 * RunningState.C: Object to manage state information for scheduler running
 * status
 *
 * Version: $Revision: 0.8 $
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
 * $Log: RunningState.C,v $
 * Revision 0.8  2003/04/09 20:23:50  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.7  2003/04/05 23:59:53  benno
 * Use Debugger
 *
 * Revision 0.6  2003/01/24 18:06:48  benno
 * Add IONULL and IOTF
 *
 * Revision 0.5  2002/09/05 21:57:34  benno
 * use try_fopen
 *
 * Revision 0.4  2002/08/06 19:56:37  selsky
 * Remove embedded nulls from format
 *
 * Revision 0.3  2002/04/04 20:11:24  benno
 * copyright
 *
 * Revision 0.2  2002/04/03 18:30:05  benno
 * rcsify date
 *
 * Revision 0.1  2002/04/03 18:29:44  benno
 * initial revision
 *
 */

#include "survivor.H"

RunningState::RunningState()
{
  // Allocate a new RunningState object.

  // Returns: A new, initialized RunningState.
}

time_t RunningState::get(char *scheduler)
{
  // Determine the time the <scheduler> performed its last update.

  // Returns: The unix time, or 0 if not found.

  time_t r = 0;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "RunningState::get(%s)", IONULL(scheduler));
#endif

  if(scheduler)
  {
    char *sf = new char[strlen(args->statedir()) + strlen(scheduler) + 10];

    if(sf)
    {
      sprintf(sf, "%s/running/%s", args->statedir(), scheduler);

      struct stat sb;
      
      if(stat(sf, &sb)==0)
	r = sb.st_mtime;
      // if stat fails, fail silently since scheduler may not have run yet
      
      delete sf;
      sf = NULL;
    }
    else
      wlog->warn("RunningState::note failed to allocate sf");
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "RunningState::get = %d", r);
#endif

  return(r);
}

bool RunningState::note(char *scheduler)
{
  // Update the running state file for the <scheduler> by updating the
  // last modified time of the file.

  // Returns: true if fully successful, false otherwise.
  
  bool r = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "RunningState::note(%s)", IONULL(scheduler));
#endif
  
  if(scheduler)
  {
    char *sf = new char[strlen(args->statedir()) + strlen(scheduler) + 10];

    if(sf)
    {
      sprintf(sf, "%s/running/%s", args->statedir(), scheduler);
  
      FILE *touch = try_fopen(sf, "w");

      if(touch)
      {
	fclose(touch);
	r = true;
      }
      else
	wlog->warn("RunningState::note failed to touch '%s'", sf);

      xdelete(sf);
    }
    else
      wlog->warn("RunningState::note failed to allocate sf");
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "RunningState::note = %s", IOTF(r));
#endif
  
  return(r);
}

RunningState::~RunningState()
{
  // Deallocate the RunningState object.
  
  // Returns: Nothing.
}
