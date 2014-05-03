/*
 * scheduler.C: Scheduler object
 *
 * Version: $Revision: 0.30 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2007/01/20 05:57:07 $
 * MT-Level: Safe
 *  Private methods run_alerts and run_checks are Unsafe, and expect to be
 *  run only from within run_loop.
 *
 * Copyright (c) 2002 - 2007
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: Scheduler.C,v $
 * Revision 0.30  2007/01/20 05:57:07  benno
 * Fix bugs in DEBUG_SCHEDS logging
 *
 * Revision 0.29  2006/01/23 02:41:29  benno
 * Add support for "do not clear state" and "clear state honors returngroups"
 * Remove extra newline when constructing dependency message
 *
 * Revision 0.28  2005/09/26 13:31:36  benno
 * Changes to work with callliststate separating person on call from
 * person to notify (due to substitutions)
 *
 * Revision 0.27  2005/04/09 02:25:27  benno
 * Fix cleanup bug
 * Toss OLDCODE
 * Use CheckResult objects
 *
 * Revision 0.26  2004/06/11 22:20:18  benno
 * Add support for "result text significant"
 * CallListState changes
 *
 * Revision 0.25  2004/04/24 15:20:28  toor
 * Fix minor memory leak
 *
 * Revision 0.24  2004/03/01 22:21:43  benno
 * Add support for staggered scheduling
 *
 * Revision 0.23  2003/10/10 16:58:10  benno
 * Type I dependencies can't be dependent on themselves
 *
 * Revision 0.22  2003/10/06 22:18:50  benno
 * Add support for "fix if defined"
 * Changes for dependency overhaul
 *
 * Revision 0.21  2003/05/28 03:34:13  benno
 * Changes for Alert and AlertModule
 *
 * Revision 0.20  2003/04/21 20:30:43  benno
 * Don't clear out old state if there is a check pending
 *
 * Revision 0.19  2003/04/13 19:57:36  benno
 * Use StateCache for FixState and AlertState
 *
 * Revision 0.18  2003/04/09 19:50:03  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.17  2003/03/31 12:50:05  benno
 * Use StateCache for CheckState reads
 *
 * Revision 0.16  2003/01/29 01:55:26  benno
 * Use IONULL, IOTF
 *
 * Revision 0.15  2002/12/31 04:23:45  benno
 * Add support for fixes
 *
 * Revision 0.14  2002/12/22 17:32:19  benno
 * Remove global timeout
 * Use xdelete
 *
 * Revision 0.13  2002/12/15 22:56:34  benno
 * Use try based AlertPlans
 * Use RecipientSets for multiple calllists
 * verify_degrade checks minimum number of failures on each host
 *
 * Revision 0.12  2002/12/10 17:21:46  benno
 * Adjust test for clearing out alertstate after notify on clear
 *
 * Revision 0.11  2002/10/25 22:57:07  benno
 * use persons
 *
 * Revision 0.10  2002/08/06 03:39:00  selsky
 * Remove embedded '\0'
 *
 * Revision 0.9  2002/04/04 20:51:22  benno
 * copyright
 *
 * Revision 0.8  2002/04/03 19:32:39  benno
 * rcsify date
 *
 * Revision 0.7  2002/04/03 19:32:32  benno
 * Fix bug to properly implement verify_degraded
 *
 * Revision 0.6  2002/04/03 19:31:37  benno
 * Implement MODEXEC_MAXRETURN
 *
 * Revision 0.5  2002/04/03 19:30:53  benno
 * Log when not running a check for failed dependency
 * Don't check for dependencies when the service isn't checked on a host
 *
 * Revision 0.4  2002/04/03 19:30:09  benno
 * Fix bug in when to call as->clearstate
 * Call apx->match with four arguments
 *
 * Revision 0.3  2002/04/03 19:29:32  benno
 * Clean up
 *
 * Revision 0.2  2002/04/03 19:29:03  benno
 * Rewrite run_check
 * Verify Type I dependencies
 *
 * Revision 0.1  2002/04/03 19:28:14  benno
 * initial revision
 *
 */

#include "scheduler.H"

Scheduler::Scheduler(ProcessControl *p)
{
  // Allocate and initialize a new Scheduler object.

  // Returns: A new Scheduler object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, (char *)"Scheduler::Scheduler(%d)", p);
  dlog->log_exit(DEBUG_MAJTRC, (char *)"Scheduler::Scheduler()");
#endif

  pc = p;
  exit = false;
}

void Scheduler::begin_alert()
{
  // Instruct the Alert Scheduler to begin operation.  It is intended
  // that this method is dedicated to one thread, and that this method
  // will not return until the program is ready to exit.

  // Returns: Nothing.

  // Sleep before commencing operation to give the check scheduler a
  // chance to run and update state.

  int s = 60;

  if(locks->cf_read())
  {
    // Use alert shift value as the delay if set, otherwise one minute.
    
    if(cf->alert_shift() > 0)
      s = cf->alert_shift();
    
    locks->cf_unlock();
  }
  else
    wlog->warn((char *)"Failed to obtain cf read lock");

  sleep(s);
  
  run_loop(true, false);
}

void Scheduler::begin_check()
{
  // Instruct the Check Scheduler to begin operation.  It is intended
  // that this method is dedicated to one thread, and that this method
  // will not return until the program is ready to exit.

  // Returns: Nothing.

  run_loop(false, true);
}

