/*
 * Counter.C: survivor Counter object
 *
 * Version: $Revision: 0.1 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2004/03/01 23:15:05 $
 * MT-Level: Safe
 *
 * Copyright (c) 2004
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: Counter.C,v $
 * Revision 0.1  2004/03/01 23:15:05  benno
 * Initial revision
 *
 */

#include "survivor.H"

Counter::Counter()
{
  // Allocate and initialize a new Counter object.

  // Returns: A new, initialized Counter object.

  init(NULL, 0, INT_MIN, INT_MAX);
}

Counter::Counter(char *name, int start)
{
  // Allocate and initialize a new Counter object named <name>,
  // starting at <start>.

  // Returns: A new, initialized Counter object.
  
  init(name, start, INT_MIN, INT_MAX);
}

Counter::Counter(char *name, int start, int min, int max)
{
  // Allocate and initialize a new Counter object named <name>,
  // starting at <start>, with permissible values ranging from
  // <min> through <max>.

  // Returns: A new, initialized Counter object.
  
  init(name, start, min, max);
}

bool Counter::decrement()
{
  // Decrement the value of this Counter.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Counter::decrement()");
#endif

  if(v > mn)
  {
    v--;
    ret = true;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Counter::decrement = %s", IOTF(ret));
#endif

  return(ret);
}

bool Counter::increment()
{
  // Increment the value of this Counter.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Counter::increment()");
#endif

  if(v < mx)
  {
    v++;
    ret = true;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Counter::increment = %s", IOTF(ret));
#endif

  return(ret);
}

int Counter::maximum()
{
  // Obtain the maximum value permitted for this Counter.

  // Returns: The maximum value.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Counter::maximum()");
  dlog->log_exit(DEBUG_MINTRC, "Counter::maximum = %d", mx);
#endif
  
  return(mx);
}

int Counter::minimum()
{
  // Obtain the minimum value permitted for this Counter.

  // Returns: The minimum value.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Counter::minimum()");
  dlog->log_exit(DEBUG_MINTRC, "Counter::minimum = %d", mn);
#endif
  
  return(mn);
}

char *Counter::name()
{
  // Obtain the name of this Counter.

  // Returns: The Counter name, if set, NULL otherwise.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Counter::name()");
  dlog->log_exit(DEBUG_MINTRC, "Counter::name = %s", IONULL(n));
#endif
  
  return(n);
}

int Counter::value()
{
  // Obtain the value of this Counter.

  // Returns: The Counter value.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Counter::value()");
  dlog->log_exit(DEBUG_MINTRC, "Counter::value = %d", v);
#endif
  
  return(v);
}

Counter::~Counter()
{
  // Delete the Counter object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Counter::~Counter()");
#endif

  xdelete(n);
  mn = 0;
  mx = 0;
  v = 0;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Counter::~Counter()");
#endif
}

void Counter::init(char *name, int start, int min, int max)
{
  // Initialize the Counter with <name> and starting value <start>,
  // that may range from <min> through <max>.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Counter::init(%s,%d,%d,%d)", IONULL(name),
		  start, min, max);
#endif

  n = xstrdup(name);
  mn = min;
  mx = max;
  v = start;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Counter::init()");
#endif   
}
