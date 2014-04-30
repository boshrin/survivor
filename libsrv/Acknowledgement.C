/*
 * Acknowledgement.C: Object to manage data for acknowledgements.
 *
 * Version: $Revision: 0.1 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2004/06/11 22:15:22 $
 * MT-Level: Unsafe
 *
 * Copyright (c) 2004
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: Acknowledgement.C,v $
 * Revision 0.1  2004/06/11 22:15:22  benno
 * Initial revision
 *
 */

#include "survivor.H"

Acknowledgement::Acknowledgement(char *who, char *why)
{
  // Allocate and initialize a new Acknowledgement object, holding
  // <who> performed the action and <why>.

  // Returns: A new Acknowledgement object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Acknowledgement::Acknowledgement(%s,%s)",
		  IONULL(who), IONULL(why));
#endif

  w = xstrdup(who);
  y = xstrdup(why);
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Acknowledgement::Acknowledgement()");
#endif
}

char *Acknowledgement::who()
{
  // Obtain who performed the acknowledgement.

  // Returns: Who performed the acknowledgement, or NULL.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Acknowledgement::who()");
  dlog->log_exit(DEBUG_MINTRC, "Acknowledgement::who = %s", IONULL(w));
#endif
  
  return(w);
}

char *Acknowledgement::why()
{
  // Obtain why the acknowledgement was performed.

  // Returns: The provided reason, or NULL.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Acknowledgement::why()");
  dlog->log_exit(DEBUG_MINTRC, "Acknowledgement::why = %s", IONULL(y));
#endif
  
  return(y);
}

Acknowledgement::~Acknowledgement()
{
  // Deallocate the Acknowledgement object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Acknowledgement::~Acknowledgement()");
#endif

  xdelete(w);
  xdelete(y);
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Acknowledgement::~Acknowledgement()");
#endif
}