bool Scheduler::end()
{
  // Tell the Scheduler to end operation.  This method returns when the
  // request has been sent, but the Scheduler may not have stopped by the
  // time this method returns.

  // Returns: true if the request is sent, false otherwise.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, (char *)"Scheduler::end");
  dlog->log_exit(DEBUG_MAJTRC, (char *)"Scheduler::end = true");
#endif
  
  exit = true;
  return(true);
}

Scheduler::~Scheduler()
{
  // Deallocate the Scheduler object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, (char *)"Scheduler::~Scheduler()");
  dlog->log_exit(DEBUG_MAJTRC, (char *)"Scheduler::~Scheduler()");
#endif

  pc = NULL;
  exit = false;
}

int Scheduler::run_alert(Check *c, List *g, int throttle)
{
  // Examine the status of the Check <c> for the list of hosts <g> and
  // determine if any Alerts need be scheduled.  A Configuration read
  // lock should be obtained before calling this method.  A maximum of
  // <throttle> alerts may be queued, unless <throttle> is -1.

  // Returns: <throttle> minus the number of alerts queued (minimun
  // return value 0), or -1 if throttling is disabled.

  int r = throttle;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, (char *)"Scheduler::run_alert(%d,%d,%d)",
		  c, g, throttle);
#endif

  // for each check module
  //  for each group of that name
  //   for each host within that group
  //    determine if there is an outstanding failure
  //    if so, generate an alert in accordance with the configuration,
  //    lastalert, acknowledgement, and escalate files.

  if(c && g)
  {
#if defined(DEBUG)
    dlog->log_progress(DEBUG_SCHEDS,
		       (char *)"Looking for alerts for Check %s using group %s",
		       IONULL(c ? c->name() : "?"),
		       IONULL(g ? g->name() : "?"));
#endif

    for(int i = 0;i < g->entries();i++)
    {
      CheckState *cs = scache->get_check_state(c, g->retrieve(i));

      if(cs)
      {
	if(cs->returncode() != MODEXEC_DEPEND)
	{
	  AlertState *as = scache->get_alert_state(c, g->retrieve(i));
	  FixState *fs = scache->get_fix_state(c, g->retrieve(i));

	  if(as && fs)
	  {
	    // Set up for a notify
	    
	    bool noteclear = false;
	    bool checkpending = false;
	    bool dontclear = false;

	    // Retrieve the AlertPlan here, although we might not need it.
	    // If there is an AlertPlan for the Check, use it, otherwise
	    // use the AlertPlan for the HostClass.
	      
	    AlertPlan *ap = c->alert_plan();
		
	    if(!ap)
	    {
	      HostClass *hc = cf->find_class(g->retrieve(i));
	      
	      if(hc)
		ap = hc->alert_plan();
	    }

	    if(ap)
	    {
	      if(cs->returncode() == 0 && as->lastalert() > 0)
	      {
		// The problem for this Check has cleared.  Indicate
		// notify on clear.
		
		noteclear = true;
		
		// However, if a fix recently ran successfully, the check
		// may not yet have run to verify the fix.  (check return
		// code = 0, lastcheck = 0.)  If this is the case, don't
		// send out an upalert just yet, instead wait until the check
		// runs.
		
		if(cs->lastcheck() == 0 && fs->fix_attempts() > 0)
		  checkpending = true;
	      }

	      if(as->alertfor() != cs->returncode())
	      {
		// Ordinarily, if the return code changes, we clear
		// state.  However, the alertplan might specify to not
		// clear state for certain return codes (eg:
		// MODEXEC_MISCONFIG is useful in case a module gets
		// uninstalled), or for return codes defined in the
		// same returngroup, so check for those here.
		
		if(!ap->clear_state_on(as->alertfor(), cs->returncode()))
		  dontclear = true;

#if defined(DEBUG)
		dlog->log_progress(DEBUG_SCHEDS,
				   (char *)"- Not clearing state for %s@%s (return code changed from %d to %d",
				   IONULL(c->name()),
				   IONULL(g->retrieve(i)),
				   as->alertfor(),
				   cs->returncode());
#endif
	      }
	      
	      if(!noteclear && !dontclear &&
		 ((as->alertfor() != cs->returncode())
		  ||
		  c->result_text_significant() && as->commentfor() &&
		  cs->comment() &&
		  (strcmp(as->commentfor(), cs->comment())!=0)))
	      {
		// We have a change of Check state, which could be a problem
		// clearing, a problem being noticed, or a problem changing
		// severity.  Clear out alert state.  Note that the original
		// test here was for consecutive()==1, which is a very bad
		// test.
	      
		// Don't do this if this is a notify on clear scenario, since
		// we need to keep the lastalert state a bit longer.
		// (Alternately, schedule_alert could be passed noteclear, but
		// it would still need to know if an alert was already sent out
		// since it shouldn't send an upalert otherwise.)
		
		as->clearstate();
		fs->clearstate();
	      }
	    
	      if(cs->returncode() > 0 || (noteclear && !checkpending))
	      {
		// There is an outstanding error on this Check, or the error
		// cleared.
		
#if defined(DEBUG)
		dlog->log_progress(DEBUG_SCHEDS,
				   (char *)"- Outstanding error return on host %s, lastalert = %d, lastfix = %d, noteclear = %s",
				   IONULL(g->retrieve(i)), as->lastalert(),
				   fs->lastfix(), IOTF(noteclear));
#endif
		
		// Make sure the error is less than MAXRETURN.  We do
		// this after the initial if > 0 to generate the above
		// debugging, and we do this at all because the
		// scheduler is sometimes reading garbage return
		// values (eg: 1008095188).
	      
		if(cs->returncode() <= MODEXEC_MAXRETURN)
		{	    
		  // Determine if we should schedule this alert by
		  // determining whom to notify.  schedule_alert will
		  // actually handle the scheduling of the alert (as
		  // opposed to schedule_check, which just determines
		  // if the check should be scheduled) because of the
		  // intricacies of the AlertPlan design.
		  
		  if(schedule_alert(ap, c, g, g->retrieve(i), as, cs, fs)
		     && r > 0)
		    r--;
		}
		else
		  wlog->warn((char *)"Scheduler::run_alert received return code %d, which exceeds maximum value of %d",
			     cs->returncode(), MODEXEC_MAXRETURN);
	      }
	    
	      if(noteclear && !checkpending && !dontclear)
	      {
		// Now we clean out the state.  Note that if the alert was
		// not successfully queued and it was a noteclear alert, we
		// clear out the state anyway, since it is more important
		// that the scheduler have accurate state information than
		// that the up alert be transmitted.
		
		// If there is a pending check (ie: from a fix), don't
		// clean out the state yet.
				
		as->clearstate();
		fs->clearstate();
	      }
	    }
#if defined(DEBUG)
	    else
	    {
	      dlog->log_progress(DEBUG_SCHEDS,
				 (char *)"Scheduler::run_alert did not find AlertPlan for Check '%s' or host '%s'",
				 IONULL(c->name()), IONULL(g->retrieve(i)));
	    }
#endif	      
	  }
	  else
	    wlog->warn((char *)"Scheduler::run_alert failed to allocate AlertState and FixState");

	  // On error, one or both of these may be undefined
	  as = scache->release(as);
	  fs = scache->release(fs);
	}
	// don't bother generating a warning for MODEXEC_DEPEND
	  
	cs = scache->release(cs);
      }
      else
	wlog->warn((char *)"Scheduler::run_alert failed to allocate CheckState");

      // If we've reached the throttle limit, break.
      
     if(r == 0)
	break;
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, (char *)"Scheduler::run_alert = %d", r);
#endif

  return(r);
}

