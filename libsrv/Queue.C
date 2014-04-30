/*
 * Queue.C: survivor queue object
 *
 * Version: $Revision: 0.7 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2007/03/29 12:11:07 $
 * MT-Level: Unsafe
 *  Locking is handled by the calling process in order to allow atomic
 *  transactions (eg: find, then append) to be performed.
 *
 * Copyright (c) 2002 - 2007
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: Queue.C,v $
 * Revision 0.7  2007/03/29 12:11:07  benno
 * Time out stalled checks in the Queue
 *
 * Revision 0.6  2003/04/09 20:23:49  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.5  2003/04/05 23:35:43  benno
 * Use Debugger
 *
 * Revision 0.4  2003/01/24 18:00:33  benno
 * Add IONULL and IOTF
 *
 * Revision 0.3  2002/04/04 20:11:07  benno
 * copyright
 *
 * Revision 0.2  2002/04/03 18:27:11  benno
 * rcsify date
 *
 * Revision 0.1  2002/04/03 18:26:52  benno
 * initial revision
 *
 */

#include "survivor.H"

Queue::Queue()
{
  // Allocate and initialize a new Queue object.

  // Returns: A new Queue object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Queue::Queue()");
#endif
  
  head = NULL;
  tail = NULL;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Queue::Queue()");
#endif
}

bool Queue::append(QueueUnit *q)
{
  // Append the QueueUnit <q> to the end of the Queue.  A write lock
  // must be obtained before calling this method.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Queue::append(%d)", q);
#endif
  
  if(q)
  {
    if(tail)
    {
      // Mark the old tail as q's previous, and make q the new tail
      
      q->set_prev(tail);
      q->set_next(NULL);
      tail->set_next(q);

      tail = q;
      ret = true;
    }
    else
    {
      // This will be the only item in the Queue

      q->set_next(NULL);
      q->set_prev(NULL);
      
      head = q;
      tail = q;
      ret = true;
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Queue::append = %s", IOTF(ret));
#endif
  
  return(ret);
}

bool Queue::find(char *check)
{
  // Determine if the Check named <check> is already in the Queue and
  // is not marked completed or timed out.  A read lock must be
  // obtained before this method is invoked.

  // Returns: true if <check> was found, false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Queue::find(%s)", IONULL(check));
#endif

  if(check)
  {
    for(QueueUnit *q = head;q != NULL;q = (QueueUnit *)q->get_next())
    {
      // A Check could be marked completed just after we look at it,
      // but for now this isn't worth worrying about
      
      if(q->check() && strcmp(q->check(), check) == 0
	 && q->status() != completed_unit && q->status() != timedout_unit)
      {
	ret = true;
	break;
      }
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Queue::find = %s", IOTF(ret));
#endif
  
  return(ret);
}

QueueUnit *Queue::next()
{
  // Obtain the next item in the Queue.  A write lock must be obtained before
  // this method is invoked.

  // Returns: A QueueUnit, which should be deleted when no longer needed,
  // or NULL when there are no more items in the Queue.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Queue::next()");
#endif

  // Perform garbage collection before retrieving the next item
  gc();

  QueueUnit *q = head;
  
  while(q)
  {
    // Take the first unassigned item
    
    if(q->status() == unassigned_unit)
    {
      q->mark(assigned_unit);
      break;
    }
    else
      q = (QueueUnit *)q->get_next();
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Queue::next = %d", q);
#endif
  
  return(q);
}

Queue::~Queue()
{
  // Deallocate the Queue object.  Any QueueUnits remaining in the Queue
  // will automatically be deleted.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Queue::~Queue()");
#endif

  while(head)
  {
    // Iterate down the Queue until there are no units left

    QueueUnit *q = head;

    head = (QueueUnit *)q->get_next();
    delete q;
  }

  tail = NULL;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Queue::~Queue()");
#endif
}

void Queue::gc()
{
  // Perform garbage collection.  A write lock must be obtained before this
  // method is called.

  // Return: Nothing

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Queue::gc()");
#endif

  QueueUnit *q = head;

  while(q)
  {
    if(q->status() == completed_unit)
    {
      // This QueueUnit is no longer needed.  Remove it and relink the Queue.

      QueueUnit *prev = (QueueUnit *)q->get_prev();
      QueueUnit *next = (QueueUnit *)q->get_next();

      if(prev)
	prev->set_next(next);
      else
	head = next;  // We have a new head (which could be NULL)

      if(next)
	next->set_prev(prev);
      else
	tail = prev;  // We have a new tail (which could be NULL)

      // Toss this QueueUnit
      
      q->set_prev(NULL);
      q->set_next(NULL);
      delete q;

      // Set the next unit to be looked at

      q = next;
    }
    else if(q->status() == assigned_unit && q->timedout())
    {
      // We just mark the QueueUnit as timed out, but we don't remove it
      // since the worker thread might still have a pointer reference
      // and we don't want to cause a seg fault.

      wlog->warn("Queue::gc found timed out queueunit for check '%s'",
		 IONULL(q->check()));
      
      q->mark(timedout_unit);
      
      q = (QueueUnit *)q->get_next();
    }
    else
      q = (QueueUnit *)q->get_next();
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Queue::gc()");
#endif
}
