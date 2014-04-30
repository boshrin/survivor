/*
 * AlertModule.C: survivor AlertModule object
 *
 * Version: $Revision: 0.1 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2003/05/29 00:30:21 $
 * MT-Level: Safe
 *
 * Copyright (c) 2003
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: AlertModule.C,v $
 * Revision 0.1  2003/05/29 00:30:21  benno
 * Initial revision
 *
 */

#include "survivor.H"

AlertModule::AlertModule(char *name, char *format, char *transmit)
{
  // Allocate and initialize a new AlertModule object named <name>,
  // used to store <format> and <transmit>.

  // Returns: A new AlertModule object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertModule::AlertModule(%s,%s,%s)",
		  IONULL(name), IONULL(format), IONULL(transmit));
#endif
  
  f = xstrdup(format);
  n = xstrdup(name);
  t = xstrdup(transmit);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertModule::AlertModule()");
#endif
}

char *AlertModule::format()
{
  // Obtain the name of the Format module for this AlertModule.

  // Returns: The name, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertModule::format()");
  dlog->log_exit(DEBUG_MINTRC, "AlertModule::format = %s", IONULL(f));
#endif
  
  return(f);
}

char *AlertModule::name()
{
  // Obtain the name of this AlertModule.

  // Returns: The name, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertModule::name()");
  dlog->log_exit(DEBUG_MINTRC, "AlertModule::name = %s", IONULL(n));
#endif
  
  return(n);
}

char *AlertModule::transmit()
{
  // Obtain the name of the Transmit module for this AlertModule.

  // Returns: The name, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertModule::transmit()");
  dlog->log_exit(DEBUG_MINTRC, "AlertModule::transmit = %s", IONULL(t));
#endif
  
  return(t);
}

AlertModule::~AlertModule()
{
  // Delete the AlertModule object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertModule::~AlertModule()");
#endif

  xdelete(f);
  xdelete(n);
  xdelete(t);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertModule::~AlertModule()");
#endif
}