void Scheduler::run_check(Check *c, List *g)
{
  // For the Check <c> and the list of hosts <g> that represents the
  // group of hosts with the same name as the Check, determine if the
  // Check should be scheduled.  A Configuration read lock should be
  // obtained before calling this method.

  // Return: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, (char *)"Scheduler::run_check(%d,%d)", c, g);
#endif

  // for each check module
  //  for each group of that name
  //   determine if the check needs to be scheduled, using the check's
  //   schedule if set or the class's schedule if not

  if(c && g)
  {
    // If an "override" schedule is set, we use that schedule, otherwise
    // we use the schedule of the hostclass of which the host is a member.
    // In either case, we look for a lastcheck file in state/host since
    // this permits manual scheduling of checks on a highly granular basis.

    Array<Schedule> *csched = c->check_schedule();

    // Hosts are checked on the schedule defined for their
    // hostclass.  For each host in the group, find its default
    // check schedule as defined by its hostclass.  If the
    // check needs to be scheduled, add it to a list of hosts.
      
    List *tocheck = new List();

    // If smart scheduling is enabled, create a counter hash for
    // schedule_check to use.

    Hashtable<Counter> *ctrs = NULL;

    if(statc)
      ctrs = new Hashtable<Counter>();
      
    if(tocheck && (!statc || ctrs))
    {
      for(int j = 0;j < g->entries();j++)
      {
	HostClass *h = cf->find_class(g->retrieve(j));
	
	if(h)
	{
	  // Use override schedule if we found one
	  
	  Array<Schedule> *dsched = csched;
	  
	  if(!dsched)
	    dsched = h->check_schedule();
	  
	  if(dsched)
	  {
	    time_t last = 0;
	    
#if defined(DEBUG)
	    dlog->log_progress(DEBUG_SCHEDS,
			       (char *)"Checking %s for %s using %s schedule",
			       c->name(), g->retrieve(j), dsched->name());
	    /* XXX The methods that return char * (c->name()
	       and dsched->name(), but also c->helpfile())
	       cause ctrl-c not to properly kill all threads
	       on exit when run -d 512

	       Is this still the case with object oriented debugging?
	    */
#endif
	      
	    // Obtain the last time the check was run for this host
	    CheckState *cs = scache->get_check_state(c, g->retrieve(j));
	      
	    if(cs)
	    {
	      if(schedule_check(c, g->retrieve(j), dsched, cs, ctrs))
	      {
#if defined(DEBUG)
		dlog->log_progress(DEBUG_SCHEDS,
				   (char *)"-- Scheduling check %s for %s",
				   c->name(), g->retrieve(j));
#endif

		tocheck->add(g->retrieve(j));
	      }

	      cs = scache->release(cs);
	    }
	  }
	}
#if defined(DEBUG)
	else
	  dlog->log_progress(DEBUG_SCHEDS, (char *)"Unable to find schedule for %s",
			     g->retrieve(j));
#endif
      }
	
      // If there are any entries in tocheck, queue the check.
      // The Queue object will toss tocheck when it no longer
      // needs it.
      
      if(tocheck->entries() > 0)
      {
	if(pc->queue_check(c->name(), tocheck))
	  tocheck = NULL;
	else
	  wlog->warn((char *)"Failed to queue check '%s' (check may already be scheduled)",
		     c->name());
      }
      
      // If tocheck is still defined at this point, that means
      // something went wrong (or there were no hosts to check).
    }
    else
      wlog->warn((char *)"Unable to allocate tocheck List and Counter hash");
    
    xdelete(tocheck);
    xhdelete(ctrs, Counter);
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, (char *)"Scheduler::run_check()");
#endif
}

