/*
 * LockData.C: Object to manage data to be locked.
 *
 * Version: $Revision: 0.2 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2003/04/09 20:29:30 $
 * MT-Level: Safe
 *
 * Copyright (c) 2002 - 2003
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: LockData.C,v $
 * Revision 0.2  2003/04/09 20:29:30  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.1  2003/03/31 12:46:00  benno
 * Initial revision
 *
 */

/* LockData is intended to be derived into an object that handles
 * actual locking.
 */

#include "scheduler.H"

LockData::LockData(char *id, void *data)
{
  // Allocate and initialize a new LockData object, holding <data>,
  // tagged with <id>.  <id> is duplicated and need not remain valid,
  // but <data> is not managed by the LockData object.

  // Returns: A new LockData object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "LockData::LockData(%s,%d)", IONULL(id), data);
#endif
  
  i = xstrdup(id);
  d = data;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "LockData::LockData()");
#endif
}

void *LockData::idata()
{
  // Obtain the data pointer stored within.

  // Returns: A pointer to the data.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "LockData::idata()");
  dlog->log_exit(DEBUG_MINTRC, "LockData::idata = %d", d);
#endif
  
  return(d);
}

char *LockData::id()
{
  // Obtain ID string for the data pointer stored within.

  // Returns: An ID string that should not be modified..

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "LockData::id()");
  dlog->log_exit(DEBUG_MINTRC, "LockData::id = %s", IONULL(i));
#endif
  
  return(i);
}

LockData::~LockData()
{
  // Deallocate the LockData object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "LockData::~LockData()");
#endif

  xdelete(i);
  d = NULL;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "LockData::~LockData()");
#endif
}
