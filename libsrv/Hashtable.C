/*
 * Hashtable.C: Hashtable optimized for enid, and moved to survivor
 *
 * Version: $Revision: 0.9 $
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
 * $Log: Hashtable.C,v $
 * Revision 0.9  2003/04/09 20:23:48  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.8  2003/04/05 23:08:01  benno
 * Use Debugger
 *
 * Revision 0.7  2003/01/24 17:22:04  benno
 * Use IONULL, IOTF, xdelete, and xadelete
 *
 * Revision 0.6  2002/04/04 20:10:21  benno
 * copyright
 *
 * Revision 0.5  2002/04/03 18:16:19  benno
 * rcsify date
 *
 * Revision 0.4  2002/04/03 18:15:50  benno
 * Versions previous to 0.4 are found in enid.
 *
 */

#include "survivor.H"

iHashtable::iHashtable()
{
  // Allocate a new Hashtable object using the default number of buckets.
  // The Hashtable will resize itself as it grows so that there are always
  // at least twice as many buckets as items stored within.  If the Hashtable
  // will be populated with more than 1000 items quickly, it is better to
  // specify the initial number of buckets when creating the Hashtable.

  // Returns: A new Hashtable.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "iHashtable::iHashtable()");
#endif
  
  initialize_hash(2000);
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "iHashtable::iHashtable()");
#endif
}

iHashtable::iHashtable(size_t buckets)
{
  // Allocate a new Hashtable with <buckets> buckets.  The Hashtable will
  // resize itself as it grows so that there are always at least twice as
  // many buckets as items stored within.  Resizing is resource intensive,
  // so <buckets> should be large enough that the table does not need to
  // resize.

  // Returns: A new Hashtable.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "iHashtable::iHashtable(%d)", buckets);
#endif
  
  initialize_hash(buckets);
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "iHashtable::iHashtable");
#endif
}

bool iHashtable::iinsert(char *key, void *data)
{
  // Insert the <key>/<data> pair into the Hashtable.  If <key> is not
  // unique to the Hashtable, the results are functional but undefined.
  // <key> may not be NULL, but <data> may be.  iinsert does not copy any
  // data passed to it (the Hashtable just stores pointers to data), so
  // <key> and <data> must be valid for the life of the Hashtable (or until
  // they are removed from it).

  // Returns: true if the <key>/<data> pair was successfully added, false
  // otherwise.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "iHashtable::iinsert(%s,%d)",
		   IONULL(key), data);
#endif

  if(unithouse && hashbuckets && key)
  {
    // If this insert would make (2*entries) = totalbuckets, resize the
    // table now.  We ignore the results of resize_hash because even if
    // it failed, we still want to try to insert the item.
    
    if((2 * (entries + 1)) >= totalbuckets)
      if(resize_hash() == 0)
      {
	wlog->warn("Hashtable may not be inconsistent");
	
#if defined(DEBUG)
	dlog->log_exit(DEBUG_MINTRC, "iHashtable::insert = false");
#endif
	return(false);
      }
    
    // Compute the hash, and allocate a HashUnit

    size_t h = compute_hash(key, totalbuckets);

    HashUnit *hu = unithouse->allocate();

    if(hu && hu->initialize(key, data))
    {
      // Try to store the HashUnit.

      if(hashbuckets[h])
      {
	// Add to the beginning of the list [O(1) operation]

	hu->set_next(hashbuckets[h]);
	hashbuckets[h]->set_prev(hu);
      }

      // Insert into place
      hashbuckets[h] = hu;
      entries++;

#if defined(DEBUG)
      dlog->log_exit(DEBUG_MINTRC,
		    "iHashtable::iinsert = true (entry %d added to bucket %d)",
		      entries, h);
#endif      

      return(true);
    }
    else
    {
      wlog->warn("Hashtable unable to allocate HashUnit");

      if(hu)
	unithouse->deallocate(hu);
      hu = NULL;
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "iHashtable::iinsert = false");
#endif

  return(false);
}

HashHandle *iHashtable::iterate_begin()
{
  // Obtain a HashHandle which can be used to iterate every entry in the
  // Hashtable in a safe fashion.  If threads are enabled, a read lock
  // must be obtained before iterate_begin() is invoked, and cannot be
  // released until the HashHandle is discarded with iterate_end().

  // Returns: A HashHandle suitable for passing to iterate_next() or
  // iterate_end().
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "iHashtable::iterate_begin()");
#endif

  HashHandle *h = new HashHandle();

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "iHashtable::iterate_begin = %d", h);
#endif

  return(h);
}