void Scheduler::run_loop(bool alert, bool check)
{
  // Loop for Check or Alert Scheduler.  Since the skeleton of the loop is
  // the same, both begin()s just call this, with <alert> true if running
  // for an Alert Scheduler, or <check> true if running for a Check
  // Scheduler.

  // Returns: Nothing.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, (char *)"Scheduler::run_loop(%s,%s)",
		  IOTF(alert), IOTF(check));
#endif

  while(!exit)
  {
    struct timeval begin, end;
    
    if(gettimeofday(&begin, NULL)==0)
    {
      // Note that we are still running.  We don't need to do this protected
      // by a lock since we're the only ones writing changes, and the reads
      // are just looking at file mod times.

      RunningState *runstate = new RunningState();

      if(runstate)
      {
	bool x = false;
	
	if(alert)
	  x = runstate->note((char *)"alert");
	else
	{
	  if(check)
	    x = runstate->note((char *)"check");
	}

	if(!x)
	  wlog->warn((char *)"Scheduler::run_loop failed to update running state");

	xdelete(runstate);
      }
      else
	wlog->warn((char *)"Scheduler::run_loop failed to allocate runstate");
      
      // Obtain a read lock on the Configuration
    
      if(locks->cf_read())
      {
	// This is the array of all checks
	Array<Check> *a = cf->get_all_checks();
	
	if(a)
	{
	  int throttle = -1;

	  if(alert && cf->alert_throttle() > 0)
	    throttle = cf->alert_throttle();

	  for(int i = 0;i < a->entries();i++)
	  {
	    // This is the check currently being examined
	    Check *c = a->retrieve(i);
	  
	    if(c)
	    {
	      // Look for a group with the same name as this check,
	      // which contains the hosts to be checked.  If no such
	      // group is found, then there will be no hosts to check,
	      // and so no scheduling need be done, unless the check
	      // is for all hosts.

	      List *g = cf->find_group(c->name());
	      
	      if(g)
	      {	      
		if(alert)
		  throttle = run_alert(c, g, throttle);
		else
		{
		  if(check)
		    run_check(c, g);
		}
	      }
#if defined(DEBUG)
	      else
		dlog->log_progress(DEBUG_SCHEDS, (char *)"No group found for %s",
				   c->name());
#endif
	    }
	    
	    if(throttle == 0)
	    {
	      wlog->warn((char *)"Scheduler::run_loop has reached alert throttle value (%d)",
			 cf->alert_throttle());
	      wlog->warn((char *)"Alert queuing will resume in about a minute");
	      break;
	    }
	  }
	}
	
	// Release the Configuration lock
	
	locks->cf_unlock();
      }
      else
	wlog->warn((char *)"Failed to obtain cf read lock");

      if(gettimeofday(&end, NULL)==0)
      {
	int diff = end.tv_sec - begin.tv_sec;

	if(diff < 60 && !exit)
	  sleep(60 - diff);  // Wait for the rest of the minute to finish up
      }
      else
      {
	// If gettimeofday fails (which shouldn't ever happen), we'll just
	// sleep for a minute, since we'll catch up with all appropriate
	// checks the next time we run through.

	if(!exit)
	  sleep(60);
      }
    }
    else
    {
      // If gettimeofday fails here, we'll display an error and sleep for a
      // minute before trying again.

      wlog->warn((char *)"gettimeofday failed");

      if(!exit)
	sleep(60);
    }
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, (char *)"Scheduler::run_loop()");
#endif
}

bool Scheduler::schedule_alert(AlertPlan *ap, Check *c, List *g, char *host,
			       AlertState *as, CheckState *cs, FixState *fs)
{
  // Determine if an alert needs to be scheduled, based on the
  // AlertPlan <ap>, the Check <c>, the group of hosts <g>, the host
  // <host>, the AlertState <as>, the CheckState <cs>, and the
  // FixState <fs>, and schedule any appropriate alerts.

  // Returns: true if an alert was queued, false otherwise, including
  // if an alert should have been scheduled but failed to queue.

  bool r = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC,
		  (char *)"Scheduler::schedule_alert(%d,%d,%d,%s,%d,%d,%d)",
		  ap, c, g, IONULL(host), as, cs, fs);
