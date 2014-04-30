/*
 * uiutils.C: UI utilities.
 *
 * Version: $Revision: 0.22 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2006/01/25 00:32:53 $
 * MT-Level: Safe, except ui_calllist_notifies and ui_person_notifies
 *
 * Copyright (c) 2002 - 2006
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: uiutils.C,v $
 * Revision 0.22  2006/01/25 00:32:53  benno
 * Clarify error message
 *
 * Revision 0.21  2005/12/22 04:06:21  benno
 * Add match_stalled to retrieve_matching_state
 *
 * Revision 0.20  2005/11/24 18:26:32  benno
 * Add ui_retrieve_matching_state
 *
 * Revision 0.19  2005/11/14 03:12:57  benno
 * ui_execute_report takes CharHandler
 *
 * Revision 0.18  2005/10/05 01:33:02  benno
 * Return transmit module in _notifies()
 *
 * Revision 0.17  2005/04/09 02:34:14  benno
 * ui_execute_fix uses FixRequests
 *
 * Revision 0.16  2004/11/27 00:50:55  benno
 * Add ui_calllist_notifies, ui_person_notifies
 *
 * Revision 0.15  2004/09/09 14:38:29  benno
 * ui_inhibit should be recorded as inhibit, not acknowledge
 *
 * Revision 0.14  2004/09/09 12:47:47  benno
 * Set nolock when looking at history
 *
 * Revision 0.13  2004/08/25 02:03:18  benno
 * Fix typo
 * Add ui_execute_report
 *
 * Revision 0.12  2004/03/01 22:23:44  benno
 * Add composite_check functions
 * Pass error pointer instead of using throw/catch
 *
 * Revision 0.11  2003/11/29 05:14:13  benno
 * Add who/why to select functions for increased history logging
 *
 * Revision 0.10  2003/06/17 15:18:17  benno
 * Add new version of ui_rc_to_text
 *
 * Revision 0.9  2003/05/04 21:27:04  benno
 * Don't use string type
 *
 * Revision 0.8  2003/04/21 20:30:16  benno
 * Add ui_unquiet_resched
 *
 * Revision 0.7  2003/04/09 20:00:23  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.6  2003/04/07 15:17:22  benno
 * Use debugger
 *
 * Revision 0.5  2003/03/06 02:38:34  benno
 * Fix escalation computation bug
 *
 * Revision 0.4  2003/03/04 17:51:49  benno
 * Add ui_acknowledge
 *
 * Revision 0.3  2003/01/26 15:11:21  benno
 * Use IONULL, IOTF
 *
 * Revision 0.2  2003/01/02 04:43:58  benno
 * Add ui_execute_fix
 *
 * Revision 0.1  2002/12/31 04:24:50  benno
 * Initial revision
 *
 */

#include "survivor.H"

bool ui_acknowledge(Check *check, char *host, char *who, char *why,
		    char **err)
{
  // Acknowledge the problem for <check> on <host> by <who> with the
  // excuse <why>.  On error, <err> will contain a newly allocated
  // string containing the error and that should be deleted when no
  // longer required.

  // Returns: true if fully successful, false otherwise.
  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "ui_acknowledge(%d,%s,%s,%s,%d)",
		  check, IONULL(host), IONULL(who), IONULL(why), err);
#endif

  if(check && host && who && why && err)
  {
    AlertState *as = new AlertState(check, host);
    CheckState *cs = new CheckState(check, host);

    if(as && cs)
    {
      if(cs->returncode() != MODEXEC_OK)
      {
	if(!as->inhibited())
	{
	  if(!as->acknowledged())
	  {
	    if(as->acknowledge(cs, who, why))
	    {
	      ui_record_command(check, host, "acknowledge", who, why);
	      ret = true;
	    }
	    else
	      *err = xstrdup("Acknowledge failed");
	  }
	  else
	  {
	    *err = xstrdup("Alert already acknowledged by ");
	    *err = xstrcat(*err, as->acknowledged());
	  }
	}
	else
	{
	  *err = xstrdup("Alert already inhibited by ");
	  *err = xstrcat(*err, as->inhibited());
	}
      }
      else
	*err = xstrdup("No outstanding error to acknowledge");
    }
    else
      wlog->warn("ui_acknowledge failed to allocate AlertState or CheckState");

    xdelete(as);
    xdelete(cs);
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "ui_acknowledge = %s", IOTF(ret));
#endif
  
  return(ret);
}

