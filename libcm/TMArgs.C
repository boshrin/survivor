/*
 * TMArgs: Transport Module arg parser
 *
 * Version: $Revision: 0.1 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2005/04/19 22:49:41 $
 * MT-Level: Safe, if parse_args is only called once.
 *
 * Copyright (c) 2005
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: TMArgs.C,v $
 * Revision 0.1  2005/04/19 22:49:41  benno
 * Transport Module Args
 *
 */

#include "survivor.H"

char *TMArgs::modtype()
{
  // Obtain the type of module to be executed.

  // Returns: The module type, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "TMArgs::modtype()");
  dlog->log_exit(DEBUG_MINTRC, "TMArgs::modtype = %s", IONULL(mtype));
#endif
  
  return(mtype);
}

char *TMArgs::module()
{
  // Obtain the module to be executed.

  // Returns: The module name, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "TMArgs::module()");
  dlog->log_exit(DEBUG_MINTRC, "TMArgs::module = %s", IONULL(mod));
#endif
  
  return(mod);
}

Array<Argument> *TMArgs::rmodargs()
{
  // Obtain the Array of Arguments for the remote module.

  // Returns: A pointer to an Array of Arguments that should not be
  // modified, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "TMArgs::rmodargs()");
  dlog->log_exit(DEBUG_MINTRC, "TMArgs::rmodargs = %d", rmargs);
#endif
  
  return(rmargs);
}
