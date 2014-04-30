/*
 * State.C: Object to manage state information
 *
 * Version: $Revision: 0.17 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2003/11/29 05:28:49 $
 * MT-Level: Unsafe
 *
 * Copyright (c) 2002 - 2003
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: State.C,v $
 * Revision 0.17  2003/11/29 05:28:49  benno
 * Move try_lock to utils.C
 *
 * Revision 0.16  2003/05/14 02:28:32  selsky
 * Remove embedded nulls
 *
 * Revision 0.15  2003/05/04 21:29:51  benno
 * Don't use string type
 *
 * Revision 0.14  2003/04/13 20:02:46  benno
 * Standardize comments and debug calls
 * Add id()
 *
 * Revision 0.13  2003/04/09 20:23:50  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.12  2003/04/06 00:17:09  benno
 * Use Debugger
 *
 * Revision 0.11  2003/03/31 12:54:57  benno
 * *_update routines for StateCache support
 *
 * Revision 0.10  2003/03/04 20:30:15  benno
 * Bump copyright
 *
 * Revision 0.9  2003/01/24 18:14:09  benno
 * Add IONULL and IOTF
 *
 * Revision 0.8  2002/12/31 04:34:05  benno
 * Use xdelete
 * Sometimes create /lock instead of .lock
 * Fix typos
 *
 * Revision 0.7  2002/08/21 21:36:58  benno
 * Add state hierarchy comment
 *
 * Revision 0.6  2002/04/04 20:11:37  benno
 * copyright
 *
 * Revision 0.5  2002/04/03 18:36:49  benno
 * rcsify date
 *
 * Revision 0.4  2002/04/03 18:36:34  benno
 * Add lock_misc_state
 *
 * Revision 0.3  2002/04/03 18:35:49  benno
 * Use try_lock with better error checking
 *
 * Revision 0.2  2002/04/03 18:35:23  benno
 * Actually do something
 *
 * Revision 0.1  2002/04/03 18:34:40  benno
 * initial revision
 *
 */

/* A note on the state hierarchy:
 *
 *  All of the XState classes know what the state hierarchy looks like,
 *  but so does Configuration::state_consistency(), which needs to verify
 *  the hierarchy after the configuration file is read.
 *
 */

#include "survivor.H"

// NOLOCK is a special return value to indicate we did not actually lock
// any state.  Generally speaking, there shouldn't be 100,000,000 open
// file handles, so this shouldn't cause a problem.
#define NOLOCK 99999999

State::State()
{
  // Allocate and initialize a new State object.

  // Returns: A new State object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "State::State()");
#endif
  
  lock = true;

  // It's up to the derived objects to set this appropriately, usually
  // something of the form "check:foo@bar".
  csid = NULL;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "State::State()");
#endif
}

char *State::id()
{
  // Obtain the ID of this State object.

  // Returns: A pointer to a string that should not be modified, or
  // NULL if no ID was set.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "State::id()");
  dlog->log_exit(DEBUG_MINTRC, "State::id = %s", IONULL(csid));
#endif
  
  return(csid);
}

bool State::nolock()
{
  // Set this State object to not lock state files when performing read
  // operations.

  // Returns: true if locking is disabled, false otherwise.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "State::nolock()");
#endif
  
  lock = false;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "State::nolock = true");
#endif

  return(true);
}

State::~State()
{
  // Deallocate the State object.

  // Returns: Nothing.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "State::~State()");
#endif

  lock = true;
  xdelete(csid);
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "State::~State()");
#endif
}

time_t State::last_host_state_update(char *host, char *service)
{
  // Determine when the last update was made to the state for
  // <service>@<host>.

  // Returns: The time of the last update, or 0 on error.

  time_t ret = 0;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "State::last_host_state_update(%s,%s)",
		   IONULL(host), IONULL(service));
#endif
  
  char *lfx = hlfile(host, service);

  if(lfx)
    ret = last_update(lfx);

  xdelete(lfx);
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "State::last_host_state_update = %d", ret);
#endif
  
  return(ret);
}

int State::lock_calllist_state(char *list)
{
  // Obtain a lock over the state for the CallList <list>.

  // Returns: A number identifying the lock that must be passed to
  // unlock_state to release the lock, or -1 on error.

  int r = -1;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "State::lock_calllist_state(%s)",
		   IONULL(list));
#endif

  if(!lock)
  {
#if defined(DEBUG)
    dlog->log_exit(DEBUG_MINTRC, "State::lock_calllist_state = %d", NOLOCK);
#endif
  
    return(NOLOCK);
  }
  
  if(list)
  {
    char *lfx = new char[strlen(args->statedir()) + strlen(list) + 16];
      
    if(lfx)
    {
      sprintf(lfx, "%s/calllist/%s/lock", args->statedir(), list);

      r = try_lock(lfx);

      xdelete(lfx);
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "State::lock_calllist_state = %d", r);
#endif
  
  return(r);
}

