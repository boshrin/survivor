/*
 * CheckRequest.C: survivor CheckRequest object
 *
 * Version: $Revision: 0.1 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2005/04/20 00:35:44 $
 * MT-Level: Safe
 *
 * Copyright (c) 2004
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: CheckRequest.C,v $
 * Revision 0.1  2005/04/20 00:35:44  benno
 * CheckRequest object
 *
 */

#include "survivor.H"

CheckRequest::CheckRequest(char *host, int timeout, Array<Argument> *modargs,
			   bool manage)
{
  // Allocate and initialize a new CheckRequest object, used to store
  // <host>, <timeout>, and <modargs>.  If <manage> is true, <modargs>
  // is managed by the CheckRequest, otherwise it must remain valid
  // for the duration of this object.
  
  // Returns: A new CheckRequest object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CheckRequest::CheckRequest(%s,%d,%d,%s)",
		  IONULL(host), timeout, modargs, IOTF(manage));
#endif

  modarg = modargs;
  xh = new List();
  if(xh)
    xh->add(host);
  h = xh;
  m = manage;
  t = timeout;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CheckRequest::CheckRequest()");
#endif
}

CheckRequest::CheckRequest(List *hosts, int timeout, Array<Argument> *modargs,
			   bool manage)
{
  // Allocate and initialize a new CheckRequest object, used to store
  // <hosts>, <timeout>, and <modargs>.  If <manage> is true, <hosts>
  // and <modargs> will be managed by the CheckRequest, otherwise they
  // must remain valid for the duration of this object.
  
  // Returns: A new CheckRequest object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CheckRequest::CheckRequest(%d,%d,%d)",
		  hosts, timeout, modargs);
#endif

  modarg = modargs;
  h = new List(hosts);
  xh = NULL;
  m = manage;
  t = timeout;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CheckRequest::CheckRequest()");
#endif
}

List *CheckRequest::hosts()
{
  // Obtain the list of hosts for this CheckRequest.

  // Returns: The list, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CheckRequest::hosts()");
  dlog->log_exit(DEBUG_MINTRC, "CheckRequest::hosts = %d", h);
#endif
  
  return(h);
}

Array<Argument> *CheckRequest::modargs()
{
  // Obtain the array of module arguments for this CheckRequest.

  // Returns: The array, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CheckRequest::modargs()");
  dlog->log_exit(DEBUG_MINTRC, "CheckRequest::modargs = %d", modarg);
#endif
  
  return(modarg);
}

int CheckRequest::timeout()
{
  // Obtain the timeout for this CheckRequest.

  // Returns: The timeout.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CheckRequest::timeout()");
  dlog->log_exit(DEBUG_MINTRC, "CheckRequest::timeout = %d", t);
#endif
  
  return(t);
}

CheckRequest::~CheckRequest()
{
  // Delete the CheckRequest object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CheckRequest::~CheckRequest()");
#endif

  if(m)
  {
    xadelete(modarg, Argument);

    if(h != xh)   // Don't delete this twice
      xdelete(h);
  }
  
  modarg = NULL;
  xdelete(xh);  // We created this
  t = 0;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CheckRequest::~CheckRequest()");
#endif
}
