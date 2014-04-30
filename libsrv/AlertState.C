/*
 * AlertState.C: Object to manage state information for Alerts
 *
 * Version: $Revision: 0.29 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2006/10/17 14:02:39 $
 * MT-Level: Unsafe
 *
 * Copyright (c) 2002 - 2006
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: AlertState.C,v $
 * Revision 0.29  2006/10/17 14:02:39  benno
 * Add since
 *
 * Revision 0.28  2006/09/23 01:13:35  benno
 * Fix file descriptor leaks
 *
 * Revision 0.27  2004/06/12 00:52:48  benno
 * State is XML based
 *
 * Revision 0.26  2003/11/29 05:17:28  benno
 * Use AlertHistory/AlertHistoryRecord instead of direct manipulation
 *
 * Revision 0.25  2003/10/06 22:52:11  benno
 * Add support for fix if defined
 *
 * Revision 0.24  2003/04/21 20:28:55  benno
 * Use lock_host_state correctly
 *
 * Revision 0.23  2003/04/13 20:00:30  benno
 * Add verify_freshness
 * Call generate_csid
 *
 * Revision 0.22  2003/04/09 20:23:44  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.21  2003/04/07 20:58:11  benno
 * Fix typo
 *
 * Revision 0.20  2003/04/03 03:45:13  benno
 * Use Debugger
 *
 * Revision 0.19  2003/03/31 12:54:21  benno
 * lock_host_state with three args
 *
 * Revision 0.18  2003/03/04 17:55:51  benno
 * Pass CheckState to acknowledge
 * Permit acks before any alerts are sent
 * Use toss_eol
 *
 * Revision 0.17  2003/01/23 22:28:08  benno
 * Use IONULL, IOTF, xdelete, and xadelete
 *
 * Revision 0.16  2003/01/02 04:46:54  benno
 * Fix bugs in historyfile() and read_status()
 *
 * Revision 0.15  2003/01/01 02:05:36  benno
 * Add read_status
 * Fold lastalert into alertstatus
 *
 * Revision 0.14  2002/12/16 00:37:59  benno
 * Use RecipientSets and accumulate results for multiple calllists
 *
 * Revision 0.13  2002/12/11 20:08:36  benno
 * Add comments to inhibitions and acknowledgements
 * Use new style AlertPlan
 *
 * Revision 0.12  2002/10/25 22:55:08  benno
 * initialize caches
 * be more careful at clean up
 *
 * Revision 0.11  2002/09/05 21:56:52  benno
 * use try_fopen
 *
 * Revision 0.10  2002/08/06 19:42:29  selsky
 * Remove embedded nulls in format
 *
 * Revision 0.9  2002/05/31 21:37:10  benno
 * unlink before verify_file
 *
 * Revision 0.8  2002/05/22 18:53:47  selsky
 * fix format string for time_t
 *
 * Revision 0.7  2002/04/04 20:06:20  benno
 * copyright
 *
 * Revision 0.6  2002/04/03 17:34:03  benno
 * rcsify date
 *
 * Revision 0.5  2002/04/03 17:33:50  benno
 * Add acktime and inhibittime methods
 * Change token from long to char *
 * Add token() method
 * Write alerthistory file
 *
 * Revision 0.4  2002/04/03 17:32:31  benno
 * Use verify_file
 *
 * Revision 0.3  2002/04/03 17:31:57  benno
 * Rename and add to escalate methods
 *
 * Revision 0.2  2002/04/03 17:31:18  benno
 * Clean up
 *
 * Revision 0.1  2002/04/03 17:29:59  benno
 * initial revision
 *
 */

#include "survivor.H"

AlertState::AlertState(Check *check, char *host)
{
  // Allocate a new AlertState object, which manages state information for
  // the Alerts related to Check <c> for the host <host>.

  // Returns: A new, initialized AlertState.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertState::AlertState(%d,%s)",
		   check, IONULL(host));
#endif

  c = check;
  h = xstrdup(host);
  af = NULL;
  ef = NULL;
  nf = NULL;
  sf = NULL;
  tf = NULL;
  uncache(true);
  r_cmt = NULL;
  r_crc = MODEXEC_MAXRETURN;
  r_rset = NULL;
  
  // Note the last state update.
  last = 0;
  verify_freshness();
  
  // All State objects must do this
  csid = generate_csid("alert",
		       (char *)(host ? host : ""),
		       (char *)(check ? check->name() : ""));

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertState::AlertState()");
#endif
}

bool AlertState::acknowledge(CheckState *cs, char *who, char *why)
{
  // Mark the alert for this service/host as acknowledged by <who>
  // because of <why>, using <cs>.  This method does not check for an
  // outstanding alert, not does it see if the alert has already been
  // acknowledged.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "AlertState::acknowledge(%d,%s,%s)",
		   cs, IONULL(who), IONULL(why));
