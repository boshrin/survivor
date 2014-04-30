/*
 * Warehouse.C: Large Scale Object Factory
 *
 * Version: $Revision: 0.6 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2003/04/09 20:23:51 $
 * MT-Level: Unsafe
 *
 * Copyright (c) 2002 - 2003
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: Warehouse.C,v $
 * Revision 0.6  2003/04/09 20:23:51  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.5  2003/04/06 14:20:07  benno
 * Use Debugger
 *
 * Revision 0.4  2003/03/04 20:51:26  benno
 * Bump copyright
 *
 * Revision 0.3  2002/04/04 20:12:06  benno
 * copyright
 *
 * Revision 0.2  2002/04/03 18:56:05  benno
 * rcsify date
 *
 * Revision 0.1  2002/04/03 18:55:48  benno
 * initial revision
 *
 */

#include "survivor.H"

iWarehouse::iWarehouse(size_t initial, size_t incremental, size_t decremental,
		       size_t size)
{
  // Allocate a Warehouse, initially with space for <initial> items of class
  // T, with new allocations of <incremental> when existing space is filled.
  // Items are allocated to be the size of <size>.
  // <decremental> indicates the block size for the freelist (which stores
  // deallocated space).  The larger the number of objects deallocated at
  // any given time, the larger <decremental> should be.
  // If any parameters passed to this constructor are less than or equal to
  // 0, undefined results may occur.
  // After instantiating a Warehouse object, the method valid() should be
  // invoked to verify that the Warehouse was properly allocated.
  
  // Returns: A new Warehouse object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "iWarehouse::iWarehouse(%d,%d,%d,%d)",
		   initial, incremental, decremental, size);
#endif
  
  if(size > 0 && initial > 0)
  {
    // Under Solaris, undefined results may occur if the size passed to
    // malloc exceeds the maximum size of the process heap.

    struct rlimit r;
    getrlimit(RLIMIT_DATA, &r);

    if(r.rlim_max == RLIM_INFINITY ||
       ((r.rlim_max > (initial * size)) &&
	(r.rlim_max > (incremental * size))))
    {    
      block = (void **)malloc(sizeof(void *));

      if(!block)
	wlog->warn("Warehouse unable to allocate block array");
    }
    else
      wlog->warn("Warehouse unable to allocate blocks larger than maximum data heap");
  }
  else
  {
    block = NULL;

#if defined(DEBUG)
    dlog->log_progress(DEBUG_MAJMEM, "Invalid size/initial value provided");
#endif
  }

  if(block)
  {
    block[0] = (void *)malloc(initial * size);

    if(block[0] == NULL)
    {
      free(block);
      block = NULL;

      wlog->warn("Warehouse unable to allocate initial block (%d)",
		 (initial * size));
    }
  }
  
  if(block)
    blocks = 1;
  else
    blocks = 0;

  itemsize = size;
  init = initial;
  incr = incremental;
  nextfree = 0;

  freelist = NULL;
  freelists = 0;
  decr = decremental;
  lastfree = -1;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "iWarehouse::iWarehouse");
#endif
}

void *iWarehouse::iallocate()
{
  // Allocate a block of storage from within the Warehouse for an
  // object of previously assigned size.  Note that the object's
  // constructor is not called.

  // Returns: A pointer to a block of storage, or NULL on error.

  // First see if we have any deallocated objects we can reallocate

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "iWarehouse::iallocate()");
#endif

  if(freelist && freelists > 0 && lastfree > -1)
  {
#if defined(DEBUG)
    dlog->log_exit(DEBUG_ANYMEM, "Reusing object from freelist");
#endif

    void *r = freelist[freelists - 1][lastfree];
    lastfree--;

    if(lastfree < 0)
    {
      // Discard the now empty free list

      free(freelist[freelists - 1]);
      freelist[freelists - 1] = NULL;
      freelists--;

      if(freelists == 0)
      {
	// If there are no more freelists, toss the array array.
      
	free(freelist);
	freelist = NULL;
      }
      else
      {
	// Otherwise point lastfree to the last element

	lastfree = decr - 1;
      }
    }

#if defined(DEBUG)
    dlog->log_exit(DEBUG_MINTRC, "iWarehouse::iallocate = %d", r);
#endif
    
    return(r);
  }

  // Next see if we need to allocate a new block
  
  if((blocks == 1 && nextfree >= init) ||
     (blocks > 1 && nextfree >= incr))
  {
    // Create a new block and resize the <block> array
    
#if defined(DEBUG)
    dlog->log_progress(DEBUG_MAJMEM, "Allocating a new block (%d)",
			(incr * itemsize));
#endif

    if(incr < 1)
      return(NULL);

    void *newblock = (void *)malloc(incr * itemsize);

    if(newblock)
    {
#if defined(DEBUG)
      dlog->log_progress(DEBUG_MINMEM, "Allocating a new block array");
#endif

      void **newarray = (void **)realloc(block,
					 (blocks + 1) * sizeof(void *));

      if(newarray)
      {
	// Store the new block, reset the nextfree value
	
	block = newarray;
	block[blocks] = newblock;
	blocks++;
	nextfree = 0;
      }
      else
      {
	// Free the allocated block and return NULL

	wlog->warn("Warehouse failed to allocate new block array");

#if defined(DEBUG)
	dlog->log_exit(DEBUG_MINTRC, "iWarehouse::iallocate = NULL");
#endif
	
	free(newblock);
	return(NULL);
      }
    }
    else
    {
      wlog->warn("Warehouse failed to allocate new block");

#if defined(DEBUG)
      dlog->log_exit(DEBUG_MINTRC, "iWarehouse::iallocate = NULL");
#endif
      
      return(NULL);
    }
  }

  // Finally assign space for the object
  
  if(block && blocks > 0)
  {
    void *r = (void *)((size_t)block[blocks - 1] +
		       (size_t)(nextfree * itemsize));
    nextfree++;

#if defined(DEBUG)
    dlog->log_exit(DEBUG_MINTRC, "iWarehouse::iallocate = %d", r);
#endif
    return(r);
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "iWarehouse::iallocate = NULL");
#endif

  return(NULL);
}

