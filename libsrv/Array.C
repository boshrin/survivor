/*
 * Array.C: A simple array object.
 *
 * Version: $Revision: 0.10 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2005/10/20 02:12:22 $
 * MT-Level: Unsafe
 *
 * Copyright (c) 2002 - 2005
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: Array.C,v $
 * Revision 0.10  2005/10/20 02:12:22  benno
 * Array(Array) constructor
 *
 * Revision 0.9  2005/09/26 13:53:48  benno
 * Add support for sorting
 *
 * Revision 0.8  2004/08/24 23:53:29  toor
 * Use malloc not realloc
 *
 * Revision 0.7  2003/04/09 20:23:45  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.6  2003/04/04 03:58:33  benno
 * Use Debugger
 *
 * Revision 0.5  2003/03/04 17:56:57  benno
 * Add ifind
 * Bump copyright
 *
 * Revision 0.4  2003/01/23 22:39:16  benno
 * Use IONULL, IOTF, xdelete, and xadelete
 *
 * Revision 0.3  2002/04/04 20:06:44  benno
 * copyright
 *
 * Revision 0.2  2002/04/03 17:38:55  benno
 * rcsify date
 *
 * Revision 0.1  2002/04/03 17:38:30  benno
 * initial revision
 *
 */

#include "survivor.H"

iArray::iArray()
{
  // Allocate and initialize a new Array object.

  // Returns: A new Array object.

  init(NULL);
}

iArray::iArray(char *name)
{
  // Allocate and initialize a new Array object named <name>.

  // Returns: A new Array object.

  init(name);
}

iArray::iArray(iArray *array)
{
  // Allocate and initialize a new Array object with the contents of <array>.

  // Returns: A new Array object.

  init(NULL);

  if(array)
  {
    for(int i = 0;i < array->entries();i++)
    {
      if(!iadd(array->iretrieve(i)))
	wlog->warn("iArray::iArray failed to add entry %d", i);
    }
  }
}

bool iArray::iadd(void *data)
{
  // Add the data <data> to the end of the Array.  <data> must remain valid,
  // for it is not duplicated.

  // Returns: true if <s> was added to the Array, false otherwise.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "iArray::iadd(%d)", data);
#endif
  
  if(data)
  {    
    void **newitems = (void **)malloc((size + 1) * sizeof(void *));

    if(newitems)
    {
      for(int i = 0;i < size;i++)
	newitems[i] = items[i];
      
      newitems[size] = data;
      size++;

      if(items)
	free(items);

      items = newitems;
      newitems = NULL;
      
#if defined(DEBUG)
      dlog->log_exit(DEBUG_MINTRC, "iArray::iadd = true");
#endif
	
      return(true);
    }
    else
      wlog->warn("iArray::add unable to reallocate array");
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "iArray::iadd = false");
#endif

  return(false);
}

int iArray::entries()
{
  // Determine the number of entries in the Array.

  // Returns: The number of entries in the Array.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "iArray::entries()");
  dlog->log_exit(DEBUG_MINTRC, "iArray::entries = %d", size);
#endif
  
  return(size);
}

int iArray::ifind(void *data)
{
  // Determine if the pointer <data> is in this Array, and if so obtain
  // its index, from 0 to entries()-1.

  // Returns: The index if found, or -1 if not.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "iArray::ifind(%d)", data);
#endif

  if(data && size > 0 && items)
  {
    for(int i = 0;i < size;i++)
    {
      if(items[i] && (items[i] == data))
      {
#if defined(DEBUG)
        dlog->log_exit(DEBUG_MINTRC, "iArray::ifind = %d", i);
#endif
  
        return(i);
      }
    }
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "iArray::ifind = -1");
#endif
  
  return(-1);
}

char *iArray::name()
{
  // Obtain the name of this Array.

  // Returns: The Array name, or NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "iArray::name()");
  dlog->log_entry(DEBUG_MINTRC, "iArray::name = %s", IONULL(arrayname));
#endif
  
  return(arrayname);
}

bool iArray::remove(int index)
{
  // Remove the item at position <index> and resize the Array.  This method
  // does not free any resources used by the item, which should be
  // retrieve()d and freed or deleted prior to calling this method.

  // Returns: true if the item is successfully removed, false otherwise.
  // On a false return, the Array remains consistent but unchanged.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "iArray::remove(%d)", index);
#endif

  if(items && (index >= 0) && (index < size))
  {
    void **newitems = (void **)malloc(sizeof(void *) * (size - 1));

    if(newitems)
    {
      int i = 0;

      for(int j = 0;j < size;j++)
      {
	if(j != index)
	{
	  newitems[i] = items[j];
	  i++;
	}
      }

      free(items);
      items = newitems;
      size--;

#if defined(DEBUG)
      dlog->log_exit(DEBUG_MINTRC, "iArray::remove = true");
#endif
      
      return(true);
    }
    else
      wlog->warn("iArray::remove unable to allocate new array");
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "iArray::remove = false");
#endif
  
  return(false);
}

