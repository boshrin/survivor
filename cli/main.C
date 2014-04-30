/*
 * survivor command line interface
 *
 * Version: $Revision: 0.30 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2006/11/13 00:09:11 $
 * MT-Level: Safe
 *
 * Copyright (c) 2002 - 2006
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: main.C,v $
 * Revision 0.30  2006/11/13 00:09:11  benno
 * Add clset to usage
 *
 * Revision 0.29  2006/10/14 03:07:30  benno
 * Exit with useful return code
 *
 * Revision 0.28  2006/10/10 13:17:02  benno
 * Add signal catcher
 *
 * Revision 0.27  2005/11/24 18:27:12  benno
 * Add status_cmd with OPT_STATUS_MATCH
 *
 * Revision 0.26  2005/04/09 03:04:30  benno
 * Add test=fqdn
 *
 * Revision 0.25  2004/08/25 02:02:56  benno
 * Add -o reverse, report
 *
 * Revision 0.24  2004/06/20 01:05:39  benno
 * Add clunsub
 *
 * Revision 0.23  2004/06/12 02:16:40  benno
 * Ack/Unack take multiple args
 *
 * Revision 0.22  2004/04/24 14:39:24  toor
 * Fix minor memory leak
 *
 * Revision 0.21  2003/11/29 05:41:35  benno
 * Add -o options
 * Use CommandInfo to dispatch commands
 * Overhaul command processing
 *
 * Revision 0.20  2003/05/29 00:42:27  benno
 * Add dtest alert
 *
 * Revision 0.19  2003/05/04 21:43:38  benno
 * Add dtest
 *
 * Revision 0.18  2003/04/21 20:26:57  benno
 * Add -L
 *
 * Revision 0.17  2003/04/09 20:12:13  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.16  2003/04/07 20:57:40  benno
 * Use Debugger
 *
 * Revision 0.15  2003/04/07 20:35:30  benno
 * Better error reporting if -i isn't provided when required
 *
 * Revision 0.14  2003/03/02 05:10:53  benno
 * Support DEBUG_CFERRS
 *
 * Revision 0.13  2002/12/31 04:43:34  benno
 * Add fix support
 *
 * Revision 0.12  2002/12/16 01:07:58  benno
 * Add comments for ack/inhibit
 *
 * Revision 0.11  2002/05/31 21:39:43  benno
 * make clstat command work again
 *
 * Revision 0.10  2002/05/24 20:55:19  benno
 * add -V
 *
 * Revision 0.9  2002/04/26 20:07:31  toor
 * call args->parse_instcf for better error information
 *
 * Revision 0.8  2002/04/04 19:56:20  benno
 * copyright
 *
 * Revision 0.7  2002/04/02 21:07:15  benno
 * rcsify date
 *
 * Revision 0.6  2002/04/02 21:07:05  benno
 * Use libsrvinit/exit
 *
 * Revision 0.5  2002/04/02 21:06:25  benno
 * Rewrite command parsing
 *
 * Revision 0.4  2002/04/02 21:05:54  benno
 * Add clsub command
 *
 * Revision 0.3  2002/04/02 21:05:14  benno
 * Add trip command
 *
 * Revision 0.2  2002/04/02 21:04:35  benno
 * Use Instances
 *
 * Revision 0.1  2002/04/02 21:03:35  benno
 * initial revision
 *
 */

#include "cli.H"

/*
 * Global Definitions
 *
 */

pid_t cppid = 0;       // Pid of current check process running from do_check()
bool abortcmd = false; // When running a series of checks, abort on ctrl-z

/*
 * usage()
 *
 */