bool iWarehouse::ideallocate(void *x)
{
  // Return the block <x> to the freelist, making it available for future
  // calls to allocate.

  // Returns: true if <x> is returned to the freelist, false otherwise.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "iWarehouse::ideallocate(%d)", x);
#endif

  if(x)
  {
    if(!freelist || lastfree < 0 || lastfree >= (decr - 1))
    {
      // We need to allocate a new freelist.

#if defined(DEBUG)
      dlog->log_progress(DEBUG_MAJMEM, "Allocating a new freelist");
#endif
      
      void **newlist = (void **)malloc(decr * sizeof(void *));

      if(newlist)
      {
	// Next expand the size of the array
	
	void ***newfreelist = (void ***)realloc(freelist, (freelists + 1)
						* sizeof(void **));

	if(newfreelist)
	{
	  // Store the new freelist, adjust counters

	  freelist = newfreelist;
	  freelist[freelists] = newlist;
	  freelists++;
	  lastfree = -1;
	}
	else
	{
	  // Free the newly allocated freelist

	  wlog->warn("Warehouse failed to allocate new freelist array");

#if defined(DEBUG)
	  dlog->log_exit(DEBUG_MINTRC, "iWarehouse::ideallocate = false");
#endif

	  free(newlist);
	  return(false);
	}
      }
      else
      {
	wlog->warn("Warehouse failed to allocate new freelist");

#if defined(DEBUG)
	dlog->log_exit(DEBUG_MINTRC, "iWarehouse::ideallocate = false");
#endif

	return(false);
      }
    }

    // Put the pointer into place

    lastfree++;
    freelist[freelists - 1][lastfree] = x;

#if defined(DEBUG)
    dlog->log_exit(DEBUG_MINTRC, "iWarehouse::ideallocate = true");
#endif

    return(true);
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "iWarehouse::ideallocate = false");
#endif
  return(false);
}

bool iWarehouse::valid()
{
  // Determined if the Warehouse is valid.  Use this method after creating a
  // new Warehouse to determine if the Warehouse was properly allocated.

  // Returns: true if the Warehouse is consistent, false otherwise.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "iWarehouse::valid()");
  dlog->log_exit(DEBUG_MAJTRC, "iWarehouse::valid = %s",
		  ((block && block > 0) ? "true" : "false"));
#endif

  if(block && block > 0)
    return(true);
  else
    return(false);
}

iWarehouse::~iWarehouse()
{
  // Deallocate the Warehouse.  All objects of class T allocated from the
  // Warehouse become invalid when the Warehouse is deleted.

  // Returns: Nothing.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "iWarehouse::~iWarehouse()");
#endif

  if(block)
  {
#if defined(DEBUG)
    dlog->log_progress(DEBUG_ANYMEM, "Freeing blocks");
#endif

    for(int i = 0;i < blocks;i++)
      if(block[i])
      {
	free(block[i]);
	block[i] = NULL;
      }
    
    free(block);
    block = NULL;
  }

  blocks = 0;
  itemsize = 0;
  init = 0;
  incr = 0;
  nextfree = 0;

  // Don't free items in the freelist as they're just pointers into block.

  if(freelist)
  {
#if defined(DEBUG)
    dlog->log_progress(DEBUG_ANYMEM, "Freeing freelists");
#endif

    for(int i = 0;i < freelists;i++)
      if(freelist[i])
      {
	free(freelist[i]);
	freelist[i] = NULL;
      }
    
    free(freelist);
    freelist = NULL;
  }

  decr = 0;
  lastfree = 0;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "iWarehouse::~iWarehouse");
#endif
}
