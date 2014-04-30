/*
 * Transport.C: Survivor transport object.
 *
 * Version: $Revision: 0.5 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2005/04/07 01:27:07 $
 * MT-Level: Safe
 *
 * Copyright (c) 2002 - 2003
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: Transport.C,v $
 * Revision 0.5  2005/04/07 01:27:07  benno
 * Use Array<Argument> instead of List
 *
 * Revision 0.4  2003/04/09 20:23:50  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.3  2003/04/06 14:02:35  benno
 * Use Debugger
 *
 * Revision 0.2  2003/01/24 18:19:34  benno
 * Add IONULL and IOTF
 *
 * Revision 0.1  2002/12/31 04:36:19  benno
 * Initial revision
 *
 */

#include "survivor.H"

Transport::Transport(char *name, char *module, Array<Argument> *modargs)
{
  // Allocate and initialize a new Transport object, identified by
  // <name>.  <module> is the name of the transport module to
  // execute, and <modargs> are the transport arguments to pass to it.

  // Returns: A new Transport object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Transport::Transport(%s,%s,%d)",
		   IONULL(name), IONULL(module), modargs);
#endif

  modarg = modargs;
  modname = xstrdup(module);
  tname = xstrdup(name);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Transport::Transport()");
#endif
}

Array<Argument> *Transport::modargs()
{
  // Obtain the array of arguments for the module for this Transport,
  // if provided.

  // Returns: The array of arguments, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Transport::modargs()");
  dlog->log_exit(DEBUG_MINTRC, "Transport::modargs = %d", modarg);
#endif
  
  return(modarg);
}

char *Transport::module()
{
  // Obtain the name of the module for this Transport.

  // Returns: The module name, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Transport::module()");
  dlog->log_exit(DEBUG_MINTRC, "Transport::module = %s", IONULL(modname));
#endif
  
  return(modname);
}

char *Transport::name()
{
  // Obtain the name of this Transport.

  // Returns: The Transport name, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Transport::name()");
  dlog->log_exit(DEBUG_MINTRC, "Transport::name = %s", IONULL(tname));
#endif
  
  return(tname);
}

Transport::~Transport()
{
  // Deallocate the Transport object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Transport::~Transport()");
#endif

  xadelete(modarg, Argument);
  xdelete(modname);
  xdelete(tname);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Transport::~Transport()");
#endif
}
