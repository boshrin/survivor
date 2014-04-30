/*
 * Functionality.C: Implements CLI functionality
 *
 * Version: $Revision: 0.42 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2006/11/13 00:06:24 $
 * MT-Level: Safe
 *
 * Copyright (c) 2002 - 2006
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: Functionality.C,v $
 * Revision 0.42  2006/11/13 00:06:24  benno
 * Fix bug preventing clset from working if alerts yet sent
 *
 * Revision 0.41  2006/10/17 13:58:57  benno
 * Display first check, execution duration
 * Reorganize output
 *
 * Revision 0.40  2006/10/14 03:07:57  benno
 * Add checkcf
 *
 * Revision 0.39  2006/10/10 13:16:37  benno
 * Add handling of SIGINT and SIGTSTP while executing checks
 *
 * Revision 0.38  2006/01/25 00:32:34  benno
 * Clarify error
 *
 * Revision 0.37  2005/12/22 03:59:43  benno
 * Add -o match=stalled status
 * Report approximate next check time
 *
 * Revision 0.36  2005/11/30 03:28:13  benno
 * Better reporting of return codes in do_check
 *
 * Revision 0.35  2005/11/24 18:23:58  benno
 * Add do_status
 *
 * Revision 0.34  2005/11/24 02:44:57  benno
 * Add comment about Person substitution and calllists
 *
 * Revision 0.33  2005/11/24 01:56:01  benno
 * ui_execute_report takes FileHandler
 *
 * Revision 0.32  2005/10/05 03:51:53  benno
 * Only report who is on call now when -f isn't provided with clcal
 * Fix looping for when -u is provided with clcal
 *
 * Revision 0.31  2005/10/05 01:47:37  benno
 * Add clset, report_substitution
 * Fix clcal algorithm
 *
 * Revision 0.30  2005/04/19 22:44:12  benno
 * Add comment for future work
 *
 * Revision 0.29  2005/04/09 03:04:54  benno
 * Add fqdn test
 * Use CheckResults
 *
 * Revision 0.28  2004/09/13 14:23:21  benno
 * Use runs_on_host
 *
 * Revision 0.27  2004/08/25 02:00:29  benno
 * Add dispatch_listarg, harness, reverse history, report
 *
 * Revision 0.26  2004/06/20 01:05:22  benno
 * Add clunsub
 *
 * Revision 0.25  2004/06/12 02:16:28  benno
 * Ack/Unack take implicit args
 *
 * Revision 0.24  2004/03/02 17:05:09  benno
 * uiutils uses error pointer instead of throw/catch
 *
 * Revision 0.23  2003/11/29 05:44:19  benno
 * Clean up command dispatching, using dispatch_[all,args,noargs]
 * Add do_archive methods
 * Revise trip, dtest, and clsub for new implementation
 *
 * Revision 0.22  2003/06/17 15:17:48  benno
 * Use new version of ui_rc_to_text
 *
 * Revision 0.21  2003/05/29 00:41:48  benno
 * Change text
 *
 * Revision 0.20  2003/04/21 20:26:31  benno
 * More informative output before first check
 * Use ui_unquiet_resched
 *
 * Revision 0.19  2003/04/09 20:12:09  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.18  2003/04/07 20:34:04  benno
 * Use Debugger
 *
 * Revision 0.17  2003/03/04 21:49:20  benno
 * Ack a problem, not an alert
 * Use ui_acknowledge
 *
 * Revision 0.16  2003/03/02 05:09:56  benno
 * Use toss_eol
 * Substitution list empty message
 *
 * Revision 0.15  2003/02/23 15:48:07  benno
 * Fix reporting of "not configured"
 *
 * Revision 0.14  2003/01/28 03:49:35  benno
 * Use IONULL, IOTF
 *
 * Revision 0.13  2003/01/02 04:48:01  benno
 * Use ui_execute_fix
 *
 * Revision 0.12  2002/12/31 04:43:58  benno
 * Add support for fixes
 * Use libui
 *
 * Revision 0.11  2002/12/16 01:09:01  benno
 * Person based calllists
 * Add clprune
 * Try based AlertPlans
 * RecipientSets for multiple calllists
 *
 * Revision 0.10  2002/08/06 16:44:56  selsky
 * Remove embedded nulls in format
 *
 * Revision 0.9  2002/05/29 21:20:11  benno
 * display automatic escalation information
 *
 * Revision 0.8  2002/04/12 15:02:30  benno
 * use BUFSIZE
 *
 * Revision 0.7  2002/04/04 19:55:56  benno
 * copyright
 *
 * Revision 0.6  2002/04/02 21:01:38  benno
 * rcsify date
 *
 * Revision 0.5  2002/04/02 21:01:28  benno
 * Warn when dispatch_singlearg is not given service@host
 * Warn when check run for service@host not ordinarily configured
 * New API to trip
 *
 * Revision 0.4  2002/04/02 21:00:45  benno
 * Add clsub command
 *
 * Revision 0.3  2002/04/02 21:00:12  benno
 * Add time information to ack and inh status
 *
 * Revision 0.2  2002/04/02 20:59:31  benno
 * Add trip
 *
 * Revision 0.1  2002/04/02 20:58:32  benno
 * initial revision
 *
 */

#include "cli.H"

Functionality::Functionality()
{
  // Allocate and initialize a new Functionality object.

  // Returns: A new Functionality object.

  pwr = NULL;
}

void Functionality::dispatch_all(CommandInfo *ci)
{
  // Dispatch a command that has no arguments over all defined
  // service@host pairs.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "Functionality::dispatch_all(%d)", ci);
#endif

  if(ci)
  {
    // Iterate through the hostclasses since that will give us each
    // defined host once.
    
    List *hcs = cf->get_all_hostclasses();

    if(hcs)
    {
      for(int i = 0;i < hcs->entries();i++)
      {
	HostClass *hc = cf->find_hostclass(hcs->retrieve(i));

	if(hc && hc->hosts())
	{
	  List *hl = hc->hosts();

	  for(int j = 0;j < hl->entries();j++)
	  {
	    char *host = hl->retrieve(j);

	    if(host)
	    {
	      // Now that we have a host name, obtain the set of groups
	      // for the host.

	      List *groups = cf->find_groups(host);

	      if(groups)
	      {
		for(int k = 0;k < groups->entries();k++)
		{
		  Check *c = cf->find_check(groups->retrieve(k));

		  if(c)
		  {
		    // We have a check and host (finally), so dispatch
		    // the command

		    if(strcmp(ci->name, archivehistory_cmd.name)==0)
		      do_archive(c, host);
		  }
		}
	      }
	      // else nothing defined, so nothing to do
	    }
	  }
	}
      }
    }
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "Functionality::dispatch_arg()");
#endif
}

void Functionality::dispatch_arg(CommandInfo *ci, char *arg)
{
  // Dispatch a command that takes an argument, with <arg> provided.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "Functionality::dispatch_arg(%d,%s)",
		  ci, IONULL(arg));
