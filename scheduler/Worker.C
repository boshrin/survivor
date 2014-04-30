/*
 * Worker.C: survivor object for processing Alerts and Checks
 *
 * Version: $Revision: 0.21 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2007/03/29 12:01:43 $
 * MT-Level: Safe
 *
 * Copyright (c) 2002 - 2007
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: Worker.C,v $
 * Revision 0.21  2007/03/29 12:01:43  benno
 * Tweak debugging
 *
 * Revision 0.20  2005/11/30 01:34:33  benno
 * Always mark tasks completed
 * Retry in 30 seconds if no results were received
 *
 * Revision 0.19  2005/04/09 02:24:35  benno
 * Use FixRequest and CheckRequest objects for XML based module communication
 *
 * Revision 0.18  2004/06/11 22:21:02  benno
 * AlertState::results_init takes summary
 *
 * Revision 0.17  2003/11/29 05:12:49  benno
 * Recheck quiet before transmitting alert in case acked/inhibited after queue
 *
 * Revision 0.16  2003/10/06 21:59:32  benno
 * Changes for type II dependency overhaul
 *
 * Revision 0.15  2003/05/28 03:34:29  benno
 * Changes for Alert and AlertModule
 *
 * Revision 0.14  2003/04/09 19:50:04  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.13  2003/03/02 14:34:50  benno
 * Use write_misconfig
 *
 * Revision 0.12  2003/01/29 02:00:58  benno
 * IOTF
 *
 * Revision 0.11  2002/12/31 04:24:12  benno
 * Add support for fixes
 *
 * Revision 0.10  2002/12/22 17:34:05  benno
 * No more global timeout
 * Use xdelete
 *
 * Revision 0.9  2002/12/15 22:55:06  benno
 * Use RecipientSet to permit multiple calllists
 *
 * Revision 0.8  2002/08/16 22:50:53  benno
 * write alert results regardless of return value
 *
 * Revision 0.7  2002/04/04 20:51:39  benno
 * copyright
 *
 * Revision 0.6  2002/04/03 19:40:54  benno
 * rcsify date
 *
 * Revision 0.5  2002/04/03 19:40:28  benno
 * Use MODEXEC_DEPEND
 * Pass AlertState to exec_alert
 *
 * Revision 0.4  2002/04/03 19:39:53  benno
 * Log when not running a check for failed dependency
 *
 * Revision 0.3  2002/04/03 19:39:20  benno
 * Use Executor object
 * Verify Type II dependencies
 * Don't mark lastalert if we sent a clear alert
 * Pass hosts to write_timeout
 *
 * Revision 0.2  2002/04/03 19:38:38  benno
 * This used to be CheckWorker.C
 * (Add Alert processing)
 *
 * Revision 0.1  2002/04/03 19:37:28  benno
 * initial revision
 *
 */

#include "scheduler.H"

Worker::Worker(ProcessControl *p, bool alert, bool check)
{
  // Allocate and initialize a new Worker object that uses <p> for
  // ProcessControl and processes either <alert>s or <check>s.

  // Returns: A new Worker object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "Worker::Worker(%d,%s,%s)",
		  p, IOTF(alert), IOTF(check));
  dlog->log_exit(DEBUG_MAJTRC, "Worker::Worker()");
#endif

  pc = p;
  task = NULL;
  aw = alert;
  cw = check;
  exit = false;
}