List *ui_calllist_notifies(char *instance, char *calllist, char **tmod,
			   char **err)
{
  // Obtain the active addresses to notify for the <calllist> within
  // <instance>.  <tmod> will contain a newly allocated string holding
  // the name of the transmit module to use.  On error, <err> will
  // contain a descriptive error string.  Both <via> and <err> should
  // be deleted when no longer required.
  // IMPORTANT: This function will temporarily modify the global <args>
  // variable while it is executing, therefore this function is not
  // MT-Safe.

  // Returns: A newly allocated List that should be deleted when no
  // longer required and contains the destination addresses on
  // success, or NULL on error.

  List *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "ui_calllist_notifies(%s,%s,%d,%d)",
		  IONULL(instance), IONULL(calllist), tmod, err);
#endif

  if(instance && calllist && err && tmod)
  {
    // Store existing args for later restoral
    Args *oldargs = args;

    args = new Args();

    if(args)
    {
      args->parse_args(instance, (char *)NULL, (char *)NULL);

      if(args->parse_instcf())
      {
	Configuration *lcf = new Configuration();
	
	if(lcf && lcf->parse_cfs())
	{
	  CallList *cl = lcf->find_calllist(calllist);

	  if(cl)
	  {
	    if(cl->module() && cl->module()->name())
	    {
	      // While we have the configuration parsed, determine the
	      // transmit module used by cl->module().

	      AlertModule *am = lcf->find_alertmodule(cl->module()->name());

	      if(am && am->transmit())
	      {
		*tmod = xstrdup(am->transmit());
	      
		// Determine who is on call and use tokenize to build a
		// List, but don't trip rotation

		if(cl->listtype() == broadcast_list)
		  ret = tokenize(cl->notify(), ",");
		else
		{
		  CallListState *cls = new CallListState(cl);
		  
		  if(cls)
		  {
		    Person *np = NULL;
		    
		    // Right now, cl->notify will only return one address
		    // below, but we'll still tokenize it in case that
		    // changes.
		    
 		    if(cl->listtype() == simple_list)
		      np = cl->notify(cls->lastnotify());
		    else // Assume rotating_list
		      np = cl->notify(cls->oncall(), cls->lastrotate(), true);

		    if(np)
		      ret = tokenize(np->find_address(cl->module()->name()),
				     ",");

		    xdelete(cls);
		  }
		  else
		    *err = xstrdup("Failed to allocate CallListState");
		}
	      }
	      else
		*err = xstrdup("Alert module not found for CallList");
	    }
	    else
	      *err = xstrdup("CallList module name not found");
	  }
	  else
	    *err = xstrdup("Requested CallList not found");
	}
	else
	  *err = xstrdup("Configuration parse failed");
	
	xdelete(lcf);
      }
      else
	*err = xstrdup("Instance configuration parse failed");

      xdelete(args);
    }
    else
      *err = xstrdup("Failed to allocate Args");
    
    // Restore old args structure
    args = oldargs;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "ui_calllist_notifies = %d", ret);
#endif
  
 return(ret);
}

int ui_composite_check_status(Check *check)
{
  // Obtain the composite status for <check>, ie: the composite
  // returncode for all hosts monitored for <check>.

  // Returns: The composite status, or -1 on error.

  int ret = -1;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "ui_composite_check_status(%d)", check);
