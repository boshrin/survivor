/*
 * ProcessControl.C: Object to manage threads for survivor.
 *
 * Version: $Revision: 0.11 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2007/03/29 12:02:31 $
 * MT-Level: Safe
 *  MT Safe only when used as expected (threads should not call establish_X
 *  or similar methods).
 *
 * Copyright (c) 2002 - 2007
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: ProcessControl.C,v $
 * Revision 0.11  2007/03/29 12:02:31  benno
 * QueueUnit takes timeout
 *
 * Revision 0.10  2003/04/09 19:50:03  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.9  2003/01/29 01:51:05  benno
 * Use IONULL, IOTF, xdelete, xadelete
 *
 * Revision 0.8  2002/04/26 20:16:35  benno
 * add BROKEN_THREAD_SIGNALS
 *
 * Revision 0.7  2002/04/04 20:51:11  benno
 * copyright
 *
 * Revision 0.6  2002/04/03 19:20:54  benno
 * rcsify date
 *
 * Revision 0.5  2002/04/03 19:20:45  benno
 * Don't block SIGSEGV
 *
 * Revision 0.4  2002/04/03 19:20:18  benno
 * Clean up
 *
 * Revision 0.3  2002/04/03 19:19:55  benno
 * New Timer object interface
 *
 * Revision 0.2  2002/04/03 19:19:17  benno
 * Revamping queue facility
 *
 * Revision 0.1  2002/04/03 19:18:09  benno
 * initial revision
 *
 */

#include "scheduler.H"

ProcessControl::ProcessControl()
{
  // Allocate and initialize a new ProcessControl object.

  // Returns: A new ProcessControl object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "ProcessControl::ProcessControl()");
#endif

  aws = NULL;
  cws = NULL;
  alertq = new Queue();
  checkq = new Queue();

  if(!alertq || !checkq)
    wlog->warn("ProcessControl failed to allocate check and alert Queues");
  
  as = NULL;
  cs = NULL;
  t = NULL;
  aworkers = NULL;
  cworkers = NULL;
  aschedtid = 0;
  cschedtid = 0;
  sigtid = 0;
  timertid = 0;
  awavail = 0;
  cwavail = 0;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "ProcessControl::ProcessControl()");
#endif
}

QueueUnit *ProcessControl::alert_worker_available()
{
  // See if any Alert work is available, and if so, accept it.  If not, mark
  // the Worker as available.  This thread should be called by AlertWorker
  // threads only.

  // Returns: A QueueUnit if one is available, or NULL if the Worker should
  // await a new task.

  QueueUnit *r = NULL;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "ProcessControl::alert_worker_available()");
#endif
  
  if(locks->aq_lock())
  {
    // See if there is an item in the Queue.

    r = alertq->next();

    if(!r)
    {
      // No item available, so increment the number of available workers.

      awavail++;
    }
    // else alertq->next() automatically marks this as assigned.
    
    locks->aq_unlock();
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "ProcessControl::alert_worker_available = %d",
		 r);
#endif
  
  return(r);
}

QueueUnit *ProcessControl::check_worker_available()
{
  // See if any Check work is available, and if so, accept it.  If not, mark
  // the Worker as available.  This thread should be called by CheckWorker
  // threads only.

  // Returns: A QueueUnit if one is available, or NULL if the Worker should
  // await a new task.

  QueueUnit *r = NULL;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "ProcessControl::check_worker_available()");
#endif
  
  if(locks->cq_lock())
  {
    // See if there is an item in the Queue.

    r = checkq->next();

    if(!r)
    {
      // No item available, so increment the number of available workers.

      cwavail++;
    }
    // else checkq->next() automatically marks this as assigned.
    
    locks->cq_unlock();
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "ProcessControl::check_worker_available = %d",
		 r);
#endif
  
  return(r);
}

