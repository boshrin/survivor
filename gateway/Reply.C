/*
 * Reply.C: survivor object for holding information from reply messages
 *
 * Version: $Revision: 0.2 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2003/04/09 20:15:39 $
 * MT-Level: Safe, if parse is only called once.
 *
 * Copyright (c) 2002 - 2003
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: Reply.C,v $
 * Revision 0.2  2003/04/09 20:15:39  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.1  2003/03/04 20:59:55  benno
 * Initial revision
 *
 */

#include "gateway.H"

Reply::Reply()
{
  // Allocate and initialize a new Reply object.

  // Returns: A new Reply object, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Reply()::Reply()");
#endif

  f = NULL;
  h = NULL;
  inst = NULL;
  svc = NULL;
  t = NULL;
  ry = unknown_reply;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Reply()::Reply()");
#endif
}

char *Reply::from()
{
  // Obtain the from address in the message.

  // Returns: The from address, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Reply::from()");
  dlog->log_exit(DEBUG_MINTRC, "Reply::from = %s", IONULL(f));
#endif
  
  return(f);
}

char *Reply::host()
{
  // Obtain the host referenced by the message.

  // Returns: The name of the host, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Reply::host()");
  dlog->log_exit(DEBUG_MINTRC, "Reply::host = %s", IONULL(h));
#endif
  
  return(h);
}

char *Reply::instance()
{
  // Obtain the instance referenced by the message.

  // Returns: The name of the instance, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Reply::instance()");
  dlog->log_exit(DEBUG_MINTRC, "Reply::instance = %s", IONULL(inst));
#endif
  
  return(inst);
}

reply_t Reply::reply()
{
  // Determine the reply in the message.

  // Returns: The reply type.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Reply::reply()");
  dlog->log_exit(DEBUG_MINTRC, "Reply::reply = %d", ry);
#endif
  
  return(ry);
}

char *Reply::service()
{
  // Obtain the service referenced by the message.

  // Returns: The name of the service, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Reply::service()");
  dlog->log_exit(DEBUG_MINTRC, "Reply::service = %s", IONULL(svc));
#endif
  
  return(svc);
}

char *Reply::token()
{
  // Obtain the token contained in the message.

  // Returns: The token, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Reply::token()");
  dlog->log_exit(DEBUG_MINTRC, "Reply::token = %s", IONULL(t));
#endif
  
  return(t);
}

Reply::~Reply()
{
  // Deallocate the Reply object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Reply()::~Reply()");
#endif

  xdelete(f);
  xdelete(h);
  xdelete(inst);
  xdelete(svc);
  xdelete(t);

  ry = unknown_reply;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Reply()::~Reply()");
#endif
}