int State::lock_host_state(char *host, char *service, bool write)
{
  // Obtain a lock over the state for <service>@<host>.  If <write>
  // is true, mark the state as updated.

  // Returns: A number identifying the lock that must be passed to
  // unlock_state to release the lock, or -1 on error.

  int r = -1;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "State::lock_host_state(%s,%s,%s)",
		   IONULL(host), IONULL(service), IOTF(write));
#endif

  if(!lock)
  {
#if defined(DEBUG)
    dlog->log_exit(DEBUG_MINTRC, "State::lock_host_state = %d", NOLOCK);
#endif
  
    return(NOLOCK);
  }

  char *lfx = hlfile(host, service);

  if(lfx)
  {
    r = try_lock(lfx);

    // If mark_update fails, we still return success since its more
    // important that we at least write state if possible

    if(r && write)
    {
#if defined(DEBUG)
      dlog->log_exit(DEBUG_CACHES, "Marking cache updated for %s@%s",
		      service, host);
#endif
      
      mark_update(lfx);
    }
  }

  xdelete(lfx);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "State::lock_host_state = %d", r);
#endif
  
  return(r);
}

int State::lock_misc_state(char *statefile)
{
  // Obtain a lock over the state in the file <statefile>.  The lockfile
  // will be statefile.lock, unless <statefile> ends in '/', in which
  // case the lockfile will be statefile/lock.

  // Returns: A number identifying the lock that must be passed to
  // unlock_state to release the lock, or -1 on error.

  int r = -1;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "State::lock_misc_state(%s)",
		   IONULL(statefile));
#endif

  if(!lock)
  {
#if defined(DEBUG)
    dlog->log_exit(DEBUG_MINTRC, "State::lock_misc_state = %d", NOLOCK);
#endif

    return(NOLOCK);
  }

  if(statefile)
  {
    char *lfx = new char[strlen(statefile) + 6];

    if(lfx)
    {
      if(statefile[strlen(statefile) - 1] == '/')
	sprintf(lfx, "%s/lock", statefile);
      else
	sprintf(lfx, "%s.lock", statefile);

      r = try_lock(lfx);

      xdelete(lfx);
    }
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "State::lock_misc_state = %d", r);
#endif

  return(r);
}

int State::lock_service_state(char *service)
{
  // Obtain a lock over the state for the service <service>.

  // Returns: A number identifying the lock that must be passed to
  // unlock_state to release the lock, or -1 on error.

  int r = -1;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "State::lock_service_state(%s)",
		   IONULL(service));
#endif

  if(!lock)
  {
#if defined(DEBUG)
    dlog->log_exit(DEBUG_MINTRC, "State::lock_service_state = %d", NOLOCK);
#endif
  
    return(NOLOCK);
  }
  
  if(service)
  {
    char *lfx = new char[strlen(args->statedir()) + strlen(service) + 15];
      
    if(lfx)
    {
      sprintf(lfx, "%s/service/%s/lock", args->statedir(), service);

      r = try_lock(lfx);

      xdelete(lfx);
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "State::lock_service_state = %d", r);
#endif
  
  return(r);
}

bool State::unlock_state(int x)
{
  // Release the lock on state obtained by lock_X_state by using the return
  // value <x> that was provided.

  // Returns: True if successful, false otherwise.

  bool r = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "State::unlock_state(%d)", x);
#endif

  // Closing the fd automatically releases the lock.
  
  if(x > -1 && (x == NOLOCK || close(x)==0))
    r = true;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "State::unlock_state = %s", IOTF(r));
#endif

  return(r);
}

char *State::hlfile(char *host, char *service)
{
  // Determine the path to the lockfile protecting the state for
  // <service>@<host>.

  // Returns: A newly allocated string containing the path that should
  // be deleted when no longer required, or NULL on error.

  char *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "State::hlfile(%s,%s)",
		   IONULL(host), IONULL(service));
#endif
  
  if(host && service)
  {
    ret = new char[strlen(args->statedir()) + strlen(host) + strlen(service)
		  + 13];

    if(ret)
      sprintf(ret, "%s/host/%s/%s/lock", args->statedir(), host, service);
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "State::hlfile = %s", ret);
#endif

  return(ret);
}

time_t State::last_update(char *lockfile)
{
  // Determine the last update time for the state protected by <lockfile>
  // by examining its mtime value.

  // Returns: The last update time, or 0 on error.

  time_t ret = 0;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "State::last_update(%s)", IONULL(lockfile));
#endif

  if(lockfile)
  {
    struct stat sb;

    if(stat(lockfile, &sb)==0)
      ret = sb.st_mtime;
    else
      wlog->warn("State::last_update failed to stat lockfile %s", lockfile);
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "State::last_update() = %d", ret);
#endif

  return(ret);
}

bool State::mark_update(char *lockfile)
{
  // Set the last update time for the state protected by <lockfile> by
  // setting its mtime value to the current time.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "State::mark_update(%s)", IONULL(lockfile));
#endif

  if(lockfile && utimes(lockfile, NULL)==0)
    ret = true;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "State::mark_update = %s", IOTF(ret));
#endif

  return(ret);
}