bool ProcessControl::discontinue()
{
  // Stop all threads in preparation for exit.

  // Returns: true when all threads have exited, or false if any problems
  // are encountered.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "ProcessControl::discontinue()");
#endif

  // Wait for the signal handler thread to exit, then stop everything else

  if(sigtid)
  {
#if defined(_BROKEN_THREAD_SIGNALS)
#if defined(DEBUG)
    dlog->log_progress(DEBUG_THREAD,
		       "Adding signal handlers for extra catcher");
#endif // DEBUG

    // Set up the same signals that handlers:signal_handler() waits on.
    sigset(SIGHUP, extra_catcher);
    sigset(SIGINT, extra_catcher);
    sigset(SIGQUIT, extra_catcher);
    sigset(SIGTERM, extra_catcher);
#endif // _BROKEN_THREAD_SIGNALS
    
#if defined(DEBUG)
    dlog->log_progress(DEBUG_THREAD,
		       "Waiting for signal handler thread to exit");
#endif
    
    pthread_join(sigtid, NULL);
    sigtid = 0;
#if defined(_BROKEN_THREAD_SIGNALS)
    glsigtid = 0;
#endif // _BROKEN_THREAD_SIGNALS
  }
  
  if(as)
  {
    // Tell the Alert scheduler to stop, and send it a signal in case
    // it is stuck in sleep

    as->end();

    if(aschedtid)
    {
      pthread_kill(aschedtid, SIGUSR1);

      // Wait for the Alert Scheduler thread to exit

#if defined(DEBUG)
      dlog->log_progress(DEBUG_THREAD,
			 "Waiting for Alert Scheduler thread to exit");
#endif
      
      pthread_join(aschedtid, NULL);
      aschedtid = 0;
    }

    // Nothing should be using the Alert Scheduler object at this point

    xdelete(as);
  }

  if(cs)
  {
    // Tell the Check scheduler to stop, and send it a signal in case
    // it is stuck in sleep

    cs->end();
    
    if(cschedtid)
    {
      pthread_kill(cschedtid, SIGUSR1);

      // Wait for the Check Scheduler thread to exit

#if defined(DEBUG)
      dlog->log_progress(DEBUG_THREAD,
			 "Waiting for Check Scheduler thread to exit");
#endif
    
      pthread_join(cschedtid, NULL);
      cschedtid = 0;
    }

    // Nothing should be using the Check Scheduler object at this point

    xdelete(cs);
  }

  if(aws)
  {
    // Flag the AlertWorkers for exit

    for(int i = 0;i < args->alert_threads();i++)
    {
      if(aws[i])
	aws[i]->end();
    }
    
    if(aworkers)
    {
      // Send the signal to the threads
      
      for(int i = 0;i < args->alert_threads();i++)
      {
	if(aworkers[i] > 0)
	{
	  // Send a SIGALRM first, in case the thread has a timeout request
	  // pending, then tell it to exit via SIGUSR1.

	  pthread_kill(aworkers[i], SIGALRM);
	  pthread_kill(aworkers[i], SIGUSR1);

#if defined(DEBUG)
	  dlog->log_progress(DEBUG_THREAD,
			     "Waiting for alert thread %d to exit", i);
#endif

	  pthread_join(aworkers[i], NULL);
	  aworkers[i] = 0;
	}
      }
      
      free(aworkers);
      aworkers = NULL;
    }

    // Delete the AlertWorkers after the threads have terminated

    for(int i = 0;i < args->alert_threads();i++)
    {
      xdelete(aws[i]);
    }

    free(aws);
    aws = NULL;
  }
  
  if(cws)
  {
    // Flag the CheckWorkers for exit

    for(int i = 0;i < args->check_threads();i++)
    {
      if(cws[i])
	cws[i]->end();
    }
    
    if(cworkers)
    {
      // Send the signal to the threads
      
      for(int i = 0;i < args->check_threads();i++)
      {
	if(cworkers[i] > 0)
	{
	  // Send a SIGALRM first, in case the thread has a timeout request
	  // pending, then tell it to exit via SIGUSR1.
	  
	  pthread_kill(cworkers[i], SIGALRM);
	  pthread_kill(cworkers[i], SIGUSR1);

#if defined(DEBUG)
	  dlog->log_progress(DEBUG_THREAD,
			     "Waiting for check thread %d to exit", i);
#endif

	  pthread_join(cworkers[i], NULL);
	  cworkers[i] = 0;
	}
      }
      
      free(cworkers);
      cworkers = NULL;
    }

    // Delete the CheckWorkers after the threads have terminated

    for(int i = 0;i < args->check_threads();i++)
    {
      xdelete(cws[i]);
    }

    free(cws);
    cws = NULL;
  }

  awavail = 0;
  cwavail = 0;
  
  if(t)
  {
    // Tell the Timer to stop, and send it a signal in case it is
    // stuck in sleep.  It would be OK to terminate the Timer thread
    // before the other threads so long as SIGALRM is sent to all
    // other threads in case they have a pending timeout.  Note,
    // however, that the Timer also kills stuck processes, so best to
    // terminate it only after all the worker threads have exited.

    t->end();

    if(timertid)
    {
      pthread_kill(timertid, SIGUSR1);

#if defined(DEBUG)
      dlog->log_progress(DEBUG_THREAD, "Waiting for Timer thread to exit");
#endif

      pthread_join(timertid, NULL);
      timertid = 0;
    }

    // Nothing should be using the Timer object at this point

    xdelete(t);
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "ProcessControl::discontinue = true");
#endif
  
  return(true);
}

