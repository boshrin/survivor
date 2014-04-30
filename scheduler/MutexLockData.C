/*
 * MutexLockData.C: Object to protect data with a mutex.
 *
 * Version: $Revision: 0.2 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2003/04/09 19:50:03 $
 * MT-Level: Safe
 *
 * Copyright (c) 2002 - 2003
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: MutexLockData.C,v $
 * Revision 0.2  2003/04/09 19:50:03  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.1  2003/03/31 12:46:18  benno
 * Initial revision
 *
 */

#include "scheduler.H"

iMutexLockData::iMutexLockData(char *id, void *data) : LockData(id, data)
{
  // Allocate and initialize an MutexLockData object, to protect <data>
  // tagged with <id>.  <id> is duplicated and need not remain valid,
  // but <data> is not managed by the MutexLockData object.

  // Returns: A new MutexLockData object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "iMutexLockData::iMutexLockData(%s,%d)",
		  IONULL(id), data);
#endif

  if(pthread_mutex_init(&dlock, NULL)==0)
    ok = true;
  else
  {
    wlog->warn("iMutexLockData::iMutexLockData failed to init mutex for %s",
	       IONULL(id));
    ok = false;
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "iMutexLockData::iMutexLockData()");
#endif
}

bool iMutexLockData::lock()
{
  // Obtain a lock over the data stored within.

  // Returns: true if a lock is successfully obtained or false on error.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "iMutexLockData::lock()");
#endif
  
  if(ok && pthread_mutex_lock(&dlock)==0)
    ret = true;
    
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "iMutexLockData::lock = %s", IOTF(ret));
#endif

  return(ret);
}

bool iMutexLockData::unlock()
{
  // Release a lock held for access to the data.

  // Returns: true if the lock is successfully released or false on error.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "iMutexLockData::unlock()");
#endif

  if(ok && pthread_mutex_unlock(&dlock)==0)
    ret = true;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "iMutexLockData::unlock = %s", IOTF(ret));
#endif

  return(ret);
}

iMutexLockData::~iMutexLockData()
{
  // Deallocate the MutexLockData object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "iMutexLockData::~iMutexLockData()");
#endif

  if(ok)
    pthread_mutex_destroy(&dlock);
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "iMutexLockData::~iMutexLockData()");
#endif
}
