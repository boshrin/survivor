/*
 * StalledCheck.C: survivor object for stalled check information
 *
 * Version: $Revision: 0.1 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2006/01/25 00:43:45 $
 * MT-Level: Safe
 *
 * Copyright (c) 2005
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: StalledCheck.C,v $
 * Revision 0.1  2006/01/25 00:43:45  benno
 * Initial revision
 *
 *
 */

#include "survivor.H"

time_t StalledCheck::lastcheck()
{
  // Obtain the last check time.

  // Returns: The last check time.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "StalledCheck::lastcheck()");
  dlog->log_exit(DEBUG_MINTRC, "StalledCheck::lastcheck = %d", lc);
#endif
  
  return(lc);
}

StalledCheck::~StalledCheck()
{
  // Deallocate the StalledCheck object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "StalledCheck::~StalledCheck()");
#endif
  
  lc = 0;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "StalledCheck::~StalledCheck()");
#endif
}