#endif

  if(cs && who && ackfile() && c && h)
  {
    SurvivorXML *sxml = new SurvivorXML();
    
    if(sxml)
    {
      int locknum = lock_host_state(h, c->name(), true);

      if(locknum > -1)
      {
	// Toss acache, we're going to recreate it

	xdelete(acache);
      
	// Unlink in case the user calling this method isn't the same
	// as the owner of the file.
	
	unlink(ackfile());
	
	int fdout = try_open(ackfile(), O_WRONLY | O_CREAT, FILE_GRP_WR);

	if(fdout > -1)
	{
	  // If why isn't given, we insert the default excuse
    
	  acache = new Acknowledgement(who, MAYBE_EXCUSE(why));
	  
	  if(acache)
	  {
	    if(sxml->generate(fdout, acache))
	    {
	      // Assume success, even if statusfile part fails

	      ret = true;
	    }
	    else
	    {
	      xdelete(acache);
	      
	      wlog->warn("AlertState::acknowledge failed to write acknowledgement for %s@%s", c->name(), h);
	    }
	  }
	  else
	    wlog->warn("AlertState::acknowledge failed to allocate Acknowledgement");

	  close(fdout);
	
	  if(!verify_file(ackfile(), FILE_GRP_WR))
	    wlog->warn("AlertState::acknowledge failed to reset permissions on %s",
		       ackfile());
	  
	  if(ret)
	  {
	    // See if there is a statusfile.
	    
	    struct stat sb;
	  
	    if(statusfile() && stat(statusfile(), &sb)!=0)
	    {
	      // There is no statusfile yet.  In order to acknowledge a
	      // problem for which an alert hasn't yet been transmitted,
	      // we need to write out a statusfile containing the
	      // check return code, otherwise the scheduler will think
	      // that the state has cleared and try to clear out the
	      // acknowledgement.  Use the CheckState to determine the
	      // appropriate crc.
	      
	      AlertStateData *asd = new AlertStateData(NULL, 0,
						       cs->returncode(),
						       NULL, 0, 0);
	      
	      if(asd)
	      {
		int fdout = try_open(statusfile(), O_WRONLY | O_CREAT,
				     FILE_GRP_WR);

		if(fdout > -1)
		{
		  sxml->generate(fdout, asd);

		  close(fdout);

		  if(!verify_file(statusfile(), FILE_GRP_WR))
		    wlog->warn("AlertState::acknowledge failed to reset permissions on %s",
			       statusfile());
		}
		else
		  wlog->warn("AlertState::acknowledge failed to open %s",
			     statusfile());
		
		xdelete(asd);
	      }
	      else
		wlog->warn("AlertState::acknowledge failed to allocate AlertStateData");
	    }
	  }
	}
	else
	  wlog->warn("AlertState::acknowledge failed to open %s", ackfile());

	unlock_state(locknum);
      }

      xdelete(sxml);
    }
    else
      wlog->warn("AlertState::acknowledge failed to allocate SurvivorXML");
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "AlertState::acknowledge = %s", IOTF(ret));
#endif

  return(ret);
}

char *AlertState::acknowledged()
{
  // Determine the identity of who acknowledged this service/host, if it is
  // so acknowledged.

  // Returns: A string that remains valid for the life of the AlertState and
  // that contains the identity, or NULL if there is no identity or on
  // error.

  char *ret = NULL;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertState::acknowledged()");
#endif

  if(!acache && c && h && ackfile())
  {
    int locknum = lock_host_state(h, c->name(), false);

    if(locknum > -1)
    {
      // Toss anything old
      xdelete(acache);

      // Open the file to pass to the XML parser

      int fdin = try_open(ackfile(), O_RDONLY, 0);

      if(fdin > -1)
      {
	SurvivorXML *sxml = new SurvivorXML();

	if(sxml)
	{
	  acache = sxml->parse_acknowledgement(fdin);
	  
	  xdelete(sxml);
	}
	else
	  wlog->warn("AlertState::acknowledged() failed to allocate SurvivorXML");

	close(fdin);
      }
      // if open fails, fail silently since the file may not exist

      unlock_state(locknum);
    }
  }

  if(acache)
    ret = acache->who();
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertState::acknowledged = %s", IONULL(ret));
#endif
    
  return(ret);
}

char *AlertState::acknowledged_for()
{
  // Determine why the alert was acknowledged.

  // Returns: The reason provided, or NULL on error.

  char *ret = NULL;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertState::acknowledged_for()");
#endif

  // Check the who cache first, if it is set then the why cache should
  // also be set.

  if(!acache)
    acknowledged();  // This will read all relevant state

  if(acache)
    ret = acache->why();
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertState::acknowledged_for = %s",
		 IONULL(ret));
#endif

  return(ret);
}