bool Worker::assign(QueueUnit *q)
{
  // Assign the task described in <q> to this Worker.  It is expected
  // that only one thread (alert_scheduler or check_scheduler) will
  // have the responsibility of calling assign(), and therefore will
  // be MT-Safe.  If more than one thread could call this method, it
  // must be protected by a Lock.  After a successful call to this
  // method, the associated thread must be sent a SIGUSR1 to notice
  // the new assignment.  When <q> is completed, it will be marked as
  // such, but not deleted.

  // Returns: true if the task is assigned, false otherwise, including if
  // a task is already assigned.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Worker::assign(%d)", q);
#endif
  
  if(q && !task)
  {
    task = q;
    ret = true;
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Worker::assign = %s", IOTF(ret));
#endif

  return(ret);
}

void Worker::begin()
{
  // Instruct the Worker to begin operation.  It is intended that this
  // method is dedicated to one thread, and that this method will not return
  // until the program is ready to exit.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "Worker::begin()");
#endif

  sigset_t sigs;
  int signal;
  
  sigemptyset(&sigs);
  sigaddset(&sigs, SIGUSR1);

  // Get the first task here, to indicate that the worker is ready to go

  if(aw)
    task = pc->alert_worker_available();
  else
  {
    if(cw)
      task = pc->check_worker_available();
  }
  
  while(!exit)
  {
    // Wait for a SIGUSR1, unless we already have work to do.  If exit
    // is set, exit, else if task has been set, process it.

    if(!exit && !task)
      sigwait(&sigs, &signal);

    if(!exit && task)
    {
      if(locks->cf_read())
      {
	if(aw)
	  process_alert();
	else
	{
	  if(cw)
	    process_check();
	}

	// Release the lock after we're done with any pointers from cf
	locks->cf_unlock();
      }
      else
	wlog->warn("Worker failed to get Configuration lock");
    }
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "Worker::begin()");
#endif
}

bool Worker::end()
{
  // Tell the Worker to end operation.  This method returns when the
  // request has been sent, but the Worker may not have stopped by the
  // time this method returns.

  // Returns: true if the request is sent, false otherwise.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "Worker::end()");
  dlog->log_exit(DEBUG_MAJTRC, "Worker::end = true");
#endif
  
  exit = true;
  return(true);
}

Worker::~Worker()
{
  // Deallocate the Worker object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "Worker::~Worker()");
  dlog->log_exit(DEBUG_MAJTRC, "Worker::~Worker()");
#endif

  pc = NULL;
  task = NULL;  // Don't delete the task, it is managed elsewhere
  aw = false;
  cw = false;
  exit = false;
}

void Worker::process_alert()
{
  // Execute the Alert and handle the results.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Worker::process_alert()");
#endif

  Alert *a = task->alert();
  RecipientSet *rset = a->recipients();

  if(a && (rset || a->fix()))
  {
    Executor *e = new Executor();

    if(e)
    {
      // We still have a cf read lock

      Check *c = cf->find_check(a->service());

      if(c)
      {
	AlertState *as = new AlertState(c, a->host());

	// Since an ack or inhibit command could have been executed
	// between the time the alert was scheduled and now, check
	// again to make sure we are clear to go.
	
	if(as && !as->quiet())
	{
	  // First, see if we are executing a fix.  If we are, we may
	  // or may not execute an alert.

	  if(a->fix())
	  {
	    if(c->fix())
	    {
	      // Execute the requested fix.

	      FixState *fs = new FixState(c, a->host());

	      if(fs)
	      {
		if(verify_type2_dependencies(c->fix()))
		{
		  FixRequest *fr = new FixRequest(c->fix()->modargs(),
						  a->host(),
						  c->fix()->fix_timeout());

		  if(fr)
		  {
		    pid_t pid = e->exec_fix(c, fr);

		    if(pid > 0)
		    {
		      // Request a timeout on the process
		      
		      pc->request_timeout(pid, SIGKILL,
					  c->fix()->fix_timeout());
		    
		      if(e->result(fs) != -1)
			fs->write_results(args->instuser());
		  
		      if(!pc->unrequest_timeout(pid))
		      {
			// Except in boundary race conditions, this indicates
			// that the fix timed out instead of being successfully
			// executed.  Make an appropriate note.
		      
			a->set_fix_status(MODEXEC_TIMEDOUT, "Fix timed out");
		      }
		      else
			a->set_fix_status(fs->returncode(), fs->comment());
		    
		      // Reschedule the check if the fix succeeded
		    
		      if(fs->returncode() == MODEXEC_OK)
		      {
			CheckState *cs = new CheckState(c, a->host());
			
			if(cs)
			{
			  cs->reschedule();
			  xdelete(cs);
			}
			else
			  wlog->warn("Worker::process_alert failed to allocate CheckState");
		      }
		    }

		    xdelete(fr);
		  }
		  else
		    wlog->warn("Worker::process_alert failed to allocate FixRequest");
		}
		else
		{
		  // Note that the fix failed due to a dependencie

		  a->set_fix_status(MODEXEC_DEPEND,
				    "Failed dependency exists");
		}
		
		xdelete(fs);
	      }
	      else
		wlog->warn("Worker::process_alert failed to allocate FixState");
	    }
	    else
	    {
	      // If there was no fix defined for this check, we generate
	      // appropriate debugging output and make a note that we
	      // "tried" so the problem can escalate.

#if defined(DEBUG)
	      dlog->log_progress(DEBUG_SCHEDS,
				 "Alert scheduled for %s@%s requested fix, but no fix is defined for %s",
				 c->name(), a->host(), c->name());
#endif

	      a->set_fix_status(MODEXEC_MISCONFIG,
				"No fix defined for this service");
	    }
	  }
	  
	  if(rset)
	  {
	    // We call exec_alert one time for each module used to contact
	    // calllist members for this Alert.  After iterating through
	    // them all, we tell the AlertState to update its information.
	    // The state files are locked only when results_write is called,
	    // so other utilities can read them even while we're still
	    // processing the Alert.  This creates a minor race condition
	    // if two threads process Alerts for the same service@host
	    // simultaneously (say, because of a delay in processing).
	    // In that case, the thread that finishes last will determine
	    // the final state.  Both threads, however, will correctly
	    // update the history.
	    
	    if(as->results_init(a->retval(), a->summary()))
	    {
	      for(int i = 0;i < rset->modules();i++)
	      {
		char *mod = rset->module(i);
		char *add = rset->addresses(i);
		AlertModule *am = cf->find_alertmodule(mod);
		
		if(mod && add && am)
		{
		  // Generate an alert for this module.
		  
		  pid_t pid = e->exec_alert(a, am);
		  
		  if(pid > 0)
		  {
		    // Since only a return code is expected, we need
		    // only time out the process.
		    
		    pc->request_timeout(pid, SIGKILL, DEFAULT_ALERT_TIMEOUT);
		    
		    // Store results and flush
		    if(e->result(as, mod) != -1)
		      as->results_write();
		    
		    if(!pc->unrequest_timeout(pid))
		    {
		      // Except in boundary race conditions, this
		      // indicates that the alert timed out instead of
		      // being successfully executed.  If this
		      // continues, this should show up via sc or sw (as
		      // escalated, unacknowledged, not transmitted,
		      // etc), but we'll dump a warning to stderr just
		      // in case.
		      
		      wlog->warn("Worker::process_alert failed to transmit alert for %s via %s",
				 c->name(), mod);
		    }
		  }
		}
	      }
	      
	      as->results_finish();
	    }
	  }
	}

	if(!as)
	  wlog->warn("process_alert failed to allocate AlertState");

	// xdelete the AlertState here because we may fall out of the
	// if/else clause due to the alert being ack'd or inhibited.
	
	xdelete(as);
      }
      else
	wlog->warn("process_alert cannot find Check for %s", a->service());

      xdelete(e);
    }
    else
      wlog->warn("Worker::process_alert failed to allocate Executor");
  }
  else
    wlog->warn("process_alert received QueueUnit with no alert specified");
    
  // Mark the task completed and reset for the next turn.
  // We do this even if the alert module failed (or fork or pipe),
  // as no lastalert file will be written and the Scheduler will
  // reschedule the alert the next time it runs.

  task->mark(completed_unit);

  // If any Alerts are Queued, we will find out right away, otherwise
  // task will be set to NULL.
    
  task = pc->alert_worker_available();
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Worker::process_alert()");
#endif
}

void Worker::process_check()
{
  // Execute the Check and handle the results.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Worker::process_check()");
#endif

  char *checkname = task->check();
	
  if(checkname)
  {
    Check *check = cf->find_check(checkname);
    List *hosts = task->hosts();
    
    if(!hosts)
    {
      // Then the name of a host group was queued, and must be retrieved
      // from the configuration.
      
      char *hostname = task->group();
      
      if(hostname)
	hosts = cf->find_group(hostname);
    }
    
    if(check && hosts && hosts->entries() > 0)
    {
      Executor *e = new Executor();

      if(e)
      {
	if(verify_type2_dependencies(check, cf->max_dependencies(), hosts))
	{
	  CheckState *cs = new CheckState(check);

	  if(cs)
	  {
	    // Create a new CheckRequest
	    
	    CheckRequest *creq = new CheckRequest(hosts, check->timeout(),
						  check->modargs(), false);

	    if(creq)
	    {
	      // If smart scheduling is enabled, we might retry if it looks
	      // like the module experienced a transient failure.

	      int etries = 2;

	      while(etries > 0)
	      {
		// We ignore the return from exec_check, as noted below.
	      
		pid_t pid = e->exec_check(check, cs, creq);
		
		if(pid > 0)
		{
		  // Since we won't block on a read from a pipe (the results
		  // are written to a tmp file), only the process need be
		  // timed out.
		  
		  pc->request_timeout(pid, SIGKILL, check->timeout());
		  
		  // Wait on the child to make sure it finishes
		  
		  e->result();
		  
		  // Parse the results from the check and write them out.
		  // Since we haven't marked the task completed yet, there
		  // shouldn't be a race condition here.

		  if(cs->parse_tmp_state_results(hosts))
		    cs->write_results();
		  
		  if(!pc->unrequest_timeout(pid))
		  {
		    // e can't definitively know that a process was timed out
		    // since it may not have done anything.  Except in boundary
		    // race conditions, unrequest_timeout will return false if
		    // the process was timed out, so we record it here.
		    
		    cs->write_timeout(hosts);
		    etries = 0;
		  }
		  else
		  {
		    // A misconfigured module may write errors to stderr,
		    // which Executor cannot currently detect (and so would
		    // look exactly like a timed out module).  If the module
		    // didn't time out, tell the CheckState to verify that
		    // it received all the responses it expected.

		    // However, if smart scheduling is enabled and no
		    // results were received, we will retry once after
		    // a thirty second pause.

		    List *w = cs->written_hosts();
		    
		    if(args->smartscheduling() && etries == 2 &&
		       (!w || w->entries()==0))
		    {
		      etries--;
		      
		      wlog->warn("Worker::process_check received no results for check %s, will retry in 30 seconds", check->name());

		      // Reset cs to be safe
		      xdelete(cs);
		      
		      sleep(30);

		      cs = new CheckState(check);

		      if(!cs)
		      {
			wlog->warn("Worker::process_check failed to reallocate CheckState");
			etries = 0;
		      }
		    }
		    else
		    {
		      cs->write_misconfig(hosts);
		      etries = 0;
		    }

		    w = NULL;
		  }
		}
		else
		  etries = 0;
	      }

	      xdelete(creq);
	    }
	    else
	      wlog->warn("Worker::process_check failed to allocate CheckRequest");

	    xdelete(cs);
	  }
	  else
	    wlog->warn("Worker::process_check failed to allocate CheckState");
	}
#if defined(DEBUG)
	else
	  dlog->log_progress(DEBUG_SCHEDS, "-- NOT executing check %s",
			     check->name());
#endif

	xdelete(e);
      }
      else
	wlog->warn("Worker::process_check failed to allocate Executor");
    }
    else
      wlog->warn("process_check received QueueUnit with invalid check ('%s') or host list (count = %d)",
		 IONULL(checkname), (hosts ? hosts->entries() : -1));
  }
  else
    wlog->warn("process_check received QueueUnit with no check specified");

  // Mark the task completed and reset for the next turn.
  // We do this even if an error occurred as if the check module
  // does not run lastcheck will not be updated and the check will
  // be requeued the next time the scheduler runs.
  
  task->mark(completed_unit);
      
  // If any Checks are Queued, we will find out right away, otherwise
  // task will be set to NULL.
  
  task = pc->check_worker_available();

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Worker::process_check()");
#endif
}

bool Worker::verify_type2_dependencies(Check *c, int rcount, List *hosts)
{
  // Execute any dependencies for the Check <c>.  No state is
  // modified.  <rcount> is for recursive loop detection, and upon
  // reaching 0 forces a false return.  <hosts> is used for generating
  // history entries if the dependency check fails.

  // Returns: false if any dependencies failed, or true otherwise.

  bool r = true;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Worker::verify_type2_dependencies(%d,%d,%d)",
		  c, rcount, hosts);
#endif

  if(rcount > 0)
  {
    if(c)
    {
      Array<Dependency> *ds = c->dependencies();

      if(ds)
      {
	for(int i = 0;i < ds->entries();i++)
	{
	  Dependency *d = ds->retrieve(i);

	  if(d && d->dep_type() == 2 && d->dependency()->name())
	  {
	    // Verify the dependency's dependencies

	    if(!d->dependency()->dependencies() ||
	       verify_type2_dependencies(d->dependency(), rcount - 1, hosts))
	    {
	      // Run this dependency

	      Executor *e = new Executor();
	      CheckRequest *creq =
		new CheckRequest(d->target(), c->timeout(),
				 d->dependency()->modargs(), false);

	      if(e && creq)
	      {
		pid_t pid = e->exec_check(d->dependency(), creq);

		if(pid > 0)
		{
		  // Since we aren't reading from a pipe, only the pid
		  // needs to be timed out.

		  pc->request_timeout(pid, SIGKILL, c->timeout());

		  int er = e->result();

		  if(!pc->unrequest_timeout(pid))
		  {
		    // Except in boundary race conditions, this indicates
		    // the check timed out.  Consider this a failure.

		    er = MODEXEC_TIMEDOUT;
		  }

		  if(er != MODEXEC_OK)
		  {
		    r = false;

		    if(hosts && hosts->entries() > 0)
		    {
		      // Attempt to log this failure, but ignore errors

		      CheckState *cs = new CheckState(c);

		      if(cs)
		      {
			// Create a set of history entries

			Array<CheckResult> *carr = new Array<CheckResult>();
			CharBuffer *cmt = new CharBuffer();

			if(carr && cmt)
			{
			  // We use the same message for every host

			  cmt->append("Dependency \"");
			  cmt->append(d->dependency()->name());
			  cmt->append("\" failed with status ");
			  cmt->append(er);
			  
			  for(int i = 0;i < hosts->entries();i++)
			  {
			    char *host = hosts->retrieve(i);

			    if(host)
			    {
			      CheckResult *cr = new CheckResult(host,
								MODEXEC_DEPEND,
								0,
								cmt->str());

			      if(cr)
			      {
				if(!carr->add(cr))
				{
				  xdelete(cr);
				}
			      }
			      // Ignore errors and move on
			    }
			  }

			  // Write out the results

			  if(cs->parse_results(carr))
			  {
			    cs->write_results();
			    carr = NULL;  // cs is now managing it
			  }
			}
			else
			  wlog->warn("Worker::verify_type2_dependencies failed to allocate Array or CharBuffer");

			// If carr is still valid, we should clear it
			xadelete(carr, CheckResult);
			xdelete(cmt);

			xdelete(cs);
		      }
		    }
		  }
		}
	      }
	      else
		wlog->warn("Memory allocation error in verify_type2_dependencies");

	      xdelete(creq);
	      xdelete(e);
	    }
	  }

	  // Don't do extra work if something failed
	  
	  if(!r)
	    break;
	}
      }
    }
  }
  else
    r = false;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Worker::verify_type2_dependencies = %s",
		 IOTF(r));
