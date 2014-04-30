/*
 * survivor scheduler
 *
 * Version: $Revision: 0.16 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2005/08/06 03:45:13 $
 * MT-Level: Safe
 *
 * Copyright (c) 2002 - 2005
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: main.C,v $
 * Revision 0.16  2005/08/06 03:45:13  benno
 * Fix cleanup ordering bug causing crash on exit
 *
 * Revision 0.15  2004/03/01 22:20:04  benno
 * Add -S
 *
 * Revision 0.14  2003/10/06 21:58:41  benno
 * Fix another libsrvexit in the wrong place
 *
 * Revision 0.13  2003/07/24 16:33:56  benno
 * Call libsrvexit after any debugging calls
 *
 * Revision 0.12  2003/04/13 19:56:27  benno
 * Add -L logmethod
 * Add broken_vsnprintf stuff
 * Enable tid in logging
 *
 * Revision 0.11  2003/04/09 19:50:04  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.10  2003/03/31 12:47:51  benno
 * Add StateCache
 *
 * Revision 0.9  2003/03/14 14:00:42  benno
 * Fix signal handling in keepalive
 *
 * Revision 0.8  2003/03/02 14:46:16  benno
 * BSD_SIGNALS
 * xdelete
 *
 * Revision 0.7  2003/01/02 04:43:21  benno
 * Throttle keepalive restarting
 *
 * Revision 0.6  2002/05/24 20:55:11  benno
 * add -V
 *
 * Revision 0.5  2002/04/26 20:16:24  toor
 * call args->parse_instcf for better error information
 * add BROKEN_THREAD_SIGNALS
 *
 * Revision 0.4  2002/04/03 19:16:54  benno
 * rcsify date
 *
 * Revision 0.3  2002/04/03 19:16:45  benno
 * Use libsrvinit/exit
 *
 * Revision 0.2  2002/04/03 19:16:16  benno
 * Use -iI instead of -chs
 *
 * Revision 0.1  2002/04/03 19:15:33  benno
 * initial revision
 *
 */

#include "scheduler.H"

/*
 * Global Definitions
 *
 */

LockManager *locks = NULL;          // Global lock manager object
StateCache *scache = NULL;          // Global state cache
StatCounter *statc = NULL;          // Global stat counter
int kasignal = 0;                   // For use between main and handler only
#if defined(_BROKEN_THREAD_SIGNALS)
pthread_t glsigtid = 0;             // For use by extra_catcher
#endif // _BROKEN_THREAD_SIGNALS

/*
 * usage()
 *
 */

void usage()
{
  cerr << "Usage: " << progname << " [-DksSV] [-L logmethod] "
#if defined(DEBUG)
       << "[-d debuglevel] "
#endif
       << "[-i instance] [-I instcf] [-m moddir] [-p pidfile] "
       << "[-t cthreads] [-T athreads]"
       << endl;
  
  cerr << "where: -D specifies to not fork into the background" << endl
       << "       -k specifies to automatically restart on unexpected exit"
       << endl
       << "       -s timestamps each line of debugging or warning output"
       << endl
       << "       -S disables smart scheduling" << endl
       << "       -V outputs the package version information and exits" << endl
       << "       -L specifies logging method (std, syslog, none)" << endl
#if defined(DEBUG)
       << "       -d generates debugging output at <debuglevel>" << endl
#endif
       << "       -i specifies to use <instance>" << endl
       << "       -I specifies where to find instance.cf" << endl
       << "       -m specifies <moddir> as the module directory" << endl
       << "       -p specifies where to write the <pidfile>" << endl
       << "       -t specifies the number of check threads to create" << endl
       << "       -T specifies the number of alert threads to create" << endl;
}

/*
 * main()
 *
 */