bool ProcessControl::establish_schedulers()
{
  // Establish the scheduler thread.

  // Returns: true if the scheduler thread was successfully established,
  // false otherwise.

  bool r = true;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "ProcessControl::establish_schedulers()");
#endif

  // Oooo! this!
  as = new Scheduler(this);

  if(as)
  {
    if(pthread_create(&aschedtid, NULL, alert_scheduler_handler, as) == 0)
    {
#if defined(DEBUG)
      dlog->log_progress(DEBUG_THREAD,
			 "Alert Scheduler thread id = %d", aschedtid);
#endif
    }
    else
    {
      wlog->warn("pthread_create failed to create Alert Scheduler thread");
      r = false;
    }
  }
  else
  {
    wlog->warn("ProcessControl failed to allocate Alert Scheduler");
    r = false;
  }

  if(r)
  {
    cs = new Scheduler(this);

    if(cs)
    {
      if(pthread_create(&cschedtid, NULL, check_scheduler_handler, cs) == 0)
      {
#if defined(DEBUG)
	dlog->log_progress(DEBUG_THREAD,
			   "Check Scheduler thread id = %d", cschedtid);
#endif
      
	return(true);
      }
      else
      {
	wlog->warn("pthread_create failed to create Check Scheduler thread");
	r = false;
      }
    }
    else
      wlog->warn("ProcessControl failed to allocate Check Scheduler");
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "ProcessControl::establish_schedulers = %s",
		 IOTF(r));
#endif
  
  return(r);
}

bool ProcessControl::establish_signals()
{
  // Establish the signal handler thread.

  // Returns: true if the signal handler thread was successfully established,
  // false otherwise.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "ProcessControl::establish_signals()");
#endif

  // Block signals from the initial thread (and subsequent ones)
  sigset_t sigset;

  sigfillset(&sigset);
  sigdelset(&sigset, SIGTSTP);
  sigdelset(&sigset, SIGSEGV);
  pthread_sigmask(SIG_BLOCK, &sigset, NULL);
	  
  if(pthread_create(&sigtid, NULL, signal_handler, NULL) == 0)
  {
#if defined(_BROKEN_THREAD_SIGNALS)
    glsigtid = sigtid;
#endif // _BROKEN_THREAD_SIGNALS
    
#if defined(DEBUG)
    dlog->log_progress(DEBUG_THREAD, "Signal handler thread id = %d", sigtid);
    dlog->log_exit(DEBUG_MAJTRC, "ProcessControl::establish_signals = true");
#endif
    
    return(true);
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "ProcessControl::establish_signals = false");
#endif
  
  return(false);
}

