/*
 * CheckState.C: Object to manage state information for Checks (services)
 *
 * Version: $Revision: 0.37 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2006/10/17 14:03:13 $
 * MT-Level: Unsafe
 *
 * Copyright (c) 2002 - 2006
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: CheckState.C,v $
 * Revision 0.37  2006/10/17 14:03:13  benno
 * Add duration and since
 *
 * Revision 0.36  2006/01/23 02:08:54  benno
 * Add duration
 *
 * Revision 0.35  2005/12/22 04:04:56  benno
 * Add written_hosts()
 *
 * Revision 0.34  2005/08/10 01:13:53  benno
 * read_xml_document takes third arg
 *
 * Revision 0.33  2005/04/27 01:57:09  benno
 * Use read_xml_document
 *
 * Revision 0.32  2005/04/09 02:35:48  benno
 * Fix bug preventing reschedule when checkstatus file is missing
 *
 * Revision 0.31  2005/04/05 22:38:10  benno
 * Toss Type III CheckState
 * parse_results takes an Array
 * rename parse_results(List) parse_tmp_state_results
 * Use XML based check modules
 * Toss unused write_depfail_only
 *
 * Revision 0.30  2004/06/12 00:59:34  benno
 * State is XML based
 *
 * Revision 0.29  2003/11/29 05:21:24  benno
 * Use CheckHistory/CheckHistoryRecord instead of direct manipulation
 *
 * Revision 0.28  2003/10/20 00:41:12  benno
 * Fix off by one memset() bug in parse_results
 *
 * Revision 0.27  2003/10/06 23:08:19  benno
 * Use unbuffered I/O for reading results
 *
 * Revision 0.26  2003/09/17 00:58:37  benno
 * Allow empty comment in write_results
 *
 * Revision 0.25  2003/06/17 15:13:37  benno
 * Remove 1024 character limitation on Type III results
 *
 * Revision 0.24  2003/04/21 20:29:22  benno
 * Reschedule no longer clears out old consecutive, rc, and comment
 *
 * Revision 0.23  2003/04/13 20:01:54  benno
 * Move id() into State
 * Call generate_csid
 *
 * Revision 0.22  2003/04/09 20:23:46  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.21  2003/04/04 21:44:24  benno
 * Use Debugger
 *
 * Revision 0.20  2003/03/31 13:25:36  benno
 * Add host list to result parsing for sanity checking
 * Add support for StateCache with id() and verify_freshness()
 *
 * Revision 0.19  2003/03/04 17:57:56  benno
 * Use toss_eol
 *
 * Revision 0.18  2003/02/23 15:18:18  benno
 * Add write_misconfig and write_unfinished
 *
 * Revision 0.17  2003/01/23 22:56:17  benno
 * Add IONULL and IOTF
 *
 * Revision 0.16  2003/01/02 04:46:06  benno
 * Fix bug in statefile()
 *
 * Revision 0.15  2003/01/01 02:04:47  benno
 * Add read_status
 * Fold lastcheck into checkstatus
 *
 * Revision 0.14  2002/12/31 04:30:00  benno
 * Correct comment typos
 *
 * Revision 0.13  2002/10/25 22:55:36  benno
 * initialize cache
 *
 * Revision 0.12  2002/09/05 21:57:26  benno
 * use try_fopen
 *
 * Revision 0.11  2002/08/06 19:42:29  selsky
 * Remove embedded nulls in format
 *
 * Revision 0.10  2002/05/31 21:38:13  benno
 * unlink before verify_file
 *
 * Revision 0.9  2002/05/22 19:03:27  selsky
 * fix format string for time_t
 *
 * Revision 0.8  2002/04/04 20:09:23  benno
 * copyright
 *
 * Revision 0.7  2002/04/03 17:59:03  benno
 * rcsify date
 *
 * Revision 0.6  2002/04/03 17:58:53  benno
 * Clean up dependency state handling
 *
 * Revision 0.5  2002/04/03 17:58:21  benno
 * Clear checkstatus file on reschedule
 *
 * Revision 0.4  2002/04/03 17:57:41  benno
 * Use verify_file
 * write_timeout accepts list of hosts
 *
 * Revision 0.3  2002/04/03 17:57:11  benno
 * Add Type III CheckState
 * Add written List
 *
 * Revision 0.2  2002/04/03 17:56:36  benno
 * Cleanup
 *
 * Revision 0.1  2002/04/03 17:54:53  benno
 * initial revision
 *
 */