#endif
  
  if(check)
  {
    // First, find the hosts for this Check

    List *group = cf->find_group(check->name());

    if(group)
    {
      // Iterate through them, composing the return status as we go.
      // We start with MODEXEC_OK.

      bool err = false;
      ret = MODEXEC_OK;
      
      for(int i = 0;i < group->entries();i++)
      {
	CheckState *cs = new CheckState(check, group->retrieve(i));

	if(cs)
	{
	  ret = compose_rc(ret, cs->returncode(), true);
	  xdelete(cs);
	}
	else
	{
	  wlog->warn("ui_composite_check_status failed to allocate CheckState");
	  err = true;
	}
      }

      // On error, prevent an inaccurate status from being returned
      
      if(err)
	ret = -1;
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "ui_composite_check_status = %d", ret);
#endif

  return(ret);
}

int ui_composite_check_status(char *host)
{
  // Obtain the composite status for <host>, ie: the composite
  // returncode for all services monitored for <host>.

  // Returns: The composite status, or -1 on error.

  int ret = -1;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "ui_composite_check_status(%s)", host);
#endif
  
  if(host)
  {
    // First, find the Checks for this host

    List *groups = cf->find_groups(host);

    if(groups)
    {
      // Iterate through them, composing the return status as we go.
      // We start with MODEXEC_OK.

      bool err = false;
      ret = MODEXEC_OK;
      
      for(int i = 0;i < groups->entries();i++)
      {
	Check *c = cf->find_check(groups->retrieve(i));

	if(c)
	{
	  CheckState *cs = new CheckState(c, host);

	  if(cs)
	  {
	    ret = compose_rc(ret, cs->returncode(), true);
	    xdelete(cs);
	  }
	  else
	  {
	    wlog->warn("ui_composite_check_status failed to allocate CheckState");
	    err = true;
	  }
	}
	// else the check may not be defined
      }

      // On error, prevent an inaccurate status from being returned
      
      if(err)
	ret = -1;
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "ui_composite_check_status = %d", ret);
#endif

  return(ret);
}

int ui_composite_check_status(List *hosts)
{
  // Obtain the composite status for <hosts>, ie: the composite
  // returncode for all services monitored for each <host>.

  // Returns: The composite status, or -1 on error.

  int ret = -1;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "ui_composite_check_status(%d)", hosts);
#endif

  if(hosts && hosts->entries() > 0)
  {
    // Iterate through <hosts>, composing as we go

    bool err = false;
    ret = MODEXEC_OK;

    for(int i = 0;i < hosts->entries();i++)
    {
      int rc = ui_composite_check_status(hosts->retrieve(i));

      if(rc > -1)
	ret = compose_rc(ret, rc, true);
      else
	err = false;
    }

    // On error, prevent an inaccurate status from being returned

    if(err)
      ret = -1;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "ui_composite_check_status = %d", ret);
#endif
  
  return(ret);
}

int ui_escalate_to(Check *check, char *host, char *who, char *why, char **err)
{
  // Escalate the error status for the Check <check> on the specified
  // <host> to the next alert level, by <who> for <why>.  If the
  // highest return value in the appropriate AlertPlan is already
  // matched, or if there is no outstanding alert, this method will
  // have no effect.  On error, <err> will contain a newly allocated
  // string containing the error and that should be deleted when no
  // longer required.

  // Returns: The new alert level, or -1 on error.

  int ret = -1;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "ui_escalate_to(%d,%s,%s,%s,%d)",
		  check, IONULL(host), IONULL(who), IONULL(why), err);
