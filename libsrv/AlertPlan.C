/*
 * AlertPlan.C: An object to hold alertplan definitions.
 *
 * Version: $Revision: 0.16 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2005/11/21 00:06:17 $
 * MT-Level: Safe
 *
 * Copyright (c) 2002 - 2005
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: AlertPlan.C,v $
 * Revision 0.16  2005/11/21 00:06:17  benno
 * Always clear_state_on MODEXEC_OK
 *
 * Revision 0.15  2005/10/20 02:12:46  benno
 * AlertPlan tracks when state should be cleared
 *
 * Revision 0.14  2003/10/06 22:50:43  benno
 * Update constructor target/replacement documentation
 *
 * Revision 0.13  2003/04/09 20:23:44  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.12  2003/04/03 18:29:42  benno
 * Use Debugger
 *
 * Revision 0.11  2003/03/04 17:54:33  benno
 * Bump copyright
 *
 * Revision 0.10  2003/01/23 20:00:12  benno
 * Use IONULL, IOTF, xdelete, and xadelete
 *
 * Revision 0.9  2002/12/22 17:10:54  benno
 * Remove global notify on clear
 *
 * Revision 0.8  2002/12/16 00:33:43  benno
 * Conversion to try based AlertPlans
 *
 * Revision 0.7  2002/05/29 21:17:39  benno
 * add alertplan checking to escalation()
 *
 * Revision 0.6  2002/04/04 20:06:10  benno
 * copyright
 *
 * Revision 0.5  2002/04/03 16:56:34  benno
 * rcsify date
 *
 * Revision 0.4  2002/04/03 16:56:15  benno
 * Add _allX methods
 * Add matchfirst flag to match()
 *
 * Revision 0.3  2002/04/03 16:55:33  benno
 * Allow more than one retval
 *
 * Revision 0.2  2002/04/03 16:54:52  benno
 * Move much functionality to AlertNugget to support escalations
 *
 * Revision 0.1  2002/04/03 16:53:59  benno
 * initial revision
 *
 */

#include "survivor.H"

AlertPlan::AlertPlan(char *name, Array<AlertReturnGroup> *retgroups,
		     bool noteclear, Array<Schedule> *notesched,
		     Array<int> *noclearon, List *noclearrg)
{
  // Allocate and initialize a new AlertPlan object named <name> and
  // holding the Array of AlertReturnGroups <retgroups>, notify on
  // clear status as determined by <noteclear> and <notesched>, and
  // clear state determined by <noclearon> and <noclearrg>.
  // <retgroups>, <noclearon>, and <noclearrg> will be maintained by
  // the AlertPlan object.

  // Returns: A new AlertPlan object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertPlan::AlertPlan(%s,%d,%s,%d,%d,%d)",
		  IONULL(name), retgroups, IOTF(noteclear), notesched,
		  noclearon, noclearrg);
#endif

  rgs = retgroups;
  noclear = noclearon;
  noclearr = noclearrg;
  nocsched = notesched;
  noc = noteclear;
  n = xstrdup(name);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertPlan::AlertPlan()");
#endif
}

AlertPlan::AlertPlan(char *name, AlertPlan *ap, CallList *target,
		     CallList *replacement)
{
  // Allocate and initialize a new AlertPlan object named <name>, based
  // on the contents of <ap>, but replacing <target> with
  // <replacement> in any AlertTry objects stored within.  If <target>
  // is NULL, <replacement> will be appended to the AlertTry CallLists
  // instead.

  // Returns: A new AlertPlan object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertPlan::AlertPlan(%s,%d,%d,%d)",
		   IONULL(name), ap, target, replacement);
