/*
 * Check.C: An object to hold host check definitions.
 *
 * Version: $Revision: 0.15 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2005/04/06 14:06:27 $
 * MT-Level: Safe
 *  MT Safe only if the object is not updated after the Configuration is
 *  parsed.
 *
 * Copyright (c) 2002 - 2004
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: Check.C,v $
 * Revision 0.15  2005/04/06 14:06:27  benno
 * Use Array<Argument> instead of List
 *
 * Revision 0.14  2004/06/12 00:59:02  benno
 * Add result_text_significant
 *
 * Revision 0.13  2003/10/06 22:53:44  benno
 * Change Dependency deletion as part of dependency overhaul
 *
 * Revision 0.12  2003/04/09 20:23:46  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.11  2003/04/04 21:44:03  benno
 * Use Debugger
 *
 * Revision 0.10  2003/03/04 17:57:45  benno
 * Add support for composite checks
 *
 * Revision 0.9  2003/01/23 22:47:56  benno
 * Add IONULL and IOTF
 *
 * Revision 0.8  2002/12/31 04:29:09  benno
 * Add fix
 *
 * Revision 0.7  2002/12/22 17:29:17  benno
 * Add Transports
 * Switch to xdelete
 *
 * Revision 0.6  2002/12/16 00:42:17  benno
 * Use new AlertPlan object
 *
 * Revision 0.5  2002/10/21 20:47:31  benno
 * add support for named args
 *
 * Revision 0.4  2002/04/04 20:09:06  benno
 * copyright
 *
 * Revision 0.3  2002/04/03 17:51:43  benno
 * rcsify date
 *
 * Revision 0.2  2002/04/03 17:51:23  benno
 * Check all option
 *
 * Revision 0.1  2002/04/03 17:50:37  benno
 * initial revision
 *
 */

#include "survivor.H"

Check::Check(char *name, char *module, Array<Argument> *modargs,
	     int timeout, bool all, Array<Schedule> *sched,
	     AlertPlan *alert, char *helpfile, Transport *transport,
	     Fix *fix, bool restextsig)
{
  // Allocate and initialize a new Check object, identified by <name>.
  // This object runs <module> with optional arguments <modargs>, and
  // times out in <timeout> seconds.  If <all> is true, examine all
  // defined hosts, not just those in a matching group.  If <sched>
  // and <alert> are specified (both of which must remain valid for
  // the duration of this object), they override the class
  // definitions.  <helpfile> is a text file to send with an alert.
  // If <transport> is specified (it must remain valid for the
  // duration of this object), the Check should be executed via that
  // Transport module.  <fix>, if specified, indicates the Fix module
  // to be used when this Check fails, and must remain valid for the
  // duration of this object.  If <restextsig> is true, the text
  // description returned by the executed check will be considered
  // significant by the scheduler. <modargs> will be managed by this
  // object and deleted when no longer required.

  // Returns: A new Check object.

  init(name, module, modargs, NULL, NULL, timeout, all, sched,
       alert, helpfile, transport, fix, restextsig);
}

Check::Check(char *name, Array<Check> *required, Array<Check> *optional,
	     int timeout, bool all, Array<Schedule> *sched, AlertPlan *alert,
	     char *helpfile, Fix *fix, bool restextsig)
{
  // Allocate and initialize a new Composite Check object, identified by
  // <name>.  This Check consists of the specified <required> and
  // <optional> Checks.  The management of both Arrays is assumed by
  // this Check.  <timeout> applies, each member check's timeout
  // is ignored.  If <all> is true, examine all defined hosts, not
  // just those in a matching group.  If <sched> and <alert> are specified
  // (both of which must remain valid for the duration of this object),
  // they override the class definitions.  <helpfile> is a text file to
  // send with an alert.  <fix>, if specified, indicates the Fix module
  // to be used when this Check fails, and must remain valid for the
  // duration of this object.  If <restextsig> is true, the text
  // description returned by the executed check will be considered
  // significant by the scheduler.
  
  // Returns: A new Check object.
  
  init(name, NULL, NULL, required, optional, timeout, all, sched, alert,
       helpfile, NULL, fix, restextsig);
}

