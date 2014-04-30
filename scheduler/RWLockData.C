/*
 * RWLockData.C: Object to protect data with a read/write lock.
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
 * $Log: RWLockData.C,v $
 * Revision 0.2  2003/04/09 19:50:03  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.1  2003/03/31 12:46:32  benno
 * Initial revision
 *
 */

#include "scheduler.H"

iRWLockData::iRWLockData(char *id, void *data) : LockData(id, data)
{
  // Allocate and initialize an RWLockData object, to protect <data>
  // tagged with <id>.  <id> is duplicated and need not remain valid,
  // but <data> is not managed by the RWLockData object.

  // Returns: A new RWLockData object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "iRWLockData::iRWLockData(%s,%d)",
		  IONULL(id), data);
#endif

  if(pthread_rwlock_init(&dlock, NULL)==0)
    ok = true;
  else
  {
    wlog->warn("iRWLockData::iRWLockData failed to init rwlock for %s",
	       IONULL(id));
    ok = false;
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "iRWLockData::iRWLockData()");
#endif
}

bool iRWLockData::read_lock()
{
  // Obtain a read lock over the data stored within.

  // Returns: true if a read lock is successfully obtained or false on error.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "iRWLockData::read_lock()");
#endif

  if(ok && pthread_rwlock_rdlock(&dlock)==0)
    ret = true;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "iRWLockData::read_lock = %s", IOTF(ret));
#endif

  return(ret);
}

bool iRWLockData::write_lock()
{
  // Obtain a write lock over the data stored within.

  // Returns: true if a write lock is successfully obtained or false on error.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "iRWLockData::write_lock()");
#endif

  if(ok && pthread_rwlock_wrlock(&dlock)==0)
    ret = true;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "iRWLockData::write_lock = %s", IOTF(ret));
#endif

  return(ret);
}

bool iRWLockData::unlock()
{
  // Release a lock held for access to the data.

  // Returns: true if the lock is successfully released or false on error.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "iRWLockData::unlock()");
#endif

  if(ok && pthread_rwlock_unlock(&dlock)==0)
    ret = true;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "iRWLockData::unlock = %s", IOTF(ret));
#endif  
}

iRWLockData::~iRWLockData()
{
  // Deallocate the RWLockData object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "iRWLockData::~iRWLockData()");
#endif

  if(ok)
    pthread_rwlock_destroy(&dlock);
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "iRWLockData::~iRWLockData()");
#endif
}