#endif

  if(ci && arg)
  {
    if(ci->argtype == opaque_args)
    {
      if(strcmp(ci->name, clcal_cmd.name)==0)
	do_clcal(arg);
      else if(strcmp(ci->name, clprune_cmd.name)==0
	      || strcmp(ci->name, clunsub_cmd.name)==0)
	do_clprune(arg);
      else if(strcmp(ci->name, clset_cmd.name)==0)
	do_clset(arg);
      else if(strcmp(ci->name, clstat_cmd.name)==0)
	do_clstat(arg);
      else if(strcmp(ci->name, clsub_cmd.name)==0)
	do_clsub(arg);
    }
    else
    {
      SHPair *shp = new SHPair(arg);
      
      if(shp)
      {
	Check *c = cf->find_check(shp->service());
	char *h = NULL;
	
        if(ci->argtype == sh_explicit_args)
	{
	  // We only accept service@host

	  h = shp->host();

	  if(!c)
	    wlog->warn("Service %s not found", IONULL(shp->service()));
	  else if(h && !cf->find_class(h))
	    wlog->warn("Host %s not found", IONULL(h));
	  else if(!c && !h)
	    wlog->warn("Command requires service@host to be specified");
	  else
	  {
	    if(strcmp(ci->name, escalate_cmd.name)==0)
	      do_escalate(c, h);
	    else if(strcmp(ci->name, fix_cmd.name)==0)
	      do_fix(c, h);
	    else if(strcmp(ci->name, trip_cmd.name)==0)
	      do_trip(c, h);
	    else
	      wlog->warn("Programmer stupidity reached in Functionality::dispatch_arg");
	  }
	}
	else
	{
	  // We accept service, host, and service@host

	  if(shp->host() == NULL)
	  {
	    // First see if there is a Check of the service name.  If not,
	    // assume it to be a hostname.
	    
	    if(!c)
	      h = shp->service();
	  }
	  else
	  {
	    h = shp->host();
	    
	    // If only an empty string was given, set hostname to NULL.
	    
	    if(h && strcmp(h, "")==0)
	      h = NULL;
	  }
	  
	  if(c && !h)
	  {
	    List *g = cf->find_group(c->name());
	    
	    if(g)
	    {
	      // abortcmd is used by check to interrupt on ctrl-z
	      // (others will just break on ctrl-c as default)

	      extern bool abortcmd;
	      abortcmd = false;
	      
	      for(int i = 0;i < g->entries();i++)
	      {
		if(abortcmd)
		  break;
		
		do_f(ci, c, g->retrieve(i));
	      }
	    }
	    else
	      wlog->warn("No corresponding group found for service %s",
			 IONULL(c->name()));
	  }
	  else if(!c && h)
	  {
	    List *g = cf->find_groups(h);
	    
	    if(g)
	    {
	      if(shp->service() && (strlen(shp->service()) > 0)
		 && shp->host())
		wlog->warn("Service %s not found", shp->service());
	      else
	      {
		// abortcmd as above

		extern bool abortcmd;
		abortcmd = false;
		
		for(int i = 0;i < g->entries();i++)
		{
		  if(abortcmd)
		    break;
		  
		  do_f(ci, cf->find_check(g->retrieve(i)), h);
		}
	      }
	    }
	    else
	      wlog->warn("No groups found for %s", h);
	  }
	  else if(c && h)
	  {
	    if(cf->find_class(h))
	      do_f(ci, c, h);
	    else
	      wlog->warn("Host %s not found", h);
	  }
	  else
	  {
	    if(shp->service())
	      wlog->warn("Service %s not found", shp->service());
	    else
	      wlog->warn("No useful information provided");
	  }
	}

	xdelete(shp);
      }
    }
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "Functionality::dispatch_arg()");
#endif
}

void Functionality::dispatch_listarg(CommandInfo *ci, int optind, int argc,
				     char **argv)
{
  // Dispatch a command that takes a list of services and hosts as an
  // argument.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "Functionality::dispatch_listarg(%d,%d,%d,%d)",
		  ci, optind, argc, argv);
#endif

  if(ci && optind > 0 && optind < argc && argv)
  {
    List *checks = new List();
    List *hosts = new List();

    if(checks && hosts)
    {
      // Iterate through the args.  For each one, see if there is a
      // Check of the service name.  If not, assume it to be a
      // hostname.

      for(int i = optind;i < argc;i++)
      {
	if(cf->find_check(argv[i]))
	  checks->add(argv[i]);
	else
	  hosts->add(argv[i]);
      }

      // Finally, run the command.

      if(strcmp(ci->name, report_cmd.name)==0)
	do_report(checks, hosts);
      else
	wlog->warn("Programmer stupidity reached in Functionality::dispatch_listarg");
    }
    else
      wlog->warn("Functionality::dispatch_listarg failed to allocate Lists");

    xdelete(checks);
    xdelete(hosts);
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "Functionality::dispatch_listarg()");
#endif
}

void Functionality::dispatch_noarg(CommandInfo *ci)
{
  // Dispatch a command that has no arguments.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "Functionality::dispatch_noarg(%d)", ci);
#endif

  if(ci)
  {
    if(strcmp(ci->name, archivehistory_cmd.name)==0)
      do_archive();
    else if(strcmp(ci->name, checkcf_cmd.name)==0)
      // If we make it here we're done
      cout << "Configuration file parse successful" << endl;
#if defined(DEBUG)
    else if(strcmp(ci->name, dtest_cmd.name)==0)
      do_dtest();
#endif
    else if(strcmp(ci->name, status_cmd.name)==0)
      do_status();
    else
      wlog->warn("Programmer stupidity reached in Functionality::dispatch_noarg");
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "Functionality::dispatch_noarg()");
#endif
}

Functionality::~Functionality()
{
  // Deallocate the Functionality object.

  // Returns: Nothing.

  pwr = NULL;
}

void Functionality::do_acknowledge(Check *check, char *host)
{
  // Acknowledge the outstanding Alert (if there is one) for the service
  // <check> on the specified <host>.  The acknowledgement will contain
  // the username of whoever invoked this program.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "Functionality::do_acknowledge(%d,%s)",
		   check, IONULL(host));
#endif

  if(check && host)
  {
    cout << "[" << IONULL(check->name()) << "@" << host << "]" << flush;

    if(cf->runs_on_host(check->name(), host))
    {
      if(username())
      {
	// gateway/main.C has similar functionality to this

	char *err = NULL;
	
	if(ui_acknowledge(check, host, username(),
			  MAYBE_EXCUSE(args->comment()), &err))
	  cout << " Acknowledged" << endl;
	else
	  cout << " " << err << endl;
	
	xdelete(err);
      }
    }
    else
      cout << " ERROR: " << IONULL(check->name())
	   << " is not configured for " << host << endl;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "Functionality::do_acknowledge()");
#endif
}

void Functionality::do_alerthistory(Check *check, char *host)
{
  // Obtain alert history information for <check>@<host>.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "Functionality::do_alerthistory(%d,%s)",
		  check, IONULL(host));
#endif

  AlertHistory *ah = new AlertHistory(check, host);

  if(ah)
  {
    bool rev = args->arg_o(OPT_HISTORY_REVERSE);
    
    if((rev &&
	ah->iterate_begin_backwards(args->fromtime(), args->untiltime()))
       || ah->iterate_begin(args->fromtime(), args->untiltime()))
    {
      AlertHistoryRecord *ahr = NULL;

      while((ahr = (rev ? ah->iterate_previous() : ah->iterate_next()))
	    != NULL)
      {
	cout << check->name() << "@" << host << ":" << ahr->timestamp_local()
	     << ":" << ahr->alertrv() << ":" << ahr->checkrv() << ":"
	     << ahr->who() << " via " << ahr->via() << endl;
	
	xdelete(ahr);
      }
      
      ah->iterate_end();
    }
    
    xdelete(ah);
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "Functionality::do_alerthistory()");
#endif
}

void Functionality::do_archive()
{
  // Archive all history in accordance with options set by -o.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "Functionality::do_archive()");
#endif
  
  // We always archive stale because we only accept two options here,
  // all and stale, and all implies stale.
  
  if(args->arg_o(OPT_ARCHIVEHISTORY_ALL))
    dispatch_all(&archivehistory_cmd);

  // Begin pruning of stale history.  To start, we obtain the list
  // of invalid service@host pairs.  Note this could become obsolete
  // while we're processing it if somebody updates the configuration
  // files, but there's not much we can (or should) do about that.
  
  Array<SHPair> *shps = cf->history_consistency();

  if(shps)
  {
    char *dest = args->arg_o(OPT_ARCHIVEHISTORY_DIRECTORY);
    bool istree = true;
	
    if(!dest)
    {
      dest = args->arg_o(OPT_ARCHIVEHISTORY_FILE);
      istree = false;
    }
	
    for(int i = 0;i < shps->entries();i++)
    {
      SHPair *shp = shps->retrieve(i);

      if(shp)
      {
	bool pok = true;
	
	cout << "[" << IONULL(shp->service()) << "@" << IONULL(shp->host())
	     << "] " << flush;

	// Archive each history file, one at a time.  Note that if a
	// specific file doesn't exist yet (say, fixes have never run
	// for this service), we'll just silently skip it.

	cout << "alerthistory" << flush;

	AlertHistory *ah = new AlertHistory(shp->service(), shp->host());

	if(ah)
	{
	  if(ah->prune(dest, istree))
	    cout << " OK" << flush;
	  else
	  {
	    cout << " FAILED" << flush;
	    pok = false;
	  }
	  
	  xdelete(ah);
	}
	else
	  wlog->warn("Functionality::do_archive failed to allocate AlertHistory");

	cout << ", checkhistory" << flush;

	CheckHistory *ch = new CheckHistory(shp->service(), shp->host());

	if(ch)
	{
	  if(ch->prune(dest, istree))
	    cout << " OK" << flush;
	  else
	  {
	    cout << " FAILED" << flush;
	    pok = false;
	  }
	  
	  xdelete(ch);
	}
	else
	  wlog->warn("Functionality::do_archive failed to allocate CheckHistory");
	
	cout << ", commandhistory" << flush;
	
	CommandHistory *cmh = new CommandHistory(shp->service(), shp->host());
	
	if(cmh)
	{
	  if(cmh->prune(dest, istree))
	    cout << " OK" << flush;
	  else
	  {
	    cout << " FAILED" << flush;
	    pok = false;
	  }
	  
	  xdelete(cmh);
	}
	else
	  wlog->warn("Functionality::do_archive failed to allocate CommandHistory");
	
	cout << ", fixhistory" << flush;
	
	FixHistory *fh = new FixHistory(shp->service(), shp->host());
	
	if(fh)
	{
	  if(fh->prune(dest, istree))
	    cout << " OK" << flush;
	  else
	  {
	    cout << " FAILED" << flush;
	    pok = false;
	  }
	  
	  xdelete(fh);
	}
	else
	  wlog->warn("Functionality::do_archive failed to allocate FixHistory");

	if(pok)
	{
	  if(cf->remove_history(shp->service(), shp->host()))
	    cout << ", removal complete" << flush;
	  else
	    cout << ", removal FAILED" << flush;
	}
	
	cout << endl;
      }
    }
    
    xadelete(shps, SHPair);
  }

  // if pruning stale, no locking is needed, just move the entire file
  // (or append it), and delete any empty directories left behind

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "Functionality::do_archive()");
#endif
}

