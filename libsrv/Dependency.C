/*
 * Dependency.C: An object to hold host dependency definitions.
 *
 * Version: $Revision: 0.11 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2005/08/02 03:18:39 $
 * MT-Level: Safe
 *
 * Copyright (c) 2002 - 2003
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: Dependency.C,v $
 * Revision 0.11  2005/08/02 03:18:39  benno
 * Fix incorrect default return value in dependent()
 *
 * Revision 0.10  2003/10/06 23:10:22  benno
 * Changes for dependency overhaul
 *
 * Revision 0.9  2003/04/09 20:23:47  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.8  2003/04/04 22:23:00  benno
 * Use Debugger
 *
 * Revision 0.7  2003/03/04 17:58:14  benno
 * Bump copyright
 *
 * Revision 0.6  2003/01/24 16:47:28  benno
 * Use IONULL, IOTF, xdelete, and xadelete
 *
 * Revision 0.5  2002/10/21 20:46:57  benno
 * add support for named args
 *
 * Revision 0.4  2002/04/04 20:10:05  benno
 * copyright
 *
 * Revision 0.3  2002/04/03 18:11:19  benno
 * rcsify date
 *
 * Revision 0.2  2002/04/03 18:11:11  benno
 * Add host exceptions
 *
 * Revision 0.1  2002/04/03 18:10:36  benno
 * initial revision
 *
 */

#include "survivor.H"

Dependency::Dependency(Check *dependency, int deptype, char *targethost,
		       List *sourcehosts, List *srcexceptions)
{
  // Allocate and initialize a new Dependency object, dependent on the
  // Check <dependency>.  <deptype> indicates whether this is of Type I
  // (existing state is examined) or Type II (dependency is checked
  // in real time).  If <targethost> is provided, then the dependency
  // is against that host, otherwise the host currently being examined
  // is implied.  <sourcehosts> indicates which hosts are subject to
  // this Dependency (NULL indicates all hosts), except those specified
  // in <srcexceptions>.  <dependency> must remain valid for the duration
  // of this Dependency, all other data is maintained internally.

  // Returns: A new Dependency object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Dependency::Dependency(%d,%d,%s,%d,%d)",
		  dependency, deptype, IONULL(targethost),
		  sourcehosts, srcexceptions);
#endif

  d = dependency;
  dt = deptype;
  th = xstrdup(targethost);
  if(sourcehosts)
    shs = new List(sourcehosts);
  else
    shs = NULL;
  if(srcexceptions)
    sxhs = new List(srcexceptions);
  else
    sxhs = NULL;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Dependency::Dependency()");
#endif
}

int Dependency::dep_type()
{
  // Obtain the Type of this Dependency.

  // Returns: The Type of this Dependency (1 for state examination,
  // 2 for real time checking).

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Dependency::dep_type()");
  dlog->log_exit(DEBUG_MINTRC, "Dependency::dep_type = %d", dt);
#endif
  
  return(dt);
}

Check *Dependency::dependency()
{
  // Obtain the dependency for this Dependency.

  // Returns: A pointer to the Check, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Dependency::dependency()");
  dlog->log_exit(DEBUG_MINTRC, "Dependency::dependency = %d", d);
#endif
  
  return(d);
}

bool Dependency::dependent(char *host)
{
  // Determine if <host> is dependent on this Check.  This examines
  // the list of source hosts and source exceptions provided at the
  // instantiation of this Dependency.

  // Returns: true if <host> depends on this Dependency, or false
  // otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Dependency::dependent(%s)", IONULL(host));
#endif
  
  if(host &&
     (!shs || shs->find(host)!=-1) &&
     (!sxhs || sxhs->find(host)==-1))
    ret = true;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Dependency::dependent = ", IOTF(ret));
#endif

  return(ret);
}

char *Dependency::target()
{
  // Obtain the name of the dependency host for this Dependency, if
  // specified.

  // Returns: The name of the host to be examined, or NULL to indicate
  // that the host being checked is the dependency host.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Dependency::target()");
  dlog->log_exit(DEBUG_MINTRC, "Dependency::target = %s", IONULL(th));
#endif
  
  return(th);
}

Dependency::~Dependency()
{
  // Deallocate the Dependency object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Dependency::~Dependency()");
#endif

  d = NULL;
  dt = 0;
  xdelete(th);
  xdelete(shs);
  xdelete(sxhs);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Dependency::~Dependency()");
#endif
}