bool ProcessControl::establish_timer()
{
  // Establish the timer thread.

  // Returns: true if the timer thread was successfully established,
  // false otherwise.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "ProcessControl::establish_timer()");
#endif

  t = new Timer();

  if(t)
  {
    if(pthread_create(&timertid, NULL, timer_handler, t) == 0)
    {
#if defined(DEBUG)
      dlog->log_entry(DEBUG_THREAD, "Timer thread id = %d", timertid);
      dlog->log_exit(DEBUG_MAJTRC, "ProcessControl::establish_timer = true");
#endif
    
      return(true);
    }
  }
  else
    wlog->warn("ProcessControl failed to allocate Timer");
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "ProcessControl::establish_timer = false");
#endif
  
  return(false);
}

bool ProcessControl::establish_workers()
{
  // Establish the worker threads.

  // Returns: true if the work threads are successfully established,
  // false otherwise.

  bool ret = true;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "ProcessControl::establish_workers()");
#endif

  // First set up the Alert workers

  aws = (AlertWorker **)malloc(sizeof(AlertWorker *) * args->alert_threads());

  if(aws)
  {
    for(int i = 0;i < args->alert_threads();i++)
    {
      aws[i] = new AlertWorker(this);

      if(!aws[i])
      {
	ret = false;
	wlog->warn("ProcessControl failed to allocate AlertWorker %d", i);
      }
    }
  }

  if(ret)
  {
    aworkers = (pthread_t *)malloc(sizeof(pthread_t) * args->alert_threads());

    if(aworkers)
    {
      for(int i = 0;i < args->alert_threads();i++)
      {
	if(pthread_create(&aworkers[i], NULL, alert_handler, aws[i]) == 0)
	{
#if defined(DEBUG)
	  dlog->log_progress(DEBUG_THREAD, "Alert thread %d id = %d",
			     i, aworkers[i]);
#endif
	}
	else
	{
	  ret = false;
	  aworkers[i] = 0;
	  wlog->warn("ProcessControl failed to create alert_handler thread %d",
		     i);
	}
      }
    }
    else
    {
      ret = false;
      wlog->warn("ProcessControl failed to allocate aworkers array");
    }
  }
  
  // Then set up the Check workers

  if(ret)
  {
    cws = (CheckWorker **)malloc(sizeof(CheckWorker *) *
				 args->check_threads());

    if(cws)
    {
      for(int i = 0;i < args->check_threads();i++)
      {
	cws[i] = new CheckWorker(this);
	
	if(!cws[i])
	{
	  ret = false;
	  wlog->warn("ProcessControl failed to allocate CheckWorker %d", i);
	}
      }
    }
  }
  
  if(ret)
  {  
    cworkers = (pthread_t *)malloc(sizeof(pthread_t) * args->check_threads());

    if(cworkers)
    {
      for(int i = 0;i < args->check_threads();i++)
      {
	if(pthread_create(&cworkers[i], NULL, check_handler, cws[i]) == 0)
	{
#if defined(DEBUG)
	  dlog->log_progress(DEBUG_THREAD, "Check thread %d id = %d",
			     i, cworkers[i]);
#endif
	}
	else
	{
	  ret = false;
	  cworkers[i] = 0;
	  wlog->warn("ProcessControl failed to create check_handler thread %d",
		     i);
	}
      }
    }
    else
    {
      ret = false;
      wlog->warn("ProcessControl failed to allocate cworkers array");
    }
  }

  // Don't set awavail or cwavail -- the threads will increment it as
  // they become available.
 
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "ProcessControl::establish_workers = %s",
		 IOTF(ret));
#endif
  
  return(ret);
}

