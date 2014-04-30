/*
 * CommandHistory.C: Object to manage command history information
 *
 * Version: $Revision: 0.2 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2004/08/24 23:51:35 $
 * MT-Level: Unsafe
 *
 * Copyright (c) 2003
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: CommandHistory.C,v $
 * Revision 0.2  2004/08/24 23:51:35  benno
 * Add iterate_previous
 *
 * Revision 0.1  2003/11/08 15:35:52  benno
 * Initial revision
 *
 */

#include "survivor.H"

CommandHistoryRecord *CommandHistory::iterate_next()
{
  // Obtain the next CommandHistoryRecord from the CommandHistory.

  // Returns: A new CommandHistoryRecord object that should be deleted
  // when no longer required, or NULL on error.

  CommandHistoryRecord *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CommandHistory::iterate_next()");
#endif

  // Allocate a CommandHistoryRecord to pass for iteration

  ret = new CommandHistoryRecord();

  if(ret)
  {
    if(!iterate_next_entry(ret))
    {
      xdelete(ret);
    }
  }
  else
    wlog->warn("CommandHistory::iterate_next failed to allocate ret");
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CommandHistory::iterate_next = %d", ret);
#endif
  
  return(ret);
}

CommandHistoryRecord *CommandHistory::iterate_previous()
{
  // Obtain the previous CommandHistoryRecord from the CommandHistory.

  // Returns: A new CommandHistoryRecord object that should be deleted
  // when no longer required, or NULL on error.

  CommandHistoryRecord *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CommandHistory::iterate_previous()");
#endif

  // Allocate a CommandHistoryRecord to pass for iteration

  ret = new CommandHistoryRecord();

  if(ret)
  {
    if(!iterate_previous_entry(ret))
    {
      xdelete(ret);
    }
  }
  else
    wlog->warn("CommandHistory::iterate_previous failed to allocate ret");
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CommandHistory::iterate_previous = %d", ret);
#endif
  
  return(ret);
}

bool CommandHistory::record(CommandHistoryRecord *chr)
{
  // Record the contents of <chr> into the command history file.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CommandHistory::record(%d)", chr);
#endif

  if(chr)
    ret = do_record(chr);
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CommandHistory::record = %s", IOTF(ret));
#endif
  
  return(ret);
}
