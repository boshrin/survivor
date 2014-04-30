/*
 * FixState.C: Object to manage fix state information.
 *
 * Version: $Revision: 0.15 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2006/10/17 14:04:07 $
 * MT-Level: Safe
 *
 * Copyright (c) 2002 - 2006
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: FixState.C,v $
 * Revision 0.15  2006/10/17 14:04:07  benno
 * Add since
 *
 * Revision 0.14  2005/04/07 01:25:52  benno
 * Fix results are XML
 *
 * Revision 0.13  2004/06/12 01:00:20  benno
 * State is XML based
 *
 * Revision 0.12  2004/03/01 23:29:40  benno
 * xstrdup host on init
 *
 * Revision 0.11  2003/11/29 05:23:11  benno
 * Use FixHistory/FixHistoryRecord instead of direct manipulation
 *
 * Revision 0.10  2003/10/06 23:14:03  benno
 * Use unbuffered I/O
 *
 * Revision 0.9  2003/04/21 20:29:44  benno
 * Use lock_host_state correctly
 *
 * Revision 0.8  2003/04/13 20:02:08  benno
 * Add verify_freshness
 * Call generate_csid
 *
 * Revision 0.7  2003/04/09 20:23:47  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.6  2003/04/05 22:50:10  benno
 * Use Debugger
 *
 * Revision 0.5  2003/03/31 13:27:06  benno
 * lock_host_state with 3 args
 *
 * Revision 0.4  2003/03/04 18:00:17  benno
 * Bump copyright
 *
 * Revision 0.3  2003/01/24 16:53:47  benno
 * Add IONULL and IOTF
 *
 * Revision 0.2  2003/01/01 02:03:44  benno
 * Change time_t format
 *
 * Revision 0.1  2002/12/31 04:36:03  benno
 * Initial revision
 *
 */

#include "survivor.H"

FixState::FixState(Check *check, char *host)
{
  // Allocate a new FixState object for <check> and <host>.

  // Returns: A new, initialized FixState.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "FixState::FixState(%d,%s)",
		  check, IONULL(host));
#endif
  
  c = check;
  lt = standard_lock;
  h = xstrdup(host);
  fld = NULL;
  lfile = NULL;
  sf = NULL;
  parsed = NULL;
  uncache(true);

  // Note the last state update.
  last = 0;
  verify_freshness();

  // All State objects must do this
  csid = generate_csid("fix",
		       (char *)(host ? host : ""),
		       (char *)(check ? check->name() : ""));

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "FixState::FixState()");
#endif
}

bool FixState::clearstate()
{
  // Clear all Fix state files.

  // Returns: true if successful, including if there were no files to be
  // removed, false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "FixState::clearstate()");
#endif

  if(c && c->name() && h && statusfile())
  {
    int locknum = lock_host_state(h, c->name(), true);

    if(locknum > -1)
    {
      // Unlink fixstatus file without checking for errors.

      unlink(statusfile());

      uncache(false);
      ret = true;
      
      unlock_state(locknum);
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "FixState::clearstate = %s", IOTF(ret));
#endif
  
  return(ret);
}

char *FixState::comment()
{
  // Determine the comment attached to the last result of this Fix.

  // Returns: A string containing the comment that remains valid for
  // the duration of the FixState object, or NULL if no comment is
  // found or if unable to read the state.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "FixState::comment()");
#endif

  char *ret = NULL;

  if(!dcache)
    read_status(true);

  if(dcache)
    ret = dcache->summary();

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "FixState::comment = %s", IONULL(ret));
#endif

  return(ret);
}

int FixState::fix_attempts()
{
  // Determine the number of times the Fix has been attempted since the
  // last error.

  // Returns: The number of fix attempts, or 0 if no attempts have
  // been made or on error.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "FixState::fix_attempts()");
#endif

  int ret = 0;

  if(!dcache)
    read_status(true);

  if(dcache)
    ret = dcache->instances();
  
  if(ret < 0)
    ret = 0;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "FixState::fix_attempts = %d", ret);
