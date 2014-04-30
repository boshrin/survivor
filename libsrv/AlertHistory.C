/*
 * AlertHistory.C: Object to manage alert history information
 *
 * Version: $Revision: 0.2 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2004/08/24 23:49:23 $
 * MT-Level: Unsafe
 *
 * Copyright (c) 2003
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: AlertHistory.C,v $
 * Revision 0.2  2004/08/24 23:49:23  benno
 * Add iterate_previous
 *
 * Revision 0.1  2003/11/08 15:35:28  benno
 * Initial revision
 *
 */

#include "survivor.H"

AlertHistoryRecord *AlertHistory::iterate_next()
{
  // Obtain the next AlertHistoryRecord from the AlertHistory.

  // Returns: A new AlertHistoryRecord object that should be deleted
  // when no longer required, or NULL on error.

  AlertHistoryRecord *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertHistory::iterate_next()");
#endif

  // Allocate an AlertHistoryRecord to pass for iteration

  ret = new AlertHistoryRecord();

  if(ret)
  {
    if(!iterate_next_entry(ret))
    {
      xdelete(ret);
    }
  }
  else
    wlog->warn("AlertHistory::iterate_next failed to allocate ret");
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertHistory::iterate_next = %d", ret);
#endif
  
  return(ret);
}

AlertHistoryRecord *AlertHistory::iterate_previous()
{
  // Obtain the previous AlertHistoryRecord from the AlertHistory.

  // Returns: A new AlertHistoryRecord object that should be deleted
  // when no longer required, or NULL on error.

  AlertHistoryRecord *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertHistory::iterate_next()");
#endif

  // Allocate an AlertHistoryRecord to pass for iteration

  ret = new AlertHistoryRecord();

  if(ret)
  {
    if(!iterate_previous_entry(ret))
    {
      xdelete(ret);
    }
  }
  else
    wlog->warn("AlertHistory::iterate_previous failed to allocate ret");
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertHistory::iterate_previous = %d", ret);
#endif
  
  return(ret);
}

bool AlertHistory::record(AlertHistoryRecord *ahr)
{
  // Record the contents of <ahr> into the alert history file.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertHistory::record(%d)", ahr);
#endif

  if(ahr)
    ret = do_record(ahr);
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertHistory::record = %s", IOTF(ret));
#endif
  
  return(ret);
}
