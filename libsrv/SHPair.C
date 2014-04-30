/*
 * SHPair.C: Simple service/host pair object
 *
 * Version: $Revision: 0.7 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2003/11/29 05:28:00 $
 * MT-Level: Safe
 *
 * Copyright (c) 2002 - 2003
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: SHPair.C,v $
 * Revision 0.7  2003/11/29 05:28:00  benno
 * Move from cli
 * Add SHPair(char,char)
 *
 * Revision 0.6  2003/04/09 20:12:19  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.5  2003/04/07 20:22:01  benno
 * Use Debugger
 *
 * Revision 0.4  2003/01/28 03:49:58  benno
 * Use IONULL, IOTF, xdelete, xadelete
 *
 * Revision 0.3  2002/04/04 19:56:31  benno
 * copyright
 *
 * Revision 0.2  2002/04/02 21:09:35  benno
 * rcsify date
 *
 * Revision 0.1  2002/04/02 21:09:10  benno
 * initial revision
 *
 */

#include "survivor.H"

SHPair::SHPair(char *arg)
{
  // Split up an <arg> of the form service@host.  If there is no '@'
  // provided, the entire string is considered a service.

  // Returns: A new, initialized SHPair.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "SHPair::SHPair(%s)", IONULL(arg));
#endif
  
  h = NULL;
  s = NULL;

  if(arg)
  {
    char *p = strchr(arg, '@');

    if(p)
    {
      // Break up the string
      
      *p = '\0';
      s = xstrdup(arg);

      *p = '@';
      p++;
      h = xstrdup(p);
    }
    else
      s = xstrdup(arg);
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "SHPair::SHPair()");
#endif
}

SHPair::SHPair(char *service, char *host)
{
  // Allocate and initialize a new SHPair object, holding <service>@<host>.
  // Both are copied and need not remain valid.

  // Returns: A new, initialized SHPair.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "SHPair::SHPair(%s,%s)",
		  IONULL(service), IONULL(host));
#endif
  
  h = xstrdup(host);
  s = xstrdup(service);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "SHPair::SHPair()");
#endif
}

char *SHPair::host()
{
  // Retrieve the host portion of the arg, if specified.  An empty
  // string returned here indicates an arg of the form foo@ was provided.
  // If just foo was provided, this will return NULL.

  // Returns: The host portion if found, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "SHPair::host()");
  dlog->log_exit(DEBUG_MINTRC, "SHPair::host = %s", IONULL(h));
#endif
  
  return(h);
}

char *SHPair::service()
{
  // Retrieve the service portion of the arg, if specified.  An empty
  // string returned here indicates an arg of the form @foo was provided.
  // If just foo was provided, this will return foo.

  // Returns: The service portion if found, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "SHPair::service()");
  dlog->log_exit(DEBUG_MINTRC, "SHPair::service = %s", IONULL(s));
#endif
  
  return(s);
}

SHPair::~SHPair()
{
  // Deallocate the SHPair object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "SHPair::~SHPair()");
#endif

  xdelete(h);
  xdelete(s);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "SHPair::~SHPair()");
#endif
}