#include "survivor.H"

CheckState::CheckState(Check *check)
{
  // Allocate a new CheckState object, which manages state information for
  // the Check <c>.  This is a Type I CheckState object.

  // Returns: A new, initialized CheckState.

  init(check, NULL);
}

CheckState::CheckState(Check *check, char *host)
{
  // Allocate a new CheckState object, which manages state information for
  // the Check <c> for the host <host>.  This is a Type II CheckState object.

  // Returns: A new, initialized CheckState.

  init(check, host);
}

char *CheckState::comment()
{
  // Determine the comment attached to the last result of this Check.
  // This is useful for Type II CheckStates only.

  // Returns: A string containing the comment that remains valid for
  // the duration of the CheckState object, or NULL if no comment is
  // found or if unable to read the state.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CheckState::comment()");
#endif

  char *ret = NULL;
  
  if(!dcache)
    read_status(true);

  if(dcache)
    ret = dcache->summary();

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CheckState::comment = %s", IONULL(ret));
#endif
  
  return(ret);
}

int CheckState::consecutive()
{
  // Determine the number of consecutive instances of the current
  // return value for the Check of the specified host.  This is useful
  // for Type II CheckStates only.

  // Returns: The number of consecutive instances of returncode(), which
  // may be 0 if the Check has not run or the state is otherwise unavailable.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CheckState::consecutive()");
#endif

  int ret = 0;
  
  if(!dcache)
    read_status(true);

  if(dcache)
    ret = dcache->instances();

  if(ret < 0)
    ret = 0;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CheckState::consecutive = %d", ret);
#endif
  
  return(ret);
}

int CheckState::duration()
{
  // Determine the execution duration for the Check of the specified
  // host.  This is useful for Type II CheckStates only.

  // Returns: The execution duration in milliseconds, or -1 if the
  // duration is unavailable.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CheckState::duration()");
#endif

  int ret = -1;
  
  if(!dcache)
    read_status(true);

  if(dcache)
    ret = dcache->duration();

  if(ret < -1)
    ret = -1;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CheckState::duration = %d", ret);
#endif
  
  return(ret);
}

time_t CheckState::lastcheck()
{
  // Determine the last time this Check was run.  This is useful for
  // Type II CheckStates only.

  // Returns: The unix time of the last check, or 0 if never or if unable
  // to access the state information.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CheckState::lastcheck()");
#endif

  int ret = 0;
  
  if(!dcache)
    read_status(true);

  if(dcache)
    ret = dcache->time();

  if(ret < 0)
    ret = 0;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CheckState::lastcheck = %d", ret);
#endif

  return(ret);
}

bool CheckState::parse_results(Array<CheckResult> *crs)
{
  // Accept the Array of CheckResults <crs> into the parse buffer for
  // future reference.  This is useful for Type I CheckStates only.
  // <crs> will be maintained by the CheckState and deleted when no
  // longer required.

  // Returns: true if fully successful, false otherwise.
  
  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CheckState::parse_results(%d)", crs);
#endif
  
  if(c && !h && crs)
  {
    // If there's anything in the buffer, toss it
    xadelete(rbuf, CheckResult);

    // Note the new data
    rbuf = crs;
    
    ret = true;
  }

  // We don't call uncache() here because we haven't yet changed any state
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CheckState::parse_results = %d", ret);
#endif
  
  return(ret);
}

bool CheckState::parse_tmp_state_results(List *hosts)
{
  // Parse the results stored in tmp_state_filename(), using the set
  // of <hosts> as a sanity check for expected data.  This is useful
  // for Type I CheckStates only.

  // Returns: true if fully successful, false otherwise.

  bool r = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CheckState::parse_tmp_state_results(%d)",
		  hosts);