#endif
  
  return(r);
}

bool Worker::verify_type2_dependencies(Fix *f)
{
  // Execute any dependencies for the Fix <f>.  No state is modified.

  // Returns: false if any dependencies failed, or true otherwise.

  bool ret = true;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Worker::verify_type2_dependencies(%d)", f);
#endif

  if(f)
  {
    Array<Dependency> *ds = f->dependencies();

    if(ds)
    {
      for(int i = 0;i < ds->entries();i++)
      {
	Dependency *d = ds->retrieve(i);

	if(d && d->dep_type() == 2 && d->dependency()->name())
	{
	  // Verify the dependency's dependencies

	  if(!d->dependency()->dependencies() ||
	     verify_type2_dependencies(d->dependency(),
				       cf->max_dependencies(), NULL))
	  {
	    // Run this dependency

	    Executor *e = new Executor();
	    CheckRequest *creq =
	      new CheckRequest(d->target(), f->fix_timeout(),
			       d->dependency()->modargs(), false);

	    if(e && creq)
	    {
	      pid_t pid = e->exec_check(d->dependency(), creq);

	      if(pid > 0)
	      {
		// Since we aren't reading from a pipe, only the pid
		// needs to be timed out.
		
		pc->request_timeout(pid, SIGKILL, f->fix_timeout());
		
		int er = e->result();
		
		if(!pc->unrequest_timeout(pid))
		{
		  // Except in boundary race conditions, this indicates
		  // the check timed out.  Consider this a failure.
		  
		  er = MODEXEC_TIMEDOUT;
		}
		
		if(er != MODEXEC_OK)
		  ret = false;
	      }
	    }
	    else
	      wlog->warn("Memory allocation error in verify_type2_dependencies");

	    xdelete(creq);
	    xdelete(e);
	  }
	}

	if(!ret)
	  break;
      }    
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Worker::verify_type2_dependencies = %s",
		 IOTF(ret));
#endif
  
  return(ret);
}