void Functionality::do_archive(Check *check, char *host)
{
  // Archive all history for <check>@<host> in accordance with
  // options set by -o.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "Functionality::do_archive(%d,%s)",
		  check, IONULL(host));
#endif

  if(check && host)
  {
    // We don't verify from/until time or destination, relying on the
    // prerequisite verification and History->prune() routine to do
    // this for us.
    
    char *dest = args->arg_o(OPT_ARCHIVEHISTORY_DIRECTORY);
    bool istree = true;
    
    if(!dest)
    {
      dest = args->arg_o(OPT_ARCHIVEHISTORY_FILE);
      istree = false;
    }
    
    cout << "[" << check->name() << "@" << host << "] " << flush;

    // Archive each history file, one at a time.  Note that if a
    // specific file doesn't exist yet (say, fixes have never run
    // for this service), we'll just silently skip it.

    cout << "alerthistory" << flush;

    AlertHistory *ah = new AlertHistory(check, host);

    if(ah)
    {
      if(ah->prune(args->fromtime(), args->untiltime(), dest, istree))
	cout << " OK" << flush;
      else
	cout << " FAILED" << flush;
      
      xdelete(ah);
    }
    else
      wlog->warn("Functionality::do_archive failed to allocate AlertHistory");

    cout << ", checkhistory" << flush;

    CheckHistory *ch = new CheckHistory(check, host);

    if(ch)
    {
      if(ch->prune(args->fromtime(), args->untiltime(), dest, istree))
	cout << " OK" << flush;
      else
	cout << " FAILED" << flush;
      
      xdelete(ch);
    }
    else
      wlog->warn("Functionality::do_archive failed to allocate CheckHistory");
    
    cout << ", commandhistory" << flush;

    CommandHistory *cmh = new CommandHistory(check, host);

    if(cmh)
    {
      if(cmh->prune(args->fromtime(), args->untiltime(), dest, istree))
	cout << " OK" << flush;
      else
	cout << " FAILED" << flush;
      
      xdelete(cmh);
    }
    else
      wlog->warn("Functionality::do_archive failed to allocate CommandHistory");
    
    cout << ", fixhistory" << flush;

    FixHistory *fh = new FixHistory(check, host);

    if(fh)
    {
      if(fh->prune(args->fromtime(), args->untiltime(), dest, istree))
	cout << " OK" << flush;
      else
	cout << " FAILED" << flush;
      
      xdelete(fh);
    }
    else
      wlog->warn("Functionality::do_archive failed to allocate FixHistory");

    cout << endl;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "Functionality::do_archive()");
#endif
}

void Functionality::do_check(Check *check, char *host)
{
  // Execute the Check <check> on the specified <host> immediately, without
  // updating any state information.  Display the results.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "Functionality::do_check(%d,%s)",
		   check, IONULL(host));
#endif

  if(check && host)
  {
    // See if check ordinarily runs on host.  If it doesn't, display
    // a warning, but run it anyway since it could be useful to manually
    // run a check without editing the configuration file.

    if(!cf->runs_on_host(check->name(), host))
      cout << "WARNING: " << IONULL(check->name())
	   << " is not configured for " << host << endl;

    cout << "[" << IONULL(check->name()) << "@" << host << "]" << flush;

    Executor *e = new Executor();
    
    if(e)
    {
      // The timeout for exec_check is advisory, so we just set it
      // rather high on the theory that the user can hit ctrl-c if
      // they get bored.
      
      CheckResult *crs = NULL;
      CheckRequest *creq = new CheckRequest(host, 6000, check->modargs(),
					    false);

      if(creq)
      {
	extern pid_t cppid;

	// ctrl-c will interrupt us, ctrl-z will abort

#if defined(_BSD_SIGNALS)
	signal(SIGINT, signal_catcher);
	signal(SIGTSTP, signal_catcher);

	sigset_t sigset;
	sigemptyset(&sigset);
	sigaddset(&sigset, SIGINT);
	sigaddset(&sigset, SIGTSTP);
	pthread_sigmask(SIG_UNBLOCK, &sigset, NULL);
#else
	sigset(SIGINT, signal_catcher);
	sigset(SIGTSTP, signal_catcher);
#endif // _BSD_SIGNALS
  
	cppid = e->exec_check(check, creq);
	
	if(cppid != -1)
	  e->result(&crs);

	// Release signals

	cppid = 0;
	
#if defined(_BSD_SIGNALS)
	signal(SIGINT, SIG_DFL);
	signal(SIGTSTP, SIG_DFL);
#else
	sigrelse(SIGINT);
	sigrelse(SIGTSTP);
#endif // _BSD_SIGNALS
	  
	xdelete(creq);
      }

      if(crs)
      {
	if(ui_rc_to_text(crs->rc()))
	  cout << " " << ui_rc_to_text(crs->rc());
	else
	  cout << " Return code " << crs->rc();
	
	cout << ": " << IONULL(crs->comment()) << endl;
      }
      else
	cout << " " << ui_rc_to_text(MODEXEC_MISCONFIG)
	     << ": No result received" << endl;

      xdelete(crs);
      xdelete(e);
    }
    else
      wlog->warn("Functionality::do_check failed to allocate Executor");
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "Functionality::do_check()");
#endif
}

void Functionality::do_checkhistory(Check *check, char *host)
{
  // Obtain check history information for <check>@<host>.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "Functionality::do_checkhistory(%d,%s)",
		  check, IONULL(host));
#endif

  CheckHistory *ch = new CheckHistory(check, host);

  if(ch)
  {
    bool rev = args->arg_o(OPT_HISTORY_REVERSE);
    
    if((rev &&
	ch->iterate_begin_backwards(args->fromtime(), args->untiltime()))
       || ch->iterate_begin(args->fromtime(), args->untiltime()))
    {
      CheckHistoryRecord *chr = NULL;

      while((chr = (rev ? ch->iterate_previous() : ch->iterate_next()))
	    != NULL)
      {
	cout << check->name() << "@" << host << ":" << chr->timestamp_local()
	     << ":" << chr->checkrv() << ":" << chr->scalar() << ":"
	     << chr->comment() << endl;
	
	xdelete(chr);
      }
      
      ch->iterate_end();
    }
    
    xdelete(ch);
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "Functionality::do_checkhistory()");
#endif
}

void Functionality::do_clcal(char *calllist)
{
  // Execute the 'clcal' command for the provided <calllist>.

  // Returns: Nothing.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "Functionality::do_clcal(%s)",
		  IONULL(calllist));