#endif

  if(ap && c && g && host && as && cs && fs)
  {
    // Don't bother doing anything if the problem is acknowledged or
    // if a noalert file exists.

    if(!as->quiet())
    {
#if defined(DEBUG)
      dlog->log_progress(DEBUG_SCHEDS, (char *)"-- AlertState is not quiet");
#endif

      if(cs->returncode() == 0)
      {
	// If notify on clear is in effect and the returncode is 0,
	// we look up who and how we last notified and notify them again.
	// These recipients may no longer be on call, but they were last
	// told about the problem, so they are the logical recipients of
	// the resolution notification.

	if(ap->notify_on_clear())
	{
	  // The AlertPlan will also verify global notify on clear status.
	  // If notify on clear is in effect, we don't verify anything
	  // else (number of alerts, other schedules, etc) except that
	  // there are addresses to notify.
	  
#if defined(DEBUG)
	  dlog->log_progress(DEBUG_SCHEDS, (char *)"-- Notify on clear is in effect");
#endif
	  
	  // Lookup previous recipients and queue an alert for them
	  // (Use as->alertfor() to find out the previous rc of the check
	  // if needed.)

	  RecipientSet *rset = as->lastnotify();

	  if(rset && rset->modules() > 0)
	  {
	    // We can't pass this rset directly, since after we queue the
	    // Alert it may be deleted.  Instead, we duplicate it and
	    // pass that one instead.

	    RecipientSet *rsetdupe = new RecipientSet(rset);

	    if(rsetdupe)
	    {
	      // Because information related to the Alert may change after
	      // the alert is queued, all pertinent information should be
	      // provided here, including handoff of control of the
	      // RecipientSet.

	      // This code is pretty similar to the general version, below.
	      
	      Alert *a = new Alert(rsetdupe, cs->lastcheck(), host,
				   c->helpfile(), args->instname(),
				   cs->consecutive(), cs->returncode(),
				   c->name(), cs->comment(), as->token(),
				   false);

	      if(a)
	      {
		if(!pc->queue_alert(a))
		{
		  wlog->warn((char *)"Scheduler::schedule_alert failed to queue alert for host '%s'",
			     host);

		  xdelete(a);
		}
		else
		  r = true;
	      }
	      else
	      {
		wlog->warn((char *)"Scheduler::schedule_alert failed to allocate Alert");

		xdelete(rsetdupe);
	      }
	    }
	    else
	      wlog->warn((char *)"Scheduler::schedule_alert failed to allocate RecipientSet");
	  }
	  // else nobody to notify, just move on
	}
#if defined(DEBUG)
	else
	  dlog->log_progress(DEBUG_SCHEDS,
			     (char *)"-- Notify on clear is not in effect");
#endif
      }
      else
      {
	// General alert notification mechanism

	int ashift = (-1 * cf->alert_shift());
	AlertReturnGroup *arg = ap->match(cs->returncode(), true,
					  cs->consecutive());

	if(arg && arg->schedules())
	{
	  // We now have a set of Schedules.  Iterate through them,
	  // stopping at the first one in effect.  Even if we have no
	  // matching Try stanzas, the first Schedule found in effect
	  // is always the one used.

	  bool found = false;

	  for(int i = 0;i < arg->schedules()->entries();i++)
	  {
	    AlertSchedule *a = arg->schedules()->retrieve(i);

	    if(a && a->now())
	    {
#if defined(DEBUG)
	      dlog->log_progress(DEBUG_SCHEDS,
				 (char *)"-- AlertSchedule entry %d is now in effect",
				 i);
#endif
	      // Next, see if it is time to schedule an(other) alert

	      time_t lasttry = as->lastalert();

	      if(fs->lastfix() > lasttry)
		lasttry = fs->lastfix();
	      
	      if(a->now(lasttry))
	      {
#if defined(DEBUG)
		dlog->log_progress(DEBUG_SCHEDS,
				   (char *)"-- AlertSchedule interval has been reached");
#endif
	      
		// We stop on the first schedule found to be in effect.
		// a->now() will handle the special cases of "at"
		// schedules.
		
		found = true;

		// Now that it's time to do something, determine what
		// the next action to take is.

		int alertnum = as->alerts() + fs->fix_attempts();

		if(as->escalated_manual() > alertnum)
		  alertnum = as->escalated_manual();

		AlertTry *at = a->alerttry(alertnum + 1,
					   c->fix() ? true : false);

		if(at && verify_degraded(at->degrade(), arg->failures(),
					 c, g, host, at->degrade_schedule()))
		{
		  // Figure out whom to notify.  In order to do that,
		  // iterate through each CallList and determine who
		  // on each list to notify, and how.  If we are
		  // executing a Fix, we may end up with no recipients.
		  // If we are executing an Alert, we must have at least
		  // one.

		  RecipientSet *rset = new RecipientSet();
		  
		  if(rset)
		  {
		    Array<CallList> *acls = at->notify();
		    
		    if(acls)
		    {
		      // Since CallLists may be modified by multiple
		      // threads simultaneously, they must be
		      // protected.  Do it once here, rather than
		      // one for each CallList.
		      
		      if(locks->cl_lock())
		      {			
			for(int i = 0;i < acls->entries();i++)
			{
			  CallList *cl = acls->retrieve(i);
			  
			  if(cl && cl->module() && cl->module()->name())
			  {
			    // Strictly speaking, call list rotation
			    // should happen after the alert is
			    // queued, since it is possible that the
			    // alert will not be queued and so (for
			    // simple lists) no rotation should
			    // happen.  But since more than one
			    // calllist may be notified via this
			    // mechanism, it is much simpler to do so
			    // here.
#if defined(DEBUG)
			    dlog->log_progress(DEBUG_SCHEDS,
					       (char *)"-- Using CallList %s",
					       cl->name());
#endif
			    CallListState *cls = new CallListState(cl);
			    
			    if(cls)
			    {
			      char *n = NULL;    // The address to notify
			      Person *np = NULL; // The Person to notify
			      char *ln = NULL;   // The name of the Person
			                         // last notified
			      Person *oc = NULL; // Person on call
			      
			      // Determine whom to notify, and add them
			      // to the recipient set.  For non-broadcast
			      // lists, if the address to notify is not
			      // the same as the last notified, update
			      // the CallListState.  For rotating lists,
			      // also update the last rotate time.

			      if(cl->listtype() != broadcast_list)
				ln = cls->lastnotify();
			      
			      switch(cl->listtype())
			      {
			      case broadcast_list:
				// Broadcast call lists do the Person to
				// address translation for us since no
				// state needs to be written
				n = cl->notify();
				break;
			      case simple_list:
				np = cl->notify(ln);
				if(np)
				{
				  n = np->find_address(cl->module()->name());
				  
				  if(n && (!ln || strcmp(n,ln)!=0))
				    cls->notenotify(np->name(),
						    cl->module()->name(),
						    n);
				}
				break;
			      case rotating_list:
				// First, figure out who is oncall,
				// ignoring substitutions.
				oc = cl->notify(cls->oncall(),
						cls->lastrotate(),
						false);
				// Next, figure out who to notify
				// (including substitutions)
				np = cl->notify(cls->oncall(),
						cls->lastrotate(),
						true);

				if(oc && np)
				{
				  n = np->find_address(cl->module()->name());
				  
				  if(n && (!ln || strcmp(n,ln)!=0))
				    cls->notenotify(np->name(),
						    cl->module()->name(),
						    n,
						    oc->name());
				}
				break;
			      default:
				wlog->warn((char *)"Unknown calllist type %s received by Scheduler::schedule_alert",
					   cl->listtype());
				break;
			      }
			      
			      if(n)
			      {
				// Store the address for later
#if defined(DEBUG)
				dlog->log_progress(DEBUG_SCHEDS,
						   (char *)"-- Notifying %s via %s for %s",
						   n, cl->module()->name(),
						   c->name());
#endif
				rset->add(n, cl->name(), cl->module()->name());
			      }

			      xdelete(cls);
			    }
			    else
			      wlog->warn((char *)"Scheduler::schedule_alert failed to retrieve CallList '%s'",
					 cl->name());
			  }
			}
			      
			locks->cl_unlock();
		      }
		      else
			wlog->warn((char *)"Scheduler::schedule_alert failed to obtain CallList lock");
		    }

		    if(rset->modules() > 0 || at->fix() == always_fix ||
		       (at->fix() == if_defined_fix && c->fix()))
		    {
		      // There is at least one address to notify, so
		      // schedule the alert.  We do this here because
		      // all the information we need is present, so it
		      // is simpler than passing everything back to
		      // run_alert.
		      
		      // Because the information related to the
		      // Alert may change after the alert is queued,
		      // all pertinent information should be
		      // provided here (including handoff of control
		      // of the RecipientSet).
		      
		      // This code is pretty similar to the notify on
		      // clear version, above.

		      // If a fix was requested but none defined, we
		      // queue it anyway because we need to escalate
		      // (for always_fix).  Fixes can be queued
		      // without recipients.
		      
		      Alert *al = new Alert(rset, cs->lastcheck(),
					    host, c->helpfile(),
					    args->instname(),
					    cs->consecutive(),
					    cs->returncode(), c->name(),
					    cs->comment(), as->token(),
					    (at->fix() == always_fix ||
					     (at->fix() == if_defined_fix
					      && c->fix())));
		      
		      if(al)
		      {
			if(!pc->queue_alert(al))
			{
			  wlog->warn((char *)"Scheduler::schedule_alert failed to queue alert for host '%s'",
				     host);
			  
			  xdelete(al);
			}
			else
			  r = true;
		      }
		      else
		      {
			wlog->warn((char *)"Scheduler::schedule_alert failed to allocate Alert");

			xdelete(rset);
		      }
		    }
#if defined(DEBUG)
		    else
		      dlog->log_progress(DEBUG_SCHEDS,
					 (char *)"-- No useful addresses to notify");
#endif
		  }
		  else
		    wlog->warn((char *)"Scheduler::schedule_alert failed to allocate RecipientSet");
		}
#if defined(DEBUG)
		else
		  dlog->log_progress(DEBUG_SCHEDS,
				     (char *)"-- No matching AlertTry for attempt %d, or degraded mode in effect",
				     alertnum);
#endif
	      }
	    }

	    // Break on first schedule in effect.

	    if(found)
	      break;
	  }
	}
#if defined(DEBUG)
	else
	  dlog->log_progress(DEBUG_SCHEDS,
			     (char *)"-- No matching ReturnGroup/failure threshold found");
#endif
      }
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, (char *)"Scheduler::schedule_alert = %s", IOTF(r));
#endif

  return(r);
}

