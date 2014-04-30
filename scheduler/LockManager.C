/*
 * LockManager.C: An object to manage locks
 *
 * Version: $Revision: 0.10 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2005/08/01 04:00:14 $
 * MT-Level: Safe
 *
 * Copyright (c) 2002 - 2005
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: LockManager.C,v $
 * Revision 0.10  2005/08/01 04:00:14  benno
 * Fix 0.9 "fix"
 *
 * Revision 0.9  2005/04/09 02:23:38  benno
 * Fix cleanup bug in sc_unprotect_all
 *
 * Revision 0.8  2003/07/31 17:21:47  benno
 * Recreate states Hashtable after sc_unprotect_all if not exiting
 *
 * Revision 0.7  2003/04/13 19:58:17  benno
 * Simplify namespace for AlertState, FixState protection
 *
 * Revision 0.6  2003/04/09 19:50:02  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.5  2003/03/31 12:50:53  benno
 * Add CheckState Cache protection
 *
 * Revision 0.4  2003/01/29 01:44:12  benno
 * IOTF
 *
 * Revision 0.3  2002/04/04 20:50:59  benno
 * copyright
 *
 * Revision 0.2  2002/04/03 19:14:07  benno
 * rcsify date
 *
 * Revision 0.1  2002/04/03 19:13:48  benno
 * initial revision
 *
 */

#include "scheduler.H"

LockManager::LockManager()
{
  // Allocate a new LockManager object.  This will initialize all locks
  // managed by this object.

  // Returns: A new, initialized LockManager.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "LockManager::LockManager()");
#endif

  bad = 0;

  bad += pthread_mutex_init(&aqlock, NULL);
  bad += pthread_mutex_init(&cllock, NULL);
  bad += pthread_mutex_init(&cqlock, NULL);
  bad += pthread_mutex_init(&talock, NULL);
  bad += pthread_rwlock_init(&cflock, NULL);

  states = new Hashtable< MutexLockData<State> >();

  if(!states)
    bad++;
  
  if(bad)
    wlog->warn("LockManager failed to initialize");

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "LockManager::LockManager()");
#endif
}

bool LockManager::aq_lock()
{
  // Obtain a lock in order to access the Alert Queue.
  // This method will block until the lock is obtained.

  // Returns: true when a lock has been obtained, or false if unable
  // to obtain the lock.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "LockManager::aq_lock()");
#endif
  
  if(pthread_mutex_lock(&aqlock) == 0)
    ret = true;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "LockManager::aq_lock = %s", IOTF(ret));
#endif

  return(ret);
}

bool LockManager::aq_unlock()
{
  // Release a lock held for access to the Alert Queue.

  // Returns: true if the lock is successfully released, false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "LockManager::aq_unlock()");
#endif
  
  if(pthread_mutex_unlock(&aqlock) == 0)
    ret = true;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "LockManager::aq_unlock = %s", IOTF(ret));
#endif

  return(ret);
}

bool LockManager::cf_read()
{
  // Obtain a read lock in order to access the Configuration object.
  // This method will block until the lock is obtained.

  // Returns: true when a read lock has been obtained, or false if unable
  // to obtain the lock.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "LockManager::cf_read()");
#endif
  
  if(pthread_rwlock_rdlock(&cflock) == 0)
    ret = true;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "LockManager::cf_read = %s", IOTF(ret));
#endif

  return(ret);
}

bool LockManager::cf_write()
{
  // Obtain a write lock in order to access the Configuration object.
  // This method will block until the lock is obtained.

  // Returns: true when a write lock has been obtained, or false if unable
  // to obtain the lock.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "LockManager::cf_write()");
#endif
  
  if(pthread_rwlock_wrlock(&cflock) == 0)
    ret = true;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "LockManager::cf_write = %s", IOTF(ret));
#endif

  return(ret);
}