#endif

  if(calllist)
  {
    cout << "[" << IONULL(calllist) << "]" << endl;
    
    CallList *cl = cf->find_calllist(calllist);

    if(cl)
    {
      char *person = args->arg_o(OPT_CLCAL_PERSON);

      if(cl->listtype() == broadcast_list)
	cout << " -> No calendar information for Broadcast CallLists" << endl;
      else
      {
	CallListState *cls = new CallListState(cl);

	if(cls)
	{
	  if(cl->listtype() == rotating_list)
	  {
	    // Display calendar for rotating.

	    time_t tf = args->fromtime();
	    time_t tu = args->untiltime();  // If 0, loop until list cycles

	    if(tf == 0)
	    {
	      // Set start time to now

	      struct timeval tp;

	      if(gettimeofday(&tp, NULL)==0)
		tf = tp.tv_sec;
	      else
		wlog->warn("gettimeofday failed in Functionality::do_clcal");
	    }

	    // We need the schedule the list uses to rotate on.  We get the
	    // name of the schedule from the CallList, but CallList actually
	    // has a pointer to a schedule stored within.  We should really
	    // use that pointer, but this is probably good enough.

	    Array<Schedule> *c = cf->find_schedule(cl->rotatename());

	    if(c)
	    {
	      Array<Substitution> *s = cls->readsubs();

	      if(s)
	      {
		// Sort the schedule Array by proximity to the next
		// time each Schedule is in effect.  In most cases,
		// there will only be one Schedule, so this will
		// usually be a lot of extra work.

		// Schedule->sort_key uses the current time to
		// determine its index, whereas we really want it
		// to sort based on a provided time (ie: tf).
		// This gets tricky to change since the Schedules
		// are part of cf.  So we create a new Array,
		// holding objects pointing to the Schedules in the
		// cf copy.

		Array<ScheduleAsOf> *c2 = new Array<ScheduleAsOf>();

		if(c2)
		{
		  for(int i = 0;i < c->entries();i++)
		  {
		    Schedule *s2 = c->retrieve(i);

		    if(s2)
		    {		    
		      ScheduleAsOf *sao = new ScheduleAsOf(s2, s2->next(tf));

		      if(sao)
		      {
			if(!c2->add(sao))
			{
			  xdelete(sao);
			}
		      }
		      else
			wlog->warn("Functionality::do_clcal failed to allocate ScheduleAsOf");
		    }
		  }

		  Array<ScheduleAsOf> *C = c2->sort(ascending_sort);

		  // Sort the substitution Array by the substitution start
		  // time

		  Array<Substitution> *S = s->sort(ascending_sort);

		  if(C && S)
		  {
		    time_t asof = tf;
		    int Dt = 0;

		    Person *P = cl->notify(cls->oncall(),
					   cls->lastrotate(),
					   asof,
					   false);

		    Person *Ps = cl->notify(cls->oncall(),
					    cls->lastrotate(),
					    asof,
					    true);

		    if(P && Ps && P->name() && Ps->name())
		    {
		      if(args->fromtime() == 0)
		      {
			// Report who is currently on call
			
			cout << " -> " << P->name() << " is now on call"
			     << endl;

			if(strcmp(P->name(), Ps->name()) != 0)
			  cout << " -> " << Ps->name()
			       << " is substituting for "
			       << P->name() << endl;

			// Report substitutions currently in effect

			for(int i = 0;i < S->entries();i++)
			{
			  // We could optimize slighty by breaking the loop
			  // early since the Substitutions are in order
			
			  Substitution *su = S->retrieve(i);
			
			  if(su && su->begins() <= asof && su->ends() > asof)
			    report_substitution(su);
			}
		      }

		      // Loop through the set of schedules and iterate
		      // who is on call
		      
		      int j = 0;
		      Schedule *Cj = C->retrieve(j)->schedule();
		      
		      if(Cj)
		      {
			Dt = Cj->next(asof);
			asof += Dt;
			Person *Pn = cl->notify(cls->oncall(),
						cls->lastrotate(),
						asof,
						false);
			
			while(Pn &&
			      ((tu == 0 && strcmp(P->name(), Pn->name())!=0)
			       ||
			       (tu > 0 && asof < tu)))
			{
			  // First report substitutions since we last reported
			  
			  for(int i = 0;i < S->entries();i++)
			  {
			    Substitution *su = S->retrieve(i);
			    
			    if(su && su->begins() >= (asof - Dt)
			       && su->begins() < asof)
			      report_substitution(su);
			  }
			  
			  // Report Pn
			  
			  if(Pn && Pn->name())
			  {
			    char buf[26];
			    
			    ctime_r(&asof, buf);
			    buf[24] = '\0';
			  
			    cout << " -> " << Pn->name() << " on call " << buf
				 << endl;
			  }
			
			  // Determine the next Schedule in effect
			  
			  if(j + 1 < C->entries())
			    j++;
			  else
			    j = 0;

			  Cj = C->retrieve(j)->schedule();
			
			  Dt = Cj->next(asof);
			  asof += Dt;
			  Pn = cl->notify(cls->oncall(),
					  cls->lastrotate(),
					  asof,
					  false);
			}
		      }

		      // Report remaining substitutions
		    
		      for(int i = 0;i < S->entries();i++)
		      {
			Substitution *su = S->retrieve(i);
		      
			if(su && su->begins() >= (asof - Dt))
			  report_substitution(su);
		      }
		      
		      // Report expired substitutions
		      
		      for(int i = 0;i < S->entries();i++)
		      {
			Substitution *su = S->retrieve(i);
			
			if(su && su->ends() < tf)
			  report_substitution(su);
		      }
		    }
		    else
		      wlog->warn("Functionality:do_clcal failed to obtain Person to notify");

		    // Don't delete C or S, the original Arrays maintain them
		    C = NULL;
		    S = NULL;
		  }
		  else
		    wlog->warn("Functionality::do_clcal failed to sort schedules and substitutions");

		  // We don't need c2 anymore
		  xadelete(c2, ScheduleAsOf);
		}
		else
		  wlog->warn("Functionality::do_clcal failed to allocate Array");
		
		xdelete(s);
	      }
	      else
		wlog->warn("Functionality::do_clcal failed to read substitutions");
	    }
	    else
	      wlog->warn("Functionality::do_clcal could not get rotation schedule");	    
	  }
	  else
	  {
	    // Display substitutions for simple

	    Array<Substitution> *sarray = cls->readsubs();
	  
	    if(sarray)
	    {
	      for(int i = 0;i < sarray->entries();i++)
	      {
		Substitution *s = sarray->retrieve(i);
	      
		if(s &&
		   (!person
		    || (s->newname() && strcmp(s->newname(), person)==0)
		    || (s->oldname() && strcmp(s->oldname(), person)==0)))
		{
		  char buf[26];
		  time_t t;
		  
		  cout << " *> " << IONULL(s->newname()) << " replaces "
		       << IONULL(s->oldname()) << " from ";
		  
		  t = s->begins();
		  ctime_r(&t, buf);
		  buf[24] = '\0';
		  
		  cout << buf << " until ";
		
		  t = s->ends();
		  ctime_r(&t, buf);
		  buf[24] = '\0';
		
		  cout << buf << endl;
		}
	      }
	      
	      xadelete(sarray, Substitution);
	    }
	  }

	  xdelete(cls);
	}
	else
	  wlog->warn("Functionality::do_clcal failed to allocate CallListState");
      }
    }
    else
      cout << " No such CallList" << endl;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "Functionality::do_clcal()");
#endif
}

void Functionality::do_clprune(char *calllist)
{
  // Execute the 'clprune' or 'clunsub' command for the provided <calllist>.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "Functionality::do_clprune(%s)",
		  IONULL(calllist));
#endif

  if(calllist)
  {
    cout << "[" << IONULL(calllist) << "]" << flush;
    
    CallList *cl = cf->find_calllist(calllist);

    if(cl)
    {
      CallListState *cls = new CallListState(cl);

      if(cls)
      {
	Array<Substitution> *subs = cls->readsubs();

	if(subs && subs->entries() > 0)
	{
	  time_t f = args->fromtime();
	  time_t u = args->untiltime();

	  // If both from and until are 0, we are pruning anything
	  // that has passed.

	  if(f == 0 && u == 0)
	  {
	    struct timeval tp;

	    if(gettimeofday(&tp, NULL)==0)
	      u = tp.tv_sec;
	    else
	      wlog->warn("gettimeofday failed in Functionality::do_clprune");
	  }
	  
	  if(cls->prunesubs(f, u))
	    cout << " Substitutions pruned" << endl;
	  else
	    cout << " Substitution pruning FAILED" << endl;
	}
	else
	  cout << " Substitution list is already empty" << endl;

	xadelete(subs, Substitution);
	xdelete(cls);
      }
    }
    else
      cout << " No such CallList" << endl;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "Functionality::do_clprune()");
