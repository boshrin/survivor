/*
 * handlers.C: Functions used to start threads.
 *
 * Version: $Revision: 0.9 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2004/03/01 22:20:45 $
 * MT-Level: Safe
 *
 * Copyright (c) 2002 - 2003
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: handlers.C,v $
 * Revision 0.9  2004/03/01 22:20:45  benno
 * Run analyze_configuration for smart scheduling
 *
 * Revision 0.8  2003/04/09 19:50:04  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.7  2003/03/31 12:49:34  benno
 * Reset StateCache on SIGHUP
 *
 * Revision 0.6  2003/03/02 14:45:52  benno
 * BSD_SIGNALS
 * xdelete
 *
 * Revision 0.5  2002/04/26 20:17:24  benno
 * add BROKEN_THREAD_SIGNALS
 *
 * Revision 0.4  2002/04/04 20:50:49  benno
 * copyright
 *
 * Revision 0.3  2002/04/03 19:13:24  benno
 * rcsify date
 *
 * Revision 0.2  2002/04/03 19:13:15  benno
 * Must call state_consistency when rereading config
 *
 * Revision 0.1  2002/04/03 19:12:37  benno
 * initial revision
 *
 */

#include "scheduler.H"

extern int kasignal;

void dummy_catcher(int i)
{
  // Do nothing but provide a function for handlers to call when
  // they get the signal to exit.
}

void keepalive_catcher(int i)
{
  // For keepalive process, catch signals and store them.

  kasignal = i;
}

#if defined(_BROKEN_THREAD_SIGNALS)
void extra_catcher(int i)
{
  // Send the scheduler thread whatever signal this catcher receives.

  if(glsigtid > 0)
    pthread_kill(glsigtid, i);
}
#endif // _BROKEN_THREAD_SIGNALS

void *alert_handler(void *x)
{
  // Function run by alert thread.  <x> is a pointer to the AlertWorker
  // object which this thread will run.  This function will not return
  // until the AlertWorker is told to exit.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "alert_handler(%d)", x);
#endif

  // a->begin() will handle all the signal processing
  
  AlertWorker *a = (AlertWorker *)x;

  if(a)
  {
#if defined(DEBUG)
    dlog->log_progress(DEBUG_THREAD, "AlertWorker thread beginning");
#endif
    
    a->begin();

    // begin doesn't return until a->end() is called by another thread

#if defined(DEBUG)
    dlog->log_progress(DEBUG_THREAD, "AlertWorker thread ending");
#endif
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "alert_handler()");
#endif

  return(NULL);
}

void *alert_scheduler_handler(void *x)
{
  // Function run by the alert scheduler thread.  <x> is a Scheduler
  // object which the thread will run.  When the ProcessControl is
  // told to discontinue, it will tell the Scheduler to exit, and only
  // then will this function return.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "alert_scheduler_handler(%d)", x);
#endif

  // SIGUSR1 will interrupt our cycle

#if defined(_BSD_SIGNALS)
  signal(SIGUSR1, dummy_catcher);

  sigset_t sigset;
  sigemptyset(&sigset);
  sigaddset(&sigset, SIGUSR1);
  pthread_sigmask(SIG_UNBLOCK, &sigset, NULL);
#else
  sigset(SIGUSR1, dummy_catcher);
#endif // _BSD_SIGNALS
  
  Scheduler *s = (Scheduler *)x;

  if(s)
  {
#if defined(DEBUG)
    dlog->log_progress(DEBUG_THREAD, "Alert Scheduler thread beginning");
#endif
    
    s->begin_alert();

    // begin_alert doesn't return until s->end() is called by another thread

#if defined(DEBUG)
    dlog->log_progress(DEBUG_THREAD, "Alert Scheduler thread ending");
#endif
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "alert_scheduler_handler()");
#endif

  return(NULL);
}

void *check_handler(void *x)
{
  // Function run by check thread.  <x> is a pointer to the CheckWorker
  // object which this thread will run.  This function will not return
  // until the CheckWorker is told to exit.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "check_handler(%d)", x);
#endif

  // c->begin() will handle all the signal processing
  
  CheckWorker *c = (CheckWorker *)x;

  if(c)
  {
#if defined(DEBUG)
    dlog->log_progress(DEBUG_THREAD, "CheckWorker thread beginning");
#endif
    
    c->begin();

    // begin doesn't return until c->end() is called by another thread

#if defined(DEBUG)
    dlog->log_progress(DEBUG_THREAD, "CheckWorker thread ending");
#endif
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "check_handler()");
#endif

  return(NULL);
}

