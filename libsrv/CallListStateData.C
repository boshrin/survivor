/*
 * CallListStateData.C: Object to manage data for CallListState objects
 *
 * Version: $Revision: 0.2 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2005/09/26 13:45:38 $
 * MT-Level: Unsafe
 *
 * Copyright (c) 2004 - 2005
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: CallListStateData.C,v $
 * Revision 0.2  2005/09/26 13:45:38  benno
 * Add support for oncall person
 *
 * Revision 0.1  2004/06/11 22:15:39  benno
 * Initial revision
 *
 */

#include "survivor.H"

CallListStateData::CallListStateData(char *lnperson, char *lnaddress,
				     char *lnmodule)
{
  // Allocate and initialize a new CallListStateData object, holding
  // <lnperson>, <lnaddress>, and <lnmodule>.

  // Returns: A new CallListStateData object.

  init(lnperson, lnaddress, lnmodule, 0, NULL);
}

CallListStateData::CallListStateData(char *lnperson, char *lnaddress,
				     char *lnmodule, time_t lrotate,
				     char *ocperson)
{
  // Allocate and initialize a new CallListStateData object, holding
  // <lnperson>, <lnaddress>, <lnmodule>, <lnrotate>, and <ocperson>.

  // Returns: A new CallListStateData object.

  init(lnperson, lnaddress, lnmodule, lrotate, ocperson);
}

char *CallListStateData::last_notified_address()
{
  // Obtain the last notified address from the state data.

  // Returns: The last notified address.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CallListStateData::last_notified_address()");
  dlog->log_exit(DEBUG_MINTRC, "CallListStateData::last_notified_address = %s",
		 IONULL(a));
#endif
  
  return(a);
}

char *CallListStateData::last_notified_person()
{
  // Obtain the last notified Person from the state data.

  // Returns: The name of the last notified Person.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CallListStateData::last_notified_person()");
  dlog->log_exit(DEBUG_MINTRC, "CallListStateData::last_notified_person = %s",
		 IONULL(p));
#endif
  
  return(p);
}

char *CallListStateData::last_notified_via()
{
  // Obtain the last alert module used for notification from the state data.

  // Returns: The alert module name.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CallListStateData::last_notified_via()");
  dlog->log_exit(DEBUG_MINTRC, "CallListStateData::last_notified_via = %s",
		 IONULL(m));
#endif
  
  return(m);
}

time_t CallListStateData::last_rotated()
{
  // Obtain the time of last rotation from the state data.

  // Returns: The time, in seconds since the epoch, or 0 if no
  // rotation has occurred.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CallListStateData::last_rotated()");
  dlog->log_exit(DEBUG_MINTRC, "CallListStateData::last_rotated = %d", r);
#endif
  
  return(r);
}

char *CallListStateData::oncall_person()
{
  // Obtain the person currently oncall.

  // Returns: The name of the oncall person.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CallListStateData::oncall_person()");
  dlog->log_exit(DEBUG_MINTRC, "CallListStateData::oncall_person = %s",
		 IONULL(ocp));
#endif
  
  return(ocp);
}

CallListStateData::~CallListStateData()
{
  // Deallocate the CallListStateData object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CallListStateData::~CallListStateData()");
#endif

  xdelete(p);
  xdelete(a);
  xdelete(m);
  r = 0;
  xdelete(ocp);
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CallListStateData::~CallListStateData()");
#endif
}

void CallListStateData::init(char *lnperson, char *lnaddress, char *lnmodule,
			     time_t lrotate, char *ocperson)
{
  // Initialize the CallListStateData object with <lnperson>, <lnaddress>,
  // <lnmodule>, <lrotate>, and <ocperson>.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CallListStateData::init(%s,%s,%s,%d,%s)",
		  IONULL(lnperson), IONULL(lnaddress), IONULL(lnmodule),
		  lrotate, IONULL(ocperson));
#endif

  p = xstrdup(lnperson);
  a = xstrdup(lnaddress);
  m = xstrdup(lnmodule);
  r = lrotate;
  ocp = xstrdup(ocperson);
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CallListStateData::init()");
#endif
}