#endif
}

void Functionality::do_clset(char *calllist)
{
  // Execute the 'clset' command for the provided <calllist>.

  // Returns: Nothing.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "Functionality::do_clset(%s)",
		  IONULL(calllist));
#endif

  if(calllist)
  {
    cout << "[" << IONULL(calllist) << "] " << flush;
    
    CallList *cl = cf->find_calllist(calllist);

    if(cl)
    {
      if(cl->listtype() == broadcast_list)
      {
	// There is no state for a broadcast calllist
	
	cout << "Broadcast CallList (cannot be set)" << endl;
      }
      else
      {      
	char *person = args->arg_o(OPT_CLSET_PERSON);

	if(person && cl->member(person))
	{	
	  CallListState *cls = new CallListState(cl);

	  if(cls)
	  {
	    switch(cl->listtype())
	    {
	    case simple_list:
	      // We fake data here because the semantics of clset for
	      // simple lists are to set the person as the last notified
	      if(cls->notenotify(person, "manual", ""))
		cout << "OK: Set" << endl;
	      else
		cout << "FAILED" << endl;
	      break;
	    case rotating_list:
	      // We leave the last notified info intact and set person
	      // as on call
	      {
		char *ln = xstrdup(cls->lastnotify() ?
				   cls->lastnotify() :
				   (char *)"(nobody)");
		char *lna = xstrdup(cls->lastnotifyaddress() ?
				    cls->lastnotifyaddress() :
				    (char *)"(none)");
		char *lnv = xstrdup(cls->lastnotifyvia() ?
				    cls->lastnotifyvia() :
				    (char *)"()");
	    
		if(cls->notenotify(ln, lnv, lna, person))
		  cout << "OK: Set" << endl;
		else
		  cout << "FAILED" << endl;

		xdelete(ln);
		xdelete(lna);
		xdelete(lnv);
	      }
	      break;
	    }

	    xdelete(cls);
	  }
	  else
	    wlog->warn("Functionality::clstat failed to allocate CallListState");
	}
	else
	  cout << person << " not found in list" << endl;
      }
    }
    else
      cout << "No such CallList" << endl;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "Functionality::do_clset()");
#endif
}

void Functionality::do_clstat(char *calllist)
{
  // Execute the 'clstat' for the provided <calllist>.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "Functionality::do_clstat(%s)",
		  IONULL(calllist));
#endif

  if(calllist)
  {
    cout << "[" << IONULL(calllist) << "]" << endl;
    
    CallList *cl = cf->find_calllist(calllist);

    if(cl)
    {
      // It is safe to call the notify methods since cls->notenotify is the
      // only way to update them.
      
      if(cl->listtype() == broadcast_list)
      {
	// There is no state for a broadcast calllist
	
	cout << " -> Broadcast CallList" << endl;
	cout << " -> Notifies: " << IONULL(cl->notify()) << endl;
      }
      else
      {      
	CallListState *cls = new CallListState(cl);

	if(cls)
	{
	  Person *np = NULL;
	  time_t t;
	  
	  if(cls->lastnotify() && strcmp(cls->lastnotify(), "")!=0)
	    cout << " -> " << IONULL(cls->lastnotify())
		 << " was last notified at "
		 << IONULL(cls->lastnotifyaddress())
		 << " via this list" << endl;
	  
	  switch(cl->listtype())
	  {
	  case simple_list:
	    np = cl->notify(cls->lastnotify());
	    cout << " -> " << IONULL(np->name()) << " is next to be notified"
		 << endl;
	    break;
	  case rotating_list:
	    t = cls->lastrotate();
	    np = cl->notify(cls->oncall(), t, true);
	    if(t > 0)                             // ctime puts \n
	      cout << " -> List last rotated at " << ctime(&t);
	    cout << " -> " << IONULL(np->name()) << " is now on call" << endl;
	    break;
	  }

	  xdelete(cls);
	}
	else
	  wlog->warn("Functionality::clstat failed to allocate CallListState");
      }
    }
    else
      cout << " No such CallList" << endl;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "Functionality::do_clstat()");
#endif
}

void Functionality::do_clsub(char *calllist)
{
  // Execute the 'clsub' command for the provided <calllist>.
  // -o replace=x,with=x supplies the replacement information,
  // and -f/-u provide time delimiters.

  // Returns: Nothing.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "Functionality::do_clsub(%s)",
		  IONULL(calllist));
#endif

  cout << "[clsub] " << flush;

  if(calllist)
  {
    // Sanity check and convert data

    CallList *cl = cf->find_calllist(calllist);

    if(cl)
    {
      char *newname = args->arg_o(OPT_CLSUB_WITH);
      char *oldname = args->arg_o(OPT_CLSUB_REPLACE);
      
      if(args->fromtime() > 0 && args->untiltime() > 0)
      {
	// Make sure both names are in this calllist.  If this ever
	// changes to allow any defined Person to substitute, make
	// sure that newname is reachable via cl->module().
	// (The config parser verifies this for all members within
	// a calllist.)

	if(cl->member(newname))
	{
	  if(cl->member(oldname))
	  {
	    // Data checks out, make the change

	    Substitution *s = new Substitution(args->fromtime(),
					       args->untiltime(),
					       newname, oldname);

	    if(s)
	    {
	      CallListState *cls = new CallListState(cl);
	      
	      if(cls)
	      {
		if(cls->addsub(s))
		  cout << "OK: Substitution added" << endl;
		else
		  cout << "FAILED: Unable to add substitution" << endl;

		xdelete(cls);
	      }
	      else
		cout << "FAILED: Failed to allocate CallListState" << endl;

	      xdelete(s);
	    }
	    else
	      cout << "FAILED: Failed to allocate Substitution" << endl;
	  }
	  else
	    cout << "FAILED: " << oldname << " is not a member of "
		 << calllist << endl;
	}
	else
	  cout << "FAILED: " << newname << " is not a member of "
	       << calllist << endl;
      }
      else
	cout << "FAILED: Valid time not specified (use -f and -u)" << endl;
    }
    else
      cout << "FAILED: CallList " << calllist << " does not exist" << endl;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "Functionality::do_clsub()");
#endif
}

void Functionality::do_commandhistory(Check *check, char *host)
{
  // Obtain command history information for <check>@<host>.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "Functionality::do_commandhistory(%d,%s)",
		  check, IONULL(host));
#endif

  CommandHistory *ch = new CommandHistory(check, host);

  if(ch)
  {
    bool rev = args->arg_o(OPT_HISTORY_REVERSE);
    
    if((rev &&
	ch->iterate_begin_backwards(args->fromtime(), args->untiltime()))
       || ch->iterate_begin(args->fromtime(), args->untiltime()))
    {
      CommandHistoryRecord *chr = NULL;

      while((chr = (rev ? ch->iterate_previous() : ch->iterate_next()))
	    != NULL)
      {
	cout << check->name() << "@" << host << ":" << chr->timestamp_local()
	     << ":" << chr->command() << ":" << chr->who() << ":"
	     << chr->comment() << endl;
	
	xdelete(chr);
      }
      
      ch->iterate_end();
    }
    
    xdelete(ch);
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "Functionality::do_commandhistory()");
#endif
}