bool ProcessControl::queue_alert(Alert *alert)
{
  // Queue the Alert <a> to be transmitted.  <a> will be deleted when no
  // longer required.

  // Returns: true if the Alert was successfully queued, false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "ProcessControl::queue_alert(%d)", alert);
#endif
  
  if(alertq && alert)
  {
    if(locks->aq_lock())
    {
      // We don't look for entries in the Queue, unlike queue_check,
      // because we'd rather generate too many alerts than not enough.

      // Allocate a QueueUnit

      QueueUnit *qu = new QueueUnit(alert);

      if(qu)
      {
	if(alertq->append(qu))
	{
	  // See if there are any available workers

	  if(awavail > 0)
	  {
	    // Find it

	    for(int i = 0;i < args->alert_threads();i++)
	    {
	      // AlertWorker->assign is not MT-Safe, but since we are in
	      // a protected section of code it should be OK

	      if(aws && aws[i] && aws[i]->assign(qu))
	      {
		// Mark the unit as assigned, reduce the number of
		// available workers, and send the thread a signal.

		qu->mark(assigned_unit);
		awavail--;
		pthread_kill(aworkers[i], SIGUSR1);

		ret = true;
		break;
	      }
	    }
	  }
	  else
	  {
	    // No workers available, leave it unassigned in the
	    // Queue.  When a worker becomes available, it will call
	    // worker_available, which will trigger a search of the
	    // Queue for unassigned Alerts.  This gives previously
	    // Queued alerts priority over newly Queued Alerts.

	    ret = true;
	  }
	}
	else
	{
	  wlog->warn("ProcessControl::queue_alert failed to append QueueUnit to the Queue");

	  xdelete(qu);
	}
      }
      else
	wlog->warn("ProcessControl::queue_alert failed to allocate QueueUnit");

      locks->aq_unlock();
    }
    else
      wlog->warn("queue_alert failed to get Alert Queue lock");
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "ProcessControl::queue_alert = %s", IOTF(ret));
#endif
  
  return(ret);
}

bool ProcessControl::queue_check(char *check, List *hosts)
{
  // Queue the Check named <check> to be processed over the set of
  // hosts <hosts>.  The name of the Check is stored rather than a
  // pointer to it in case the configuration file is updated while the
  // Check is queued.  The List <hosts> will be deleted after the
  // Check is processed.  If <check> is already queued, it will not be
  // queued again, even if the existing queue is for a different set
  // of hosts.

  // Returns: true if the Check was successfully queued, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "ProcessControl::queue_check(%s,%d)",
		  IONULL(check), hosts);
#endif
  
  if(checkq && check && hosts)
  {
    int timeout = 120;  // start with a default

    // Figure out the timeout for this check.  It's possible that this
    // timeout will change between now and when we actually execute
    // the check, but that's pretty unlikely and anyway we're not going
    // to delete the queueunit, just mark it expired.

    if(locks->cf_read())
    {
      Check *c = cf->find_check(check);

      timeout += c->timeout();

      locks->cf_unlock();
    }
    else
      wlog->warn("queue_check failed to get configuration lock");
    
    if(locks->cq_lock())
    {
      // First verify that <check> isn't in the Queue

      if(!checkq->find(check))
      {
	// Allocate a QueueUnit

	QueueUnit *qu = new QueueUnit(check, hosts, timeout);

	if(qu)
	{
	  if(checkq->append(qu))
	  {
	    // See if there are any available workers
	    
	    if(cwavail > 0)
	    {
	      // Find it

	      for(int i = 0;i < args->check_threads();i++)
	      {
		// CheckWorker->assign is not MT-Safe, but since we are
		// in a protected section of code it should be OK
		
		if(cws && cws[i] && cws[i]->assign(qu))
		{
		  // Mark the unit as assigned, reduce the number of
		  // available workers, and send the thread a signal.
		  
		  qu->mark(assigned_unit);
		  cwavail--;
		  pthread_kill(cworkers[i], SIGUSR1);
		  
		  ret = true;
		  break;
		}
	      }
	    }
	    else
	    {
	      // No workers available, leave it unassigned in the
	      // Queue. When a worker becomes available, it will call
	      // worker_available, which will trigger a search of the
	      // Queue for unassigned Checks.  This gives previously
	      // Queued checks priority over newly Queued Checks.

	      ret = true;
	    }
	  }
	  else
	  {
	    wlog->warn("ProcessControl failed to append QueueUnit to the Queue");

	    xdelete(qu);
	  }
	}
	else
	  wlog->warn("ProcessControl failed to allocate QueueUnit");
      }
#if defined(DEBUG)
      else
	dlog->log_progress(DEBUG_SCHEDS, "Check '%s' is already queued",
			   check);
#endif
      
      locks->cq_unlock();
    }
    else
      wlog->warn("queue_check failed to get Check Queue lock");
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "ProcessControl::queue_check = %s", IOTF(ret));
#endif
  
  return(ret);
}