#endif
  
  if(c && !rbuf && tmp_state_filename() && hosts)
  {
    SurvivorXML *sxml = new SurvivorXML();

    if(sxml)
    {
      rbuf = new Array<CheckResult>();

      if(rbuf)
      {
	int fdin = try_open(tmp_state_filename(), O_RDONLY, 0);

	if(fdin > -1)
	{
	  /* It would be much simpler to do something like this, but
	     we need to examine the lines as we read them in order
	     to separate out the individual documents.
	  
	  for(CheckResult *cr = sxml->parse_checkresult(fdin);
	      cr != NULL;
	      cr = sxml->parse_checkresult(fdin))
	  {
	    if(!rbuf->add(cr))
	    {
	      xdelete(cr);
	    }
	  }
	  */
	  
	  // At this point, we want to return true to record as many
	  // results as we were able to parse
	  r = true;
	  
	  // We read one document at a time (using "</SurvivorCheckResult>"
	  // as an end marker) until there is no more data to read.

	  bool xdone = false;

	  while(!xdone)
	  {
	    char *xmldoc = read_xml_document(fdin, "</SurvivorCheckResult>",
					     0);

	    if(xmldoc)
	    {
	      // Parse the document

	      CheckResult *cr =
		sxml->parse_checkresult(xmldoc, strlen(xmldoc));

	      if(cr)
	      {
		if(!rbuf->add(cr))
		{
		  xdelete(cr);
		}
	      }
	      else
		wlog->warn("CheckState::parse_tmp_state_results failed to parse CheckResult document");
	      
	      xdelete(xmldoc);
	    }
	    else
	    {
	      // NULL return means no full document left to read
	      xdone = true;
	    }
	  }
	
	  close(fdin);
	}
	else
	{
	  // Nothing to do?

	  xdelete(rbuf);
	}
      }
      else
	wlog->warn("CheckState::parse_tmp_state_results failed to allocate Array");

      xdelete(sxml);
    }
    else
      wlog->warn("CheckState::parse_tmp_state_results failed to allocate sxml");
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CheckState::parse_tmp_state_results = %s",
                 IOTF(r));
#endif

  return(r);
}

bool CheckState::reschedule()
{
  // Reschedule the Check to run again by removing the checkstatus
  // file.  This is useful for Type II CheckStates only.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "CheckState::reschedule()");
#endif

  if(c && h && statusfile())
  {
    int locknum = lock_host_state(h, c->name(), true);

    if(locknum > -1)
    {
      // We want to maintain as much state as possible, which means
      // reading the existing status and writing out all the old
      // information except the last check time, which we set to 0,
      // and execution duration.

      // To do this, we determine (if we can) the existing values of
      // the data we want to keep, write out the new values, and
      // uncache.

      read_status(false);

      CheckStateData *newdata = NULL;
      
      if(dcache)
	newdata = new CheckStateData(dcache->instances(),
				     dcache->returncode(),
				     dcache->summary(),
				     0,
				     dcache->since(),
				     -1);
      else // problem reading, use default data
	newdata = new CheckStateData(0, MODEXEC_NOTYET, "", 0, 0, -1);

      // We can toss dcache since we're about to recreate it

      xdelete(dcache);
      
      // Unlink the old file to deal with permission and ownership issues
      // but don't bother checking for error.

      unlink(statusfile());

      if(newdata)
      {
	int fdout = try_open(statusfile(), O_WRONLY | O_CREAT, FILE_GRP_WR);

	if(fdout > -1)
	{
	  SurvivorXML *sxml = new SurvivorXML();
	  
	  if(sxml)
	  {
	    if(sxml->generate(fdout, newdata))
	      ret = true;
	    else
	      wlog->warn("CheckState::reschedule failed to write reschedule for %s@%s", c->name(), h);
	    
	    xdelete(sxml);
	  }
	  else
	    wlog->warn("CheckState::reschedule failed to allocate SurvivorXML");

	  close(fdout);

	  if(!verify_file(statusfile(), FILE_GRP_WR))
	    wlog->warn("CheckState::reschedule failed to reset permissions on %s",
		       statusfile());
	}
	else
	  wlog->warn("CheckState::reschedule failed to open %s", statusfile());
	
	// Adjust pointers, may as well cache the new info
	
	dcache = newdata;
	newdata = NULL;
      }
      // else there's no point writing out empty information
      
      unlock_state(locknum);
    }
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "CheckState::reschedule = %s", IOTF(ret));
#endif
  
  return(ret);
}

