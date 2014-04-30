/*
 * FixRequest.C: survivor FixRequest object
 *
 * Version: $Revision: 0.1 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2005/04/20 00:35:25 $
 * MT-Level: Safe
 *
 * Copyright (c) 2004
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: FixRequest.C,v $
 * Revision 0.1  2005/04/20 00:35:25  benno
 * FixRequest object
 *
 */

#include "survivor.H"

FixRequest::FixRequest(Array<Argument> *modargs, char *host, int timeout)
{
  // Allocate and initialize a new FixRequest object, used to store
  // <modargs>, <host> and <timeout>.  <modargs> is not managed by the
  // FixRequest, and must remain valid for the duration of this
  // object.
  
  // Returns: A new FixRequest object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "FixRequest::FixRequest(%d,%s,%d)",
		  modargs, IONULL(host), timeout);
#endif

  modarg = modargs;
  h = xstrdup(host);
  t = timeout;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "FixRequest::FixRequest()");
#endif
}

char *FixRequest::host()
{
  // Obtain the host for this FixRequest.

  // Returns: The host, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "FixRequest::host()");
  dlog->log_exit(DEBUG_MINTRC, "FixRequest::host = %s", IONULL(h));
#endif
  
  return(h);
}

Array<Argument> *FixRequest::modargs()
{
  // Obtain the array of module arguments for this FixRequest.

  // Returns: The list, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "FixRequest::modargs()");
  dlog->log_exit(DEBUG_MINTRC, "FixRequest::modargs = %d", modarg);
#endif
  
  return(modarg);
}

int FixRequest::timeout()
{
  // Obtain the requested timeout.

  // Returns: The timeout.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "FixRequest::timeout()");
  dlog->log_exit(DEBUG_MINTRC, "FixRequest::timeout = %d", t);
#endif
  
  return(t);
}

FixRequest::~FixRequest()
{
  // Delete the FixRequest object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "FixRequest::~FixRequest()");
#endif
  
  modarg = NULL;
  xdelete(h);
  t = 0;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "FixRequest::~FixRequest()");
#endif
}
