/*
 * StatCounter.C: survivor StatCounter object
 *
 * Version: $Revision: 0.2 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2004/08/25 00:47:15 $
 * MT-Level: Safe, if analysis is performed exactly once.  Under threaded
 *  circumstances, access should be protected identically to the
 *  Configuration object.
 *
 * Copyright (c) 2004
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: StatCounter.C,v $
 * Revision 0.2  2004/08/25 00:47:15  benno
 * Fix typo
 *
 * Revision 0.1  2004/03/01 23:15:39  benno
 * Initial revision
 *
 */

#include "survivor.H"

StatCounter::StatCounter()
{
  // Allocate and initialize a new StatCounter object.

  // Returns: A new, initialized StatCounter object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "StatCounter::StatCounter()");
#endif

  ctrs = NULL;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "StatCounter::StatCounter()");
#endif
}

bool StatCounter::analyze_configuration()
{
  // Perform analysis of the current Configuration.  For threaded
  // applications, a configuration read lock should be obtained
  // before invoking this method.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "StatCounter::analyze_configuration()");
#endif

  if(cf)
  {
    if(ctrs)
    {
      xhdelete(ctrs, Counter);
    }
    
    ctrs = new Hashtable<Counter>();

    if(ctrs)
    {
      // Start by getting all checks
      
      Array<Check> *checks = cf->get_all_checks();

      if(checks)
      {
	// Assume success unless otherwise noted
	ret = true;
	
	for(int i = 0;i < checks->entries();i++)
	{
	  Check *c = checks->retrieve(i);

	  if(c)
	  {
	    // Find all hosts for this check

	    List *hosts = cf->find_group(c->name());

	    if(hosts)
	    {
	      // If the check has a schedule defined, that's the
	      // schedule in use, otherwise dig up the host's class.

	      if(c->check_schedule())
	      {
		CharBuffer *cb = new CharBuffer("cf:svc=");

		if(cb)
		{
		  cb->append(c->name());
		  cb->append(":sched=");
		  cb->append(c->check_schedule()->name());

		  increment_counter(cb->str());
		  
		  xdelete(cb);
		}
		else
		{
		  wlog->warn("StatCounter::analyze failed to allocate CharBuffer");
		  ret = false;
		}
	      }
	      else
	      {
		// Iterate through hosts

		for(int j = 0;j < hosts->entries();j++)
		{
		  char *h = hosts->retrieve(j);

		  if(h)
		  {
		    HostClass *hc = cf->find_class(h);

		    if(hc && hc->check_schedule())
		    {
		      CharBuffer *cb = new CharBuffer("cf:svc=");

		      if(cb)
		      {
			cb->append(c->name());
			cb->append(":sched=");
			cb->append(hc->check_schedule()->name());

			increment_counter(cb->str());

			xdelete(cb);
		      }
		      else
		      {
			wlog->warn("StatCounter::analyze_configuration failed to allocate CharBuffer");
			ret = false;
		      }
		    }
		  }
		}
	      }
	    }
	  }
	}
      }
    }
    else
      wlog->warn("StatCounter::analyze_configuration failed to allocate Hashtable");
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "StatCounter::analyze_configuration = %s", IOTF(ret));
#endif

  return(ret);
}

int StatCounter::cf_hosts_checked_on_schedule(char *schedule, char *service)
{
  // Obtain the number of hosts checked for <service> on <schedule>
  // according to the configuration.

  // Returns: The number of hosts checked for <service> on <schedule>.

  int ret = 0;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "StatCounter::cf_hosts_checked_on_schedule()");
#endif

  if(ctrs && schedule && service)
  {
    CharBuffer *cb = new CharBuffer("cf:svc=");

    if(cb)
    {
      cb->append(service);
      cb->append(":sched=");
      cb->append(schedule);

      Counter *c = ctrs->retrieve(cb->str());

      if(c)
	ret = c->value();

      xdelete(cb);
    }
    else
      wlog->warn("StatCounter::analyze failed to allocate CharBuffer");
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC,
		 "StatCounter::cf_hosts_checked_on_schedule = %d", ret);
#endif

  return(ret);
}

StatCounter::~StatCounter()
{
  // Delete the StatCounter object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "StatCounter::~StatCounter()");
#endif

  xhdelete(ctrs, Counter);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "StatCounter::~StatCounter()");
#endif
}

bool StatCounter::increment_counter(char *name)
{
  // Increment the value for the Counter <name>.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "StatCounter::increment_counter(%s)",
		  IONULL(name));
#endif

  if(ctrs && name)
  {
    Counter *c = ctrs->retrieve(name);

    if(!c)
    {
      c = new Counter(name, 1);

      if(c)
      {
	if(!ctrs->insert(c->name(), c))
	  xdelete(c);
      }
      else
	wlog->warn("StatCounter::increment_counter failed to allocate Counter");
    }

    if(c)
      ret = c->increment();
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "StatCounter::increment_counter = %s",
		 IOTF(ret));
#endif

  return(ret);
}