bool ProcessControl::request_timeout(int signal, int seconds)
{
  // Request <signal> be sent to the calling thread in <seconds>.
  // The thread should call sigset(SIGALRM, dummy_catcher) before
  // requesting a timeout.

  // Returns: true if the request is noted, false otherwise.
  
  if(t)
    return(t->request(signal, seconds));
  else
    return(false);
}

bool ProcessControl::request_timeout(pid_t pid, int signal, int seconds)
{
  // Request <signal> be sent to the <pid> in <seconds>.

  // Returns: true if the request is noted, false otherwise.
  
  if(t)
    return(t->request(pid, signal, seconds));
  else
    return(false);
}

bool ProcessControl::unrequest_timeout()
{
  // Remove a pending signal request for the calling thread.  Note
  // that the signal may be generated between the time this method is
  // invoked and the time the removal actually happens.  The thread
  // should call sigrelse(SIGALRM) before unrequesting the timeout.

  // Returns: true if the request is removed, false otherwise.

  if(t)
    return(t->unrequest());
  else
    return(false);
}

bool ProcessControl::unrequest_timeout(pid_t pid)
{
  // Remove a pending signal request for <pid>.  Note that the signal
  // may be generated between the time this method is invoked and the
  // time the removal actually happens.

  // Returns: true if the request is removed, false otherwise.

  if(t)
    return(t->unrequest(pid));
  else
    return(false);
}

ProcessControl::~ProcessControl()
{
  // Delete the ProcessControl object.  All threads should be terminated
  // before this method is called.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "ProcessControl::~ProcessControl()");
#endif

  if(aws)
    wlog->warn("AlertWorker objects not cleanly removed");
  
  if(cws)
    wlog->warn("CheckWorker objects not cleanly removed");

  // At this point, we need not bother with obtaining locks

  xdelete(alertq);
  xdelete(checkq);
  
  if(as)
    wlog->warn("Alert Scheduler object not cleanly removed");

  if(cs)
    wlog->warn("Check Scheduler object not cleanly removed");

  if(t)
    wlog->warn("Timer object not cleanly removed");

  if(aworkers)
    wlog->warn("AlertWorker threads not cleanly terminated");
  
  if(cworkers)
    wlog->warn("CheckWorker threads not cleanly terminated");
  
  if(aschedtid != 0)
    wlog->warn("Alert Scheduler thread not cleanly terminated");

  if(cschedtid != 0)
    wlog->warn("Check Scheduler thread not cleanly terminated");

  if(sigtid != 0)
    wlog->warn("Signal handler thread not cleanly terminated");

  if(timertid != 0)
    wlog->warn("Timer thread not cleanly terminated");

  if(awavail != 0)
    wlog->warn("Available AlertWorker count not cleanly reset");
  
  if(cwavail != 0)
    wlog->warn("Available CheckWorker count not cleanly reset");
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "ProcessControl::~ProcessControl()");
#endif
}