#endif

  // Set up safe defaults in case we encounter an error

  rgs = NULL;
  noclear = NULL;
  noclearr = NULL;
  nocsched = NULL;
  noc = false;
  n = NULL;

  if(ap)
  {
    // We maintain rgs here, so we need to recreate it
    Array<AlertReturnGroup> *srcrgs = ap->_allrgs();
    rgs = new Array<AlertReturnGroup>();

    if(rgs && srcrgs)
    {
      for(int i = 0;i < srcrgs->entries();i++)
      {
	if(srcrgs->retrieve(i))
	{
	  AlertReturnGroup *rg = new AlertReturnGroup(srcrgs->retrieve(i),
						      target,
						      replacement);

	  if(rg)
	  {
	    if(!rgs->add(rg))
	    {
	      delete rg;
	      wlog->warn("AlertPlan::AlertPlan unable to insert AlertReturnGroup");
	    }
	  }
	  else
	    wlog->warn("AlertPlan::AlertPlan unable to allocate AlertReturnGroup");
	}
      }
    }
    else
      wlog->warn("AlertPlan::AlertPlan unable to allocate Array");

    // Copy clear state, noclear is easy since it's a bunch of ints

    noclear = (Array<int> *)new iArray(ap->_noclr());
    noclearr = new List(ap->_noclrrg());
    
    // Copy notify on clear

    nocsched = ap->_nocscd();
    noc = ap->_nocset();
    
    // Copy the name

    n = xstrdup(name);
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertPlan::AlertPlan()");
#endif
}

Array<AlertReturnGroup> *AlertPlan::_allrgs()
{
  // This method is for use by the configuration parser only and is otherwise
  // not to be used.

  // Returns: Something or nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertPlan::_allrgs()");
  dlog->log_exit(DEBUG_MINTRC, "AlertPlan::_allrgs = %d", rgs);
#endif
  
  return(rgs);
}

Array<int> *AlertPlan::_noclr()
{
  // This method is for use by the configuration parser only and is otherwise
  // not to be used.

  // Returns: Something or nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertPlan::_noclr()");
  dlog->log_exit(DEBUG_MINTRC, "AlertPlan::_noclr = %d", noclear);
#endif
  
  return(noclear);
}

List *AlertPlan::_noclrrg()
{
  // This method is for use by the configuration parser only and is otherwise
  // not to be used.

  // Returns: Something or nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertPlan::_noclrrg()");
  dlog->log_exit(DEBUG_MINTRC, "AlertPlan::_noclrrg = %d", noclearr);
#endif
  
  return(noclearr);
}

Array<Schedule> *AlertPlan::_nocscd()
{
  // This method is for use by the configuration parser only and is otherwise
  // not to be used.

  // Returns: Something or nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertPlan::_nocscd()");
  dlog->log_exit(DEBUG_MINTRC, "AlertPlan::_nocscd = %d", nocsched);
#endif
  
  return(nocsched);
}

bool AlertPlan::_nocset()
{
  // This method is for use by the configuration parser only and is otherwise
  // not to be used.

  // Returns: Something or nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertPlan::_nocset()");
  dlog->log_exit(DEBUG_MINTRC, "AlertPlan::_nocset = %s", IOTF(noc));
#endif
  
  return(noc);
}

bool AlertPlan::clear_state_on(int fromrc, int torc)
{
  // Determine if state should be cleared on change from <fromrc> to
  // <torc>.  Note that this method will access configuration state,
  // and so in a multithreaded environment a read lock must be
  // obtained on the configuration before it is called
  
  // Returns: true if state should be cleared, false otherwise.

  bool ret = true;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertPlan::clear_state_on(%d,%d)",
		  fromrc, torc);