#endif

  return(ret);
}

time_t FixState::lastfix()
{
  // Determine the time of the last fix attempt.

  // Returns: The unix time of the last fix, or 0 if no fix has been
  // sent or if unable to access the state information.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "FixState::lastfix()");
#endif

  time_t ret = 0;

  if(!dcache)
    read_status(true);

  if(dcache)
    ret = dcache->time();

  if(ret < 0)
    ret = 0;
    
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "FixState::lastfix = %d", ret);
#endif
  
  return(ret);
}

char *FixState::lastfix_by()
{
  // Determine who attempted the last fix.

  // Returns: A string identifying who attempted the last fix, or NULL
  // if no fix has been attempted or on error.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "FixState::lastfix_by()");
#endif

  char *ret = NULL;
  
  if(!dcache)
    read_status(true);

  if(dcache)
    ret = dcache->initiatedby();
    
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "FixState::lastfix_by = %s", IONULL(ret));
#endif
  
  return(ret);
}

bool FixState::lock_fix(fix_lock_t locktype, int expire)
{
  // Obtain a lock over the fix of type <locktype>.  If a lock exists
  // and is older than <expire> seconds old, that lock will be removed.
  // If a lock exists and has not yet expired, this method will not
  // block.  Both lock_fix() and unlock_fix() must be called within the same
  // instantiation of the FixState object.

  // Returns: true if a lock was successfully obtained, false otherwise.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "FixState::lock_fix(%d,%d)",
		   locktype, expire);
#endif

  bool ret = false;

  // Testing for lfile prevents lock_fix from being called again without
  // first calling unlock_fix.
  
  if(c && c->name() && h && !lfile)
  {
    // The purpose of the expire time is to remove locks from processes that
    // may not have completed successfully (hanging, exited unexpectedly).
    // Treating them any other way or for any other purpose is not supported.

    if(!fld)
      fld = new char[strlen(args->statedir()) + 12];
    
    if(fld)
    {
      sprintf(fld, "%s/fix/", args->statedir());  // we do this again below
      
      // Obtain a lock over the set of fix locks.  This allows us to
      // manipulate the fix locks without having to worry about deadlocks.
      
      int locknum = lock_misc_state(fld);
      bool proceed = false;
      
      if(locknum > -1)
      {
	sprintf(fld, "%slocks", fld);
	
	// Verify there are no outstanding locks in the way, or if there are
	// see if they have expired.
	
	struct dirent *entry = allocate_dirent(fld);
	struct dirent *dp;
	
	if(entry)
	{
	  DIR *dirp = opendir(fld);
	  
	  if(dirp)
	  {
	    // We stop if we see lockfiles that get in our way and haven't
	    // expired.
	    proceed = true;
	    
	    while(readdir_r(dirp, entry, &dp) == 0 && dp)
	    {
	      // Iterate through the directory, looking for locks that
	      // might get in our way.  If we find a stale lock we try
	      // to remove it
	      
	      char *sx = dp->d_name;
	      char *hx = strchr(sx, '@');
	      
	      if(hx)
	      {
		// The if(hx) test spares us from . and ..
		
		switch(locktype)
		{
		case standard_lock:
		  // We fail on "service@host", "*@host", or "service@*"
		  if((strncmp(sx, c->name(), strlen(c->name()))==0
		      && sx[strlen(c->name())]=='@'
		      && strcmp(hx+1, h)==0)
		     ||
		     (strncmp(sx, "*@", 2)==0 && strcmp(hx+1, h)==0)
		     ||
		     (strncmp(sx, c->name(), strlen(c->name()))==0
		      && sx[strlen(c->name())]=='@'
		      && strlen(hx)==2 && hx[1]=='*'))
		    if(!expire_file(fld, dp->d_name, expire))
		      proceed = false;
		  break;
		case host_lock:
		  // We fail on *"@host"
		  if(strcmp(hx+1, h)==0)
		    if(!expire_file(fld, dp->d_name, expire))
		      proceed = false;
		  break;
		case service_lock:
		  // We fail on "service@"*
		  if(strncmp(sx, c->name(), strlen(c->name()))==0
		     && sx[strlen(c->name())]=='@')
		    if(!expire_file(fld, dp->d_name, expire))
		      proceed = false;
		  break;
		default:
		  wlog->warn("Programmer error in FixState::lock_fix");
		  break;
		}
	      }
	    }
	    
	    closedir(dirp);
	  }
	  else
	    wlog->warn("FixState::lock_fix unable to read %s", fld);
	
	  free(entry);
	  entry = NULL;
	}

	if(proceed)
	{
	  // Now create the requested lock, if possible.
	  
	  lfile = new char[strlen(fld) + strlen(c->name()) + strlen(h) + 3];
	  
	  if(lfile)
	  {
	    sprintf(lfile, "%s/%s@%s", fld,
		    (locktype == host_lock) ? "*" : c->name(),
		    (locktype == service_lock) ? "*" : h);
	    
	    int fd = open(lfile, O_WRONLY | O_CREAT | O_EXCL, FILE_OTH_NO);
	    
	    if(fd > -1)
	    {
	      verify_file(lfile, FILE_GRP_WR);
	      
	      ret = true;
	      close(fd);
	    }
	  }
	  else
	    wlog->warn("FixState::lock_fix failed to allocate lfile");
	}
	
	// Finally, unlock the fix lock state lock
      
	unlock_state(locknum);

	// Restore fld, we use it later in unlock
	sprintf(fld, "%s/fix/", args->statedir());
      }
    }
    else
      wlog->warn("FixState::lock_fix failed to allocate fld");
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "FixState::lock_fix = %s", IOTF(ret));
#endif

  return(ret);
}