#endif
  
  if(check && host && who && err)
  {
    AlertState *as = new AlertState(check, host);

    if(as)
    {
      CheckState *cs = new CheckState(check, host);

      if(cs)
      {
	FixState *fs = new FixState(check, host);

	if(fs)
	{
	  // First, there needs to be an outstanding error

	  if(cs->returncode() != MODEXEC_OK)
	  {
	    // Next, find the current AlertPlan

	    AlertPlan *ap = check->alert_plan();

	    if(!ap)
	    {
	      // Use the AlertPlan for the Hostclass if there is none
	      // for the Check.

	      HostClass *hc = cf->find_class(host);

	      if(hc)
		ap = hc->alert_plan();
	    }

	    if(ap)
	    {
	      // Determine the current number of tries, which is the
	      // greater of as->alerts+fs->fix_attempts and
	      // as->escalated_manual.

	      int escfrom = as->alerts() + fs->fix_attempts();

	      if(as->escalated_manual() > escfrom)
		escfrom = as->escalated_manual();

	      int escto = escfrom;

	      // Retrieve the current AlertSchedules.

	      AlertReturnGroup *arg = ap->match(cs->returncode(), true,
						cs->consecutive());

	      if(arg && arg->schedules())
	      {
		// Find the current AlertSchedule

		for(int i = 0;i < arg->schedules()->entries();i++)
		{
		  AlertSchedule *a = arg->schedules()->retrieve(i);

		  if(a && a->now())
		  {
		    // Ask the AlertSchedule for the next level of escalation

		    escto = a->escalate_next(escfrom);
		    break;
		  }
		}
	      }

	      if(escto > escfrom)
	      {
		if(as->escalate_manual(escto))
		{
		  ui_record_command(check, host, "escalate", who, why);
		  ret = escto;
		}
		else
		  *err = xstrdup("Escalate FAILED");
	      }
	      else
		*err = xstrdup("Already at maximum escalation level");
	    }
	    else
	      *err = xstrdup("No AlertPlan found");
	  }
	  else
	    *err = xstrdup("No outstanding error");
	  
	  xdelete(fs);
	}
	else
	  wlog->warn("ui_escalate_to failed to allocate FixState");
	
	xdelete(cs);
      }
      else
	wlog->warn("ui_escalate_to failed to allocate CheckState");
      
      xdelete(as);
    }
    else
      wlog->warn("ui_escalate_to failed to allocate AlertState");
  }
  else
    *err = xstrdup("Bad arguments");

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "ui_escalate_to = %d", ret);
#endif

  return(ret);
}

int ui_execute_fix(Check *check, char *host, CheckState *cs,
		   FixState *fs, char *who, char *why, char **err)
{
  // Attempt to execute the fix for <check>@<host> using the
  // previously allocated <cs> and <fs> for <check>@<host>..
  // Attribute the fix attempt to <who>, with optional comment <why>.
  // On error, <err> will contain a newly allocated string containing
  // the error and that should be deleted when no longer required.

  // Returns: The return code of the fix attempt.

  int ret = MODEXEC_MISCONFIG;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "ui_execute_fix(%d,%s,%d,%d,%s,%s)",
		   check, IONULL(host), cs, fs, IONULL(who), IONULL(why));
#endif

  if(check && host && cs && fs && who)
  {
    // There must be a Fix defined, or we have nothing to do

    if(check->fix())
    {
      // Obtain a fix lock.  This might fail if a fix is already running.
      // If so, we abort.
      
      if(fs->lock_fix(check->fix()->lock_type(),
		      check->fix()->lock_timeout()))
      {
	// Execute the Fix
	
	Executor *e = new Executor();
	FixRequest *fr = new FixRequest(check->fix()->modargs(), host,
					check->fix()->fix_timeout());
	
	if(e && fr)
	{
	  if(e->exec_fix(check, fr) > -1)
	  {
	    if(e->result(fs) > -1)
	    {
	      // Unlike other state, we write FixState since all fixes
	      // are recorded, not just those run by the scheduler.
	      
	      fs->write_results(who);
	      
	      ret = fs->returncode();

	      // We also write a command history record, even though
	      // the fix history will have the same information.

	      ui_record_command(check, host, "fix", who, why);
	      
	      // If there was an outstanding error, we reschedule
	      // the check.
	      
	      // Don't throw an error since we were mostly successful
		  
	      if(cs->returncode() != MODEXEC_OK)
		cs->reschedule();
	    }
	    else
	      *err = xstrdup("Error reading fix result");
	  }
	  else
	    *err = xstrdup("Error exec'ing fix");
	}
	else
	  *err = xstrdup("ui_execute_fix failed to allocate Executor or FixRequest");
		  
	xdelete(e);
	xdelete(fr);
	
	fs->unlock_fix();
      }
      else
	*err = xstrdup("Unable to establish lock, the fix may already be running");
    }
    else
      *err = xstrdup("No fix is defined for this service");
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "ui_execute_fix = %d", ret);
#endif

  return(ret);
}

