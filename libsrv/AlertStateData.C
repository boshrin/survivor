/*
 * AlertStateData.C: Object to manage alert state data
 *
 * Version: $Revision: 0.3 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2006/10/17 14:02:52 $
 * MT-Level: Unsafe
 *
 * Copyright (c) 2004 - 2006
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: AlertStateData.C,v $
 * Revision 0.3  2006/10/17 14:02:52  benno
 * Add since
 *
 * Revision 0.2  2005/04/06 14:02:20  benno
 * Move constructor code here from .H
 *
 * Revision 0.1  2004/06/11 22:10:09  benno
 * Initial revision
 *
 */

#include "survivor.H"
	
AlertStateData::AlertStateData(RecipientSet *recipients, int instances,
			       int checkreturncode, char *checksummary,
			       time_t time, time_t since) :
  CommonStateData(instances, checkreturncode, checksummary, time, since, -1)
{
  // Allocate and initialize a new AlertStateData object, holding
  // <recipients>, <instance>, <checkreturncode>, <checksummary>,
  // <time>, and <since>.  <recipients> will be duplicated and need
  // not remain valid.

  // Returns: A new AlertStateData object.
    
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC,
		  "AlertStateData::AlertStateData(%d,%d,%d,%s,%d,%d)",
		  recipients, instances, checkreturncode,
		  IONULL(checksummary), time, since);
#endif

  if(recipients)
    rs = new RecipientSet(recipients);
  else
    rs = NULL;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertStateData::AlertStateData()");
#endif
}

RecipientSet *AlertStateData::recipients()
{
  // Obtain the recipients for this alert state.

  // Returns: A pointer to a RecipientSet object that should not be
  // modified, or NULL on error.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertStateData::recipients()");
  dlog->log_exit(DEBUG_MINTRC, "AlertStateData::recipients = %d", rs);
#endif

  return(rs);
}

AlertStateData::~AlertStateData()
{
  // Deallocate the AlertStateData object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertStateData::~AlertStateData()");
#endif

  xdelete(rs);
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertStateData::~AlertStateData()");
#endif
}