#endif

  // We always clear if the new rc is OK
  
  if(torc != MODEXEC_OK)
  {
    if(noclear)
    {
      // The array says which states to NOT clear on, so we default to true
      // unless <rc> is found.

      for(int i = 0;i < noclear->entries();i++)
      {
	int rc = (int)noclear->retrieve(i);
	
	if(rc == fromrc || rc == torc)
	{
	  ret = false;
	  break;
	}
      }
    }

    if(ret && noclearr)
    {
      // If both from and to are found in the same alertplan, don't clear state
      
      for(int i = 0;i < noclearr->entries();i++)
      {
	Array<int> *rg = cf->find_returngroup(noclearr->retrieve(i));
	
	if(rg)
	{
	  bool f = false;
	  bool t = false;
	  
	  for(int j = 0;j < rg->entries();j++)
	  {
	    int rc = (int)rg->retrieve(j);
	    
	    if(rc == fromrc)
	      f = true;
	    
	    if(rc == torc)
	      t = true;
	  }
	  
	  if(f && t)
	  {
	    ret = false;
	    break;
	  }
	}
      }
    }
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertPlan::clear_state_on = %s", IOTF(ret));
#endif
  
  return(ret);
}

AlertReturnGroup *AlertPlan::match(int retval, bool matchdef, int failures)
{
  // Determine if this AlertPlan contains a stanza matching the return
  // value <retval>.  If <matchdef> is true, then if the return value for
  // a "default" stanza is found, <retval> will match it.  <failures> is
  // the number of consecutive check failures that have occurred.

  // A pointer to an AlertReturnGroup (the contents of the stanza
  // matching the return value) if found, NULL otherwise.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertPlan::match(%d,%s,%d)",
		   retval, IOTF(matchdef), failures);
#endif

  if(rgs)
  {
    for(int i = 0;i < rgs->entries();i++)
    {
      AlertReturnGroup *rg = rgs->retrieve(i);

      // We need to verify two things about this ReturnGroup: that
      // the number of failures matches and that the return value
      // matches.  We check return value first and then failures
      // because that is how it is encoded in the configuration file,
      // but in practice it shouldn't matter since both have to be
      // checked anyway.  (It is possible to define multiple stanzas
      // for the same returngroup, one after n failures, the other
      // after n+m failures.)

      if(rg && rg->match(retval, matchdef) && (failures >= rg->failures())
	 && rg->schedules())
      {	
#if defined(DEBUG)
	dlog->log_exit(DEBUG_MINTRC, "AlertPlan::match = %d", rg);
#endif  
	return(rg);
      }
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertPlan::match = NULL");
#endif
  
  return(NULL);
}

char *AlertPlan::name()
{
  // Obtain the name of this AlertPlan.

  // Returns: The name, or NULL on error.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertPlan::name()");
  dlog->log_exit(DEBUG_MINTRC, "AlertPlan::name = %s", IONULL(n));
#endif
  
  return(n);
}

bool AlertPlan::notify_on_clear()
{
  // Determine if notify on clear is currently in effect for this
  // AlertPlan.

  // Returns: true if notify on clear is currently in effect for this
  // AlertPlan, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertPlan::notify_on_clear()");
#endif

  if(noc)
  {
    if(nocsched)
    {
      for(int i = 0;i < nocsched->entries();i++)
      {
	Schedule *s = nocsched->retrieve(i);

	// We use alert_shift here to keep notify_on_clear in line with
	// the alert schedules.
	
	if(s && s->now(-1 * cf->alert_shift()))
	{
#if defined(DEBUG)
	  dlog->log_progress(DEBUG_SCHEDS,
	       " -- Schedule %s, line %d is now in effect for notify on clear",
			      IONULL(nocsched->name()), i+1);
#endif

	  ret = true;
	  break;
	}
      }
    }
    else
      ret = true;
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertPlan::notify_on_clear = %s", IOTF(ret));
#endif
  
  return(ret);
}

AlertPlan::~AlertPlan()
{
  // Deallocate the AlertPlan object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertPlan::~AlertPlan()");
#endif

  // We have to clean up rgs

  xadelete(rgs, AlertReturnGroup);
  xdelete(noclear);  // This is int and not pointers, so no xadelete
  xdelete(noclearr);
  nocsched = NULL;
  noc = false;
  xdelete(n);
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertPlan::~AlertPlan()");
#endif
}