bool FixState::parse_results(int fd)
{
  // Read the results from the fix executed via the pipe <fd> and parse
  // them.  (write_results must be called to write the results to storage.)

  // Returns: true if fully successful, false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "FixState::parse_results(%d)" , fd);
#endif

  if(fd > -1)
  {
    uncache(false);

    xdelete(parsed);

    SurvivorXML *sxml = new SurvivorXML();

    if(sxml)
    {
      parsed = sxml->parse_fixresult(fd);

      if(parsed)
	ret = true;
      
      xdelete(sxml);
    }
    else
      wlog->warn("FixState::parse_results failed to allocate SurvivorXML");
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "FixState::parse_results = %s", IOTF(ret));
#endif
  
  return(ret);
}

int FixState::returncode()
{
  // Determine the returncode attached to the last result of this Fix.

  // Returns: The return code, or 0 if no fix has been attempted or
  // on error.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "FixState::returncode()");
#endif

  int ret = 0;
  
  if(!dcache)
    read_status(true);

  if(dcache)
    ret = dcache->returncode();
  
  if(ret < 0)
    ret = 0;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "FixState::returncode = %d", ret);
#endif

  return(ret);
}

time_t FixState::since()
{
  // Determine the time of the first fix attempt.

  // Returns: The unix time of the first fix, or 0 if no fix has been
  // sent or if unable to access the state information.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "FixState::since()");
#endif

  time_t ret = 0;

  if(!dcache)
    read_status(true);

  if(dcache)
    ret = dcache->since();

  if(ret < 0)
    ret = 0;
    
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "FixState::since = %d", ret);
#endif
  
  return(ret);
}