void usage()
{
  cerr << "Usage: " << progname << " [-sV] [-c comment] [-L logmethod] "
#if defined(DEBUG)
       << "[-d debuglevel] "
#endif
       << "[-f fromtime] [-i instance] [-I instcf] [-m moddir] [-o commandopts] [-u untiltime] command [commandargs]"
       << endl;
  
  cerr << "where: -s timestamps each line of debugging or warning output"
       << endl
       << "       -V outputs the package version information and exits"
       << endl
       << "       -c specifies a comment for acknowledge or inhibit" << endl
       << "       -L specifies logging method (std, syslog, none)" << endl
#if defined(DEBUG)
       << "       -d generates debugging output at <debuglevel>" << endl
#endif
       << "       -f specifies from time (for history or clsub/clunsub) in form [HHMM]MMDDYYYY"
       << endl
       << "       -i specifies to use <instance>" << endl
       << "       -I specifies where to find instance.cf" << endl
       << "       -m specifies <moddir> as the module directory" << endl
       << "       -u specifies until time (for history or clsub/clunsub) in form [HHMM]MMDDYYYY"
       << endl
       << endl
       << "and command is one of (only first unique letter(s) are required): "
       << endl << endl
       << "       status: Get current status for service, host, service@host"
       << endl
       << "       check: Immediately recheck service, host, service@host"
       << " (no state update)" << endl
       << "       reschedule: Reschedule service, host, service@host"
       << " (state update on run)" << endl
       << "       acknowledge: Acknowledge current alert for service, host, service@host"
       << endl
       << "       unacknowledge: Undo acknowledge for service, host, service@host" << endl
       << "       escalate: Escalate current alert for service@host" << endl
       << "       inhibit: Silence all alerts for service, host, service@host"
       << endl
       << "       uninhibit: Undo inhibit for service, host, service@host"
       << endl
       << "       fix: Run predefined fix for service@host" << endl
       << "       alerthistory: Obtain alert history for service, host, service@host"
       << endl
       << "       checkhistory: Obtain check history for service, host, service@host"
       << endl
       << "       commandhistory: Obtain command history for service, host, service@host"
       << endl
       << "       fixhistory: Obtain fix history for service, host, service@host"
       << endl
       << "        Use -o reverse to obtain most recent history first "
       << endl
       << "       report: Run report module, -o module=<module> " << endl
       << "        -o type=[alert|check|command|fix]" << endl
       << "        Use -o reverse to obtain most recent history first "
       << endl
       << "       archivehistory: service, host, service@host or -o [all|stale]"
       << endl
       << "        and -o directory=<destination> or file=<destination>"
       << endl
       << "       trip: Set an error status: service@host" << endl
       << "        (use -o rc=<return code>)" << endl
#if defined(DEBUG)
       << "       dtest: Run debug test, -o test=reparse, -o test=fqdn, or "
       << endl
       << "         -o test=alert,module=<module>,address=<address>" << endl
#endif
       << "       clstat: Get current status for calllist" << endl
       << "       clcal: See calendar for calllist" << endl
       << "       clprune: Delete old substitutions for calllist" << endl
       << "       clset: Set currently on call person, -o person=<person>"
       << endl
       << "       clsub: Set substitution for calllist" << endl
       << "        (use -o replace=<oldname>,with=<newname>)" << endl
       << "        -f and -u are required for clsub" << endl
       << "       clunsub: Remove substitutions for calllist" << endl
       << "        -f and -u are required for clunsub"
       << endl;
}

/*
 * signal handler
 *
 */

void signal_catcher(int i)
{
  // If we receive a signal (we might get SIGINT (^C) or SIGTSTP (^Z))
  // and cppid is non-zero, terminate cppid.

  if(cppid > 0)
  {
    kill(cppid, SIGKILL);   // No need to be polite
    cppid = 0;
  }

  // If that signal was SIGTSTP, stop the command.

  if(i == SIGTSTP)
    abortcmd = true;
}

/*
 * main()
 *
 */

