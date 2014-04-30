/*
 * FixStateData.C: Object to manage fix state data
 *
 * Version: $Revision: 0.1 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2004/06/11 22:12:18 $
 * MT-Level: Unsafe
 *
 * Copyright (c) 2004
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: FixStateData.C,v $
 * Revision 0.1  2004/06/11 22:12:18  benno
 * Initial revision
 *
 */

#include "survivor.H"

char *FixStateData::initiatedby()
{
  // Obtain the identifier of who initiated the fix.

  // Returns: The identifier, or NULL on error.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "FixStateData::initiatedby()");
  dlog->log_exit(DEBUG_MINTRC, "FixStateData::initiatedby = %d", by);
#endif
  
  return(by);
}

FixStateData::~FixStateData()
{
  // Deallocate the FixStateData object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "FixStateData::~FixStateData()");
#endif

  xdelete(by);
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "FixStateData::~FixStateData()");
#endif
}
