/*
 * FixHistory.C: Object to manage fix history information
 *
 * Version: $Revision: 0.2 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2004/08/24 23:52:08 $
 * MT-Level: Unsafe
 *
 * Copyright (c) 2003
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: FixHistory.C,v $
 * Revision 0.2  2004/08/24 23:52:08  benno
 * Add iterate_previous
 *
 * Revision 0.1  2003/11/08 15:36:02  benno
 * Initial revision
 *
 */

#include "survivor.H"

FixHistoryRecord *FixHistory::iterate_next()
{
  // Obtain the next FixHistoryRecord from the FixHistory.

  // Returns: A new FixHistoryRecord object that should be deleted
  // when no longer required, or NULL on error.

  FixHistoryRecord *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "FixHistory::iterate_next()");
#endif

   // Allocate a FixHistoryRecord to pass for iteration

  ret = new FixHistoryRecord();

  if(ret)
  {
    if(!iterate_next_entry(ret))
    {
      xdelete(ret);
    }
  }
  else
    wlog->warn("FixHistory::iterate_next failed to allocate ret");
 
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "FixHistory::iterate_next = %d", ret);
#endif
  
  return(ret);
}

FixHistoryRecord *FixHistory::iterate_previous()
{
  // Obtain the previous FixHistoryRecord from the FixHistory.

  // Returns: A new FixHistoryRecord object that should be deleted
  // when no longer required, or NULL on error.

  FixHistoryRecord *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "FixHistory::iterate_previous()");
#endif

   // Allocate a FixHistoryRecord to pass for iteration

  ret = new FixHistoryRecord();

  if(ret)
  {
    if(!iterate_previous_entry(ret))
    {
      xdelete(ret);
    }
  }
  else
    wlog->warn("FixHistory::iterate_previous failed to allocate ret");
 
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "FixHistory::iterate_previous = %d", ret);
#endif
  
  return(ret);
}

bool FixHistory::record(FixHistoryRecord *fhr)
{
  // Record the contents of <fhr> into the fix history file.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "FixHistory::record(%d)", fhr);
#endif

  if(fhr)
    ret = do_record(fhr);
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "FixHistory::record = %s", IOTF(ret));
#endif
  
  return(ret);
}
