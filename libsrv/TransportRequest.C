/*
 * TransportRequest.C: survivor TransportRequest object
 *
 * Version: $Revision: 0.1 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2005/04/20 00:35:05 $
 * MT-Level: Safe
 *
 * Copyright (c) 2004
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: TransportRequest.C,v $
 * Revision 0.1  2005/04/20 00:35:05  benno
 * TransportRequest object
 *
 */

#include "survivor.H"

TransportRequest::TransportRequest(List *hosts, int timeout,
				   Array<Argument> *modargs, char *module,
				   char *modtype, Array<Argument> *rmodargs,
				   bool manage)
{
  // Allocate and initialize a new TransportRequest object, used to
  // store <hosts>, <timeout>, <modargs>, <module>, <modtype>, and
  // <rmodargs>.  If <manage> is true, <hosts>, <modargs>, and
  // <rmodargs> are managed by the TransportRequest, otherwise they
  // must remain valid for the duration of this object.
  
  // Returns: A new TransportRequest object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC,
		  "TransportRequest::TransportRequest(%d,%d,%d,%s,%s,%d,%s)",
		  hosts, timeout, modargs, IONULL(module), IONULL(modtype),
		  rmodargs, IOTF(manage));
#endif

  modarg = modargs;
  rmodarg = rmodargs;
  h = hosts;
  m = manage;
  mod = xstrdup(module);
  modt = xstrdup(modtype);
  t = timeout;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "TransportRequest::TransportRequest()");
#endif
}

List *TransportRequest::hosts()
{
  // Obtain the list of hosts for this TransportRequest.

  // Returns: The list, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "TransportRequest::hosts()");
  dlog->log_exit(DEBUG_MINTRC, "TransportRequest::hosts = %d", h);
#endif
  
  return(h);
}

Array<Argument> *TransportRequest::modargs()
{
  // Obtain the array of module arguments for this TransportRequest.

  // Returns: The array, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "TransportRequest::modargs()");
  dlog->log_exit(DEBUG_MINTRC, "TransportRequest::modargs = %d", modarg);
#endif
  
  return(modarg);
}

char *TransportRequest::modtype()
{
  // Obtain the module type for this TransportRequest.

  // Returns: The module type.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "TransportRequest::modtype()");
  dlog->log_exit(DEBUG_MINTRC, "TransportRequest::modtype = %s", IONULL(modt));
#endif
  
  return(modt);
}

char *TransportRequest::module()
{
  // Obtain the module for this TransportRequest.

  // Returns: The module type.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "TransportRequest::module()");
  dlog->log_exit(DEBUG_MINTRC, "TransportRequest::module = %s", IONULL(mod));
#endif
  
  return(mod);
}

Array<Argument> *TransportRequest::rmodargs()
{
  // Obtain the array of remote module arguments for this TransportRequest.

  // Returns: The array, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "TransportRequest::rmodargs()");
  dlog->log_exit(DEBUG_MINTRC, "TransportRequest::rmodargs = %d", rmodarg);
#endif
  
  return(rmodarg);
}

int TransportRequest::timeout()
{
  // Obtain the timeout for this TransportRequest.

  // Returns: The timeout.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "TransportRequest::timeout()");
  dlog->log_exit(DEBUG_MINTRC, "TransportRequest::timeout = %d", t);
#endif
  
  return(t);
}

TransportRequest::~TransportRequest()
{
  // Delete the TransportRequest object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "TransportRequest::~TransportRequest()");
#endif

  if(m)
  {
    xadelete(modarg, Argument);
    xadelete(rmodarg, Argument);
    xdelete(h);
  }
  
  modarg = NULL;
  rmodarg = NULL;
  h = NULL;
  xdelete(mod);
  xdelete(modt);
  t = 0;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "TransportRequest::~TransportRequest()");
#endif
}