bool ui_execute_report(CharHandler *chout, ReportFormatting *rf, List *hosts,
		       List *checks, int which, time_t from, time_t until,
		       bool backwards, char **err)
{
  // Execute the report described in <rf> for each of the <checks>
  // defined on each of the <hosts> for the histories of types <which>
  // (or'd together UI_REPORT_* values) for the time period <from>
  // until <until> (0 specifies beginning or end, respectively).  If
  // <backwards> is true, history is read from most recent first.
  // Output from the report module will be written to <chout>.  On
  // error, <err> will contain a newly allocated string containing the
  // error and that should be deleted when no longer required.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC,
		  "ui_execute_report(%d,%d,%d,%d,%d,%d,%d,%s,%d)",
		  chout, rf, hosts, checks, which, from, until,
		  IOTF(backwards), err);
#endif

  if(chout && rf && hosts && checks)
  {
    Executor *e = new Executor();

    if(e)
    {
      pid_t pid = e->exec_report_begin(rf);

      if(pid > -1)
      {
	for(int i = 0;i < hosts->entries();i++)
	{
	  // For each host

	  for(int j = 0;j < checks->entries();j++)
	  {
	    // For each service
	    
	    // We don't bother seeing if host@service is defined since
	    // we're looking at history.  We'll set nolock() to
	    // prevent warnings about the directory not existing, and
	    // also because the web interface shouldn't be able to
	    // delay the scheduler.

	    HistorySet *hs = new HistorySet(hosts->retrieve(i),
					    checks->retrieve(j));

	    if(hs)
	    {
	      // We're a little lax with errors here since it's hard to
	      // report them correctly and it isn't too important if
	      // we don't
	      
	      if(which & UI_REPORT_ALERT)
	      {
		AlertHistory *ah = new AlertHistory(checks->retrieve(j),
						    hosts->retrieve(i));

		if(ah)
		{
		  ah->nolock();
		  
		  if((backwards && !ah->iterate_begin_backwards(from, until))
		     ||
		     (!backwards && !ah->iterate_begin(from, until))
		     ||
		     !hs->attach_history(ah, backwards))
		  {
		    xdelete(ah);
		  }
		}
	      }
	      
	      if(which & UI_REPORT_CHECK)
	      {
		CheckHistory *ch = new CheckHistory(checks->retrieve(j),
						    hosts->retrieve(i));

		if(ch)
		{
		  ch->nolock();
		  
		  if((backwards && !ch->iterate_begin_backwards(from, until))
		     ||
		     (!backwards && !ch->iterate_begin(from, until))
		     ||
		     !hs->attach_history(ch, backwards))
		  {
		    xdelete(ch);
		  }
		}
	      }
	      
	      if(which & UI_REPORT_COMMAND)
	      {
		CommandHistory *ch = new CommandHistory(checks->retrieve(j),
							hosts->retrieve(i));

		if(ch)
		{
		  ch->nolock();
		  
		  if((backwards && !ch->iterate_begin_backwards(from, until))
		     ||
		     (!backwards && !ch->iterate_begin(from, until))
		     ||
		     !hs->attach_history(ch, backwards))
		  {
		    xdelete(ch);
		  }
		}
	      }
	      
	      if(which & UI_REPORT_FIX)
	      {
		FixHistory *fh = new FixHistory(checks->retrieve(j),
						hosts->retrieve(i));

		if(fh)
		{
		  fh->nolock();
		  
		  if((backwards && !fh->iterate_begin_backwards(from, until))
		     ||
		     (!backwards && !fh->iterate_begin(from, until))
		     ||
		     !hs->attach_history(fh, backwards))
		  {
		    xdelete(fh);
		  }
		}
	      }
	      
	      e->exec_report_continue(hs);  // should check for success
	      
	      xdelete(hs);
	    }
	    // else we should report an error
	  }
	}
	
	int fdin = e->exec_report_end();

	if(fdin > -1)
	{
	  // Copy to stdout

	  while(char *p = read_line(fdin))
	  {
	    chout->append(p);
	    chout->append('\n');
	    xdelete(p);
	  }
	  
	  ret = true;
	}
      }
      else
	*err = xstrdup("exec_report_begin failed to exec module");
      
      xdelete(e);
    }
    else
      *err = xstrdup("ui_execute_report failed to allocate Executor");
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "ui_execute_report = %s", IOTF(ret));
#endif

  return(ret);
}

