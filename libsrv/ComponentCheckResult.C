/*
 * ComponentCheckResult.C: Object to manage composite check result information
 *
 * Version: $Revision: 0.1 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2003/06/25 21:34:37 $
 * MT-Level: Unsafe, unless set_X methods are not called
 *
 * Copyright (c) 2003
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: ComponentCheckResult.C,v $
 * Revision 0.1  2003/06/25 21:34:37  benno
 * Initial revision
 *
 */

#include "survivor.H"

bool ComponentCheckResult::invalid()
{
  // Determine if this ComponentCheckResult has been invalidated.

  // Returns: true if it has been invalidated, false otherwise.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "ComponentCheckResult::invalid()");
  dlog->log_exit(DEBUG_MINTRC, "ComponentCheckResult::invalid = %s",
		 IOTF(inv));
#endif
  
  return(inv);
}

void ComponentCheckResult::invalidate()
{
  // Invalidate this ComponentCheckResult.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "ComponentCheckResult::invalidate()");
  dlog->log_exit(DEBUG_MINTRC, "ComponentCheckResult::invalidate()");
#endif
  
  inv = true;
}

ComponentCheckResult::~ComponentCheckResult()
{
  // Deallocate the ComponentCheckResult object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC,
		  "ComponentCheckResult::~ComponentCheckResult()");
#endif

  inv = false;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC,
		 "ComponentCheckResult::~ComponentCheckResult()");
#endif
}