bool Scheduler::schedule_check(Check *c, char *host, Array<Schedule> *sched,
			       CheckState *cs, Hashtable<Counter> *ctrs)
{
  // Determine if the check <c> should be scheduled for <host>, based on the
  // array of Schedules <sched>, and the current CheckState <cs>.  If
  // smart scheduling is enabled, <ctrs> is used to track scheduling
  // progress.

  // Returns: true if the check should be scheduled, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, (char *)"Scheduler::schedule_check(%d,%s,%d,%d,%d)",
		  c, host, sched, cs, ctrs);
#endif

  if(c && host && sched && cs)
  {
#if defined(DEBUG)
    dlog->log_progress(DEBUG_SCHEDS, (char *)"- Last check at %d", cs->lastcheck());
#endif

    Schedule *matchsched = NULL;
    
    for(int i = 0;i < sched->entries();i++)
    {
      Schedule *s = sched->retrieve(i);

      if(s && s->now(0))
      {
#if defined(DEBUG)
	dlog->log_progress(DEBUG_SCHEDS,
			   (char *)"- Using %s schedule, line %d, for %s",
			   sched->name(), i+1, c->name());
#endif
	if(s->now(cs->lastcheck(), 0))
	{
	  matchsched = s;
	  ret = true;
	}

	// Break on first schedule in effect, unless we didn't run and s
	// is an "at" schedule

	if(ret || !s->at())
	  break;
      }
      // Else this line of this schedule is not in effect
    }

    if(ret)
    {
      // The schedule is in effect, check some other stuff before
      // returning true.

      if(ctrs && statc && matchsched->frequency() > -1 &&
	 (statc->cf_hosts_checked_on_schedule(sched->name(), c->name()) >=
	  STAGGERSCHED_HOST_MINIMUM))
      {
	// Staggered Scheduling.
	
	// Check to see if there are at least
	// STAGGERSCHED_HOST_MINIMUM hosts in <sched>.  If not, or if
	// cs->returncode != OK, or if an at schedule is in effect,
	// don't override.

	// Otherwise, if we have reached the maximum number of
	// hosts, override.

	Counter *ctr = ctrs->retrieve(sched->name());

	if(ctr)
	{
	  // If haven't scheduled the limit yet, we're good to go
	  
	  if(ctr->value() < ctr->maximum())
	    ctr->increment();
	  else
	  {
	    // We've hit the limit, don't schedule unless failed

	    if(cs->returncode() == MODEXEC_OK)
	    {
#if defined(DEBUG)
	      dlog->log_progress(DEBUG_SCHEDS,
				 (char *)"-- NOT scheduling check %s for %s (stagger limit reached)",
				 c->name(), host);
#endif	      
	      ret = false;
	    }
	  }
	}
	else
	{
	  // First encounter for this schedule, so allocate a Counter
	  // for it.  We also compute how many hosts may be checked at
	  // once, which is spread across the interval divided by the
	  // timeout.

	  int max = statc->cf_hosts_checked_on_schedule(sched->name(),
							c->name())
	    / (matchsched->frequency() / c->timeout());

	  ctr = new Counter(sched->name(), 1, 0, max);

	  if(ctr)
	  {
	    if(!ctrs->insert(ctr->name(), ctr))
	      xdelete(ctr);
	  }
	  else
	    wlog->warn((char *)"Scheduler::schedule_check failed to allocate Counter");
	}
      }
	    
     // Verify Type I dependencies before queueing
	      
      if(ret && !verify_type1_dependencies(c, host))
      {
#if defined(DEBUG)
	dlog->log_progress(DEBUG_SCHEDS,
			   (char *)"-- NOT scheduling check %s for %s (outstanding dependency)",
			   c->name(), host);
#endif

	ret = false;
      }
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, (char *)"Scheduler::schedule_check = %s", IOTF(ret));
#endif
  
  return(ret);
}

