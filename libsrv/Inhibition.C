/*
 * Inhibition.C: Object to manage data for acknowledgements.
 *
 * Version: $Revision: 0.1 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2004/06/11 22:15:01 $
 * MT-Level: Unsafe
 *
 * Copyright (c) 2004
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: Inhibition.C,v $
 * Revision 0.1  2004/06/11 22:15:01  benno
 * Initial revision
 *
 */

#include "survivor.H"

Inhibition::Inhibition(char *who, char *why)
{
  // Allocate and initialize a new Inhibition object, holding
  // <who> performed the action and <why>.

  // Returns: A new Inhibition object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Inhibition::Inhibition(%s,%s)",
		  IONULL(who), IONULL(why));
#endif

  w = xstrdup(who);
  y = xstrdup(why);
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Inhibition::Inhibition()");
#endif
}

char *Inhibition::who()
{
  // Obtain who performed the inhibition.

  // Returns: Who performed the inhibition, or NULL.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Inhibition::who()");
  dlog->log_exit(DEBUG_MINTRC, "Inhibition::who = %s", IONULL(w));
#endif
  
  return(w);
}

char *Inhibition::why()
{
  // Obtain why the inhibition was performed.

  // Returns: The provided reason, or NULL.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Inhibition::why()");
  dlog->log_exit(DEBUG_MINTRC, "Inhibition::why = %s", IONULL(y));
#endif
  
  return(y);
}

Inhibition::~Inhibition()
{
  // Deallocate the Inhibition object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Inhibition::~Inhibition()");
#endif

  xdelete(w);
  xdelete(y);
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Inhibition::~Inhibition()");
#endif
}