bool Check::add_dependency(Dependency *d)
{
  // Add <d> as a Dependency this Check depends on.  Dependencies are processed
  // in the order added.

  // Returns: true if the Dependency was successfully added, false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Check::add_dependency(%d)", d);
#endif
  
  if(d)
  {
    if(!depends)
    {
      // Allocate a new Array

      depends = new Array<Dependency>();
    }

    if(depends)
      ret = depends->add(d);
    else
      wlog->warn("Check unable to allocate Dependency Array");
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Check::add_dependency = %s", IOTF(ret));
#endif
  
  return(ret);
}

AlertPlan *Check::alert_plan()
{
  // Obtain the override alert schedule for this Check, if provided.

  // Returns: A pointer to the default AlertPlan, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Check::alert_plan()");
  dlog->log_exit(DEBUG_MINTRC, "Check::alert_plan = %d", alertplan);
#endif
  
  return(alertplan);
}

bool Check::all_hosts()
{
  // Determine if all hosts should be checked, not just those in the
  // corresponding group.

  // Returns: true if so, false otherwise.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Check::all_hosts()");
  dlog->log_exit(DEBUG_MINTRC, "Check::all_hosts = %s", IOTF(checkall));
#endif
  
  return(checkall);
}

Array<Schedule> *Check::check_schedule()
{
  // Obtain the override check schedule for this Check, if provided.

  // Returns: A pointer to the default check Schedule, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Check::check_schedule()");
  dlog->log_exit(DEBUG_MINTRC, "Check::check_schedule = %d", checksched);
#endif
  
  return(checksched);
}

Array<Check> *Check::composite_optional()
{
  // Obtain the set of optional Checks if this is a Composite Check.

  // Returns: A pointer to an Array of Checks that should not be modified,
  // or NULL if this is not a Composite Check or if there are no optional
  // Checks.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Check::composite_optional()");
  dlog->log_exit(DEBUG_MINTRC, "Check::composite_optional = %d", compopt);
#endif
  
  return(compopt);
}

Array<Check> *Check::composite_required()
{
  // Obtain the set of required Checks if this is a Composite Check.

  // Returns: A pointer to an Array of Checks that should not be modified,
  // or NULL if this is not a Composite Check or if there are no required
  // Checks.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Check::composite_required()");
  dlog->log_exit(DEBUG_MINTRC, "Check::composite_required = %d", compreq);
#endif
  
  return(compreq);
}

Array<Dependency> *Check::dependencies()
{
  // Obtain the Dependencies that this Check depends on, if any.

  // Returns: A pointer to an Array of Dependencies that should not be
  // modified.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Check::dependencies()");
  dlog->log_exit(DEBUG_MINTRC, "Check::dependencies = %d", depends);
#endif
  
  return(depends);
}

Fix *Check::fix()
{
  // Obtain the Fix for this Check, if provided.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Check::fix()");
  dlog->log_exit(DEBUG_MINTRC, "Check::fix = %d", fx);
#endif
  
  return(fx);
}

char *Check::helpfile()
{
  // Obtain the helpfile for this Check, if provided.

  // Returns: The helpfile name, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Check::helpfile()");
  dlog->log_exit(DEBUG_MINTRC, "Check::helpfile = %s", IONULL(help));
#endif
  
  return(help);
}

Array<Argument> *Check::modargs()
{
  // Obtain the array of arguments for the module for this Check, if provided.

  // Returns: The array of arguments, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Check::modargs()");
  dlog->log_exit(DEBUG_MINTRC, "Check::modargs = %d", modarg);
#endif
  
  return(modarg);
}

char *Check::module()
{
  // Obtain the name of the module for this Check.

  // Returns: The module name, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Check::module()");
  dlog->log_exit(DEBUG_MINTRC, "Check::module = %s", IONULL(modname));
#endif
  
  return(modname);
}

char *Check::name()
{
  // Obtain the name of this Check.

  // Returns: The Check name, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Check::name()");
  dlog->log_exit(DEBUG_MINTRC, "Check::name = %s", IONULL(checkname));
#endif
  
  return(checkname);
}

bool Check::result_text_significant()
{
  // Determine if the result text description is significant for this
  // Check, for purposes of the Scheduler determining if status has
  // changed.

  // Returns: true if the result text description is significant, false
  // otherwise.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Check::result_text_significant()");
  dlog->log_exit(DEBUG_MINTRC, "Check::result_text_significant = %s",
		 IOTF(rtsig));
#endif
  
  return(rtsig);
}

int Check::timeout()
{
  // Obtain the timeout of this Check.

  // Returns: The timeout.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Check::timeout()");
  dlog->log_exit(DEBUG_MINTRC, "Check::timeout = %d", t);
#endif
  
  return(t);
}

Transport *Check::transport()
{
  // Obtain the Transport module this check should use, if provided.

  // Returns: The Transport module to use, or NULL if the scheduler
  // should execute this Check directly.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Check::transport()");
  dlog->log_exit(DEBUG_MINTRC, "Check::transport = %d", tr);
#endif
  
  return(tr);
}

Check::~Check()
{
  // Deallocate the Check object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Check::~Check()");
#endif

  alertplan = NULL;
  
  // Don't delete the pointers to the checks.
  xdelete(compopt);
  xdelete(compreq);

  // Delete the Dependency Array, but not the Dependencies which are
  // stored in cf.
  xdelete(depends);
  checksched = NULL;
  fx = NULL;
  checkall = false;
  rtsig = false;
  xdelete(checkname);
  xdelete(help);
  xdelete(modname);
  xadelete(modarg, Argument);
  t = -1;
  tr = NULL;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Check::~Check()");
#endif
}

void Check::init(char *name, char *module, Array<Argument> *modargs,
		 Array<Check> *required, Array<Check> *optional,
		 int timeout, bool all, Array<Schedule> *sched,
		 AlertPlan *alert, char *helpfile,
		 Transport *transport, Fix *fix, bool restextsig)
{
  // Initializer for constructors.
  
  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC,
		   "Check::init(%s,%s,%d,%d,%d,%d,%s,%d,%d,%s,%d,%d,%s)",
		   IONULL(name), IONULL(module), modargs, required,
		   optional, timeout, IOTF(all), sched, alert,
		   IONULL(helpfile), transport, fix, IOTF(restextsig));
#endif

  alertplan = alert;
  compopt = optional;
  compreq = required;
  depends = NULL;
  checksched = sched;
  fx = fix;
  checkall = all;
  rtsig = restextsig;
  checkname = xstrdup(name);
  help = xstrdup(helpfile);
  modname = xstrdup(module);
  modarg = modargs;

  if(timeout > -1)
    t = timeout;
  else
    t = -1;

  tr = transport;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Check::init()");
#endif
}
