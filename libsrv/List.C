/*
 * List.C: A simple list object.  It is designed for use with the Configuration
 * object and so currently is not optimized for high intensity performance.
 *
 * Version: $Revision: 0.10 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2005/09/26 13:43:47 $
 * MT-Level: Unsafe
 *
 * Copyright (c) 2002 - 2005
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: List.C,v $
 * Revision 0.10  2005/09/26 13:43:47  benno
 * Fix some documentation
 *
 * Revision 0.9  2004/08/25 00:08:37  toor
 * Use malloc instead of realloc
 * Use xstrcat to build commaline
 * Add sorting via quicksort
 *
 * Revision 0.8  2003/04/09 20:23:48  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.7  2003/04/05 23:24:56  benno
 * Use Debugger
 *
 * Revision 0.6  2003/01/24 17:57:41  benno
 * Use IONULL, IOTF, xdelete, and xadelete
 *
 * Revision 0.5  2002/08/06 19:56:37  selsky
 * Remove embedded nulls from format
 *
 * Revision 0.4  2002/04/04 20:10:50  benno
 * copyright
 *
 * Revision 0.3  2002/04/03 18:23:16  benno
 * rcsify date
 *
 * Revision 0.2  2002/04/03 18:23:03  benno
 * Add initialize from existing List
 *
 * Revision 0.1  2002/04/03 18:21:57  benno
 * initial revision
 *
 */

#include "survivor.H"

List::List()
{
  // Allocate and initialize a new List object.

  // Returns: A new List object.

  init(NULL, NULL);
}

List::List(char *name)
{
  // Allocate and initialize a new List object named <name>.

  // Returns: A new List object.

  init(NULL, name);
}

List::List(List *l)
{
  // Allocate and initialize a new List object, whose contents are
  // copied from the existing List <l>.

  // Returns: A new List object.

  init(l, NULL);
}

List::List(List *l, char *name)
{
  // Allocate and initialize a new List object named <name>, whose
  // contents are copied from the existing List <l>.

  // Returns: A new List object.

  init(l, name);
}

bool List::add(char *s)
{
  // Add the string <s> to the end of the List.  <s> is duplicated, so it need
  // not remain valid for the life of the List object.

  // Returns: true if <s> was added to the List, false otherwise.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "List::add(%s)", IONULL(s));
#endif
  
  if(s)
  {
    char *p = xstrdup(s);

    if(p)
    {
      char **newitems = (char **)malloc((size + 1) * sizeof(char *));

      if(newitems)
      {
	for(int i = 0;i < size;i++)
	  newitems[i] = items[i];

	newitems[size] = p;
	size++;

	if(items)
	  free(items);
	
	items = newitems;
	newitems = NULL;

	// Now that we've added s to the array, append it to commaline.

	if(commaline)
	  commaline = xstrcat(commaline, ",");
	
	commaline = xstrcat(commaline, s);

	if(commaline)
	{
#if defined(DEBUG)
	  dlog->log_exit(DEBUG_MINTRC, "List::add = true");
#endif
	  
	  return(true);
	}
      }
      else
      {
	wlog->warn("List::add unable to reallocate array");

	delete p;
      }
    }
    else
      wlog->warn("List::add unable to duplicate string");
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "List::add = false");
#endif

  return(false);
}

char *List::comma_list()
{
  // Obtain a one line, comma separated list of the items in this List.

  // Returns: A pointer to a string that should not be modified, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "List::comma_list()");
  dlog->log_exit(DEBUG_MINTRC, "List::comma_list = %s", IONULL(commaline));
#endif
  
  return(commaline);
}

int List::entries()
{
  // Determine the number of entries in the List.

  // Returns: The number of entries in the List.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "List::entries()");
  dlog->log_exit(DEBUG_MINTRC, "List::entries = %d", size);
#endif
  
  return(size);
}

int List::find(char *s)
{
  // Determine if the string <s> is in this List, and if so obtain its index,
  // from 0 to entries()-1.

  // Returns: The index if found, or -1 if not.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "List::find(%s)", IONULL(s));
#endif

  if(s && size > 0 && items)
  {
    for(int i = 0;i < size;i++)
    {
      if(items[i] && strcmp(items[i], s)==0)
      {
#if defined(DEBUG)
	dlog->log_exit(DEBUG_MINTRC, "List::find = %d", i);
#endif
  
	return(i);
      }
    }
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "List::find = -1");
#endif
  
  return(-1);
}