time_t AlertState::acktime()
{
  // Determine the time at which the alert was acknowledged.

  // Returns: The unix time of the acknowledgement, or 0 if there is none.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertState::acktime()");
#endif

  int r = 0;
  
  if(c && h && ackfile())
  {
    // Don't bother locking the host state since we're basically
    // performing an atomic read.  Just stat the file and return the
    // mod time.  (We could return the create time, but in case the
    // file gets updated...)
    
    struct stat sb;
    
    if(stat(ackfile(), &sb)==0)
      r = sb.st_mtime;
    // if stat fails, fail silently since the file may not exist
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertState::acktime = %d", r);
#endif
    
  return(r);
}

int AlertState::alertfor()
{
  // Determine the check result code that this alert was for.

  // Returns: The check result code, or 0 if no result code was found.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertState::alertfor()");
#endif

  int ret = 0;

  if(!dcache)
    read_status(true);

  if(dcache)
    ret = dcache->checkreturncode();
  
  if(ret < 0)
    ret = 0;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertState::alertfor = %d", ret);
#endif
  
  return(ret);
}

int AlertState::alerts()
{
  // Determine the number of alerts that have been generated.

  // Returns: The number of alerts, or 0 if no alerts have been sent or
  // if unable to access the state information.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertState::alerts()");
#endif

  int ret = 0;

  if(!dcache)
    read_status(true);

  if(dcache)
    ret = dcache->instances();
  
  if(ret < 0)
    ret = 0;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertState::alerts = %d", ret);
#endif
  
  return(ret);
}

bool AlertState::clearstate()
{
  // Clear all Alert state files (except noalert, which must be manually
  // removed).

  // Returns: true if successful, including if there were no files to be
  // removed, false otherwise.

  bool r = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertState::clearstate()");
#endif

  if(c && c->name() && h)
  {
    int locknum = lock_host_state(h, c->name(), true);

    if(locknum > -1)
    {
      // Unlink acknowledge, alertstatus, escalate, and token files
      // without checking for errors.  Don't unlink noalert.

      if(ackfile())
	unlink(ackfile());

      if(statusfile())
	unlink(statusfile());

      if(escalatefile())
	unlink(escalatefile());

      if(tokenfile())
	unlink(tokenfile());
	
      uncache(false);
      
      r = true;

      unlock_state(locknum);
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertState::clearstate = %s", IOTF(r));
#endif
  
  return(r);
}

char *AlertState::commentfor()
{
  // Determine the check comment that this alert was for.

  // Returns: The check result code, or 0 if no result code was found.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertState::commentfor()");
#endif

  char *ret = NULL;

  if(!dcache)
    read_status(true);

  if(dcache)
    ret = dcache->checksummary();

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertState::commentfor = %s", IONULL(ret));
#endif
  
  return(ret);
}

bool AlertState::escalate_manual(int to)
{
  // Manually escalate the Alert to <to> trys.  No sanity checks are
  // performed to determine if there is an outstanding warning or if
  // <to> is too large.

  // Returns: true if fully successful, false otherwise.
  
  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "AlertState::escalate_manual(%d)", to);
#endif
  
  if(escalatefile() && c && h)
  {
    SurvivorXML *sxml = new SurvivorXML();

    if(sxml)
    {
      int locknum = lock_host_state(h, c->name(), true);

      if(locknum > -1)
      {
	// Toss ecache, we're going to recreate it
	
	xdelete(ecache);
	
	// Unlink in case the user calling this method isn't the same
	// as the owner of the file.
      
	unlink(escalatefile());

	int fdout = try_open(escalatefile(), O_WRONLY | O_CREAT, FILE_GRP_WR);

	if(fdout > -1)
	{
	  // If why isn't given, we insert the default excuse

	  ecache = new Escalation(to);

	  if(ecache)
	  {
	    if(sxml->generate(fdout, ecache))
	      ret = true;
	    else
	    {
	      xdelete(ecache);
	      
	      wlog->warn("AlertState::escalate_manual failed to write escalation for %s@%s", c->name(), h);
	    }
	  }
	  else
	    wlog->warn("AlertState::escalate_manual failed to allocate Escalation");

	  close(fdout);

	  if(!verify_file(escalatefile(), FILE_GRP_WR))
	    wlog->warn("AlertState::escalate_manual failed to reset permissions on %s",
		       escalatefile());
	}

	unlock_state(locknum);
      }
     
      xdelete(sxml);
    }
    else
      wlog->warn("AlertState::escalate_manual failed to allocate SurvivorXML");
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "AlertState::escalate_manual = %s", IOTF(ret));
#endif
  
  return(ret);
}

bool AlertState::escalated(AlertPlan *ap, CheckState *cs)
{
  // Determine if this AlertState has escalated, in accordance with
  // the AlertPlan <ap> and CheckState <cs>.  <cs> must be for the
  // same Check as this AlertState.

  // Returns: true if this AlertState has escalated, false otherwise.

  bool r = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertState::escalated(%d,%d)", ap, cs);
#endif

  if(ap && cs)
  {
    if(cs->lastcheck() > 0)
    {
      // A service cannot be escalated if it hasn't been checked yet.
      // In addition, (1) there must be an outstanding failure, (2)
      // the number of warnings transmitted is greater than or equal
      // to the second "try" stanza or the "try" stanza flagged for
      // escalation for the given return value, and (3) an Alert has
      // already been transmitted.  A problem shouldn't be considered
      // escalated if no alert has been sent.

      if(cs->returncode() > 0)
      {
	int alertcount = escalated_manual();

	if(alertcount == 0)
	  alertcount = alerts();
	
	if(alertcount > 0 && !quiet())
	{
	  AlertReturnGroup *arg = ap->match(cs->returncode(), true,
					    cs->consecutive());

	  // Find a schedule that is in use

	  if(arg && arg->schedules())
	  {
	    for(int i = 0;i < arg->schedules()->entries();i++)
	    {
	      AlertSchedule *a = arg->schedules()->retrieve(i);

	      if(a && a->now())
	      {
		// Obtain the AlertTry for the number of alerts that have
		// already been sent
		
		AlertTry *t = a->alerttry(alertcount,
					  (c->fix() ? true : false));

		if(t)
		{
		  // Tell the AlertSchedule to figure out if we're in
		  // an escalated state.  We can't just look at t->escalated()
		  // since that only indicates which AlertTry marks the
		  // beginning of the escalated AlertTrys, and only then
		  // if it isn't the default.

		  r = a->escalated(t);
		}
		
		break;
	      }
	    }
	  }
	}
      }
    }
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertState::escalated = %s", IOTF(r));
#endif
  
  return(r);
}

int AlertState::escalated_manual()
{
  // Determine if this Alert has been manually escalated, and if so
  // what level (number of tries) it has been escalated to.

  // Returns: The number of tries escalated to, or 0 if no escalation
  // has been set or if unable to access the state information.

  int ret = -1;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertState::escalated_manual()");
#endif

  if(!ecache && escalatefile() && c && h)
  {
    int locknum = lock_host_state(h, c->name(), false);

    if(locknum > -1)
    {
      // Toss anything old
      xdelete(ecache);
 
      // Open the file to pass to the XML parser
      
      int fdin = try_open(escalatefile(), O_RDONLY, 0);
      
      if(fdin > -1)
      {
	SurvivorXML *sxml = new SurvivorXML();

	if(sxml)
	{
	  ecache = sxml->parse_escalation(fdin);
	  
	  xdelete(sxml);
	}
	else
	  wlog->warn("AlertState::escalated_manual() failed to allocate SurvivorXML");

	close(fdin);
      }
      // if open fails, fail silently since the file may not exist
      
      unlock_state(locknum);
    }
  }

  if(ecache)
    ret = ecache->escalated_to();
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertState::escalated_manual = %d", ret);
#endif
  
  return(ret);
}

bool AlertState::inhibit(char *who, char *why)
{
  // Inhibit alerts from being generated for this service/host, as
  // requested by <who> because of <why>.  This method does not see if
  // an inhibition is already in place.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "AlertState::inhibit(%s,%s)",
		  IONULL(who), IONULL(why));
#endif

  if(who && noalertfile() && c && h)
  {
    SurvivorXML *sxml = new SurvivorXML();

    if(sxml)
    {
      int locknum = lock_host_state(h, c->name(), true);
      
      if(locknum > -1)
      {
	// Toss icache, we're going to recreate it

	xdelete(icache);
	
	// Unlink in case the user calling this method isn't the same
	// as the owner of the file.
	
	unlink(noalertfile());

	int fdout = try_open(noalertfile(), O_WRONLY | O_CREAT, FILE_GRP_WR);

	if(fdout > -1)
	{
	  // If why isn't given, we insert the default excuse

	  icache = new Inhibition(who, MAYBE_EXCUSE(why));

	  if(icache)
	  {
	    if(sxml->generate(fdout, icache))
	      ret = true;
	    else
	    {
	      xdelete(icache);
	      
	      wlog->warn("AlertState::inhibit failed to write inhibition for %s@%s", c->name(), h);
	    }
	  }
	  else
	    wlog->warn("AlertState::inhibit failed to allocate Inhibition");

	  close(fdout);

	  if(!verify_file(noalertfile(), FILE_GRP_WR))
	    wlog->warn("AlertState::inhibit failed to reset permissions on %s",
		       noalertfile());
	}
	
	unlock_state(locknum);
      }
      
      xdelete(sxml);
    }
    else
      wlog->warn("AlertState::inhibit failed to allocate SurvivorXML");
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "AlertState::inhibit = %s", IOTF(ret));
#endif

  return(ret);
}

char *AlertState::inhibited()
{
  // Determine the identity of who inhibited this service/host, if it is
  // so inhibited.

  // Returns: A string that remains valid for the life of the AlertState and
  // that contains the identity, or NULL if there is no identity or on
  // error.

  char *ret = NULL;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertState::inhibited()");
#endif

  if(!icache && c && h && noalertfile())
  {
    int locknum = lock_host_state(h, c->name(), false);

    if(locknum > -1)
    {
      // Toss anything old
      xdelete(icache);
 
      // Open the file to pass to the XML parser

      int fdin = try_open(noalertfile(), O_RDONLY, 0);

      if(fdin > -1)
      {
	SurvivorXML *sxml = new SurvivorXML();

	if(sxml)
	{
	  icache = sxml->parse_inhibition(fdin);
	  
	  xdelete(sxml);
	}
	else
	  wlog->warn("AlertState::inhibited() failed to allocate SurvivorXML");

	close(fdin);
      }
      // if open fails, fail silently since the file may not exist

      unlock_state(locknum);
    }
  }

  if(icache)
    ret = icache->who();
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertState::inhibited = %s", IONULL(ret));
#endif
    
  return(ret);
}

char *AlertState::inhibited_for()
{
  // Determine why the alert was inhibited.

  // Returns: The reason provided, or NULL on error.

  char *ret = NULL;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertState::inhibited_for()");
#endif

  // Check the who cache first, if it is set then the why cache should
  // also be set.

  if(!icache)
    inhibited();  // This will read all relevant state

  if(icache)
    ret = icache->why();
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertState::inhibited_for = %s", IONULL(ret));
#endif

  return(ret);
}

time_t AlertState::inhibittime()
{
  // Determine the time at which the alert was inhibited.

  // Returns: The unix time of the inhibition, or 0 if there is none.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertState::inhibittime()");
#endif

  int r = 0;
  
  if(c && h)
  {
    // No need to lock the host state for this operation, since we're
    // basically performing an atomic read.  Just stat the file and
    // return the mod time.  (We could return the create time, but in
    // case the file gets updated...)
  
    struct stat sb;
      
    if(stat(noalertfile(), &sb)==0)
      r = sb.st_mtime;
    // if stat fails, fail silently since the file may not exist
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertState::inhibittime = %d", r);
#endif
    
  return(r);
}

time_t AlertState::lastalert()
{
  // Determine the last time an Alert was generated.

  // Returns: The unix time of the last alert, or 0 if no alert has been
  // sent or if unable to access the state information.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertState::lastalert()");
#endif
  
  int ret = 0;
  
  if(!dcache)
    read_status(true);

  if(dcache)
    ret = dcache->time();

  if(ret < 0)
    ret = 0;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertState::lastalert = %d", ret);
#endif
  
  return(ret);
}

RecipientSet *AlertState::lastnotify()
{
  // Determine the last address(es) notified.

  // Returns: A RecipientSet that remains valid for the duration of the
  // AlertState, or NULL on error.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertState::lastnotify()");
#endif

  RecipientSet *ret = NULL;

  if(!dcache)
    read_status(true);

  if(dcache)
    ret = dcache->recipients();
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertState::lastnotify = %d", ret);
#endif
    
  return(ret);
}

bool AlertState::quiet()
{
  // Determine if the host/service is quiet, so that no alert should be
  // generated.  This is true if (1) an acknowledge file exists or
  // (2) a noalert file exists.

  // Returns: true if the AlertState is quiet, false otherwise.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertState::quiet()");
#endif

  if(qcache == -1 && c && h && ackfile() && noalertfile())
  {
    int locknum = lock_host_state(h, c->name(), false);

    if(locknum > -1)
    {
      if(access(ackfile(), F_OK)==0)
	qcache = 1;
      else
      {
	if(access(noalertfile(), F_OK)==0)
	  qcache = 1;
      }

      unlock_state(locknum);
    }
  }

  bool r = false;

  if(qcache == 1)
    r = true;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertState::quiet = %s", IOTF(r));
#endif
  
  return(r);
}

bool AlertState::results_accumulate(int rc, char *module, char *notify)
{
  // Add the Alert results for <module>, exiting with return code <rc>,
  // using addresses <notify>.  If this method is called more than once
  // for a given <module>, only the highest <rc> will be retained for
  // that module.

  // Returns: true if the information was successful stored, false
  // otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertState::results_accumulate(%d,%s,%s)",
		   rc, IONULL(module), IONULL(notify));
#endif

  if(r_rset && r_rset->add(notify, module, rc))
    ret = true;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertState::results_accumulate = %s",
		 IOTF(ret));
#endif

  return(ret);
}

void AlertState::results_finish()
{
  // Clean up the results caching.  Call this only after the results have
  // been written, or else any changes will be lost.

  // Returns: Nothing.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertState::results_finish()");
#endif

  xdelete(r_cmt);
  r_crc = MODEXEC_MAXRETURN;
  xdelete(r_rset);
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertState::results_finish()");
#endif
}

bool AlertState::results_init(int crc, char *cmt)
{
  // Initialize the results caching for the check return code <crc>
  // and check comment <cmt>.  Only one collection may be outstanding
  // at any time, use results_write to save the changes and
  // results_finish to end the collection.

  // Returns: true if initialization is fully successful, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertState::results_init(%d,%s)", crc,
		  IONULL(cmt));
#endif

  // We assume everything is init'd already, by the constructor or
  // results_finish, so we just try to allocate storage.
  
  r_rset = new RecipientSet();
  r_cmt = xstrdup(cmt);
  r_crc = crc;

  if(r_rset)
    ret = true;
  else
    results_finish();
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertState::results_init = %s", IOTF(ret));
#endif

  return(ret);
}