void *iHashtable::iiterate_next(HashHandle *h)
{
  // Retrieve the next item from the Hashtable, using HashHandle as a handle
  // into the Hashtable.

  // Returns: A pointer to the next item, or NULL when the last item has been
  // returned.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "iHashtable::iterate_next(%d)", h);
#endif

  if(h)
  {
    if(h->unit)
    {
      // Advance to next entry (if we find an entry we leave h->unit pointing
      // to the place we found).

      h->unit = h->unit->get_next();
    }

    if(!h->unit)
    {
      // Move to the next bucket

      h->bucket++;
      
      while((h->unit == NULL) && (h->bucket < totalbuckets))
      {
	// Iterate through more buckets until we run out or find a match.
	
	h->unit = hashbuckets[h->bucket];
	
	if(h->unit)
	  break;
	else
	  h->bucket++;
      }
    }
    
    if(h->unit)
    {
#if defined(DEBUG)
      dlog->log_exit(DEBUG_MINTRC, "iHashtable::iterate_next = %d",
		      h->unit->get_data());
#endif

      return(h->unit->get_data());
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "iHashtable::iterate_next = NULL");
#endif

  return(NULL);
}

void iHashtable::iterate_end(HashHandle *h)
{
  // Deallocate the HashHandle <h> to end the iteration session.  After
  // iterate_end() is invoked, the read_lock obtained on the Hashtable may
  // be released.

  // Returns: Nothing.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "iHashtable::iterate_end(%d)", h);
#endif

  if(h)
    delete h;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "iHashtable::iterate_end()");
#endif
}

bool iHashtable::remove(char *key)
{
  // Remove the <key> and its associated data from the Hashtable.  If <key>
  // is not unique to the Hashtable, the results are functional but undefined.
  // Removing only removes the pointers to <key> and its data.  It does not
  // free() any structures.

  // Returns: true if the <key> was found and removed, false otherwise.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "iHashtable::remove(%s)", IONULL(key));
#endif

  if(unithouse && hashbuckets && key)
  {
    // Compute the hash, then try to remove it from that bucket

    size_t h = compute_hash(key, totalbuckets);

    HashUnit *hu = hashbuckets[h];
    HashUnit *hp = NULL;
    
    while(hu != NULL)
    {
      char *huk = hu->get_key();

      if(huk && strcmp(huk, key)==0)
      {
	// Match found.  Remove the entry and seal the chain.

	HashUnit *hn = hu->get_next();

	// Set the previous to point to the next
	
	if(hp)
	  hp->set_next(hn);     // if hn is NULL, we have a new tail
	else
	  hashbuckets[h] = hn;  // new head of chain (or empty is !hn)

	// Set the next to point to the previous

	if(hn)
	  hn->set_prev(hp);
	
	// Deallocate this one

	hu->clear();
	unithouse->deallocate(hu);
	
	entries--;

#if defined(DEBUG)
	dlog->log_exit(DEBUG_MINTRC,
		    "iHashtable::remove = true (entry %d removed from bin %d)",
			entries+1, h);
#endif
	return(true);
      }

      // else move on

      hp = hu;
      hu = hu->get_next();
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "iHashtable::remove = false");
#endif
  
  return(false);
}

void *iHashtable::iretrieve(char *key)
{
  // Retrieve the data associated with <key> from the Hashtable.  If <key>
  // is not unique to the Hashtable, the results are functional but undefined.

  // Returns: The data, if found, or NULL.  Note that NULL is valid data if
  // inserted, so it is up to the application to distinguish NULL (meaning no
  // answer) from NULL (meaning: the answer is 'NULL').
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "iHashtable::iretrieve(%s)", IONULL(key));
#endif

  if(unithouse && hashbuckets && key)
  {
    // Compute the hash, then try to retrieve it from that bucket

    size_t h = compute_hash(key, totalbuckets);

    HashUnit *hu = hashbuckets[h];
    
    while(hu != NULL)
    {
      char *huk = hu->get_key();

      if(huk && strcmp(huk, key)==0)
      {
	// Match found.  Return the entry.

	void *r = hu->get_data();
	
#if defined(DEBUG)
	dlog->log_exit(DEBUG_MINTRC, "iHashtable::iretrieve = %d", r);
#endif
	
	return(r);
      }

      // else move on

      hu = hu->get_next();
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "iHashtable::iretrieve = NULL");
#endif
  
  return(NULL);
}

size_t iHashtable::compute_hash(char *key, size_t buckets)
{
  // Hash <key> to determine the bucket (numbered [0,<buckets>) ) that
  // <key> belongs to.

  // Returns: The index of the bucket, or -1 on error.  (Note that since
  // size_t is unsigned, -1 is actually returning a very large number.)

  size_t r = -1;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "iHashtable::compute_hash(%s,%d)",
		   IONULL(key), buckets);
#endif
  
  if(key)
  {
    // hashpjw (dragon book sec 7.6).  Note that we just continue the
    // expectation of 32-bit numbers here.

    char *p = key;
    unsigned long g,h = 0;

    for(p;*p != '\0';p++)
    {
      h = (h << 4) + *p;

      if(g = h&0xf0000000)
      {
	h = h ^ (g >> 24);
	h = h ^ g;
      }
    }

    r = h % buckets;
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "iHashtable::compute_hash = %d", r);
#endif
  
  return(r);
}