bool LockManager::cf_unlock()
{
  // Release a lock held for access to the Configuration object.

  // Returns: true if the lock is successfully released, false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "LockManager::cf_unlock()");
#endif
  
  if(pthread_rwlock_unlock(&cflock) == 0)
    ret = true;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "LockManager::cf_unlock = %s", IOTF(ret));
#endif

  return(ret);
}

bool LockManager::cl_lock()
{
  // Obtain a lock in order to access the CallList state.
  // This method will block until the lock is obtained.

  // Returns: true when a lock has been obtained, or false if unable
  // to obtain the lock.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "LockManager::cl_lock()");
#endif
  
  if(pthread_mutex_lock(&cllock) == 0)
    ret = true;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "LockManager::cl_lock = %s", IOTF(ret));
#endif

  return(ret);
}

bool LockManager::cl_unlock()
{
  // Release a lock held for access to the CallList state.

  // Returns: true if the lock is successfully released, false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "LockManager::cl_unlock()");
#endif
  
  if(pthread_mutex_unlock(&cllock) == 0)
    ret = true;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "LockManager::cl_unlock = %s", IOTF(ret));
#endif

  return(ret);
}

bool LockManager::cq_lock()
{
  // Obtain a lock in order to access the Check Queue.
  // This method will block until the lock is obtained.

  // Returns: true when a lock has been obtained, or false if unable
  // to obtain the lock.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "LockManager::cq_lock()");
#endif
  
  if(pthread_mutex_lock(&cqlock) == 0)
    ret = true;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "LockManager::cq_lock = %s", IOTF(ret));
#endif

  return(ret);
}

bool LockManager::cq_unlock()
{
  // Release a lock held for access to the Check Queue.

  // Returns: true if the lock is successfully released, false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "LockManager::cq_unlock()");
#endif
  
  if(pthread_mutex_unlock(&cqlock) == 0)
    ret = true;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "LockManager::cq_unlock = %s", IOTF(ret));
#endif

  return(ret);
}

bool LockManager::ok()
{
  // Determine if the LockManager is in a consistent state.  This need
  // only be checked after the LockManager is allocated.  If false is
  // returned, the LockManager should not be used.

  // Returns: true if the LockManager is consistent, false otherwise.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "LockManager::ok()");
  dlog->log_exit(DEBUG_MINTRC, "LockManager::ok = %s", IOTF(!bad));
#endif
  
  return(!bad);
}

State *LockManager::sc_lock(char *id)
{
  // Obtain a lock over the State identified by <id>.  This method
  // will block until the lock is obtained.

  // Returns: A pointer to the State when a lock has been obtained, or
  // NULL if unable to obtain the lock or if <id> is not found.

  State *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "LockManager::sc_lock(%s)", IONULL(id));
#endif

  if(id && states)
  {
    MutexLockData<State> *md = states->retrieve(id);

    if(md && md->lock())
      ret = md->data();
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "LockManager::sc_lock = %s", IOTF(ret));
#endif

  return(ret);
}

bool LockManager::sc_protect(char *id, State *data)
{
  // Add <data>, identified by <id>, to the set of States protected by
  // the LockManager.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "LockManager::sc_protect(%s,%d)",
		  IONULL(id), data);
#endif
  
  if(id && data && states)
  {
    MutexLockData<State> *md = new MutexLockData<State>(id, data);

    if(md)
    {
      if(states->insert(md->id(), md))
	ret = true;
      else
      {
	xdelete(md);
	wlog->warn("LockManager::sc_protect failed to insert MutexLockData into hashtable");
      }
    }
    else
      wlog->warn("LockManager::sc_protect failed to allocate new MutexLockData");
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "LockManager::sc_protect = %s", IOTF(ret));
#endif
  
  return(ret);
}

bool LockManager::sc_unlock(char *id)
{
  // Release a lock held for access to the State identified by <id>.

  // Returns: true if the lock is successfully released, false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "LockManager::sc_unlock(%s)", IONULL(id));
#endif

  if(id && states)
  {
    MutexLockData<State> *md = states->retrieve(id);

    if(md && md->unlock())
      ret = true;
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "LockManager::sc_unlock = %s", IOTF(ret));
#endif

  return(ret);
}