bool AlertState::results_write()
{
  // Write out the currently accumulated results.  This method may be
  // called multiple times (presumably after each accumulation), and
  // should be followed by results_finish() when accumulation is
  // complete.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertState::results_write()");
#endif

  if(r_rset && c && h)
  {
    struct timeval tv;

    if(gettimeofday(&tv, NULL)==0)
    {
      int locknum = lock_host_state(h, c->name(), true);

      if(locknum > -1)
      {
	// Successful unless otherwise noted.
	
	ret = true;

	// Don't write out state files if the check results was OK,
	// since that indicates an upalert and upalerts go out when
	// the alert state is cleared.  We do want to write out the
	// history records, though.
	
	if(r_crc != MODEXEC_OK && statusfile())
	{
	  // Try to get the previous state information and see if we should
	  // increment the consecutive counter.  If any of this fails, just
	  // move on.
	
	  int consecutive = 0;
	  time_t st = tv.tv_sec;

	  read_status(false);

	  if(dcache)
	  {
	    // While CheckState resets counters when crc changes, we don't.
	    // (ie: once we start paging, we keep incrementing the counter
	    // until we stop.)
	    
	    consecutive = dcache->instances();

	    if(dcache->since() > 0)
	      st = dcache->since();
	  }
	  
	  consecutive++;
	  
	  // Toss dcache, we're going to recreate it

	  xdelete(dcache);

	  // Write out the current status information, including time,
	  // address, and success of each attempted transmission.
	  
	  // Unlink in case the user calling this method isn't the same
	  // as the owner of the file.
	  
	  unlink(statusfile());

	  int fdout = try_open(statusfile(), O_WRONLY | O_CREAT,
			       FILE_GRP_WR);

	  if(fdout > -1)
	  {
	    SurvivorXML *sxml = new SurvivorXML();

	    if(sxml)
	    {
	      dcache = new AlertStateData(r_rset, consecutive, r_crc,
					  r_cmt, tv.tv_sec, st);

	      if(dcache)
	      {
		if(!sxml->generate(fdout, dcache))
		{
		  ret = false;
		  xdelete(dcache);
			    
		  wlog->warn("AlertState::write_results failed to write results for %s@%s", c->name(), h);
		}
	      }
	      else
	      {
		ret = false;
		wlog->warn("AlertState::write_results failed to allocate AlertStateData");
	      }

	      xdelete(sxml);
	    }
	    else
	    {
	      ret = false;
	      wlog->warn("AlertState::write_results failed to allocate SurvivorXML");
	    }

	    close(fdout);

	    if(!verify_file(statusfile(), FILE_GRP_WR))
	      wlog->warn("AlertState::results_write failed to reset permissions on %s",
			 statusfile());
	  }
	  else
	  {
	    wlog->warn("AlertState::results_write failed to open %s",
		       statusfile());
	    ret = false;
	  }

	  // Write out the token

	  if(token() && tokenfile())
	  {
	    unlink(tokenfile());
	    
	    FILE *out = try_fopen(tokenfile(), "w");
	    
	    if(out)
	    {
	      fprintf(out, "%s\n", token());
	      fclose(out);
	      
	      if(!verify_file(tokenfile(), FILE_GRP_WR))
		wlog->warn("AlertState::results_write failed to reset permissions on %s",
			   tokenfile());
	      
	    }
	    else
	    {
	      wlog->warn("AlertState::results_write failed to open %s",
			 tokenfile());
	      ret = false;
	    }
	  }
	  else
	  {
	    wlog->warn("AlertState::results_write cannot write token");
	    ret = false;
	  }
	}

	// Write out a history entry for each module.

	// Failure to update the history file is not sufficient to cause
        // the entire method to fail

	AlertHistory *ah = new AlertHistory(c, h);

	if(ah)
	{
	  for(int i = 0;i < r_rset->modules();i++)
	  {
	    char *xm = r_rset->module(i);
	    char *xa = r_rset->addresses(i);
	    int xrc = r_rset->rc(i);
	
	    AlertHistoryRecord *ahr = new AlertHistoryRecord(xrc,
							     r_crc,
							     xa,
							     xm);
	    if(ahr)
	    {
	      if(!ah->record(ahr))
		wlog->warn("AlertState::results_write failed to record history");
	      
	      xdelete(ahr);
	    }
	    else
	      wlog->warn("AlertState::results_write failed to allocate ahr");
	  }

	  xdelete(ah);
	}
	else
	  wlog->warn("AlertState::results_write failed to allocate ah");
	
	uncache(false);
	unlock_state(locknum);
      }
    }
    else
      wlog->warn("gettimeofday() failed in AlertState::results_write()");
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertState::results_write = %s", IOTF(ret));
#endif
  
  return(ret);
}

time_t AlertState::since()
{
  // Determine the first time an Alert was generated.

  // Returns: The unix time of the first alert, or 0 if no alert has been
  // sent or if unable to access the state information.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertState::since()");
#endif
  
  int ret = 0;
  
  if(!dcache)
    read_status(true);

  if(dcache)
    ret = dcache->since();

  if(ret < 0)
    ret = 0;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertState::since = %d", ret);
#endif
  
  return(ret);
}

char *AlertState::token()
{
  // Determine the token for this alert.  If no token exists, a new one
  // will be generated.

  // Returns: A string that remains valid for the life of the AlertState and
  // that contains the token, or NULL if there is no token or on error.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertState::token()");
#endif

  if(!tcache && c && h)
  {
    // XXX This will actually prematurely mark an update, since a token
    // file may already exist.  However, it is likely that this bug will
    // be masked since in most cases an alert will shortly be generated,
    // causing the state to be updated anyway.
    
    int locknum = lock_host_state(h, c->name(), true);

    if(locknum > -1)
    {
      // Use state to find out the size of the file, then use fgets to transfer
      // not more than that characters.
  
      struct stat sb;
      
      if(stat(tokenfile(), &sb)==0)
      {
	tcache = new char[sb.st_size + 1];
	
	if(tcache)
	{
	  memset(tcache, 0, sb.st_size + 1);
	  FILE *in = try_fopen(tokenfile(), "r");
	  
	  if(in)
	  {
	    fgets(tcache, sb.st_size, in);
	    fclose(in);
	    
	    // Chop off the newline.  We don't really verify the contents of
	    // what we read.

	    toss_eol(tcache);
	  }
	  else
	  {
	    xdelete(tcache);
	  }
	}
	else
	  wlog->warn("AlertState::acknowledged failed to allocate tcache");
      }
      // if stat fails, fail silently since the file may not exist

      unlock_state(locknum);
    }

    if(!tcache)
    {
      // Generate a new 32-bit number to use as a token.

      tcache = new char[11];

      if(tcache)
      {
	long l = prng->n32bit();
	
	// Replace the first number in the token with a letter so as
	// not to confuse Nextel phones into thinking the token is a
	// phone number.
	
	sprintf(tcache, "%10.10ld", l);
	tcache[0] += 33;  // 33 is 'Q' - '0'
      }
      else
	wlog->warn("AlertState::token failed to allocate tcache");
    }
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertState::token = %s", IONULL(tcache));
#endif
    
  return(tcache);
}

bool AlertState::unacknowledge()
{
  // Unacknowledge this alert.

  // Returns: true if fully successful, false otherwise.

  bool r = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "AlertState::unacknowledge()");
#endif

  if(ackfile() && c && h)
  {
    int locknum = lock_host_state(h, c->name(), true);

    if(locknum > -1)
    {
      if(unlink(ackfile())==0)
	r = true;

      unlock_state(locknum);
    }
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "AlertState::unacknowledge = %s", IOTF(r));
#endif

  return(r);
}

bool AlertState::uninhibit()
{
  // Remove the inhibition for this alert.

  // Returns: true if fully successful, false otherwise.

  bool r = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "AlertState::uninhibit()");
#endif

  if(noalertfile() && c && h)
  {
    int locknum = lock_host_state(h, c->name(), true);

    if(locknum > -1)
    {
      if(unlink(noalertfile())==0)
	r = true;

      unlock_state(locknum);
    }
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "AlertState::uninhibit = %s", IOTF(r));
#endif

  return(r);
}