int CheckState::returncode()
{
  // Determine the current return value for the Check of the specified
  // host.  This is useful for Type II CheckStates only.

  // Returns: 0 if there is no outstanding error or if return status cannot
  // be determined (eg: the Check hasn't run yet), or a return code.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CheckState::returncode()");
#endif

  int ret = 0;
  
  if(!dcache)
    read_status(true);

  if(dcache)
    ret = dcache->returncode();

  if(ret < 0)
    ret = 0;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CheckState::returncode = %d", ret);
#endif
  
  return(ret);
}

time_t CheckState::since()
{
  // Determine the time of the first instance of this return value for
  // the Check of the specified host.  This is useful for Type II
  // CheckStates only.

  // Returns: The unix time of the first check of this return value,
  // or 0 if never or if unable to access the state information.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CheckState::since()");
#endif

  int ret = 0;
  
  if(!dcache)
    read_status(true);

  if(dcache)
    ret = dcache->since();

  if(ret < 0)
    ret = 0;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CheckState::since = %d", ret);
#endif
  
  return(ret);
}

char *CheckState::tmp_state_filename()
{
  // Obtain the filename to which the output from a check may be
  // redirected for later parsing by the CheckState.  This is useful
  // for Type I CheckStates only.

  // Returns: A pointer to the filename, or NULL on error.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CheckState::tmp_state_filename()");
#endif

  if(!tsf)
  {
    if(c && c->name())
    {
      tsf = new char[strlen(args->statedir()) + strlen(c->name()) + 20];
	
      if(tsf)
	sprintf(tsf, "%s/service/%s/tmpstatus", args->statedir(),
		c->name());
      else
	wlog->warn("CheckState::tmp_state_filename failed to allocate tsf");
    }
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CheckState::tmp_state_file = %s",
		  IONULL(tsf));
#endif

  return(tsf);
}

bool CheckState::verify_freshness()
{
  // Determine if the underlying state has changed, and if so expire
  // any cached information.  This method is useful for Type II
  // CheckStates only.

  // Returns: true if the verification is successful, regardless of
  // freshness, false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CheckState::verify_freshness()");
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
      dlog->log_progress(DEBUG_CACHES, "Expiring check cache for %s@%s",
			  c->name(), h);
#endif
      
      last = lu;
      uncache(false);
    }

    ret = true;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CheckState::verify_freshness = %s",
		  IOTF(ret));
#endif

  return(ret);
}

bool CheckState::write_misconfig(List *hosts)
{
  // Update the check state to indicate that the check is
  // misconfigured.  <hosts> contains each host that should have been
  // checked.  A misconfigured check is considered completed to
  // prevent runaway checking, and so the lastcheck time will be
  // updated.  This method should only be called after write_results(),
  // and should not be called with write_timeout().  A read lock over
  // the configuration must be obtained before calling.  This is useful
  // for Type I CheckStates only.

  // Returns: true if fully successful, false otherwise.

  return(write_unfinished(hosts, MODEXEC_MISCONFIG,
			  "Module is misconfigured (no output received)"));
}

bool CheckState::write_results()
{
  // Write out any results read by parse_results().  Once this method
  // is invoked, the internal cache of parsed results will be deleted.
  // This method should therefore only be called after all results
  // from the Check are known.  This method will also update the
  // lastcheck time.  This method is useful for Type I CheckStates
  // only.

  // Returns: true if fully successful, false otherwise.
  
  return(write_results(NULL));
}