char *List::name()
{
  // Obtain the name of this List.

  // Returns: The List name, or NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "List::name()");
  dlog->log_exit(DEBUG_MINTRC, "List::name = %s", IONULL(listname));
#endif
  
  return(listname);
}

char *List::retrieve(int index)
{
  // Obtain the entry at <index> where indexes run from 0 to entries()-1.

  // Returns: A pointer to the entry, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "List::retrieve(%d)", index);
#endif

  if(items && index >= 0 && index < size)
  {
#if defined(DEBUG)
    dlog->log_exit(DEBUG_MINTRC, "List::retrieve = %s", IONULL(items[index]));
#endif

    return(items[index]);
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "List::retrieve = NULL");
#endif

  return(NULL);
}

bool List::sort(sort_t order)
{
  // Sort the contents of the list according to <order>.

  // Returns: True if fully successful, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "List::sort(%d)", order);
#endif

  qs_sort(0, size-1, order);

  // Regenerate commaline

  xdelete(commaline);

  for(int i = 0;i < size;i++)
  {
    if(i > 0)
      commaline = xstrcat(commaline, ",");
    
    commaline = xstrcat(commaline, items[i]);
  }
  
  ret = true;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "List::sort = %s", IOTF(ret));
#endif

  return(ret);
}

List::~List()
{
  // Deallocate the List object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "List::~List()");
#endif

  if(size && items)
  {
#if defined(DEBUG)
    dlog->log_progress(DEBUG_MINMEM, "Freeing items");
#endif

    for(int i = 0;i < size;i++)
      if(items[i])
      {
	delete items[i];
	items[i] = NULL;
      }

    free(items);
  }

  items = NULL;
  size = 0;

  xdelete(commaline);
  xdelete(listname);
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "List::~List()");
#endif
}

void List::init(List *l, char *name)
{
  // Initializer for constructors.

  // Returns: Nothing.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "List::init(%d,%s)", l, IONULL(name));
#endif

  items = NULL;
  commaline = NULL;
  listname = xstrdup(name);
  size = 0;

  if(l)
  {
    for(int i = 0;i < l->entries();i++)
      add(l->retrieve(i));
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "List::init()");
#endif
}

void List::qs_sort(int p, int r, sort_t order)
{
  // Implement quicksort algorithm.  The list is sorted in place.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "List::qs_sort(%d,%d,%d)", p, r, order);
#endif

  // It would probably make sense to use STL's sort or something, but
  // that would require a fair amount of rewriting.  Array.C does
  // something similar.
  
  if(p < r)
  {
    int q = qs_partition(p, r, order);
    qs_sort(p, q, order);
    qs_sort(q+1, r, order);
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "List::qs_sort()");
#endif
}

int List::qs_partition(int p, int r, sort_t order)
{
  // Implement quicksort partition algorithm.  The list is partitioned
  // in place.

  // Returns: The partition point.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "List::qs_partition(%d,%d,%d)", p, r, order);
#endif

  int i = p - 1;
  int j = r + 1;

  while(1)
  {
    switch(order)
    {
    case ascending_sort:
      for(j = j - 1;j >= p && strcmp(items[j], items[p]) > 0;j--)
	;

      for(i = i + 1;i <= r && strcmp(items[i], items[p]) < 0;i++)
	;
      break;
    case ascending_caseless_sort:
      for(j = j - 1;j >= p && strcasecmp(items[j], items[p]) > 0;j--)
	;

      for(i = i + 1;i <= r && strcasecmp(items[i], items[p]) < 0;i++)
	;
      break;
    case descending_sort:
      for(j = j - 1;j >= p && (-1 * strcmp(items[j], items[p])) > 0;j--)
	;

      for(i = i + 1;i <= r && (-1 * strcmp(items[i], items[p])) < 0;i++)
	;
      break;
    case descending_caseless_sort:
      for(j = j - 1;j >= p && (-1 * strcasecmp(items[j], items[p])) > 0;j--)
	;

      for(i = i + 1;i <= r && (-1 * strcasecmp(items[i], items[p])) < 0;i++)
	;
      break;
    default:
      wlog->warn("Unknown sort type %d in qs_partition", order);
      break;
    }

    if(i < j)
    {
      char *x = items[i];

      items[i] = items[j];
      items[j] = x;
    }
    else
    {
#if defined(DEBUG)
      dlog->log_exit(DEBUG_MINTRC, "List::qs_partition = %d", j);
#endif
      
      return(j);
    }
  }
}