bool FixState::unlock_fix()
{
  // Unlock the fix lock obtained via the lock_fix() method.  Both lock_fix()
  // and unlock_fix() must be called within the same instantiation of the
  // FixState object.

  // Returns: true if the lock was successfully removed, false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "FixState::unlock_fix()");
#endif

  // We don't really need to lock the general fix state since in the
  // worst case a query returns a false lock, but better to be
  // consistent.
  
  if(fld && lfile)
  {
    int locknum = lock_misc_state(fld);

    if(locknum > -1)
    {
      // Remove the appropriate lock

      unlink(lfile);
      xdelete(lfile);
      unlock_state(locknum);
    }
  }
  
  // No need to lock the general fix lock since we're only touching
  // things we already own
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "FixState::unlock_fix = %s", IOTF(ret));
#endif

  return(ret);
}

bool FixState::verify_freshness()
{
  // Determine if the underlying state has changed, and if so expire
  // any cached information.

  // Returns: true if the verification is successful, regardless of
  // freshness, false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "FixState::verify_freshness()");
#endif

  if(c && c->name() && h)
  {
    // Stat the lockfile and look at ctime, which is updated every time
    // try_lock is called, since that calls verify_file.
    
    time_t lu = last_host_state_update(h, c->name());

    if(lu == 0 || lu > last)
    {
      // Do the expire

#if defined(DEBUG)
      dlog->log_progress(DEBUG_CACHES, "Expiring fix cache for %s@%s",
			  c->name(), h);
#endif
      
      last = lu;
      uncache(false);
    }

    ret = true;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "FixState::verify_freshness = %s",
		  IOTF(ret));
#endif

  return(ret);
}

bool FixState::write_results(char *who)
{
  // Write out any results read by parse_results.

  // Returns: true if fully successful, false otherwise.
  
  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "FixState::write_results(%s)", IONULL(who));
#endif

  if(who && parsed && c && h && statusfile())
  {
    struct timeval tv;

    if(gettimeofday(&tv, NULL)==0)
    {
      // First, lock the host state.

      int locknum = lock_host_state(h, c->name(), true);

      if(locknum > -1)
      {
	// At this point we are successful unless something bad happens
	  
	ret = true;
	
	// First see if there is an outstanding alert.  If there is not,
	// we just go write history.  To do this, we let CheckState do
	// the dirty work, but tell it not to bother locking since we
	// already hold the lock over the state.
	  
	CheckState *cs = new CheckState(c, h);
	
	if(cs)
	{
	  cs->nolock();
	  
	  if(cs->returncode() != MODEXEC_OK)
	  {
	    // Try to get the previous state information and see if we
	    // should increment the attempts counter.  If any of
	    // this fails, just move on.
	    
	    int fixes = 0;
	    time_t st = tv.tv_sec;

	    // Try to pull up the latest state (we already have a lock)
	    
	    read_status(false);

	    if(dcache)
	    {
	      fixes = dcache->instances();

	      if(dcache->since() > 0)
		st = dcache->since();
	    }

	    fixes++;

	    // Toss dcache, we're going to recreate it

	    xdelete(dcache);
	    
	    // Write out the current status information.
	    
	    // Unlink the statusfile to clear permissions and owner,
	    // but don't bother checking for errors since it may
	    // not exist.
	    
	    unlink(statusfile());
	    
	    int fdout = try_open(statusfile(), O_WRONLY | O_CREAT,
				 FILE_GRP_WR);

	    if(fdout > -1)
	    {
	      SurvivorXML *sxml = new SurvivorXML();

	      if(sxml)
	      {
		dcache = new FixStateData(who, fixes, parsed->rc(),
					  parsed->comment(), tv.tv_sec, st);

		if(dcache)
		{
		  if(!sxml->generate(fdout, dcache))
		  {
		    ret = false;
		    xdelete(dcache);
		    
		    wlog->warn("FixState::write_results failed to write results for %s@%s", c->name(), h);
		  }
		}
		else
		{
		  ret = false;
		  wlog->warn("FixState::write_results failed to allocate FixStateData");
		}

		xdelete(sxml);
	      }
	      else
	      {
		ret = false;
		wlog->warn("FixState::write_results failed to allocate SurvivorXML");
	      }
	      
	      close(fdout);
	      
	      if(!verify_file(statusfile(), FILE_GRP_WR))
		wlog->warn("FixState::write_results failed to reset permissions on %s",
			   statusfile());
	    }
	    else
	    {
	      wlog->warn("FixState::write_results failed to open %s",
			 statusfile());
	      ret = false;
	    }
	  }
	    
	  xdelete(cs);
	}
	else
	  wlog->warn("FixState::write_results failed to allocate CheckState");

	// Write out history.  Failure to do so does not constitute
	// failure of this method.

	FixHistory *fh = new FixHistory(c, h);

	if(fh)
	{
	  FixHistoryRecord *fhr = new FixHistoryRecord(parsed->rc(),
						       who,
						       parsed->comment());

	  if(fhr)
	  {
	    if(!fh->record(fhr))
	      wlog->warn("FixState::write_results failed to record history");
	    
	    xdelete(fhr);
	  }
	  else
	    wlog->warn("FixHistory::write_results failed to allocate fhr");
	  
	  xdelete(fh);
	}
	else
	  wlog->warn("FixHistory::write_results failed to allocate fh");
	
	unlock_state(locknum);
      }
    }
    else
      wlog->warn("gettimeofday() failed in FixState::write_results()");

    if(ret)
    {
      // Toss parsed, we don't need it anymore

      xdelete(parsed);
    }
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "FixState::write_results = %s", IOTF(ret));
#endif

  return(ret);
}