bool CheckState::write_timeout(List *hosts)
{
  // Update the check state to indicate that the check was timed out.
  // <hosts> contains each host that should have been checked.  A
  // timed out check is considered completed to prevent runaway
  // checking, and so the lastcheck time will be updated.  This method
  // should only be called after write_results(), and should not be
  // called with write_timeout().  A read lock over the configuration
  // must be obtained before calling.  This is useful for Type I
  // CheckStates only.

  // Returns: true if fully successful, false otherwise.

  return(write_unfinished(hosts, MODEXEC_TIMEDOUT, "Timed out"));
}

List *CheckState::written_hosts()
{
  // Obtain the list of hosts for which results were written by
  // write_results().

  // Returns: A pointer to a List that should not be modified, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CheckState::written_hosts()");
  dlog->log_exit(DEBUG_MINTRC, "CheckState::written_hosts = %d", written);
#endif
  
  return(written);
}

CheckState::~CheckState()
{
  // Deallocate the CheckState object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CheckState::~CheckState()");
#endif
  
  xadelete(rbuf, CheckResult);
  c = NULL;
  xdelete(written);
  xdelete(h);
  xdelete(hf);
  xdelete(sf);
  xdelete(tsf);
  last = 0;

  uncache(false);
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CheckState::~CheckState()");
#endif
}

void CheckState::init(Check *check, char *host)
{
  // Initializer for Constructors.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CheckState::init(%d,%s)",
		  check, IONULL(host));
#endif

  rbuf = NULL;
  c = check;
  written = NULL;
  h = xstrdup(host);
  hf = NULL;
  sf = NULL;
  tsf = NULL;
  uncache(true);

  // Note the last state update.  This will end up calling uncache again.
  last = 0;
  verify_freshness();

  // All State objects must do this
  csid = generate_csid("check", 
		       (char *)(host ? host : ""),
		       (char *)(check ? check->name() : ""));
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CheckState::init()");
#endif
}