#if defined(DEBUG)
void Functionality::do_dtest()
{
  // Execute the requested test, as specified by -o.

  // Returns: Nothing.

  char *cmd = args->arg_o(OPT_DTEST_TEST);

  if(cmd)
  {
    if(strcmp(cmd, "alert")==0)
    {
      char *addr = args->arg_o(OPT_DTEST_ADDRESS);
      char *mod = args->arg_o(OPT_DTEST_MODULE);

      if(addr && mod)
      {
	AlertModule *am = cf->find_alertmodule(mod);
		      
	if(am)
	{
	  struct timeval tv;
	  char hbuf[BUFSIZE];
		      
	  gettimeofday(&tv, NULL);
	  memset(hbuf, 0, BUFSIZE);
	  gethostname(hbuf, BUFSIZE-1);
			
	  Executor *e = new Executor();
	  RecipientSet *rset = new RecipientSet();
	  Alert *a = new Alert(rset, tv.tv_sec, hbuf, NULL,
			       args->instname(), 1,
			       MODEXEC_NOTICE,
			       "sc-dtest",
			       "test alert requested",
			       "Q12345", false);

	  if(e && rset && a)
	  {
	    rset->add(addr, "sc-dtest", mod);
			  
	    pid_t pid = e->exec_alert(a, am);
	    cout << "Alert module pid=" << pid << endl;
	    int r = e->result();
	    cout << "Return code=" << r << endl;
	  }
	  else
	    wlog->warn("Failed to allocate all objects");
	  
	  xdelete(a);  // this gets rset also
	  xdelete(e);
	}
	else
	  wlog->warn("Requested alert module '%s' is not defined", mod);
      }
      else
	wlog->warn("dtest alert requires -o %s=x,%s=y",
		   OPT_DTEST_ADDRESS, OPT_DTEST_MODULE);
    }
    else if(strcmp(cmd, "fqdn")==0)
    {
      // Get the local fqdn

      char *hostname = get_localhost_fqdn();

      if(hostname)
      {
	cout << "local fqdn: " << hostname << endl;
	
	xdelete(hostname);
      }
      else
	cout << "local fqdn not found" << endl;
    }
    else if(strcmp(cmd, "harness")==0)
    {
      // Whatever we happen to be testing right now.

      List *l = new List("test");

      if(l)
      {
	l->add("zed");
	l->add("Foo");
	l->add("bar");
	l->add("bar");
	l->add("foo");
	l->add("baz");

	for(int i = 0;i < l->entries();i++)
	  cout << i << ": " << l->retrieve(i) << endl;

	cout << l->comma_list() << endl;

	l->sort(ascending_sort);

	for(int i = 0;i < l->entries();i++)
	  cout << i << ": " << l->retrieve(i) << endl;

	cout << l->comma_list() << endl;

	l->sort(descending_sort);

	for(int i = 0;i < l->entries();i++)
	  cout << i << ": " << l->retrieve(i) << endl;

	cout << l->comma_list() << endl;

	l->sort(ascending_caseless_sort);

	for(int i = 0;i < l->entries();i++)
	  cout << i << ": " << l->retrieve(i) << endl;

	cout << l->comma_list() << endl;

	xdelete(l);
      }
    }
    else if(strcmp(cmd, "reparse")==0)
    {
      // Attempt to reparse the configuration files to test for lex
      // programmer errors.  Create a new cf to avoid trying to reset
      // config options that cannot be changed once set.
		      
      Configuration *newcf = new Configuration();
		    
      if(newcf)
      {
	if(!newcf->parse_cfs())
	  wlog->warn("Configuration file reparse failed");
	else
	  cout << "dtest reparse successful" << endl;
	
	xdelete(newcf);
      }
      else
	wlog->warn("Failed to allocate new Configuration");
    }
    else
      wlog->warn("Unknown dtest test %s", cmd);
  }
  else
    wlog->warn("dtest not provided cmd to execute (-o %s=x)", OPT_DTEST_TEST);
}
#endif

void Functionality::do_escalate(Check *check, char *host)
{
  // Escalate the error status for the Check <check> on the specified
  // <host> to the next alert level.  If the highest return value in
  // the appropriate AlertPlan is already matched, or if there is no
  // outstanding alert, this method will have no effect.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "Functionality::do_escalate(%d,%s)",
		   check, IONULL(host));
#endif

  if(check && host)
  {
    cout << "[" << check->name() << "@" << IONULL(host) << "]" << flush;

    if(cf->runs_on_host(check->name(), host))
    {
      int ret = -1;
      char *err = NULL;
    
      ret = ui_escalate_to(check, host, username(),
			   MAYBE_EXCUSE(args->comment()), &err);

      if(err)
	cout << " " << err << endl;
      else
	cout << " Escalated to " << ret << " tries" << endl;

      xdelete(err);
    }
    else
      cout << " ERROR: " << IONULL(check->name())
	   << " is not configured for " << host << endl;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "Functionality::do_escalate()");
#endif
}

void Functionality::do_f(CommandInfo *ci, Check *check, char *host)
{
  // Hand off the arguments to the appropriate function.

  // Returns: Nothing.

  // Don't even bother with debugging here.
  
  if(strcmp(ci->name, acknowledge_cmd.name)==0)
    do_acknowledge(check, host);
  else if(strcmp(ci->name, alerthistory_cmd.name)==0)
    do_alerthistory(check, host);
  else if(strcmp(ci->name, archivehistory_cmd.name)==0)
    do_archive(check, host);
  else if(strcmp(ci->name, check_cmd.name)==0)
    do_check(check, host);
  else if(strcmp(ci->name, checkhistory_cmd.name)==0)
    do_checkhistory(check, host);
  else if(strcmp(ci->name, commandhistory_cmd.name)==0)
    do_commandhistory(check, host);
  else if(strcmp(ci->name, fixhistory_cmd.name)==0)
    do_fixhistory(check, host);
  else if(strcmp(ci->name, inhibit_cmd.name)==0)
    do_inhibit(check, host);
  else if(strcmp(ci->name, reschedule_cmd.name)==0)
    do_reschedule(check, host);
  else if(strcmp(ci->name, status_cmd.name)==0)
    do_status(check, host);
  else if(strcmp(ci->name, unacknowledge_cmd.name)==0)
    do_unacknowledge(check, host);
  else if(strcmp(ci->name, uninhibit_cmd.name)==0)
    do_uninhibit(check, host);
  else
    wlog->warn("Programmer stupidity reached in Functionality::do_f");
}

void Functionality::do_fix(Check *check, char *host)
{
  // Run the Fix defined for the service <check> on the specified
  // <host>, if a Fix is defined.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "Functionality::do_fix(%d,%s)",
		   check, IONULL(host));
#endif

  if(check && host && username())
  {
    cout << "[" << IONULL(check->name()) << "@" << IONULL(host) << "]"
	 << flush;

    if(cf->runs_on_host(check->name(), host))
    {
      CheckState *cs = new CheckState(check, host);
      FixState *fs = new FixState(check, host);

      if(cs && fs)
      {
	char *err = NULL;
      
	int rc = ui_execute_fix(check, host, cs, fs, username(),
				MAYBE_EXCUSE(args->comment()), &err);

	if(err)
	  cout << " " << err << endl;
	else
	{
	  if(ui_rc_to_text(rc))
	    cout << " " << ui_rc_to_text(rc);
	  else
	    cout << " Return code " << rc;

	  // XXX it would be better to get the comment directly from
	  // the exec rather than fs, since if checkstatus is MODEXEC_OK
	  // and a fix is manually run, the fixstate won't be written
	  // and the comment will be NULL.
	  
	  cout << ": " << IONULL(fs->comment());
	  
	  if(rc == MODEXEC_OK && cs->returncode() != MODEXEC_OK)
	    cout << " (Check has been rescheduled)";
	
	  cout << endl;
	}

	xdelete(err);
      }
      else
	wlog->warn("Functionality::do_fix failed to allocated State objects");
      
      xdelete(cs);
      xdelete(fs);
    }
    else
      cout << " ERROR: " << IONULL(check->name())
	   << " is not configured for " << host << endl;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "Functionality::do_fix()");
#endif
}

void Functionality::do_fixhistory(Check *check, char *host)
{
  // Obtain fix history information for <check>@<host>.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "Functionality::do_fixhistory(%d,%s)",
		  check, IONULL(host));
#endif

  FixHistory *fh = new FixHistory(check, host);

  if(fh)
  {
    bool rev = args->arg_o(OPT_HISTORY_REVERSE);
    
    if((rev &&
	fh->iterate_begin_backwards(args->fromtime(), args->untiltime()))
       || fh->iterate_begin(args->fromtime(), args->untiltime()))
    {
      FixHistoryRecord *fhr = NULL;

      while((fhr = (rev ? fh->iterate_previous() : fh->iterate_next()))
	    != NULL)
      {
	cout << check->name() << "@" << host << ":" << fhr->timestamp_local()
	     << ":" << fhr->fixrv() << ":" << fhr->who() << ":"
	     << fhr->comment() << endl;
	
	xdelete(fhr);
      }
      
      fh->iterate_end();
    }
    
    xdelete(fh);
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "Functionality::do_fixhistory()");
#endif
}

