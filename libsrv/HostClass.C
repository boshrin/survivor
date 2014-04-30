/*
 * HostClass.C: An object to hold host class definitions.
 *
 * Version: $Revision: 0.8 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2003/04/09 20:23:48 $
 * MT-Level: Safe
 *
 * Copyright (c) 2002 - 2003
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: HostClass.C,v $
 * Revision 0.8  2003/04/09 20:23:48  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.7  2003/04/05 23:10:12  benno
 * Use Debugger
 *
 * Revision 0.6  2003/03/04 20:29:03  benno
 * Bump copyright
 *
 * Revision 0.5  2003/01/24 17:24:39  benno
 * Use IONULL, IOTF, xdelete, and xadelete
 *
 * Revision 0.4  2002/12/16 00:47:27  benno
 * Use new AlertPlan object
 *
 * Revision 0.3  2002/04/04 20:10:36  benno
 * copyright
 *
 * Revision 0.2  2002/04/03 18:18:34  benno
 * rcsify date
 *
 * Revision 0.1  2002/04/03 18:17:57  benno
 * initial revision
 *
 */

#include "survivor.H"

HostClass::HostClass(char *name, List *hosts, AlertPlan *alert,
		     Array<Schedule> *sched)
{
  // Allocate and initialize a new HostClass object, identified by <name>,
  // and containing the set of <hosts> that are scheduled by default on the
  // <sched> schedule, and alerted by default on the <alert> schedule.
  // It is assumed that the HostClass object will maintain exclusive control
  // over <hosts>, and delete it when the HostClass is deleted.  <name> is
  // duplicated, and so need not remain active.  <alert> and <sched> are
  // pointers to objects that must remain valid for the life of this object.

  // Returns: A new HostClass object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "HostClass::HostClass(%s,%d,%d,%d)",
		   IONULL(name), hosts, alert, sched);
#endif

  classname = xstrdup(name);
  hostlist = hosts;
  alertplan = alert;
  checksched = sched;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "HostClass::HostClass()");
#endif
}

AlertPlan *HostClass::alert_plan()
{
  // Obtain the default alert schedule for this HostClass.

  // Returns: A pointer to the default AlertPlan, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "HostClass::alert_plan()");
  dlog->log_exit(DEBUG_MINTRC, "HostClass::alert_plan = %d", alertplan);
#endif
  
  return(alertplan);
}

Array<Schedule> *HostClass::check_schedule()
{
  // Obtain the default check schedule for this HostClass.

  // Returns: A pointer to the default check Schedule, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "HostClass::check_schedule()");
  dlog->log_exit(DEBUG_MINTRC, "HostClass::check_schedule = %d", checksched);
#endif
  
  return(checksched);
}

List *HostClass::hosts()
{
  // Obtain the List of hosts of this HostClass.

  // Returns: A pointer to the List of hosts, which should not be deleted,
  // or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "HostClass::hosts()");
  dlog->log_exit(DEBUG_MINTRC, "HostClass::hosts = %d", hostlist);
#endif
  
  return(hostlist);
}

char *HostClass::name()
{
  // Obtain the name of this HostClass.

  // Returns: The HostClass name, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "HostClass::name()");
  dlog->log_exit(DEBUG_MINTRC, "HostClass::name = %s", IONULL(classname));
#endif
  
  return(classname);
}

HostClass::~HostClass()
{
  // Deallocate the HostClass object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "HostClass::~HostClass()");
#endif

  alertplan = NULL;
  checksched = NULL;

  xdelete(classname);
  xdelete(hostlist);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "HostClass::~HostClass()");
#endif
}
