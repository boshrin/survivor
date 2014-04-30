/*
 * HashUnit.C: Data storage for Hashtable
 *
 * Version: $Revision: 0.6 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2003/04/09 20:23:48 $
 * MT-Level: Unsafe
 *
 * Copyright (c) 2002 - 2003
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: HashUnit.C,v $
 * Revision 0.6  2003/04/09 20:23:48  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.5  2003/04/05 22:54:56  benno
 * Use Debugger
 *
 * Revision 0.4  2003/01/24 17:24:01  benno
 * Add IONULL and IOTF
 *
 * Revision 0.3  2002/04/04 20:10:29  benno
 * copyright
 *
 * Revision 0.2  2002/04/03 18:17:24  benno
 * rcsify date
 *
 * Revision 0.1  2002/04/03 18:16:59  benno
 * initial revision
 *
 */

#include "survivor.H"

HashUnit::HashUnit()
{
  // Allocate a new HashUnit object.  HashUnits are designed to be Warehoused,
  // so this constructor performs no initialization.  After a new HashUnit is
  // created (either with new or Warehouse->allocate), the initialize() method
  // should be invoked.

  // Returns: A new, uninitialized HashUnit.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "HashUnit::HashUnit()");
  dlog->log_progress(DEBUG_MAJTRC, "HashUnit is a Warehouse aware object");
  dlog->log_exit(DEBUG_MAJTRC, "HashUnit::HashUnit()");
#endif
}

bool HashUnit::clear()
{
  // Clear the HashUnit.  This method should be called before the HashUnit is
  // deallocated, and may also be called whenever the contents of the HashUnit
  // are to be purged.  (Only the internal data structures are cleared, not
  // any data those structures may point to.)  Note that the positional
  // information (next and prev) is also cleared.

  // Returns: true if the HashUnit was successfully cleared, false otherwise.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "HashUnit::Clear()");
#endif

  k = NULL;
  d = NULL;
  next = NULL;
  prev = NULL;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "HashUnit::clear = true");
#endif

  return(true);
}

void *HashUnit::get_data()
{
  // Retrieve the data from the HashUnit.

  // Returns: The data stored in the HashUnit, or NULL if no data is
  // stored.  Note that a NULL return value may indicate that the data
  // pointer stored was NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "HashUnit::get_data()");
  dlog->log_exit(DEBUG_MINTRC, "HashUnit::get_data = %d", d);
#endif

  return(d);
}

char *HashUnit::get_key()
{
  // Retrieve the key from the HashUnit.

  // Returns: The key stored in the HashUnit, or NULL if no key is
  // stored.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "HashUnit::get_key()");
  dlog->log_exit(DEBUG_MINTRC, "HashUnit::get_key = %s", IONULL(k));
#endif

  return(k);
}

HashUnit *HashUnit::get_next()
{
  // Retrieve the next HashUnit (ie: the one attached after this).

  // Returns: The next HashUnit, or NULL if this is the last HashUnit.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "HashUnit::get_next()");
  dlog->log_exit(DEBUG_MINTRC, "HashUnit::get_next = %d", next);
#endif

  return(next);
}

HashUnit *HashUnit::get_prev()
{
  // Retrieve the previous HashUnit (ie: the one attached before this).

  // Returns: The previous HashUnit, or NULL if this is the first HashUnit.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "HashUnit::get_prev()");
  dlog->log_exit(DEBUG_MINTRC, "HashUnit::get_prev = %d", prev);
#endif

  return(prev);
}

bool HashUnit::initialize(char *key, void *data)
{
  // Initialize the HashUnit.  This method should only be invoked immediately
  // after the HashUnit is created.  <key> may not be NULL, although <data>
  // may be.

  // Returns: true if initialization was successful, false otherwise.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "HashUnit::initialize(%s,%d)",
		   IONULL(key), data);
#endif

  k = key;
  d = data;
  next = NULL;
  prev = NULL;

  if(!key)
  {
    d = NULL;
    
#if defined(DEBUG)
    dlog->log_exit(DEBUG_MINTRC, "HashUnit::initialize = false");
#endif
  
    return(false);
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "HashUnit::initialize = true");
#endif

  return(true);
}

bool HashUnit::set_data(void *data)
{
  // Set the data in this HashUnit to be <data>.  <data> may be NULL.

  // Returns: true if fully successful, false otherwise.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "HashUnit::set_data(%d)", data);
  dlog->log_exit(DEBUG_MINTRC, "HashUnit::set_data = true");
#endif
  
  d = data;
  return(true);
}

bool HashUnit::set_next(HashUnit *h)
{
  // Set the next HashUnit in the chain to be <h>.  If <h> is NULL, this
  // HashUnit becomes the last in the chain.

  // Returns: true if fully successful, false otherwise.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "HashUnit::set_next(%d)", h);
  dlog->log_exit(DEBUG_MINTRC, "HashUnit::set_next = true");
#endif
  
  next = h;
  return(true);
}

bool HashUnit::set_prev(HashUnit *h)
{
  // Set the previous HashUnit in the chain to be <h>.  If <h> is NULL, this
  // HashUnit becomes the first in the chain.

  // Returns: true if fully successful, false otherwise.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "HashUnit::set_prev(%d)", h);
  dlog->log_exit(DEBUG_MINTRC, "HashUnit::set_prev = true");
#endif
  
  prev = h;
  return(true);
}

HashUnit::~HashUnit()
{
  // Deallocate the HashUnit object.  HashUnits are designed to be Warehoused,
  // so this destructor performs no cleanup.  Before a HashUnit is deallocated
  // (either with delete or Warehouse->deallocate), the clear() method should
  // be invoked.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "HashUnit::~HashUnit()");
  dlog->log_progress(DEBUG_MAJTRC, "HashUnit is a Warehouse aware object");
  dlog->log_exit(DEBUG_MAJTRC, "HashUnit::~HashUnit()");
#endif
}
