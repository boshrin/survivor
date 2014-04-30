/*
 * CheckHistory.C: Object to manage check history information
 *
 * Version: $Revision: 0.2 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2004/08/24 23:51:18 $
 * MT-Level: Unsafe
 *
 * Copyright (c) 2003
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: CheckHistory.C,v $
 * Revision 0.2  2004/08/24 23:51:18  benno
 * Add iterate_previous
 *
 * Revision 0.1  2003/11/08 15:35:43  benno
 * Initial revision
 *
 */

#include "survivor.H"

CheckHistoryRecord *CheckHistory::iterate_next()
{
  // Obtain the next CheckHistoryRecord from the CheckHistory.

  // Returns: A new CheckHistoryRecord object that should be deleted
  // when no longer required, or NULL on error.

  CheckHistoryRecord *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CheckHistory::iterate_next()");
#endif

  // Allocate a CheckHistoryRecord to pass for iteration

  ret = new CheckHistoryRecord();

  if(ret)
  {
    if(!iterate_next_entry(ret))
    {
      xdelete(ret);
    }
  }
  else
    wlog->warn("CheckHistory::iterate_next failed to allocate ret");
 
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CheckHistory::iterate_next = %d", ret);
#endif
  
  return(ret);
}

CheckHistoryRecord *CheckHistory::iterate_previous()
{
  // Obtain the previous CheckHistoryRecord from the CheckHistory.

  // Returns: A new CheckHistoryRecord object that should be deleted
  // when no longer required, or NULL on error.

  CheckHistoryRecord *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CheckHistory::iterate_previous()");
#endif

  // Allocate a CheckHistoryRecord to pass for iteration

  ret = new CheckHistoryRecord();

  if(ret)
  {
    if(!iterate_previous_entry(ret))
    {
      xdelete(ret);
    }
  }
  else
    wlog->warn("CheckHistory::iterate_previous failed to allocate ret");
 
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CheckHistory::iterate_previous = %d", ret);
#endif
  
  return(ret);
}

bool CheckHistory::record(CheckHistoryRecord *chr)
{
  // Record the contents of <chr> into the check history file.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CheckHistory::record(%d)", chr);
#endif

  if(chr)
    ret = do_record(chr);
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CheckHistory::record = %s", IOTF(ret));
#endif
  
  return(ret);
}