void Functionality::do_inhibit(Check *check, char *host)
{
  // Inhibit all Alerts for the service <check> on the specified
  // <host>.  The acknowledgement will contain the username of whoever
  // invoked this program.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "Functionality::do_inhibit(%d,%s)",
		   check, IONULL(host));
#endif

  if(check && host)
  {
    cout << "[" << check->name() << "@" << IONULL(host) << "]" << flush;

    if(cf->runs_on_host(check->name(), host))
    {
      if(username())
      {
	char *err = NULL;

	if(ui_inhibit(check, host, username(), MAYBE_EXCUSE(args->comment()),
		      &err))
	  cout << " Inhibited" << endl;
	else
	  cout << " " << err << endl;

	xdelete(err);
      }
    }
    else
      cout << " ERROR: " << IONULL(check->name())
	   << " is not configured for " << host << endl;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "Functionality::do_inhibit()");
#endif
}

void Functionality::do_report(List *checks, List *hosts)
{
  // Execute the requested report module for the requested report type
  // over all <checks> on all <hosts>.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "Functionality::do_report(%d,%d)",
		  checks, hosts);
#endif

  if(checks && hosts)
  {
    char *err = NULL;

    ReportFormatting *rf = new ReportFormatting(args->arg_o(OPT_REPORT_MODULE),
						"text",
						args->tmpdir());

    if(rf)
    {
      FileHandler *fout = new FileHandler(STDOUT_FILENO);
      
      if(fout)
      {
	int rtype = 0;

	if(strcmp(args->arg_o(OPT_REPORT_TYPE), "alert")==0)
	  rtype = UI_REPORT_ALERT;
	else if(strcmp(args->arg_o(OPT_REPORT_TYPE), "check")==0)
	  rtype = UI_REPORT_CHECK;
	else if(strcmp(args->arg_o(OPT_REPORT_TYPE), "command")==0)
	  rtype = UI_REPORT_COMMAND;
	else if(strcmp(args->arg_o(OPT_REPORT_TYPE), "fix")==0)
	  rtype = UI_REPORT_FIX;
	
	if(!ui_execute_report(fout, rf, hosts, checks, rtype,
			      args->fromtime(), args->untiltime(),
			      args->arg_o(OPT_REPORT_REVERSE), &err))
	  cout << "ERROR: " << err << endl;

	xdelete(fout);
      }
      else
	wlog->warn("Functionality::do_report failed to allocate FileHandler");
      
      xdelete(rf);
    }
    else
      wlog->warn("Functionality::do_report failed to allocate ReportFormatting");
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "Functionality::do_report()");
#endif
}

void Functionality::do_reschedule(Check *check, char *host)
{
  // Reschedule the Check <check> to run on the specified <host> the next
  // time the Scheduler loops.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "Functionality::do_reschedule(%d,%s)",
		   check, IONULL(host));
#endif

  if(check && host)
  {
    cout << "[" << IONULL(check->name()) << "@" << IONULL(host) << "]"
	 << flush;

    if(cf->runs_on_host(check->name(), host))
    {
      CheckState *cs = new CheckState(check, host);

      if(cs)
      {
	if(cs->reschedule())
	  cout << " Rescheduled" << endl;
	else
	  cout << " Reschedule FAILED" << endl;
	
	xdelete(cs);
      }
      else
	wlog->warn("Functionality::do_reschedule failed to allocate CheckState");
    }
    else
      cout << " ERROR: " << IONULL(check->name())
	   << " is not configured for " << host << endl;
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "Functionality::do_reschedule()");
#endif
}

void Functionality::do_status()
{
  // Obtain and display status information of the specified match type.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "Functionality::do_status()");
#endif

  matchstate_t mtype = match_all;  // We don't currently do anything for 'all'
  
  char *match = args->arg_o(OPT_STATUS_MATCH);

  if(match)
  {
    if(match[0]=='a' && match[1]=='d')
      mtype = match_addressed;
    else if(match[0]=='e' && match[1]=='r')
      mtype = match_error;
    else if(match[0]=='e' && match[1]=='s')
      mtype = match_escalated;
    else if(match[0]=='s' && match[1]=='t')
      mtype = match_stalled;
    
    if(mtype != match_all)
    {
      List *matching = ui_retrieve_matching_state(mtype);

      if(matching && matching->entries() > 0)
      {
	for(int i = 0;i < matching->entries();i++)
	{
	  SHPair *sh = new SHPair(matching->retrieve(i));

	  if(sh && sh->host() && sh->service())
	  {
	    Check *c = cf->find_check(sh->service());

	    if(c)
	    {
	      cout << "[" << matching->retrieve(i) << "] ";

	      switch(mtype)
	      {
	      case match_addressed:
		{
		  AlertState *as = new AlertState(c, sh->host());

		  if(as)
		  {
		    time_t tsrc;
		    char tbuf[26];
		    
		    if(as->acknowledged())
		    {
		      tsrc = as->acktime();
		      if(ctime_r(&tsrc, tbuf))
			tbuf[24] = '\0';  // Chop off the newline
		      		      
		      cout << "Acknowledged by " << as->acknowledged()
			   << " at " << tbuf;
		      
		      if(as->inhibited())
			cout << ", ";
		    }

		    if(as->inhibited())
		    {
		      tsrc = as->inhibittime();
		      if(ctime_r(&tsrc, tbuf))
			tbuf[24] = '\0';  // Chop off the newline
		      		      
		      cout << "Inhibited by " << as->inhibited()
			   << " at " << tbuf;
		    }
		    
		    xdelete(as);
		  }
		}
		break;
	      case match_error:
		{
		  CheckState *cs = new CheckState(c, sh->host());

		  if(cs)
		  {
		    cout << ui_rc_to_text(cs->returncode()) << ": "
			 << cs->comment();
		    
		    xdelete(cs);
		  }
		}
		break;
	      case match_escalated:
		cout << "Currently escalated";
		break;
	      case match_stalled:
		cout << "Check stalled";
		break;
	      }
	      
	      cout << endl;
	    }
	  }

	  xdelete(sh);
	}
      }
      else
	cout << "No matching state found" << endl;

      xdelete(matching);
    }
    else
      cout << "ERROR: Unknown match type '" << match << "'" << endl;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "Functionality::do_status()");
#endif
}

void Functionality::do_status(Check *check, char *host)
{
  // Obtain and display status information for the Check <check> on the
  // specified <host>.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "Functionality::do_status(%d,%s)",
		   check, IONULL(host));
