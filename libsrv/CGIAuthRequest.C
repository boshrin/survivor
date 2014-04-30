/*
 * CGIAuthRequest.C: survivor CGIAuthRequest object
 *
 * Version: $Revision: 0.1 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2004/03/01 23:13:16 $
 * MT-Level: Safe
 *
 * Copyright (c) 2004
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: CGIAuthRequest.C,v $
 * Revision 0.1  2004/03/01 23:13:16  benno
 * Initial revision
 *
 */

#include "survivor.H"

CGIAuthRequest::CGIAuthRequest(CGIAuthModule *module, char *uri, char *ip)
{
  // Allocate and initialize a new CGIAuthRequest object, using the
  // <module>, and providing the requested <uri> and source <ip>.

  // Returns: A new CGIAuthRequest object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CGIAuthRequest::CGIAuthRequest(%d,%s,%s)",
		  module, IONULL(uri), IONULL(ip));
#endif

  mod = module;
  oi = xstrdup(ip);
  u = xstrdup(uri);
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CGIAuthRequest::CGIAuthRequest()");
#endif
}

char *CGIAuthRequest::ip()
{
  // Obtain the source IP for this CGIAuthRequest.

  // Returns: The IP address as a char string, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CGIAuthRequest::ip()");
  dlog->log_exit(DEBUG_MINTRC, "CGIAuthRequest::ip = %s", IONULL(oi));
#endif
  
  return(oi);
}

CGIAuthModule *CGIAuthRequest::module()
{
  // Obtain the module for this CGIAuthRequest.

  // Returns: The module, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CGIAuthRequest::module()");
  dlog->log_exit(DEBUG_MINTRC, "CGIAuthRequest::module = %d", mod);
#endif
  
  return(mod);
}

char *CGIAuthRequest::uri()
{
  // Obtain the requested URI for this CGIAuthRequest.

  // Returns: The requested URI, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CGIAuthRequest::uri()");
  dlog->log_exit(DEBUG_MINTRC, "CGIAuthRequest::uri = %s", IONULL(u));
#endif
  
  return(u);
}

CGIAuthRequest::~CGIAuthRequest()
{
  // Delete the CGIAuthRequest object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CGIAuthRequest::~CGIAuthRequest()");
#endif

  mod = NULL;
  xdelete(oi);
  xdelete(u);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CGIAuthRequest::~CGIAuthRequest()");
#endif
}