int main(int argc, char **argv)
{
  // Global setup

  if(!libsrvinit(argv[0]))
    exit(1);

  // Parse args

  args = new Args();

  if(args)
  {
    if(!args->parse_args(
#if defined(DEBUG)
			 "Dd:i:I:kL:m:p:sSt:T:V",
#else
			 "Di:I:kL:m:p:sSt:T:V",
#endif
			 argc, argv))
    {
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
#if defined(DEBUG)
      wlog->warn("Instance configuration parse failed (use -d %d, %d, or %d for details)",
		 DEBUG_CFERRS, (DEBUG_CONFIG + DEBUG_CFLESS), DEBUG_CONFIG);
#else
      wlog->warn("Instance configuration parse failed");
#endif
      
#if defined(DEBUG)
      dlog->log_exit(DEBUG_MAJTRC, "main = 1");
#endif
      
      exit(1);
    }
    
    if(args->background())
    {
      // Detach from the tty
      
#if defined(DEBUG)
      dlog->log_progress(DEBUG_CONFIG, "backgrounding");
#endif
      
      pid_t child = fork();
      
      switch(child)
      {
      case 0:
	// We are the child, just keep going
	break;
      case -1:
	// Error
	wlog->warn("Unable to fork into background.  Exiting.");
#if defined(DEBUG)
	dlog->log_exit(DEBUG_MAJTRC, "main = 2");
#endif
	exit(2);
	break;
      default:
	// Parent, exit
#if defined(DEBUG)
	dlog->log_progress(DEBUG_CONFIG, "Child process is %d", child);
	dlog->log_exit(DEBUG_MAJTRC, "main (parent backgrounding) = 0");
#endif
	exit(0);
      }
    }

    // Write the pidfile if requested
      
    if(args->pidfile())
    {
      ofstream pidout(args->pidfile());
      
      if(!pidout)
	wlog->warn("Unable to write pid to %s", args->pidfile());
      else
      {
	pidout << getpid() << endl;
	
#if defined(DEBUG)
	dlog->log_progress(DEBUG_FILEIO, "pid written to %s", args->pidfile());
#endif
      }
    }

    // If we are to keepalive, start from here.  pidfile will refer to
    // the monitor, not the running process.  Keepalive only works
    // when backgrounding.

    if(args->keepalive() && args->background())
    {
      bool done = false;
      pid_t child = -1;

      // Note how often we restart, and exit if we're restarting too
      // frequently.
      time_t laststart = 0;

      while(!done)
      {
	if(kasignal == 0)
	{
	  // Only start a new child if we weren't interrupted in this loop
	  // by a signal

	  struct timeval tv;
	  
	  // If gettimeofday is failing, we probably don't want to start
	  // the scheduler up anyway.
	  gettimeofday(&tv, NULL);

	  if(tv.tv_sec - laststart < 60)
	  {
	    // Exit if we've restarted within the last minute.  This should
	    // prevent looping schedulers on configuration error, etc.
	    
	    wlog->warn("Keepalive process is restarting the scheduler too frequently.");
	    wlog->warn("There may be a configuration file error or some other problem.");
	    wlog->warn("Keepalive process is exiting as a precaution.");
	    
#if defined(DEBUG)
	    dlog->log_exit(DEBUG_MAJTRC, "main (keepalive) = 1");
#endif
	    
	    exit(1);
	  }
	  else
	    laststart = tv.tv_sec;
	  
	  child = fork();
	  
	  switch(child)
	  {
	  case 0:
	    // We are the child
	    done = true;
	    break;
	  case -1:
	    // Error
	    wlog->warn("Keepalive process failed to restart scheduler, trying again...");
	    sleep(15);
	    break;
	  default:
	    // We are the parent
#if defined(DEBUG)
	    dlog->log_progress(DEBUG_MAJTRC,
			       "Keepalive process starting a new scheduler");
#endif
	    break;
	  }
	}

	if(child > 0)
	{
	  // Set up signals we listen for, reset kasignal

#if defined(_BSD_SIGNALS)
	  signal(SIGHUP, keepalive_catcher);
	  signal(SIGINT, keepalive_catcher);
	  signal(SIGQUIT, keepalive_catcher);
	  signal(SIGTERM, keepalive_catcher);
#else	  
	  sigset(SIGHUP, keepalive_catcher);
	  sigset(SIGINT, keepalive_catcher);
	  sigset(SIGQUIT, keepalive_catcher);
	  sigset(SIGTERM, keepalive_catcher);
#endif // _BSD_SIGNALS
	  kasignal = 0;
	  
	  if(waitpid(child, NULL, 0) == -1)
	  {
	    // We were probably interrupted by a signal
	  
	    switch(kasignal)
	    {
	    case 0:
	      break;
	    case SIGHUP:
	      // pass the signal to the child;
#if defined(DEBUG)
	      dlog->log_progress(DEBUG_MAJTRC, "Sending sighup to child");
#endif
	      kill(child, kasignal);
	      break;
	    default:
	      // exit on anything else, and tell the child to, also
#if defined(DEBUG)
	      dlog->log_progress(DEBUG_MAJTRC, "Sending signal %d to child",
				 kasignal);
#endif
	      kill(child, kasignal);
	      done = true;
	      break;
	    }
	  }
	  else           // Wait ten seconds before restarting to prevent
	    sleep(10);   // Running away

	  // Release signals so as not to interfere with the child process

#if defined(_BSD_SIGNALS)
	  signal(SIGHUP, SIG_DFL);
	  signal(SIGINT, SIG_DFL);
	  signal(SIGQUIT, SIG_DFL);
	  signal(SIGTERM, SIG_DFL);
#else
	  sigrelse(SIGHUP);
	  sigrelse(SIGINT);
	  sigrelse(SIGQUIT);
	  sigrelse(SIGTERM);
#endif // _BSD_SIGNALS
	}
	// else let the child break the loop and fall through
      }

      if(child > 0)
      {
	// If we are the parent and we make it here, exit.

#if defined(DEBUG)
	dlog->log_exit(DEBUG_MAJTRC, "main (keepalive) = 0");
#endif
	libsrvexit();
	
	exit(0);
      }
    }
    
    cf = new Configuration();
    
    if(cf)
    {
      // Read configuration files
      
      if(cf->parse_cfs())
      {
	if(cf->state_consistency())
	{
	  // Initialize the lock manager and state cache, and if smart
	  // scheduling is enabled, also the stat counter.
	  
	  locks = new LockManager();
	  scache = new StateCache();

	  if(args->smartscheduling())
	  {
#if defined(DEBUG)
	    dlog->log_progress(DEBUG_SCHEDS, "Enabling smart scheduling");
#endif
	    statc = new StatCounter();

	    if(statc)
	    {
	      if(!statc->analyze_configuration())
	      {
		wlog->warn("StatCounter configuration analysis failed");
		xdelete(statc);
	      }
	    }
	    else
	      wlog->warn("Failed to allocate StatCounter");
	  }
	    
	  if(locks && locks->ok() && scache && scache->enable())
	  {
	    // Raise file descriptor ulimit
	    
	    if(getuid()==0)
	    {
	      struct rlimit rl;
	      rl.rlim_cur = RLIM_INFINITY;
	      rl.rlim_max = RLIM_INFINITY;
	      
	      if(setrlimit(RLIMIT_NOFILE, &rl) != 0)
		wlog->warn("Unable to raise file descriptor limit");
	    }

#if defined(_BROKEN_VSNPRINTF)
	    // Before we start any threads, switch to the less safe vsprintf
	    // since vsnprintf is not thread safe.

#if defined(DEBUG)
	    dlog->enable_vsprintf();
#endif
	    wlog->enable_vsprintf();
#endif
	    
	    // Now that we're about to start threading, enable tid
	    // display in logging

#if defined(DEBUG)
	    dlog->enable_tid();
#endif
	    wlog->enable_tid();
	    
	    ProcessControl *pc = new ProcessControl();
	      
	    if(pc)
	    {
	      // If any of the establishing fails, we'll delete pc and exit,
	      // generating some warnings about improperly terminated threads
	      // along the way.
	      
#if defined(DEBUG)
	      dlog->log_progress(DEBUG_THREAD, "Beginning thread creation");
#endif
		
	      if(pc->establish_signals())
	      {
		// pc->discontinue will block until the signal_handler exits.
		// Timer must be established before scheduler, which must be
		// established before workers.
		
		if(pc->establish_timer() && pc->establish_schedulers()
		   && pc->establish_workers())
		{
#if defined(DEBUG)
		  dlog->log_progress(DEBUG_THREAD, "Thread creation complete");
#endif
		    
		  pc->discontinue();
		    
#if defined(DEBUG)
		  dlog->log_progress(DEBUG_THREAD,
				     "Thread termination complete, cleaning up");
#endif
		}
	      }
	      
	      xdelete(pc);
	    }
	    else
	      wlog->warn("Failed to initialize ProcessControl");
	  }
	  else
	    wlog->warn("Failed to initialize LockManager and StateCache");
	}
	else
	  wlog->warn("State consistency check failed");
      }
      else
      {
#if defined(DEBUG)
        wlog->warn("Configuration parse failed (use -d %d, %d, or %d for details)",
                   DEBUG_CFERRS, (DEBUG_CONFIG + DEBUG_CFLESS), DEBUG_CONFIG);
#else
        wlog->warn("Configuration parse failed");
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
#endif

  // scache refers to locks, so it must be deleted first
  xdelete(scache);
  xdelete(locks);
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "main = 0");
#endif
  
  libsrvexit();

  exit(0);
}