bool AlertState::verify_freshness()
{
  // Determine if the underlying state has changed, and if so expire
  // any cached information.

  // Returns: true if the verification is successful, regardless of
  // freshness, false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertState::verify_freshness()");
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
      dlog->log_progress(DEBUG_CACHES, "Expiring alert cache for %s@%s",
			 c->name(), h);
#endif
      
      last = lu;
      uncache(false);
    }

    ret = true;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertState::verify_freshness = %s", IOTF(ret));
#endif

  return(ret);
}

AlertState::~AlertState()
{
  // Deallocate the AlertState object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertState::~AlertState()");
#endif
  
  c = NULL;
  xdelete(h);
  xdelete(af);
  xdelete(ef);
  xdelete(nf);
  xdelete(sf);
  xdelete(tf);

  uncache(false);
  results_finish();

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertState::~AlertState()");
#endif
}

char *AlertState::ackfile()
{
  // Obtain or generate the filename containing the acknowledgement status.

  // Returns: A pointer to the filename, or NULL on error.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertState::ackfile()");
#endif

  if(!af && c && h)
  {
    af = new char[strlen(args->statedir()) + strlen(h) + strlen(c->name())
		 + 20];
      
    if(af)
      sprintf(af, "%s/host/%s/%s/acknowledge", args->statedir(), h,
	      c->name());
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertState::ackfile = %s", IONULL(af));
#endif

  return(af);
}

char *AlertState::escalatefile()
{
  // Obtain or generate the filename containing the escalate value.

  // Returns: A pointer to the filename, or NULL on error.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertState::escalatefile()");
#endif

  if(!ef && c && h)
  {
    ef = new char[strlen(args->statedir()) + strlen(h) + strlen(c->name())
		 + 17];
      
    if(ef)
      sprintf(ef, "%s/host/%s/%s/escalate", args->statedir(), h, c->name());
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertState::escalatefile = %s", IONULL(ef));
#endif

  return(ef);
}

char *AlertState::noalertfile()
{
  // Obtain or generate the filename containing the noalert status.

  // Returns: A pointer to the filename, or NULL on error.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertState::noalertfile()");
#endif

  if(!nf && c && h)
  {
    nf = new char[strlen(args->statedir()) + strlen(h) + strlen(c->name())
		 + 16];
      
    if(nf)
      sprintf(nf, "%s/host/%s/%s/noalert", args->statedir(), h, c->name());
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertState::noalertfile = %s", IONULL(nf));
#endif

  return(nf);
}

bool AlertState::read_status(bool dolock)
{
  // Read the Alert status from statusfile() and cache the results.
  // If <dolock> is false, read_status will assume a suitable lock has
  // already been obtained.

  // Returns: true if fully successful, false otherwise, including if
  // there is no status to cache.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertState::read_status(%s)", IOTF(dolock));
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

      // Let the XML Parser do the hard work, but we have to open the
      // file first

      int fdin = try_open(statusfile(), O_RDONLY, 0);

      if(fdin > -1)
      {
	SurvivorXML *sxml = new SurvivorXML();

	if(sxml)
	{
	  dcache = sxml->parse_alertstatedata(fdin);

	  if(dcache)
	    ret = true;
	  
	  xdelete(sxml);
	}
	else
	  wlog->warn("AlertState::read_status() failed to allocate SurvivorXML");

	close(fdin);
      }

      if(dolock)
	unlock_state(locknum);
    }
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertState::read_status() = %s", IOTF(ret));
#endif
  
  return(ret);
}

char *AlertState::statusfile()
{
  // Obtain or generate the filename containing the last alert status.

  // Returns: A pointer to the filename, or NULL on error.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertState::statusfile()");
#endif

  if(!sf && c && h)
  {
    sf = new char[strlen(args->statedir()) + strlen(h) + strlen(c->name())
		 + 20];
      
    if(sf)
      sprintf(sf, "%s/host/%s/%s/alertstatus", args->statedir(), h,
	      c->name());
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertState::statusfile = %s", IONULL(sf));
#endif

  return(sf);
}

char *AlertState::tokenfile()
{
  // Obtain or generate the filename containing the alert token.

  // Returns: A pointer to the filename, or NULL on error.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertState::tokenfile()");
#endif

  if(!tf && c && h)
  {
    tf = new char[strlen(args->statedir()) + strlen(h) + strlen(c->name())
		 + 14];
    
    if(tf)
      sprintf(tf, "%s/host/%s/%s/token", args->statedir(), h, c->name());
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertState::tokenfile = %s", IONULL(tf));
#endif

  return(tf);
}

void AlertState::uncache(bool init)
{
  // Clear any cached results.  <init> should only be true when called
  // by the constructor.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertState::uncache(%s)", IOTF(init));
#endif
  
  qcache = -1;

  if(!init)
  {
    if(acache)
      delete acache;

    if(dcache)
      delete dcache;

    if(ecache)
      delete ecache;
    
    if(icache)
      delete icache;

    if(tcache)
      delete tcache;
  }

  acache = NULL;
  dcache = NULL;
  ecache = NULL;
  icache = NULL;
  tcache = NULL;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertState::uncache()");
#endif
}
