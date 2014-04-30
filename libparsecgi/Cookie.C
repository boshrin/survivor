/*
 * Cookie.C: Cookie storage object
 *
 * Version: $Revision: 0.1 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2004/03/02 03:19:29 $
 * MT-Level: Safe
 *
 * Copyright (c) 2004
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: Cookie.C,v $
 * Revision 0.1  2004/03/02 03:19:29  benno
 * Initial revision
 *
 */

#include "cgi-parser.H"

Cookie::Cookie(char *name, char *value)
{
  // Allocate and initialize a new Cookie object, holding the
  // cookie <name> with contents <value>.

  // Returns: A new, initialized Cookie.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Cookie::Cookie(%s,%s)",
		  IONULL(name), IONULL(value));
#endif

  n = xstrdup(name);
  p = NULL;
  v = xstrdup(value);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Cookie::Cookie()");
#endif
}

Cookie::Cookie(char *name, char *value, char *path)
{
  // Allocate and initialize a new Cookie object, holding the
  // cookie <name> with contents <value> for <path>.

  // Returns: A new, initialized Cookie.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Cookie::Cookie(%s,%s,%s)",
		  IONULL(name), IONULL(value), IONULL(path));
#endif

  n = xstrdup(name);
  p = xstrdup(path);
  v = xstrdup(value);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Cookie::Cookie()");
#endif
}

char *Cookie::name()
{
  // Retrieve the cookie name.

  // Returns: The cookie name, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Cookie::name()");
  dlog->log_exit(DEBUG_MINTRC, "Cookie::name = %s", IONULL(n));
#endif
  
  return(n);
}

char *Cookie::path()
{
  // Retrieve the cookie path.

  // Returns: The cookie path, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Cookie::path()");
  dlog->log_exit(DEBUG_MINTRC, "Cookie::path = %s", IONULL(p));
#endif
  
  return(p);
}

char *Cookie::value()
{
  // Retrieve the cookie value.

  // Returns: The cookie value, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Cookie::value()");
  dlog->log_exit(DEBUG_MINTRC, "Cookie::value = %s", IONULL(v));
#endif
  
  return(v);
}

Cookie::~Cookie()
{
  // Deallocate the Cookie object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Cookie::~Cookie()");
#endif

  xdelete(n);
  xdelete(p);
  xdelete(v);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Cookie::~Cookie()");
#endif
}