bool iHashtable::initialize_hash(size_t buckets)
{
  // Initialize the Hashtable with <buckets> buckets.

  // Returns: true if successful, false otherwise.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "iHashtable::initialize_hash(%d)", buckets);
#endif

  unithouse = NULL;
  hashbuckets = NULL;
  totalbuckets = 0;
  entries = 0;

  if(buckets > 0)
  {
    // First create a unithouse to allocate HashUnits from.  We deallocate
    // HashUnits when items are removed from the Hashtable, which for now is
    // assumed to be relatively infrequent.

    unithouse = new Warehouse<HashUnit>(buckets, buckets, 100);

    if(unithouse)
    {
      // Now allocate the array from which we access each Bucket

      hashbuckets = (HashUnit **)malloc(buckets * (sizeof(HashUnit *)));

      if(hashbuckets)
      {
	// Initialize the Hashtable.

	for(size_t i = 0;i < buckets;i++)
	  hashbuckets[i] = NULL;

	totalbuckets = buckets;
	
#if defined(DEBUG)
	dlog->log_exit(DEBUG_MAJTRC, "iHashtable::initialize_hash = true");
#endif

	return(true);
      }
      else
      {
	delete unithouse;
	unithouse = NULL;

	wlog->warn("Hashtable failed to allocate bucket array");
      }
    }
    else
      wlog->warn("Hashtable failed to allocate unithouse");
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "iHashtable::initialize_hash = false");
#endif

  return(false);
}

bool iHashtable::resize_hash()
{
  // Resize the Hashtable by doubling the number of buckets.  This involves
  // (1) creating a new Hashtable, (2) reassigning everything in the old
  // Hashtable, and (3) deleting the old Hashtable.  If the resize fails, the
  // old Hashtable is left in tact if possible.  Note that resizing the
  // Hashtable is not designed for speed (make the common case fast, etc),
  // and so Hashtables should be created large enough so that resizing does
  // not occur (or does not occur often).

  // Returns: true if the hash table was resized successfully, false otherwise.
  // On a false return, the hash table is still usable.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "iHashtable::resize_hash()");
#endif

  // There's no need to allocate a new unithouse since it can increase its
  // own size (and this may be better performance-wise, anyway, since it can
  // allocate smaller blocks of memory as needed.
  
  size_t newtotalbuckets = 2 * totalbuckets;
  HashUnit **newhashbuckets = NULL;

  // First try to allocate a new Array

  newhashbuckets = (HashUnit **)malloc(newtotalbuckets * sizeof(HashUnit *));

  if(newhashbuckets)
  {
    // First initialize the new table

    for(size_t i = 0;i < newtotalbuckets;i++)
      newhashbuckets[i] = NULL;

    // Next, iterate through the old table, taking each chain one at a time.
    // Break each HashUnit off the chain and insert it into the new table.

    for(size_t i = 0;i < totalbuckets;i++)
    {
      HashUnit *hu = hashbuckets[i];

      while(hu != NULL)
      {
	HashUnit *nexthu = hu->get_next();

	size_t h = compute_hash(hu->get_key(), newtotalbuckets);

	hu->set_prev(NULL);
	hu->set_next(newhashbuckets[h]);
	
	if(newhashbuckets[h])
	{
	  // Insert at the beginning of the chain [O(1) operation]

	  newhashbuckets[h]->set_prev(hu);
	}

	// And assign it into place
	newhashbuckets[h] = hu;
	
	hu = nexthu;
      }
    }

    HashUnit **oldhashbuckets = hashbuckets;
    hashbuckets = newhashbuckets;
    totalbuckets = newtotalbuckets;

    free(oldhashbuckets);

#if defined(DEBUG)
    dlog->log_exit(DEBUG_MAJTRC, "iHashtable::resize_hash = true (resized)");
#endif
    
    return(true);
  }
  else
    wlog->warn("Hashtable failed to allocate new Bucket array");
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC,
	       "iHashtable::resize_hash =  false (not resized, still usable)");
#endif

  return(false);
}

iHashtable::~iHashtable()
{
  // Deallocate the Hashtable.  This cleans up internal data structures, but
  // does not touch pointers that were inserted.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "iHashtable::~iHashtable()");
#endif

  if(hashbuckets)
  {
    free(hashbuckets);
    hashbuckets = NULL;
  }

  xdelete(unithouse);
  
  totalbuckets = 0;
  entries = 0;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "iHashtable::~iHashtable");
#endif
}
