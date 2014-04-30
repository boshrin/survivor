/*
 * ModuleConfigError.C: survivor object for module configuration errors
 *
 * Version: $Revision: 0.1 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2003/09/29 13:50:47 $
 * MT-Level: Safe
 *
 * Copyright (c) 2003
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: ModuleConfigError.C,v $
 * Revision 0.1  2003/09/29 13:50:47  benno
 * Initial revision
 *
 */

#include "survivor.H"

ModuleConfigError::ModuleConfigError(char *modname, char *modtype,
				     char *moderr)
{
  // Allocate and initialize a new ModuleConfigError object, used
  // to store <moderr> for <modname> of <modtype>.

  // Returns: A new ModuleConfigError object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC,
		  "ModuleConfigError::ModuleConfigError(%s,%s,%s)",
		  IONULL(modname), IONULL(modtype), IONULL(moderr));
#endif

  e = xstrdup(moderr);
  n = xstrdup(modname);
  t = xstrdup(modtype);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "ModuleConfigError::ModuleConfigError()");
#endif
}

char *ModuleConfigError::moderr()
{
  // Obtain the error string.

  // Returns: The error string, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "ModuleConfigError::moderr()");
  dlog->log_exit(DEBUG_MINTRC, "ModuleConfigError::moderr = %s", IONULL(e));
#endif
  
  return(e);
}

char *ModuleConfigError::modname()
{
  // Obtain the module name.

  // Returns: The module name, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "ModuleConfigError::modname()");
  dlog->log_exit(DEBUG_MINTRC, "ModuleConfigError::modname = %s", IONULL(n));
#endif
  
  return(n);
}

char *ModuleConfigError::modtype()
{
  // Obtain the module type.

  // Returns: The module type, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "ModuleConfigError::modtype()");
  dlog->log_exit(DEBUG_MINTRC, "ModuleConfigError::modtype = %s", IONULL(t));
#endif
  
  return(t);
}

ModuleConfigError::~ModuleConfigError()
{
  // Deallocate the ModuleConfigError object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "ModuleConfigError::~ModuleConfigError()");
#endif
  
  xdelete(e);
  xdelete(n);
  xdelete(t);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "ModuleConfigError::~ModuleConfigError()");
#endif
}