bool ui_inhibit(Check *check, char *host, char *who, char *why, char **err)
{
  // Inhibit the problem for <check> on <host> by <who> with the
  // excuse <why>.  On error, <err> will contain a newly allocated
  // string containing the error and that should be deleted when no
  // longer required.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "ui_inhibit(%d,%s,%s,%s,%d)",
		  check, IONULL(host), IONULL(who), IONULL(why), err);
#endif

  if(check && host && who && why && err)
  {
    AlertState *as = new AlertState(check, host);

    if(as)
    {
      if(!as->inhibited())
      {
	if(as->inhibit(who, why))
	{
	  ui_record_command(check, host, "inhibit", who, why);
	  ret = true;
	}
	else
	  *err = xstrdup("Inhibit failed");
      }
      else
      {
	*err = xstrdup("Alert already inhibited by ");
	*err = xstrcat(*err, as->inhibited());
      }
    }
    else
      wlog->warn("ui_acknowledge failed to allocate AlertState");

    xdelete(as);
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "ui_inhibit = %s", IOTF(ret));
#endif
  
  return(ret);
}

char *ui_person_notifies(char *instance, char *person, char *via, char **tmod,
			 char **err)
{
  // Obtain the active address to notify for the <person> using
  // transmit module <via> within <instance>.  <tmod> will contain a
  // newly allocated string holding the name of the transmit module to
  // use.  On error, <err> will contain a descriptive error string and
  // should be deleted when no longer required.
  // IMPORTANT: This function will temporarily modify the global
  // <args> variable while it is executing, therefore this function is
  // not MT-Safe.

  // Returns: A newly allocated string that should be deleted when no
  // longer required, or NULL on error.

  char *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "ui_person_notifies(%s,%s,%s,%d,%d)",
		  IONULL(instance), IONULL(person), IONULL(via), tmod, err);
#endif

  if(instance && person && via && tmod && err)
  {
    // Store existing args for later restoral
    Args *oldargs = args;

    args = new Args();

    if(args)
    {
      args->parse_args(instance, (char *)NULL, (char *)NULL);

      if(args->parse_instcf())
      {
	Configuration *lcf = new Configuration();
	
	if(lcf && lcf->parse_cfs())
	{
	  Person *p = lcf->find_person(person);

	  if(p)
	  {
	    // Now find the Alert module <via>

	    AlertModule *am = lcf->find_alertmodule(via);

	    if(am)
	    {
	      ret = xstrdup(p->find_address(via));

	      // Copy the transmit module to use

	      *tmod = xstrdup(am->transmit());
	    }
	    else
	      *err = xstrdup("Alert module not found");
	  }
	  else
	    *err = xstrdup("Person not found");
	}
	else
	  *err = xstrdup("Configuration parse failed");
	
	xdelete(lcf);
      }
      else
	*err = xstrdup("Instance configuration parse failed");

      xdelete(args);
    }
    else
      *err = xstrdup("Failed to allocate Args");
    
    // Restore old args structure
    args = oldargs;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "ui_person_notifies = %d", ret);
#endif
  
 return(ret);
}

char *ui_rc_to_text(int rc)
{
  // Generate a textual description for the return code <rc>.

  // Returns: A description string for known return codes, NULL otherwise.

  char *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "ui_rc_to_text(%d)", rc);