bool Scheduler::verify_degraded(int hostfail, int checkfail, Check *c,
				List *g, char *host, Array<Schedule> *sched)
{
  // Determine if there are at least <hostfail> failed hosts of the
  // set of hosts <g> and the check <c>, not counting <host> which is
  // assumed to have failed.  <checkfail> indicates the number of
  // times the check must have failed on each host.  If <hostfail> is
  // less than 1, this method will always return true.  If <sched> is
  // provided and no Schedule in <sched> is currently in effect, this
  // method will always return true.

  // Returns: true if there are at least <hostfail> failed hosts,
  // false otherwise.

  bool r = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC,
		  (char *)"Scheduler::verify_degraded(%d,%d,%d,%d,%s,%d)",
		  hostfail, checkfail, c, g, IONULL(host), sched);
#endif

  if(hostfail > 0 && checkfail > 0 && c && g && host)
  {
    // If sched is provided check to see if any schedule is in effect.

    bool schedok = false;

    if(sched)
    {
      // Only allow degraded if schedule is in effect
      
      for(int i = 0;i < sched->entries();i++)
      {
	Schedule *s = sched->retrieve(i);

	if(s && s->now(0))
	{
	  // We're just looking for any schedule currently in effect

#if defined(DEBUG)
	  dlog->log_progress(DEBUG_SCHEDS,
			     (char *)"- Degraded mode in effect by schedule %s, line %d",
			     sched->name(), i+1);
#endif
	  
	  schedok = true;
	  break;
	}
      }
    }
    else
    {
      // No schedule provided, so always allow degraded
      
      schedok = true;
    }

    if(schedok)
    {
      int found = 0;
    
      for(int i = 0;i < g->entries();i++)
      {
	// We could actually just compare the pointers rather than strings
	// since host comes from the same array, but that will just manifest
	// itself as a bug sometime in the future.
      
	if(strcmp(g->retrieve(i), host)!=0)
	{
	  CheckState *cs = scache->get_check_state(c, g->retrieve(i));
	  
	  if(cs)
	  {
	    if(cs->returncode() > 0 && cs->consecutive() >= checkfail)
	    {
	      // This host must be on the same AlertPlan as <host>
	      
	      if(c->alert_plan())   // Check define AlertPlan, must be the same
		found++;
	      else
	      {
		HostClass *hc1 = cf->find_class(host);
		HostClass *hc2 = cf->find_class(g->retrieve(i));
		
		// We could just compare hc1 and hc2, but that will probably
		// just manifest itself as a bug in the future.
		
		if(hc1 && hc2 && hc1->alert_plan() && hc2->alert_plan()
		   && strcmp(hc1->alert_plan()->name(),
			     hc2->alert_plan()->name())==0)
		  found++;
	      }
	    }
	    
	    cs = scache->release(cs);
	  }
	}
	
	if(found >= hostfail)
	{
	  r = true;
	  break;
	}
      }
    }
    else
    {
      // Provided schedules are not in effect, so no degraded mode.
      // (ie: Always alert.)
      
      r = true;
    }
  }
  else
    r = true;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, (char *)"Scheduler::verify_degraded = %s", IOTF(r));