int main(int argc, char **argv)
{
  int r = 1;
  
  // Global setup

  if(!libsrvinit(argv[0]))
    exit(1);
  
  // Parse args

  args = new Args();

  if(args)
  {
    // The suboptions accepted by -o.  This is all possible options,
    // even though they should really be dependent on the command
    // being run.  (We don't know the command yet, so we can't limit
    // the array.)  The order doesn't matter since the parsed entries
    // are stored in a hashtable, keyed by name.
    
    char *oopts[] = {
      OPT_ARCHIVEHISTORY_ALL,
      OPT_ARCHIVEHISTORY_DIRECTORY,
      OPT_ARCHIVEHISTORY_FILE,
      OPT_ARCHIVEHISTORY_STALE,
      OPT_CLCAL_PERSON,
      OPT_CLSUB_REPLACE,
      OPT_CLSUB_WITH,
#if defined(DEBUG)
      OPT_DTEST_ADDRESS,
      OPT_DTEST_MODULE,
      OPT_DTEST_TEST,
#endif
      OPT_HISTORY_REVERSE,
      OPT_REPORT_MODULE,
      OPT_REPORT_REVERSE,
      OPT_REPORT_TYPE,
      OPT_STATUS_MATCH,
      OPT_TRIP_RC,
      NULL
    };
    
    if(!args->parse_args(
#if defined(DEBUG)
			 "c:d:f:i:I:L:m:o:su:V",
#else
			 "c:f:i:I:L:m:o:su:V",
#endif
			 oopts, argc, argv))
    {
      wlog->warn("Argument parse failed");
      usage();
      
#if defined(DEBUG)
      dlog->log_exit(DEBUG_MAJTRC, "main = 1");
#endif

      libsrvexit();
      exit(1);
    }

    // If we're just displaying the version, we don't have to parse anything.
    if(args->version())
    {
      cout << args->version(build_version) << endl;
      
      libsrvexit();
      exit(0);
    }
    
    if(!args->parse_instcf())
    {
      if(argc < 2)  // Dump usage instead, since user probably forget -i
	usage();
      else
      {
#if defined(DEBUG)
	wlog->warn("Instance configuration parse failed (use -d %d, %d, or %d for details)",
		   DEBUG_CFERRS, (DEBUG_CONFIG + DEBUG_CFLESS), DEBUG_CONFIG);
#else
	wlog->warn("Instance configuration parse failed");
#endif
      }
      
#if defined(DEBUG)
      dlog->log_exit(DEBUG_MAJTRC, "main = 1");
#endif

      libsrvexit();
      exit(1);
    }

    // At this point, we can do a root check

    if(!args->cli_allow_root() && getuid() == 0)
    {
      wlog->warn("Instance %s does not permit root to run the command interface",
		 args->instname());
      
      libsrvexit();
      exit(1);
    }
    
    cf = new Configuration();
    
    if(cf)
    {
      // Read configuration files
      
      if(cf->parse_cfs())
      {
	Functionality *f = new Functionality();

	if(f)
	{
	  extern int optind;

	  if(optind < argc)
	  {
	    // First, determine which command to run.

	    char *cmd = argv[optind];
	    int cmdlen = strlen(cmd);
	    CommandInfo *ci = NULL;

	    for(int i = 0;i < commandslen;i++)
	    {
	      ci = commands[i];
	      
	      if(cmdlen >= ci->minlen && cmdlen <= strlen(ci->name)
		 && strncasecmp(cmd, ci->name, cmdlen)==0)
		break;
	      else
		ci = NULL;  // reset so we don't default to the last entry
	    }

	    if(ci)
	    {
	      bool prereqok = true;
	      
	      // Next, validate any prerequisites for the command

	      if(ci->comments && args->cli_require_comment()
		 && !args->comment())
	      {
		// If comments are required, one must be provided
		
		wlog->warn("Instance %s requires a comment with this command (use -c)",
			   args->instname());
		
		prereqok = false;
	      }

	      if(strcmp(ci->name, archivehistory_cmd.name)==0)
	      {
		// Archiving requires directory or file (and not both).
		// If no argv args are left, then other, non mutually
		// exclusive options are also permitted.

		if(!args->cli_allow_archive())
		{
		  wlog->warn("Instance %s does not permit the command %s",
			     args->instname(), ci->name);
		  
		  prereqok = false;
		}
		if((!args->arg_o(OPT_ARCHIVEHISTORY_FILE) &&
		    !args->arg_o(OPT_ARCHIVEHISTORY_DIRECTORY))
		   ||
		   (args->arg_o(OPT_ARCHIVEHISTORY_FILE) &&
		    args->arg_o(OPT_ARCHIVEHISTORY_DIRECTORY))
		   ||
		   (args->arg_o(OPT_ARCHIVEHISTORY_FILE) &&
		    strlen(args->arg_o(OPT_ARCHIVEHISTORY_FILE))==0)
		   ||
		   (args->arg_o(OPT_ARCHIVEHISTORY_DIRECTORY) &&
		    strlen(args->arg_o(OPT_ARCHIVEHISTORY_DIRECTORY))==0))
		{
		  wlog->warn("Command %s requires suboption -o %s=x or %s=x",
			     ci->name, OPT_ARCHIVEHISTORY_FILE,
			     OPT_ARCHIVEHISTORY_DIRECTORY);
		  
		  prereqok = false;
		}
		else
		{
		  // Count to make sure args aren't overspecified

		  int ocnt = 0;
		  
		  if(args->arg_o(OPT_ARCHIVEHISTORY_ALL))
		    ocnt++;
		  if(args->arg_o(OPT_ARCHIVEHISTORY_STALE))
		    ocnt++;

		  if(optind + 1 == argc && ocnt != 1)
		  {
		    wlog->warn("Command %s requires one suboption -o (%s or %s) without host, service, service@host",
			       ci->name, OPT_ARCHIVEHISTORY_ALL,
			       OPT_ARCHIVEHISTORY_STALE);
		  
		    prereqok = false;
		  }
		  else if(optind + 1 < argc && ocnt > 0)
		  {
		    // This is mostly for clearer semantics, since
		    // "-o stale archivehistory foo@bar" isn't
		    // necessarily contradictory under all
		    // interpretations.
		    
		    wlog->warn("Command %s does not accept suboption -o %s or %s with host, service, service@host",
			       ci->name, OPT_ARCHIVEHISTORY_ALL,
			       OPT_ARCHIVEHISTORY_STALE);
		  
		    prereqok = false;
		  }
		}
	      }
	      else if(strcmp(ci->name, clsub_cmd.name)==0
		      &&
		      (!args->arg_o(OPT_CLSUB_REPLACE)
		       || !args->arg_o(OPT_CLSUB_WITH)))
	      {
		wlog->warn("Command %s requires suboptions -o %s=x,%s=y",
			   ci->name, OPT_CLSUB_REPLACE, OPT_CLSUB_WITH);

		prereqok = false;
	      }
	      else if(strcmp(ci->name, report_cmd.name)==0
		      &&
		      (!args->arg_o(OPT_REPORT_MODULE)
		       || !args->arg_o(OPT_REPORT_TYPE)))
	      {
		wlog->warn("Command %s requires suboptions -o %s=x,%s=y",
			   ci->name, OPT_REPORT_MODULE, OPT_REPORT_TYPE);

		prereqok = false;
	      }
	      else if(strcmp(ci->name, status_cmd.name)==0
		      &&
		      (args->arg_o(OPT_STATUS_MATCH) && optind + 1 < argc))
	      {
		// Status can be given a match arg or shpairs, but not both

		wlog->warn("Command %s cannot use %s with host, service, service@host",
			   ci->name, OPT_STATUS_MATCH);
		
		prereqok = false;
	      }
	      
	      if(optind + 1 + ci->minargs > argc ||
		 (ci->maxargs > -1
		  && ((argc - optind - 1) > ci->maxargs)))
	      {
		// Make sure we got the right number of args

		usage();
		prereqok = false;
	      }
	      
	      // Finally, loop over the arguments for the command.
	      // We do this regardless of minargs and maxargs because
	      // the loop will break properly now that we've validated
	      // them.

	      if(prereqok)
	      {
		if(ci->maxargs == 0 || ci->argtype == no_args ||
		   (ci->minargs == 0 && optind + 1 == argc))
		  f->dispatch_noarg(ci);
		else if(ci->argtype == sh_list_args)
		  f->dispatch_listarg(ci, optind + 1, argc, argv);
		else
		{
		  for(int i = optind + 1;i < argc;i++)
		    f->dispatch_arg(ci, argv[i]);
		}

		// We assume successful execution.

		r = 0;
	      }
	    }
	    else
	    {
	      wlog->warn("Unknown command: %s", cmd);
	      usage();
	    }
	  }
	  else
	  {
	    wlog->warn("No command provided");
	    usage();
	  }

	  xdelete(f);
	}
	else
	  wlog->warn("Failed to allocate Functionality");
      }
      else
      {
#if defined(DEBUG)
	wlog->warn("Instance configuration parse failed (use -d %d, %d, or %d for details)",
		   DEBUG_CFERRS, (DEBUG_CONFIG + DEBUG_CFLESS), DEBUG_CONFIG);
#else
	wlog->warn("Instance configuration parse failed");
#endif
      }
    }
    else
      wlog->warn("Unable to allocate Configuration object");
  }
  else
    wlog->warn("Unable to allocate Args object");
  
  // Clean up and exit

#if defined(DEBUG)
  dlog->log_progress(DEBUG_MAJTRC, "Cleaning up in preparation for exit");
  dlog->log_exit(DEBUG_MAJTRC, "main = 0");
#endif

  libsrvexit();

  exit(r);
}
