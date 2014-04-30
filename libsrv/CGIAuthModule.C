/*
 * CGIAuthModule.C: survivor CGIAuthModule object
 *
 * Version: $Revision: 0.2 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2005/04/06 14:02:47 $
 * MT-Level: Safe
 *
 * Copyright (c) 2004
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: CGIAuthModule.C,v $
 * Revision 0.2  2005/04/06 14:02:47  benno
 * Use Array<Argument> instead of List
 *
 * Revision 0.1  2004/03/01 23:12:39  benno
 * Initial revision
 *
 */

#include "survivor.H"

CGIAuthModule::CGIAuthModule(char *module, Array<Argument> *modopts)
{
  // Allocate and initialize a new CGIAuthModule object, for the
  // <module>, which is configured with options <modopts>.
  // <modopts> will be maintained by the CGIAuthModule and deleted
  // when no longer required.

  // Returns: A new CGIAuthModule object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CGIAuthModule::CGIAuthModule(%s,%d)",
		  IONULL(module), modopts);
#endif

  mname = xstrdup(module);
  mopts = modopts;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CGIAuthModule::CGIAuthModule()");
#endif
}

char *CGIAuthModule::module()
{
  // Obtain the module for this CGIAuthModule.

  // Returns: The module, or NULL on error.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CGIAuthModule::module()");
  dlog->log_exit(DEBUG_MINTRC, "CGIAuthModule::module = %s", IONULL(mname));
#endif
  
  return(mname);
}

Array<Argument> *CGIAuthModule::modopts()
{
  // Obtain the module options.

  // Returns: A pointer to an array of module options if set, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CGIAuthModule::modopts()");
  dlog->log_exit(DEBUG_MINTRC, "CGIAuthModule::modopts = %d", mopts);
#endif
  
  return(mopts);
}

CGIAuthModule::~CGIAuthModule()
{
  // Delete the CGIAuthModule object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CGIAuthModule::~CGIAuthModule()");
#endif

  xdelete(mname);
  xadelete(mopts, Argument);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CGIAuthModule::~CGIAuthModule()");
#endif
}
