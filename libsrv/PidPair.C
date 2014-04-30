/*
 * PidPair.C: survivor PidPair object
 *
 * Version: $Revision: 0.4 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2003/04/09 20:23:49 $
 * MT-Level: Safe
 *
 * Copyright (c) 2002 - 2003
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: PidPair.C,v $
 * Revision 0.4  2003/04/09 20:23:49  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.3  2003/04/05 23:31:42  benno
 * Use Debugger
 *
 * Revision 0.2  2003/03/04 20:29:33  benno
 * Bump copyright
 *
 * Revision 0.1  2002/05/02 22:58:59  toor
 * initial revision
 *
 */

#include "survivor.H"

PidPair::PidPair(pid_t pid, pid_t ppid)
{
  // Allocate and initialize a new PidPair object, used to store <pid>
  // and its parent <ppid>.

  // Returns: A new PidPair object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "PidPair::PidPair(%d,%d)", pid, ppid);
  dlog->log_exit(DEBUG_MINTRC, "PidPair::PidPair()");
#endif

  p = pid;
  pp = ppid;
}

pid_t PidPair::pid()
{
  // Obtain the pid for this PidPair.

  // Returns: The pid.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "PidPair::pid()");
  dlog->log_exit(DEBUG_MINTRC, "PidPair::pid = %d", p);
#endif
  
  return(p);
}

pid_t PidPair::ppid()
{
  // Obtain the parent pid for this PidPair.

  // Returns: The parent pid (as set when this PidPair was allocated).

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "PidPair::ppid()");
  dlog->log_exit(DEBUG_MINTRC, "PidPair::ppid = %d", pp);
#endif
  
  return(pp);
}

PidPair::~PidPair()
{
  // Delete the PidPair object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "PidPair::~PidPair()");
#endif
  
  p = -1;
  pp = -1;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "PidPair::~PidPair()");
#endif
}
