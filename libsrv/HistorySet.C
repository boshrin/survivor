/*
 * HistorySet.C: Object to manage a set of history objects
 *
 * Version: $Revision: 0.1 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2004/09/09 12:57:53 $
 * MT-Level: Unsafe
 *
 * Copyright (c) 2004
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: HistorySet.C,v $
 * Revision 0.1  2004/09/09 12:57:53  benno
 * Initial Revision
 *
 */

#include "survivor.H"

HistorySet::HistorySet(char *host, char *service)
{
  // Allocate and initialize a new HistorySet object for <host>@<service>.

  // Returns: A new HistorySet object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "HistorySet::HistorySet(%s,%s)",
		  IONULL(host), IONULL(service));
#endif

  ah = NULL;
  ch = NULL;
  cmh = NULL;
  fh = NULL;
  ar = false;
  cr = false;
  cmr = false;
  fr = false;
  h = xstrdup(host);
  s = xstrdup(service);
    
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "HistorySet::HistorySet()");
#endif
}

bool HistorySet::attach_history(AlertHistory *ahist, bool backwards)
{
  // Attach <ahist> to the HistorySet.  <ahist> will be adopted and
  // deleted when no longer required.  If <backwards> is true,
  // the history records will be iterated backwards.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;
    
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "HistorySet::attach_history(%d,%s)",
		  ahist, IOTF(backwards));
#endif

  if(!ah)
  {
    ah = ahist;
    ar = backwards;
    ret = true;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "HistorySet::attach_history = %s", IOTF(ret));
#endif

  return(ret);
}

bool HistorySet::attach_history(CheckHistory *chist, bool backwards)
{
  // Attach <chist> to the HistorySet.  <chist> will be adopted and
  // deleted when no longer required.  If <backwards> is true,
  // the history records will be iterated backwards.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;
    
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "HistorySet::attach_history(%d,%s)",
		  chist, IOTF(backwards));
#endif

  if(!ch)
  {
    ch = chist;
    cr = backwards;
    ret = true;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "HistorySet::attach_history = %s", IOTF(ret));
#endif

  return(ret);
}

bool HistorySet::attach_history(CommandHistory *cmhist, bool backwards)
{
  // Attach <cmhist> to the HistorySet.  <cmhist> will be adopted and
  // deleted when no longer required.  If <backwards> is true,
  // the history records will be iterated backwards.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;
    
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "HistorySet::attach_history(%d,%s)",
		  cmhist, IOTF(backwards));
#endif

  if(!cmh)
  {
    cmh = cmhist;
    cmr = backwards;
    ret = true;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "HistorySet::attach_history = %s", IOTF(ret));
#endif

  return(ret);
}

bool HistorySet::attach_history(FixHistory *fhist, bool backwards)
{
  // Attach <fhist> to the HistorySet.  <fhist> will be adopted and
  // deleted when no longer required.  If <backwards> is true,
  // the history records will be iterated backwards.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;
    
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "HistorySet::attach_history(%d,%s)",
		  fhist, IOTF(backwards));
#endif

  if(!fh)
  {
    fh = fhist;
    fr = backwards;
    ret = true;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "HistorySet::attach_history = %s", IOTF(ret));
#endif

  return(ret);
}

char *HistorySet::host()
{
  // Obtain the host for this HistorySet.

  // Returns: The host, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "HistorySet::host()");
  dlog->log_exit(DEBUG_MINTRC, "HistorySet::host = %s", IONULL(h));
#endif
  
  return(h);
}

AlertHistoryRecord *HistorySet::iterate_alerthistory()
{
  // Iterate the next alert history record.
  
  // Returns: A new AlertHistoryRecord object that should be deleted
  // when no longer required, or NULL if no further records are
  // available.

  AlertHistoryRecord *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "HistorySet::iterate_alerthistory()");
#endif

  if(ah)
  {
    if(ar)
      ret = ah->iterate_previous();
    else
      ret = ah->iterate_next();
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "HistorySet::iterate_alerthistory = %d", ret);
#endif

  return(ret);
}

CheckHistoryRecord *HistorySet::iterate_checkhistory()
{
  // Iterate the next check history record.
  
  // Returns: A new CheckHistoryRecord object that should be deleted
  // when no longer required, or NULL if no further records are
  // available.

  CheckHistoryRecord *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "HistorySet::iterate_checkhistory()");
#endif

  if(ch)
  {
    if(cr)
      ret = ch->iterate_previous();
    else
      ret = ch->iterate_next();
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "HistorySet::iterate_checkhistory = %d", ret);
#endif

  return(ret);
}

CommandHistoryRecord *HistorySet::iterate_commandhistory()
{
  // Iterate the next command history record.
  
  // Returns: A new CommandHistoryRecord object that should be deleted
  // when no longer required, or NULL if no further records are
  // available.

  CommandHistoryRecord *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "HistorySet::iterate_commandhistory()");
#endif

  if(cmh)
  {
    if(cmr)
      ret = cmh->iterate_previous();
    else
      ret = cmh->iterate_next();
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "HistorySet::iterate_commandhistory = %d", ret);
#endif

  return(ret);
}

FixHistoryRecord *HistorySet::iterate_fixhistory()
{
  // Iterate the next fix history record.
  
  // Returns: A new FixHistoryRecord object that should be deleted
  // when no longer required, or NULL if no further records are
  // available.

  FixHistoryRecord *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "HistorySet::iterate_fixhistory()");
#endif

  if(fh)
  {
    if(fr)
      ret = fh->iterate_previous();
    else
      ret = fh->iterate_next();
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "HistorySet::iterate_fixhistory = %d", ret);
#endif

  return(ret);
}

char *HistorySet::service()
{
  // Obtain the service for this HistorySet.

  // Returns: The service, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "HistorySet::service()");
  dlog->log_exit(DEBUG_MINTRC, "HistorySet::service = %s", IONULL(s));
#endif
  
  return(s);
}

HistorySet::~HistorySet()
{
  // Deallocate the HistorySet object.

  // Returns: Nothing.
   
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "HistorySet::~HistorySet()");
#endif

  xdelete(ah);
  xdelete(ch);
  xdelete(cmh);
  xdelete(fh);
  ar = false;
  cr = false;
  cmr = false;
  fr = false;
  xdelete(h);
  xdelete(s);
   
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "HistorySet::~HistorySet()");
#endif
}