#endif
  
  switch(rc)
  {
  case -1:
    ret = "Exec error";
    break;
  case MODEXEC_OK:
    ret = "OK";
    break;
  case MODEXEC_PROBLEM:
    ret = "PROBLEM";
    break;
  case MODEXEC_WARNING:
    ret = "WARNING";
    break;
  case MODEXEC_NOTICE:
    ret = "NOTICE";
    break;
  case MODEXEC_MISCONFIG:
    ret = "Module is misconfigured";
    break;
  case MODEXEC_TIMEDOUT:
    ret = "TIMED OUT";
    break;
  case MODEXEC_DEPEND:
    ret = "Failed dependency exists";
    break;
  case MODEXEC_NOTYET:
    ret = "Not yet executed";
    break;
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "ui_rc_to_text = %s", IONULL(ret));
#endif
  
  return(ret);
}

char *ui_rc_to_text(int rc, time_t lastcheck, int instances)
{
  // Generate a textual description for the return code <rc>.
  // If <lastcheck> is 0 and <instances> in not, the string will
  // indicate that a reschedule is pending.

  // Returns: A description string for known return codes, NULL otherwise.

  char *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "ui_rc_to_text(%d,%d,%d)", rc, lastcheck,
		  instances);
#endif

  if(lastcheck == 0 && instances > 0)
    ret = "Reschedule request pending";
  else
    ret = ui_rc_to_text(rc);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "ui_rc_to_text = %s", IONULL(ret));
#endif
  
  return(ret);
}

bool ui_record_command(Check *check, char *host, char *command,
		       char *who, char *comment)
{
  // Make a history record for the <command> executed for <check>@<host>
  // by <who>, with <comment>.  If <comment> is NULL, the default
  // excuse will be used.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "ui_record_command(%d,%s,%s,%s,%s)",
		  check, IONULL(host), IONULL(command), IONULL(who),
		  IONULL(comment));
#endif

  if(check && host && command && who)
  {
    CommandHistory *ch = new CommandHistory(check, host);

    if(ch)
    {
      CommandHistoryRecord *chr = new CommandHistoryRecord(command,
							   who,
							   MAYBE_EXCUSE(comment));

      if(chr)
      {
	if(ch->record(chr))
	  ret = true;
	else
	  wlog->warn("ui_record_command failed to record history for %s %s@%s",
		     command, check->name(), host);
	
	xdelete(chr);
      }
      else
	wlog->warn("ui_record_command failed to allocate CommandHistoryRecord");
      
      xdelete(ch);
    }
    else
      wlog->warn("ui_record_command failed to allocate CommandHistory");
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "ui_record_command = %s", IOTF(ret));
#endif
  
  return(ret);
}

List *ui_retrieve_matching_state(matchstate_t match)
{
  // Obtain a List of service@host pairs that match the criteria
  // <match>.

  // Returns: A newly allocated List containing the names of the
  // matching pairs (not data objects) and that should be deleted
  // when no longer required, or NULL on error.

  List *ret = new List();
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "ui_retrieve_matching_state(%d)",
		  match);