bool CheckState::read_status(bool dolock)
{
  // Read the Check status from statusfile() and cache the results.
  // This method is useful for Type II CheckStates only.  If <dolock>
  // is false, read_status will assume a suitable lock has already
  // been obtained.

  // Returns: true if fully successful, false otherwise, including if
  // there is no status to cache.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CheckState::read_status(%s)", IOTF(dolock));
#endif

  if(c && c->name() && h && statusfile())
  {
    int locknum = -1;

    if(dolock)
      locknum = lock_host_state(h, c->name(), false);

    if(!dolock || locknum > -1)
    {
      // Toss anything old.
      uncache(false);

      // Let the XML Parser do the hard work, but we have to open
      // the file first

      int fdin = try_open(statusfile(), O_RDONLY, 0);

      if(fdin > -1)
      {
	SurvivorXML *sxml = new SurvivorXML();

	if(sxml)
	{
	  dcache = sxml->parse_checkstatedata(fdin);

	  if(dcache)
	    ret = true;
	  
	  xdelete(sxml);
	}
	else
	  wlog->warn("CheckState::read_status() failed to allocate SurvivorXML");

	close(fdin);
      }

      if(dolock)
	unlock_state(locknum);
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CheckState::read_status = %s", IOTF(ret));
#endif

  return(ret);
}

char *CheckState::statusfile()
{
  // Obtain or generate the filename containing the last check status.
  // This is useful for Type II CheckStates only.

  // Returns: A pointer to the filename, or NULL on error.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CheckState::statusfile()");
#endif

  if(!sf)
  {
    if(c && h)
    {
      // Type II

      sf = new char[strlen(args->statedir()) + strlen(h)
		    + strlen(c->name()) + 20];
	
      if(sf)
	sprintf(sf, "%s/host/%s/%s/checkstatus", args->statedir(),
		h, c->name());
    }
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CheckState::statusfile = %s", IONULL(sf));
#endif

  return(sf);
}

void CheckState::uncache(bool init)
{
  // Clear any cached results.  <init> should only be true when called
  // by the constructor.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CheckState::uncache(%s)", IOTF(init));
#endif
  
  if(!init)
  {
    xdelete(dcache);
  }
  else
    dcache = NULL;

  // We'll also uncache the Type II filenames, since write_results
  // uses them to convert from Type I to Type II

  xdelete(sf);
  xdelete(hf);
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CheckState::uncache()");
#endif
}

bool CheckState::write_results(List *hosts)
{
  // Write out any results read by parse_results().  Once this method
  // is invoked, the internal cache of parsed results will be deleted,
  // including any partially read lines.  This method should therefore
  // only be called after all results from the Check are known.  This
  // method will also update the lastcheck time.  If <hosts> is
  // provided, any host found in the internal cache but not found in
  // <hosts> will be considered an error.  This method is useful for
  // Type I CheckStates only.

  // Returns: true if fully successful, false otherwise.
  
  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CheckState::write_results(%d)", hosts);
#endif

  if(c && !h && rbuf)
  {
    // We can't use statusfile() here since we are in effect
    // translating from Type I to Type II.
    
    struct timeval tv;
    
    if(gettimeofday(&tv, NULL)==0)
    {
      for(int i = 0;i < rbuf->entries();i++)
      {
	CheckResult *cr = rbuf->retrieve(i);

	if(cr)
	{
	  if(!hosts || hosts->find(cr->hostname()) > -1)
	  {
	    // Write out state.  There shouldn't be any race
	    // conditions here since only one Check module for a given
	    // service can be processed at a time.  However, other
	    // threads or processes could be reading these files, so
	    // state output must be protected.

	    int locknum = lock_host_state(cr->hostname(), c->name(), true);
		
	    if(locknum > -1)
	    {
	      // At this point we are successful unless something bad
	      // happens.
	      
	      ret = true;
		  
	      // In order for read_status to work, it needs to know
	      // what the current hostname is.  Now that we
	      // (temporarily) have one, we can set it.  This will
	      // also make statusfile work.

	      // Make sure we don't have any baggage
	      uncache(false);
	      
	      if(h)
		wlog->warn("Memory leak detected in CheckState::write_results");

	      // Don't bother copying since we're just going to toss it
	      h = cr->hostname();

	      if(statusfile())
	      {
		// Try to get the previous state information and see
		// if we should increment the consecutive counter.  If
		// any of this fails, just move on.

		int consecutive = 0;
		int prevret = 0;
		time_t since = tv.tv_sec;
		    
		read_status(false);
		    
		if(dcache)
		{
		  prevret = dcache->returncode();
		  
		  if(prevret == cr->rc())
		  {
		    consecutive = dcache->instances();

		    if(dcache->since() > 0)
		      since = dcache->since();
		  }
		}
		    
		consecutive++;
		    
		// Toss dcache, we're going to recreate it
		    
		xdelete(dcache);
		    
		// Write out the current status information.

		// Unlink statusfile to clear permissions and owner,
		// but don't bother checking for errors since it may
		// not exist.
		    
		unlink(statusfile());

		int fdout = try_open(statusfile(), O_WRONLY | O_CREAT,
				     FILE_GRP_WR);

		if(fdout > -1)
		{
		  // XXX Allocating one SurvivorXML object per iteration
		  // is probably excessive

		  SurvivorXML *sxml = new SurvivorXML();

		  if(sxml)
		  {
		    dcache = new CheckStateData(consecutive, cr->rc(),
						cr->comment(), tv.tv_sec,
						since, cr->duration());

		    if(dcache)
		    {
		      if(!sxml->generate(fdout, dcache))
		      {
			ret = false;
			xdelete(dcache);
			    
			wlog->warn("CheckState::write_results failed to write results for %s@%s", c->name(), h);
		      }
		    }
		    else
		    {
		      ret = false;
		      wlog->warn("CheckState::write_results failed to allocate CheckStateData");
		    }

		    xdelete(sxml);
		  }
		  else
		  {
		    ret = false;
		    wlog->warn("CheckState::write_results failed to allocate SurvivorXML");
		  }
		  
		  close(fdout);

		  if(!verify_file(statusfile(), FILE_GRP_WR))
		    wlog->warn("CheckState::write_results failed to reset permissions on %s",
			       statusfile());
		}
		else
		{
		  wlog->warn("CheckState::write_results failed to open %s",
			     statusfile());
		  ret = false;
		}

		// At this point, we could release the lock, however
		// we'll hold it until the end anyway.

		if(ret)
		{
		  // Append to the history file.

		  CheckHistory *ch = new CheckHistory(c, cr->hostname());
		  
		  if(ch)
		  {
		    CheckHistoryRecord *chr =
		      new CheckHistoryRecord(cr->rc(),
					     cr->scalar(),
					     cr->comment(),
					     cr->duration());

		    if(chr)
		    {
		      if(!ch->record(chr))
			wlog->warn("CheckState::write_results failed to record history");
		      
		      xdelete(chr);
		    }
		    else
		      wlog->warn("CheckState::write_results failed to allocate chr");
		    
		    xdelete(ch);
		  }
		  else
		    wlog->warn("CheckState::write_results failed to allocate ch");
		}
	      }
	      else
		wlog->warn("CheckState::write_results failed to allocate statusfile");
		  
	      // At this point, we no longer need h.  Reset back
	      // to a Type I CheckState.

	      h = NULL;
		  
	      // Uncache anything we may have cached		  
	      uncache(false);
		    
	      if(ret)
	      {
		// Note that we have written the state for this host,
		// in case write_timeout is called.
		      
		if(!written)
		  written = new List();

		if(!written || !written->add(cr->hostname()))
		  wlog->warn("CheckState::write_results failed to allocate written or add %s",
			     cr->hostname());
	      }
		  
	      unlock_state(locknum);
	    }
	  }
	  else
	    wlog->warn("CheckState::write_results did not find '%s' in expected list of hosts (retval=%d, scaleval=%d, comment=%s, check=%s)",
		       cr->hostname(), cr->rc(), cr->scalar(), cr->comment(),
		       c->name());
	}
      }
    }
    else
      wlog->warn("gettimeofday() failed in CheckState::write_results()");

    xadelete(rbuf, CheckResult);
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CheckState::write_results = %s", IOTF(ret));
#endif
  
  return(ret);
}

bool CheckState::write_unfinished(List *hosts, int rc, char *comment)
{
  // Update the check state to indicate information about uncompleted
  // hosts for a check, due to <rc> and <comment>.  <hosts> contains
  // each host that should have been checked.  To prevent runaway
  // checking, the lastcheck time will be updated.  This method should
  // only be called once per check execution.  A read lock over the
  // configuration must be obtained before calling.  This is useful
  // for Type I CheckStates only.

  // Returns: true if fully successful, false otherwise.

  bool r = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CheckState::write_unfinished(%d,%d,%s)",
		   hosts, rc, IONULL(comment));
#endif
  
  // It might be useful not to consider a timed out or misconfigured
  // check completed so it will be rescheduled again in the next
  // minute in case the problem clears, but most likely it won't and
  // so this is the preferred behavior.  Also, this makes it easy to
  // implement.  All we do is create a dummy <rbuf> with return codes
  // and call write_results.

  if(hosts && hosts->entries() > 0 && comment && c && !h && !rbuf)
  {
    // rbuf should be NULL at this point since write_results would have
    // cleared it out.  If <written> exists, results for those hosts
    // were already successfully written, so we don't write results
    // for them here.

    // Use <hosts> rather than cf->find_group(c->name()) because not all
    // hosts in a given group may be checked according to the same
    // schedule.

    rbuf = new Array<CheckResult>();

    if(rbuf)
    {
      for(int i = 0;i < hosts->entries();i++)
      {
	if(hosts->retrieve(i) &&
	   (!written || written->find(hosts->retrieve(i))==-1))
	{
	  CheckResult *cr = new CheckResult(hosts->retrieve(i),
					    rc,
					    0,
					    comment);

	  if(cr)
	  {
	    if(!rbuf->add(cr))
	    {
	      xdelete(cr);
	    }
	  }
	  else
	    wlog->warn("CheckState::write_unfinished failed to allocate CheckResult");
	}
      }

      r = write_results();
    }
    else
      wlog->warn("CheckState::write_unfinished failed to allocate Array");
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CheckState::write_unfinished = %s",
		 IOTF(r));
#endif
  
  return(r);
}