#endif

  if(check && host)
  {
    cout << "[" << IONULL(check->name()) << "@" << IONULL(host) << "]" << endl;

    if(cf->runs_on_host(check->name(), host))
    {
      CheckState *cs = new CheckState(check, host);
      AlertState *as = new AlertState(check, host);
      FixState *fs = new FixState(check, host);

      if(cs && as && fs)
      {
	time_t t = cs->lastcheck();
	time_t st = cs->since();
	
	if(ui_rc_to_text(cs->returncode(), t, cs->consecutive()))
	  cout << " " << ui_rc_to_text(cs->returncode(), t, cs->consecutive());
	else
	  cout << " Error return code " << cs->returncode();

	if(t > 0)
	  cout << " (" << cs->consecutive()
	       << (char*)(cs->consecutive() == 1 ? " instance)" : " instances)")
	       << endl;
	else
	  cout << endl;

	if(cs->comment())
	  cout << " -> Comment: " << cs->comment() << endl;

	if(t > 0)
	  cout << " -> Execution duration: " << cs->duration() << "ms" << endl;
	
	cout << " -> Last check: ";
	if(t > 0)
	{
	  cout << ctime(&t);  // ctime puts \n
	
	  if(st > 0)
	    cout << " -> First check (this result): " << ctime(&st);
	  // ctime puts \n
	}
	else
	  cout << "This check has not yet successfully executed" << endl;
	      
	Array<Schedule> *csched = check->check_schedule();
	
	if(!csched)
	{
	  HostClass *hc = cf->find_class(host);
	  
	  if(hc)
	    csched = hc->check_schedule();
	}
	
	time_t nt = next_schedule_time(csched, cs->lastcheck());
	
	cout << " -> Approximate next check: ";
	
	if(nt > 0)
	  cout << ctime(&nt) << flush;  // ctime puts \n
	else
	  cout << "Unable to calculate time" << endl;
      
	if(as->inhibited())
	{
	  time_t tt = as->inhibittime();
	  
	  cout << " => Alerts have been inhibited by " << as->inhibited()
	       << " since " << ctime(&tt);  // ctime puts \n
	  
	  if(as->inhibited_for())
	    cout << " => Reason: " << as->inhibited_for() << endl;
	}
	else
	{
	  time_t ta = as->lastalert();
	  time_t sta = as->since();
	
	  if(as->escalated_manual() > 0)
	    cout << " => Manually escalated to " << as->escalated_manual()
		 << " tries" << endl;
	  
	  // Determine which AlertPlan is in effect in order to determine
	  // escalation status.
	
	  AlertPlan *ap = check->alert_plan();
	
	  if(!ap)
	  {
	    HostClass *hc = cf->find_class(host);
	    
	    if(hc)
	      ap = hc->alert_plan();
	  }
	
	  if(ap && as->escalated(ap, cs))
	    cout << " => Alert has automatically escalated" << endl;
	  
	  if(as->alerts() > 0)
	    cout << " => " << as->alerts()
		 << (as->alerts() == 1 ? " alert has" : " alerts have")
		 << " been transmitted" << endl;

	  if(sta > 0)
	    cout << " => First alert transmitted at " << ctime(&sta);
	  // ctime puts \n
	
	  RecipientSet *rset = as->lastnotify();
	
	  if(rset)
	  {
	    for(int i = 0;i < rset->modules();i++)
	      cout << " => " << IONULL(rset->addresses(i))
		   << " notified via " << IONULL(rset->module(i))
		   << " at " << ctime(&ta);
	    // ctime puts \n
	  }
	
	  if(as->acknowledged())
	  {
	    time_t tt = as->acktime();
	  
	    cout << " => This problem was acknowledged by "
		 << as->acknowledged() << " at " << ctime(&tt);
	    // ctime puts \n
	    
	    if(as->acknowledged_for())
	      cout << " => Reason: " << as->acknowledged_for() << endl;
	  }
	}

	if(fs->fix_attempts() > 0)
	{
	  time_t tf = fs->lastfix();
	  time_t stf = fs->since();
	  
	  cout << " +> Fix attempted by " << IONULL(fs->lastfix_by()) << " at "
	       << ctime(&tf);
	  // ctime puts \n
	  
	  cout << " +> Fix result: ";
	
	  if(ui_rc_to_text(fs->returncode()))
	    cout << ui_rc_to_text(fs->returncode());
	  else
	    cout << " Error return code " << fs->returncode();
	
	  cout << ": " << IONULL(fs->comment()) << endl;
	
	  if(fs->fix_attempts() == 1)
	    cout << " +> There has been 1 fix attempt" << endl;
	  else
	    cout << " +> There have been " << fs->fix_attempts()
		 << " fix attempts" << endl;

	  if(stf > 0)
	    cout << " => First fix attempt at " << ctime(&stf);
	  // ctime puts \n

	}
      }
      else
	wlog->warn("Functionality::do_status failed to allocate State objects");
      
      xdelete(as);
      xdelete(cs);
      xdelete(fs);
    }
    else
      cout << " ERROR: " << check->name() << " not configured for " << host
	   << endl;
    
    cout << endl;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "Functionality::do_status()");
#endif
}

void Functionality::do_trip(Check *check, char *host)
{
  // Execute the 'trip' command for <check>@<host>, tripping an
  // alert with a return code and comment specified by -o rc=X and
  // -c, respectively.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "Functionality::do_trip(%d,%s)",
		  check, IONULL(host));
#endif

  if(check && host)
  {
    if(cf->runs_on_host(check->name(), host))
    {
      char *rc = args->arg_o(OPT_TRIP_RC);
      char *comment = args->comment();

      // The chars above are just pointers, so we can point them to
      // static strings.  (ie: No cleanup needs to happen here.)
      
      if(!rc)
	rc = "1";
      
      if(!comment)
	comment = "Alert manually tripped";
      
      cout << "[" << IONULL(check->name()) << "@" << IONULL(host) << "]"
	   << flush;
      
      // We create a Type I CheckState even though a Type II
      // CheckState seems more logical.  This is because we are
      // passing a fabricated result to parse_results, which only
      // works with Type I.

      CheckState *cs = new CheckState(check);

      if(cs)
      {
	bool cok = false;    // Whether we've handed off carr
	
	Array<CheckResult> *carr = new Array<CheckResult>();

	if(carr)
	{
	  CheckResult *cr = new CheckResult(host, atoi(rc), 0, comment);

	  if(cr)
	  {
	    if(carr->add(cr))
	    {
	      if(cs->parse_results(carr))
	      {
		cok = true;  // We're no longer responsible for carr

		if(cs->write_results())
		  cout << " Tripped" << endl;
		else
		  cout << " Trip write FAILED" << endl;
	      }
	      else
		cout << " Trip parse FAILED" << endl;
	    }
	    else
	      xdelete(cr);
	  }
	  else
	    wlog->warn("Functionality::do_trip failed to allocate CheckResult");

	  if(!cok)
	    xadelete(carr, CheckResult);
	}
	else
	  wlog->warn("Functionality::do_trip failed to allocate Array");
	
	xdelete(cs);
      }
      else
	wlog->warn("Functionality::do_trip failed to allocate CheckState");
    }
    else
      cout << " ERROR: " << IONULL(check->name())
	   << " is not configured for " << host << endl;
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "Functionality::do_trip()");
#endif
}

void Functionality::do_unacknowledge(Check *check, char *host)
{
  // Unacknowledge the acknowledgement for the outstanding Alert (if
  // there is one) for the service <check> on the specified <host>.
  // This will also reschedule the check.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "Functionality::do_unacknowledge(%d,%s)",
		   check, IONULL(host));
#endif

  if(check && host)
  {
    cout << "[" << IONULL(check->name()) << "@" << IONULL(host) << "]"
	 << flush;

    if(cf->runs_on_host(check->name(), host))
    {
      char *err = NULL;

      if(ui_unquiet_resched(check, host, false, username(),
			    MAYBE_EXCUSE(args->comment()), &err))
	cout << " Unacknowledged" << endl;
      else
	cout << " " << err << endl;
      
      xdelete(err);
    }
    else
      cout << " ERROR: " << IONULL(check->name())
	   << " is not configured for " << host << endl;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "Functionality::do_unacknowledge()");
#endif
}

void Functionality::do_uninhibit(Check *check, char *host)
{
  // Remove the inhibition for Alerts for the service <check> on the
  // specified <host>.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "Functionality::do_uninhibit(%d,%s)",
		   check, IONULL(host));
#endif

  if(check && host)
  {
    cout << "[" << IONULL(check->name()) << "@" << IONULL(host) << "]"
	 << flush;

    if(cf->runs_on_host(check->name(), host))
    {
      char *err = NULL;

      if(ui_unquiet_resched(check, host, true, username(),
			    MAYBE_EXCUSE(args->comment()), &err))
	cout << " Uninhibited" << endl;
      else
	cout << " " << err << endl;
      
      xdelete(err);
    }
    else
      cout << " ERROR: " << IONULL(check->name())
	   << " is not configured for " << host << endl;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "Functionality::do_uninhibit()");
#endif
}

void Functionality::report_substitution(Substitution *sub)
{
  // Generate output describing the Substitution <sub>

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "Functionality::report_substitution(%d)", sub);
#endif

  if(sub)
  {
    char buf[26];
    time_t t;
		  
    cout << " *> " << IONULL(sub->newname()) << " replaces "
	 << IONULL(sub->oldname()) << " from ";
		  
    t = sub->begins();
    ctime_r(&t, buf);
    buf[24] = '\0';
		  
    cout << buf << " until ";
		
    t = sub->ends();
    ctime_r(&t, buf);
    buf[24] = '\0';
    
    cout << buf << endl;
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "Functionality::report_substitution()");
#endif
}

char *Functionality::username()
{
  // Obtain the current user.

  // Returns: The current user, or NULL on error.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "Functionality::username()");
#endif

  char *ret = NULL;
  
  if(!pwr)
  {
    if(getpwuid_r(getuid(), &pw, pwbuf, BUFSIZE, &pwr)!=0)
      wlog->warn("getpwuid_r failed in Functionality::username");
  }

  if(pwr)
    ret = pwr->pw_name;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "Functionality::username = %s", IONULL(ret));
#endif

  return(ret);
}