#endif

  if(ret)
  {
    // We don't bother caching anything here since we're looking at
    // everything once and it's unlikely that we'll look at it all
    // again.  This may not be a valid assumption under all scenarios,
    // and XXX we could probably improve performance in those scenarios
    // by implementing a cache, similar to the scheduler's.

    // For each host, lookup the state for each service run on that
    // host

    List *hcs = cf->get_all_hostclasses();

    if(hcs)
    {
      for(int i = 0;i < hcs->entries();i++)
      {
	HostClass *hc = cf->find_hostclass(hcs->retrieve(i));
	
	if(hc && hc->hosts())
	{
	  for(int j = 0;j < hc->hosts()->entries();j++)
	  {
	    char *h = hc->hosts()->retrieve(j);
	    
	    List *svcs = cf->find_groups(h);

	    if(svcs)
	    {
	      for(int k = 0;k < svcs->entries();k++)
	      {
		Check *c = cf->find_check(svcs->retrieve(k));

		if(c)
		{
		  bool doadd = false;
		
		  if(match == match_all)
		  {
		    // Simple, just add it to the list
		    
		    doadd = true;
		  }
		  else
		  {
		    // We need some state.  Not everything uses both,
		    // but just allocating the objects is pretty
		    // inexpensive.
		    
		    AlertState *as = new AlertState(c, h);
		    CheckState *cs = new CheckState(c, h);
		    
		    if(as && cs)
		    {
		      switch(match)
		      {
		      case match_addressed:
			if(as->inhibited() || as->acknowledged())
			  doadd = true;
			break;
		      case match_error:
			if(cs->returncode() != MODEXEC_OK)
			  doadd = true;
			break;
		      case match_escalated:
			if(cs->lastcheck() > 0)
			{
			  // Determine which AlertPlan is in effect in order
			  // to determine escalation status.

			  AlertPlan *ap = c->alert_plan();
			  
			  if(!ap && hc)
			    ap = hc->alert_plan();
		      
			  if(ap)
			    doadd = as->escalated(ap, cs);
			}
			break;
		      case match_stalled:
			{
			  // Determine next expected check time.  If it's
			  // more than 30 minutes ago, complain.

			  struct timeval tp;

			  // We are getting the current time each run of the
			  // loop, which is sort of excessive, but correct
			  // anyway.

			  if(gettimeofday(&tp, NULL)==0)
			  {
			    Array<Schedule> *csched = c->check_schedule();

			    if(!csched && hc)
			      csched = hc->check_schedule();
      
			    time_t nt = next_schedule_time(csched,
							   cs->lastcheck());
			    
			    if(nt > 0 && tp.tv_sec - nt > 1800)
			      doadd = true;
			  }
			}
			break;
		      default:
			wlog->warn("ui_retrieve_matching_state found unknown match type '%d'", match);
			break;
		      }
		    }
		    else
		      wlog->warn("ui_retrieve_matching_state failed to allocate State");
		    
		    xdelete(as);
		    xdelete(cs);
		  }
		  
		  if(doadd)
		  {
		    CharBuffer *cb = new CharBuffer(c->name());

		    if(cb)
		    {
		      cb->append('@');
		      cb->append(h);

		      ret->add(cb->str());
		      
		      xdelete(cb);
		    }
		  }
		}
		// no warning, the service may not exist anymore
	      }
	    }
	  }
	}
      }
    }
  }
  else
    wlog->warn("ui_retrieve_matching_state failed to allocate List");
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "ui_retrieve_matching_state = %d", ret);
#endif

  return(ret);
}

bool ui_unquiet_resched(Check *check, char *host, bool uninh, char *who,
			char *why, char **err)
{
  // Unquiet and reschedule the <check> on <host>, by <who> for <why>.
  // If <uninh> is true, the unquiet command in uninhibit, otherwise
  // it is unacknowledge.  On error, <err> will contain a newly
  // allocated string containing the error and that should be deleted
  // when no longer required.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "ui_unquiet_resched(%d,%s,%s,%s,%s,%d)",
		  check, IONULL(host), IOTF(uninh), IONULL(who), IONULL(why),
		  err);
#endif

  if(check && host && who && err)
  {
    // First, we reschedule the check, in order to prevent a race
    // condition of an alert being scheduled before the reschedule
    // is recorded.  (We don't control the state lock files here.)

    AlertState *as = new AlertState(check, host);
    CheckState *cs = new CheckState(check, host);

    if(as && cs)
    {
      if(cs->reschedule())
      {
	if(uninh)
	{
	  if(as->inhibited())
	    ret = as->uninhibit();
	  else
	    *err = xstrdup("No outstanding inhibition");
	}
	else
	{
	  if(as->acknowledged())
	    ret = as->unacknowledge();
	  else
	    *err = xstrdup("No outstanding acknowledgement");
	}

	if(ret)
	  ui_record_command(check, host,
			    (char *)(uninh ? "uninhibit" : "unacknowledge"),
			    who, why);
      }
      else
	*err = xstrdup("Failed to reschedule check");
    }
    else
      wlog->warn("ui_unquiet_resched failed to allocate AlertState or CheckState");

    xdelete(as);
    xdelete(cs);
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "ui_unquiet_resched = %s", IOTF(ret));
#endif
  
  return(ret);
}
