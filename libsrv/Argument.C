/*
 * Argument.C: survivor Argument object
 *
 * Version: $Revision: 0.1 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2005/04/20 00:36:08 $
 * MT-Level: Safe
 *
 * Copyright (c) 2004
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: Argument.C,v $
 * Revision 0.1  2005/04/20 00:36:08  benno
 * Argument object
 *
 */

#include "survivor.H"

Argument::Argument(char *name, char *value, bool trailingspaces)
{
  // Allocate and initialize a new Argument object, used to store <name>
  // and <value>.  If <trailingspaces> is false, trailing whitespace
  // will be removed from <value>
  
  // Returns: A new Argument object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Argument::Argument(%s,%s,%s)",
		  IONULL(name), IONULL(value), IOTF(trailingspaces));
#endif

  n = xstrdup(name);
  v = xstrdup(value);

  if(v && !trailingspaces)
  {
    // Work backwards from the end of v, chopping off whitespace.

    for(int i = strlen(v) - 1;i >= 0;i--)
    {
      if(isspace(v[i]))
	v[i] = '\0';
      else
	break;
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Argument::Argument()");
#endif
}

char *Argument::name()
{
  // Obtain the name for this Argument.

  // Returns: The name, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Argument::name()");
  dlog->log_exit(DEBUG_MINTRC, "Argument::name = %s", IONULL(n));
#endif
  
  return(n);
}

char *Argument::value()
{
  // Obtain the value for this Argument.

  // Returns: The value, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Argument::value()");
  dlog->log_exit(DEBUG_MINTRC, "Argument::value = %s", IONULL(v));
#endif
  
  return(v);
}

Argument::~Argument()
{
  // Delete the Argument object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Argument::~Argument()");
#endif
  
  xdelete(n);
  xdelete(v);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Argument::~Argument()");
#endif
}