bool LockManager::sc_unprotect_all()
{
  // Remove all cached States from LockManager protection.  There should
  // be no outstanding locks when this method is called.

  // Returns: true if fully successful, false otherwise.

  return(sc_unprotect_all(true));
}

bool LockManager::sc_unprotect_all(bool reset)
{
  // Remove all cached States from LockManager protection.  There should
  // be no outstanding locks when this method is called.  If <reset> is
  // true, the LockManager will be "restarted", otherwise it will not be
  // usable after this method completes.

  // Returns: true if fully successful, false otherwise.

  bool ret = true;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "LockManager::sc_unprotect_all(%s)",
		  IOTF(reset));
#endif

  // This is trickier than it looks because we need to cast each
  // MutexLockData to the correct sort of state so that its contents
  // are deleted correctly.  The alternative is to have one hash table
  // and two or three extra methods for each type of State object.

  if(states)
  {
    HashHandle *__h = states->iterate_begin();

    if(__h)
    {
      MutexLockData<State> *__a;

      while((__a = states->iterate_next(__h)) != NULL)
      {
	State *__s = __a->data();

	if(__s)
	{
	  char *__i = __s->id();

	  if(__i)
	  {
	    // We finally know what type of state this is, so we can cast
	    // it for deletion.

	    switch(__i[0])
	    {
	    case 'a':
	      {
		MutexLockData<AlertState> *x =
		  (MutexLockData<AlertState> *)__a;

		xdelete(x);
	      }
	      break;
	    case 'c':
	      {
		MutexLockData<CheckState> *x =
		  (MutexLockData<CheckState> *)__a;

		xdelete(x);
	      }
	      break;
	    case 'f':
	      {
		MutexLockData<FixState> *x = (MutexLockData<FixState> *)__a;

		xdelete(x);
	      }
	      break;
	    default:
	      wlog->warn("LockManager::sc_unprotect_all found State with unknown id type: %s",
			 __i);
	      break;
	    }
	  }
	  else
	    wlog->warn("LockManager::sc_unprotect_all found State with no id");
	}
      }

      states->iterate_end(__h);
    }

    // Only xdelete here, since we've already cleared out the entries.
    
    xdelete(states);
  }

  if(reset)
  {
    states = new Hashtable< MutexLockData<State> >();

    if(!states)
    {
      wlog->warn("LockManager failed to reinitialize Hashtable");
      bad++;
    }
  }
  else
    bad++;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "LockManager::sc_unprotect_all = %s",
		 IOTF(ret));
#endif

  return(ret);
}

bool LockManager::ta_lock()
{
  // Obtain a lock for access to the Timer Array.  This method will block
  // until the lock becomes available.

  // Returns: true if the lock is successfully obtained, false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "LockManager::ta_lock()");
#endif
  
  if(pthread_mutex_lock(&talock) == 0)
    ret = true;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "LockManager::ta_lock = %s", IOTF(ret));
#endif

  return(ret);
}

bool LockManager::ta_unlock()
{
  // Release a lock held for access to the Timer Array.

  // Returns: true if the lock is successfully released, false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "LockManager::ta_unlock()");
#endif
  
  if(pthread_mutex_unlock(&talock) == 0)
    ret = true;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "LockManager::ta_unlock = %s", IOTF(ret));
#endif

  return(ret);
}

LockManager::~LockManager()
{
  // Deallocate the LockManager object.  All locks are destroyed.  Any
  // locks not released become undefined.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "LockManager::~LockManager()");
#endif

  if(!bad)
  {
    pthread_mutex_destroy(&aqlock);
    pthread_mutex_destroy(&cllock);
    pthread_mutex_destroy(&cqlock);
    pthread_mutex_destroy(&talock);
    pthread_rwlock_destroy(&cflock);
  }

  sc_unprotect_all(false);

  bad = 0;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "LockManager::~LockManager()");
#endif  
}