void *iArray::iretrieve(int index)
{
  // Obtain the entry at <index> where indexes run from 0 to entries()-1.

  // Returns: A pointer to the entry, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "iArray::retrieve(%d)", index);
#endif

  if(items && index >= 0 && index < size)
  {
#if defined(DEBUG)
    dlog->log_exit(DEBUG_MINTRC, "iArray::retrieve = %d", items[index]);
#endif

    return(items[index]);
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "iArray::retrieve = NULL");
#endif

  return(NULL);
}

iArray *iArray::isort(sort_t order)
{
  // Sort the Array in <order>.  IMPORTANT: Only Arrays holding
  // objects derived from the Sortable class may be sorted.

  // Returns: A pointer to an iArray that remains valid for the duration
  // of this iArray or until the Array is sorted again, or NULL on error.

  // We don't sort the original <items> in case it was, eg, a configuration
  // defined Array, where we want to leave the original order.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "iArray::isort()");
#endif

  xdelete(sorted);

  if(sitems)
  {
    free(sitems);
    sitems = NULL;
  }

  if(size > 0)
  {
    // Copy items into a new array for sorting.

    sitems = (void **)malloc(size * sizeof(void *));

    if(sitems)
    {
      for(int i = 0;i < size;i++)
	sitems[i] = items[i];

      // Now run the sort

      qs_sort(0, size-1, order);

      // Finally, allocate a new Array to return

      sorted = new iArray();

      if(sorted)
      {
	for(int i = 0;i < size;i++)
	  sorted->iadd(sitems[i]);
      }
      else
	wlog->warn("iArray::isort failed to allocate sorted");
    }
    else
      wlog->warn("iArray::isort failed to allocate sitems");
  }
  else
  {
    // Empty array

    sorted = new iArray();
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "iArray::isort = %d", sorted);
#endif

  return(sorted);
}

iArray::~iArray()
{
  // Deallocate the iArray object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "iArray::~iArray()");
#endif

  if(items)
  {
    free(items);
    items = NULL;
  }

  size = 0;
  xdelete(arrayname);
  xdelete(sorted);

  if(sitems)
  {
    free(sitems);
    sitems = NULL;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "iArray::~iArray()");
#endif
}

void iArray::init(char *name)
{
  // Initializer for constructors.

  // Returns: Nothing.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "iArray::init(%s)", IONULL(name));
#endif
  
  items = NULL;
  arrayname = xstrdup(name);
  size = 0;
  sorted = NULL;
  sitems = NULL;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "iArray::init()");
#endif
}

void iArray::qs_sort(int p, int r, sort_t order)
{
  // Implement quicksort algorithm.  <sitems> is sorted in place.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "iArray::qs_sort(%d,%d,%d)", p, r, order);
#endif

  // It would probably make sense to use STL's sort or something, but
  // that would require a fair amount of rewriting.  Instead, we reuse
  // List.C's sort.
  
  if(p < r)
  {
    int q = qs_partition(p, r, order);
    qs_sort(p, q, order);
    qs_sort(q+1, r, order);
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "iArray::qs_sort()");
#endif
}

int iArray::qs_partition(int p, int r, sort_t order)
{
  // Implement quicksort partition algorithm.  <sitems> is partitioned
  // in place.

  // Returns: The partition point.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "iArray::qs_partition(%d,%d,%d)", p, r, order);
#endif

  int i = p - 1;
  int j = r + 1;

  while(1)
  {
    switch(order)
    {
    case ascending_sort:
    case ascending_caseless_sort:
      for(j = j - 1;
	  j >= p &&
	    ((Sortable *)sitems[j])->sort_key() >
	    ((Sortable *)sitems[p])->sort_key();
	  j--)
	;

      for(i = i + 1;
	  i <= r &&
	    ((Sortable *)sitems[i])->sort_key() <
	    ((Sortable *)sitems[p])->sort_key();
	  i++)
	;
      break;
    case descending_sort:
    case descending_caseless_sort:
      for(j = j - 1;
	  j >= p &&
	    ((Sortable *)sitems[j])->sort_key() <
	    ((Sortable *)sitems[p])->sort_key();
	  j--)
	;

      for(i = i + 1;
	  i <= r &&
	    ((Sortable *)sitems[i])->sort_key() >
	    ((Sortable *)sitems[p])->sort_key();
	  i++)
	;
      break;
    default:
      wlog->warn("Unknown sort type %d in qs_partition", order);
      break;
    }

    if(i < j)
    {
      void *x = sitems[i];

      sitems[i] = sitems[j];
      sitems[j] = x;
    }
    else
    {
#if defined(DEBUG)
      dlog->log_exit(DEBUG_MINTRC, "iArray::qs_partition = %d", j);
#endif
      
      return(j);
    }
  }
}