FixState::~FixState()
{
  // Deallocate the FixState object.

  // Returns: Nothing.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "FixState::~FixState()");
#endif

  c = NULL;
  lt = standard_lock;
  xdelete(h);
  xdelete(fld);
  xdelete(lfile);
  xdelete(sf);
  xdelete(parsed);
  uncache(false);
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "FixState::~FixState()");
#endif
}

bool FixState::read_status(bool dolock)
{
  // Read the Fix status from statusfile() and cache the results.
  // If <dolock> if true, lock the host state before reading.

  // Returns: true if fully successful, false otherwise, including if
  // there is no status to cache.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "FixState::read_status(%s)", IOTF(dolock));
#endif

  bool ret = false;

  if(c && c->name() && h && statusfile())
  {
    int locknum = (dolock ? lock_host_state(h, c->name(), false) : -1);

    if(!dolock || locknum > -1)
    {
      // Toss anything old.
      uncache(false);

      // Let the XML Parser do the hard work, but we have to open the
      // file first

      int fdin = try_open(statusfile(), O_RDONLY, 0);

      if(fdin > -1)
      {
	SurvivorXML *sxml = new SurvivorXML();

	if(sxml)
	{
	  dcache = sxml->parse_fixstatedata(fdin);

	  if(dcache)
	    ret = true;
	  
	  xdelete(sxml);
	}
	else
	  wlog->warn("FixState::read_status() failed to allocate SurvivorXML");
	
	close(fdin);
      }

      if(dolock)
	unlock_state(locknum);
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "FixState::read_status = %s", IOTF(ret));
#endif
  
  return(ret);
}

char *FixState::statusfile()
{
  // Obtain or generate the filename containing the last fix status.

  // Returns: A pointer to the filename, or NULL on error.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "FixState::statusfile()");
#endif

  if(!sf && h && c && c->name())
  {
    sf = new char[strlen(args->statedir()) + strlen(h) + strlen(c->name())
		 + 18];

    if(sf)
      sprintf(sf, "%s/host/%s/%s/fixstatus", args->statedir(), h, c->name());
    else
      wlog->warn("FixState::statusfile failed to allocate sf");
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "FixState::statusfile = %s", IONULL(sf));
#endif

  return(sf);
}

void FixState::uncache(bool init)
{
  // Clear any cached results.  <init> should only be true when called by
  // the constructor.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "FixState::uncache(%s)", IOTF(init));
#endif

  if(!init)
  {
    xdelete(dcache);
  }
  else
    dcache = NULL;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "FixState::uncache()");
#endif
}