void *check_scheduler_handler(void *x)
{
  // Function run by the check scheduler thread.  <x> is a Scheduler
  // object which the thread will run.  When the ProcessControl is
  // told to discontinue, it will tell the Scheduler to exit, and only
  // then will this function return.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "check_scheduler_handler(%d)", x);
#endif

  // SIGUSR1 will interrupt our cycle

#if defined(_BSD_SIGNALS)
  signal(SIGUSR1, dummy_catcher);

  sigset_t sigset;
  sigemptyset(&sigset);
  sigaddset(&sigset, SIGUSR1);
  pthread_sigmask(SIG_UNBLOCK, &sigset, NULL);
#else
  sigset(SIGUSR1, dummy_catcher);
#endif // _BSD_SIGNALS
  
  Scheduler *s = (Scheduler *)x;

  if(s)
  {
#if defined(DEBUG)
    dlog->log_progress(DEBUG_THREAD, "Check Scheduler thread beginning");
#endif

    s->begin_check();

    // begin_check doesn't return until s->end() is called by another thread

#if defined(DEBUG)
    dlog->log_progress(DEBUG_THREAD, "Check Scheduler thread ending");
#endif
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "check_scheduler_handler()");
#endif

  return(NULL);
}

void *signal_handler(void *x)
{
  // Function run by signal handler thread.  When a HUP is received,
  // the configuration file will be reread.  When a QUIT is received,
  // the signal_handler will return, indicating that all threads should
  // be instructed to exit.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "signal_handler()");
#endif
  
  sigset_t sigs;
  int signal;

  sigemptyset(&sigs);
  sigaddset(&sigs, SIGHUP);
  sigaddset(&sigs, SIGINT);
  sigaddset(&sigs, SIGQUIT);
  sigaddset(&sigs, SIGTERM);

  bool done = false;

  while(!done)
  {
    sigwait(&sigs, &signal);

#if defined(DEBUG)
    dlog->log_progress(DEBUG_THREAD, "Received signal %d", signal);
#endif

    switch(signal)
    {
    case SIGHUP:
      // While reparsing, all other signals are queued.  This is probably
      // a good behaviour.  Obtain a write lock on the Configuration, then
      // reparse.
      if(locks->cf_write())
      {
	Configuration *newcf = new Configuration();

	if(newcf)
	{
	  if(newcf->parse_cfs())
	  {
	    // If parse_cfs returns true, then try to make the state
	    // directories consistent.

	    if(newcf->state_consistency())
	    {
	      // Swap into place.  newcf must be used at this point.

	      delete cf;
	      cf = newcf;
	      newcf = NULL;

	      // Reset the StateCache.  Since we currently have a
	      // write lock to the configuration, nothing should
	      // be accessing the StateCache.

	      scache->disable();
	      scache->enable();

	      // Reset the stat counter

	      if(args->smartscheduling())
	      {
#if defined(DEBUG)
		dlog->log_progress(DEBUG_SCHEDS,
				   "Restarting smart scheduling");
#endif

		if(statc && !statc->analyze_configuration())
		{
		  wlog->warn("StatCounter configuration analysis failed");
		  xdelete(statc);
		}
	      }
	    }
	    else
	    {
	      // Stick with the old config, though we may now be inconsistent
	      
	      wlog->warn("State consistency check failed");
	      xdelete(newcf);
	    }
	  }
	  else
	  {
	    wlog->warn("Reparse of configuration files failed");
	    xdelete(newcf);
	  }
	}
	else
	  wlog->warn("Unable to allocate new Configuration object");

	// Release the lock
	locks->cf_unlock();
      }
      else
	wlog->warn("Failed to get write lock on config");
      break;
    default:
      // Exiting the loop will cause the thread to exit, which will signal
      // main to exit.
      done = true;
      break;
    }
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "signal_handler()");
#endif

  return(NULL);
}

void *timer_handler(void *x)
{
  // Function run by timer thread.  <x> is a Timer object which the
  // thread will run.  This thread continues until SIGUSR1 is
  // received.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "timer_handler(%d)", x);
#endif

  // SIGUSR1 will interrupt our cycle

#if defined(_BSD_SIGNALS)
  signal(SIGUSR1, dummy_catcher);

  sigset_t sigset;
  sigemptyset(&sigset);
  sigaddset(&sigset, SIGUSR1);
  pthread_sigmask(SIG_UNBLOCK, &sigset, NULL);
#else
  sigset(SIGUSR1, dummy_catcher);
#endif // _BSD_SIGNALS

  Timer *t = (Timer *)x;

  if(t)
  {
#if defined(DEBUG)
    dlog->log_progress(DEBUG_THREAD, "Timer thread beginning");
#endif

    t->begin();

    // begin doesn't return until t->end() is called by another thread

#if defined(DEBUG)
    dlog->log_progress(DEBUG_THREAD, "Timer thread ending");
#endif
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "timer_handler()");
#endif

  return(NULL);
}
