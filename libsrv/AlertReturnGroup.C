/*
 * AlertReturnGroup: An object to hold alertplan returngroup information.
 *
 * Version: $Revision: 0.6 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2003/10/06 22:51:04 $
 * MT-Level: Safe
 *
 * Copyright (c) 2002 - 2003
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: AlertReturnGroup.C,v $
 * Revision 0.6  2003/10/06 22:51:04  benno
 * Update constructor target/replacement documentation
 *
 * Revision 0.5  2003/04/09 20:23:44  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.4  2003/04/03 18:35:12  benno
 * Use Debugger
 *
 * Revision 0.3  2003/03/04 17:54:44  benno
 * Bump copyright
 *
 * Revision 0.2  2003/01/23 20:02:09  benno
 * Use IONULL, IOTF, xdelete, and xadelete
 *
 * Revision 0.1  2002/12/16 00:31:01  benno
 * Initial revision
 *
 */

#include "survivor.H"

AlertReturnGroup::AlertReturnGroup(Array<int> *retvals, int failures,
				   Array<AlertSchedule> *schedules)
{
  // Allocate and initialize a new AlertReturnGroup object.  <retvals>
  // is the set of return groups that apply.  Although the Array is
  // technically (int *), each entry is treated as a number (int).
  // <failures> indicates the number of failed checks required before
  // this ReturnGroup applies, and <schedules> is the Array of
  // AlertSchedules that describe the behavior of this ReturnGroup.
  // <retvals> and <schedules> will be maintained by this object.

  // Returns: A new AlertReturnGroup object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC,
		   "AlertReturnGroup::AlertReturnGroup(%d,%d,%d)",
		   retvals, failures, schedules);
#endif
  
  as = schedules;
  rvs = retvals;
  fails = failures;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertReturnGroup::AlertReturnGroup()");
#endif
}

AlertReturnGroup::AlertReturnGroup(AlertReturnGroup *arg, CallList *target,
				   CallList *replacement)
{
  // Allocate and initialize a new AlertReturnGroup object, based on the
  // contents of <arg>, but replacing <target> with <replacement> in any
  // AlertTry objects stored within.  If <target> is NULL, <replacement>
  // will be appended instead.

  // Returns: A new AlertReturnGroup object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC,
		   "AlertReturnGroup::AlertReturnGroup(%d,%d,%d)",
		   arg, target, replacement);
#endif

  // Set up safe defaults in case we encounter an error
  
  as = NULL;
  rvs = NULL;
  fails = 0;

  if(arg)
  {
    // We maintain as here, so we need to recreate it.
    Array<AlertSchedule> *srcscheds = arg->schedules();
    as = new Array<AlertSchedule>();

    if(as && srcscheds)
    {
      for(int i = 0;i < srcscheds->entries();i++)
      {
	if(srcscheds->retrieve(i))
	{
	  AlertSchedule *s = new AlertSchedule(srcscheds->retrieve(i),
					       target,
					       replacement);

	  if(s)
	  {
	    if(!as->add(s))
	    {
	      delete s;
	      wlog->warn("AlertReturnGroup::AlertReturnGroup unable to insert AlertSchedule");
	    }
	  }
	  else
	    wlog->warn("AlertReturnGroup::AlertReturnGroup unable to allocate AlertSchedule");
	}
      }

      // We also need to recreate rvs only if it is not named.
      // Otherwise, it is a pointer into the configuration object and
      // we can just store a pointer.

      Array<int> *srcrvs = arg->_allrvs();

      if(srcrvs)
      {
	if(srcrvs->name())
	  rvs = srcrvs;
	else
	{
	  rvs = new Array<int>();
	  
	  if(rvs)
	  {
	    // We're storing numbers cast as pointers
	    
	    for(int i = 0;i < srcrvs->entries();i++)
	      rvs->add(srcrvs->retrieve(i));
	  }
	}
      }
      // else "default", so leave rvs as NULL

      // Finally, set fails
      
      fails = arg->failures();
    }
    else
      wlog->warn("AlertReturnGroup::AlertReturnGroup unable to allocate Array");
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertReturnGroup::AlertReturnGroup()");
#endif  
}

Array<int> *AlertReturnGroup::_allrvs()
{
  // This method is for use by the configuration parser only and is otherwise
  // not to be used.

  // Returns: Something or nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertReturnGroup::_allrvs()");
  dlog->log_exit(DEBUG_MINTRC, "AlertReturnGroup::_allrvs = %d", rvs);
#endif
  
  return(rvs);
}

int AlertReturnGroup::failures()
{
  // Determine the number of consecutive check failures required for
  // this ReturnGroup.

  // Returns: The number of failures required.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertReturnGroup::failures()");
  dlog->log_exit(DEBUG_MINTRC, "AlertReturnGroup::failures = %d", fails);
#endif
  
  return(fails);
}

bool AlertReturnGroup::match(int matchval, bool matchdef)
{
  // Determine if this ReturnGroup matches the return value <matchval>.
  // If <matchdef> is true, then if the return value for this ReturnGroup
  // was specified as "default", <matchval> will always match.

  // Returns: true if this ReturnGroup matches, or false otherwise.

  bool ret = false;

  // rvs of NULL indicates use default

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertReturnGroup::match(%d,%s)",
		   matchval, IOTF(matchdef));
#endif
  
  if((!rvs && matchdef) || find_rv(matchval))
    ret = true;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertReturnGroup::match = %s", IOTF(ret));
#endif
  
  return(ret);
}

Array<AlertSchedule> *AlertReturnGroup::schedules()
{
  // Obtain the Array of AlertSchedules for this ReturnGroup.

  // Returns: A pointer to the Array of AlertSchedules, or NULL on error.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertReturnGroup::schedules()");
  dlog->log_exit(DEBUG_MINTRC, "AlertReturnGroup::schedules = %d", as);
#endif
  
  return(as);
}

AlertReturnGroup::~AlertReturnGroup()
{
  // Deallocate the AlertReturnGroup object.

  // Returns: Nothing.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertReturnGroup::~AlertReturnGroup()");
#endif

  xadelete(as, AlertSchedule);

  if(rvs && !rvs->name())
  {
    // Don't delete the contents, they're ints, not pointers.
    // Don't delete rvs if it is not named.  Named Arrays are
    // pointers from the Configuration file.

    xdelete(rvs);
  }

  fails = 0;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertReturnGroup::~AlertReturnGroup()");
#endif
}

bool AlertReturnGroup::find_rv(int rv)
{
  // Determine if <rv> is in <rvs>.

  // Returns: true if <rv> is found, false otherwise, including if <rvs>
  // is NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertReturnGroup::find_rv(%d)", rv);
#endif
  
  if(rvs)
  {
    for(int i = 0;i < rvs->entries();i++)
    {
      if((int)rvs->retrieve(i) == rv)
      {
#if defined(DEBUG)
        dlog->log_exit(DEBUG_MINTRC, "AlertReturnGroup::find_rv = true");
#endif
        
        return(true);
      }
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertReturnGroup::find_rv = false");
#endif
        
  return(false);
}
