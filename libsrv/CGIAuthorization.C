/*
 * CGIAuthorization.C: survivor CGIAuthorization object
 *
 * Version: $Revision: 0.1 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2004/03/01 23:14:04 $
 * MT-Level: Safe
 *
 * Copyright (c) 2004
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: CGIAuthorization.C,v $
 * Revision 0.1  2004/03/01 23:14:04  benno
 * Initial revision
 *
 */

#include "survivor.H"

CGIAuthorization::CGIAuthorization(authz_type_t aztype, authz_level_t azlevel,
				   char *name)
{
  // Allocate and initialize a new CGIAuthorization object, indicating
  // authorization of <name> (of type <aztype>) at <azlevel>.

  // Returns: A new CGIAuthorization object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC,
		  "CGIAuthorization::CGIAuthorization(%d,%d,%s)",
		  aztype, azlevel, IONULL(name));
#endif

  al = azlevel;
  at = aztype;
  n = xstrdup(name);
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CGIAuthorization::CGIAuthorization()");
#endif
}

authz_level_t CGIAuthorization::authz_level()
{
  // Obtain the authorization level for this CGIAuthorization.

  // Returns: The authorization level.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CGIAuthorization::authz_level()");
  dlog->log_exit(DEBUG_MINTRC, "CGIAuthorization::authz_level = %d", al);
#endif
  
  return(al);
}

authz_type_t CGIAuthorization::authz_type()
{
  // Obtain the type of the identifier for this CGIAuthorization.

  // Returns: The type.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CGIAuthorization::authz_type()");
  dlog->log_exit(DEBUG_MINTRC, "CGIAuthorization::authz_type = %d", at);
#endif
  
  return(at);
}

char *CGIAuthorization::name()
{
  // Obtain the name (identifier) for this CGIAuthorization.

  // Returns: The name, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CGIAuthorization::name()");
  dlog->log_exit(DEBUG_MINTRC, "CGIAuthorization::name = %s", IONULL(n));
#endif
  
  return(n);
}

CGIAuthorization::~CGIAuthorization()
{
  // Delete the CGIAuthorization object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CGIAuthorization::~CGIAuthorization()");
#endif

  al = none_authz;
  xdelete(n);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CGIAuthorization::~CGIAuthorization()");
#endif
}