#endif
  
  return(r);
}

bool Scheduler::verify_type1_dependencies(Check *c, char *host)
{
  // Determine if there are any outstanding failed dependencies for
  // the Check <c> on <host>.  If so, this Check should not be scheduled.
  // A Configuration read lock should be obtained before calling this
  // method.

  // Returns: false if any outstanding failed dependencies are found,
  // true otherwise.

  bool r = true;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, (char *)"Scheduler::verify_type1_dependencies(%d,%s)",
		  c, IONULL(host));
#endif

  if(c && host)
  {
    Array<Dependency> *da = c->dependencies();
    List *hostgroups = cf->find_groups(host);

    if(da && hostgroups)
    {
      for(int i = 0;i < da->entries();i++)
      {
	Dependency *d = da->retrieve(i);

	// A host must be a member of the group that the dependency
	// checks.  If it isn't, we keep going down the list (in the
	// rare case the host will be a member of a group further on).
	// This check prevents spurious warnings about being unable to
	// open state files.  Additionally, the dependency is only run
	// if this host is dependent on it.

	// We check here to make sure <c> isn't dependent on itself,
	// since Type I Dependencies can stuck that way.  (We could
	// do this by comparing the d->dependency directly against c,
	// but murphy's law says that would change at some point,
	// breaking the test.)
	
	if(d && d->dep_type() == 1 && d->dependent(host)
	   && d->dependency()->name()
	   && (strcmp(c->name(), d->dependency()->name()) != 0)
	   && (hostgroups->find(d->dependency()->name()) != -1))
	{
	  // The Dependency may specify another host.  The Configuration
	  // parser should already have determined that host is a member
	  // of the appropriate group, so state should exist for it.
	  
	  CheckState *cs = scache->get_check_state(d->dependency(),
						   (d->target() ?
						    d->target() : host));

	  if(cs)
	  {
	    if(cs->returncode() != MODEXEC_OK)
	    {
#if defined(DEBUG)
	      dlog->log_progress(DEBUG_SCHEDS,
				 (char *)"-- %s is dependent on %s, which has an error status of %d",
				 c->name(),
				 d->dependency()->name(),
				 cs->returncode());
#endif
	      
	      // Attempt to log this failure, but ignore errors.  We
	      // need to release our CheckState read lock in order to
	      // obtain a write lock.

	      int crc = cs->returncode();

	      cs = scache->release(cs);

	      CheckState *cs2 = new CheckState(c);
		
	      if(cs2)
	      {
		// Create a history entry

		Array<CheckResult> *cra = new Array<CheckResult>();
		CharBuffer *cbuf = new CharBuffer();

		if(cra && cbuf)
		{
		  cbuf->append((char *)"Dependency \"");
		  cbuf->append(d->dependency()->name());
		  cbuf->append((char *)"\" has error status ");
		  cbuf->append(crc);
		  
		  CheckResult *cr = new CheckResult(host,
						    MODEXEC_DEPEND,
						    0,
						    cbuf->str());

		  if(cr)
		  {
		    if(cra->add(cr))
		    {
		      if(cs2->parse_results(cra))
		      {
			cra = NULL;  // We don't need to manage it anymore

			cs2->write_results();
		      }
		    }
		    else
		    {
		      xdelete(cr);
		    }
		  }
		  else
		    wlog->warn((char *)"verify_type1_dependencies failed to allocate CheckResult");
		}
		else
		  wlog->warn((char *)"verify_type1_dependencies failed to allocate Array or CharBuffer");

		// If cra is still valid, we need to toss it
		xadelete(cra, CheckResult);
		xdelete(cbuf);
		
		xdelete(cs2);
	      }
	      
	      r = false;
	    }
	    else
	      cs = scache->release(cs);
	  }
	  else
	    wlog->warn((char *)"verify_type1_dependencies failed to allocate CheckState for %s@%s",
		       d->dependency()->name(), host);
	}

	if(!r)
	  break;
      }
    }
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, (char *)"Scheduler::verify_type1_dependencies = %s",
		 IOTF(r));
#endif
  
  return(r);
}
