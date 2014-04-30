/*
 * Escalation.C: Object to manage data for acknowledgements.
 *
 * Version: $Revision: 0.1 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2004/06/11 22:14:43 $
 * MT-Level: Unsafe
 *
 * Copyright (c) 2004
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: Escalation.C,v $
 * Revision 0.1  2004/06/11 22:14:43  benno
 * Initial revision
 *
 */

#include "survivor.H"

Escalation::Escalation(int escto)
{
  // Allocate and initialize a new Escalation object, holding an
  // escalation level of <escto>.

  // Returns: A new Escalation object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Escalation::Escalation(%d)", escto);
#endif

  e = escto;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Escalation::Escalation()");
#endif
}

int Escalation::escalated_to()
{
  // Obtain the escalation level.

  // Returns: The escalation level.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Escalation::escalated_to()");
  dlog->log_exit(DEBUG_MINTRC, "Escalation::escalated_to = %d", e);
#endif
  
  return(e);
}

Escalation::~Escalation()
{
  // Deallocate the Escalation object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Escalation::~Escalation()");
#endif

  e = 0;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Escalation::~Escalation()");
#endif
}
