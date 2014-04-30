/*
 * QueueUnit.C: survivor object for placing in Queues
 *
 * Version: $Revision: 0.8 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2007/03/29 12:11:37 $
 * MT-Level: Unsafe
 *
 * Copyright (c) 2002 - 2007
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: QueueUnit.C,v $
 * Revision 0.8  2007/03/29 12:11:37  benno
 * Store timeout for checks
 *
 * Revision 0.7  2003/04/09 20:23:49  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.6  2003/04/05 23:40:09  benno
 * Use Debugger
 *
 * Revision 0.5  2003/01/24 18:01:55  benno
 * Use IONULL, IOTF, xdelete, and xadelete
 *
 * Revision 0.4  2002/04/04 20:11:15  benno
 * copyright
 *
 * Revision 0.3  2002/04/03 18:29:01  benno
 * rcsify date
 *
 * Revision 0.2  2002/04/03 18:28:52  benno
 * Don't use objects from cf, which might go away
 *
 * Revision 0.1  2002/04/03 18:28:07  benno
 * initial revision
 *
 */

#include "survivor.H"

QueueUnit::QueueUnit(Alert *alert) : HashUnit()
{
  // Allocate and initialize a new QueueUnit object, containing the
  // Alert object <alert>.  <alert> will be exclusively managed by
  // this QueueUnit and will be deleted when the QueueUnit is deleted.

  // Returns: A new QueueUnit object.

  init(alert, NULL, NULL, NULL, 0);
}

QueueUnit::QueueUnit(char *check, char *group, int timeout) : HashUnit()
{
  // Allocate and initialize a new QueueUnit object, containing a reference
  // to the check named <check> and a reference to the hostgroup <group>.
  // Both are duplicated and need not remain valid.  The QueueUnit will
  // expire after <timeout> seconds.

  // Returns: A new QueueUnit object.
  
  init(NULL, check, group, NULL, timeout);
}

QueueUnit::QueueUnit(char *check, List *hosts, int timeout) : HashUnit()
{
  // Allocate and initialize a new QueueUnit object, containing a
  // reference to the check named <check> and the list of hosts
  // <hosts>, which will be delete'd when the destructor is called.
  // <check> is duplicated and need not remain valid.  The QueueUnit
  // will expire after <timeout> seconds.

  // Returns: A new QueueUnit object.

  init(NULL, check, NULL, hosts, timeout);
}

Alert *QueueUnit::alert()
{
  // Obtain the Alert object that is Queued, if set.

  // Returns: The Alert object, which should not be deleted, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "QueueUnit::alert()");
  dlog->log_exit(DEBUG_MINTRC, "QueueUnit::alert = %d", a);
#endif
  
  return(a);
}

char *QueueUnit::check()
{
  // Obtain the name of the Check object that is Queued, if set.

  // Returns: The name of the Check object, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "QueueUnit::check()");
  dlog->log_exit(DEBUG_MINTRC, "QueueUnit::check = %s", IONULL(c));
#endif
  
  return(c);
}

char *QueueUnit::group()
{
  // Obtain the name of the hostgroup for the Check that is Queued, if set.

  // Returns: The name of the Group, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "QueueUnit::group()");
  dlog->log_exit(DEBUG_MINTRC, "QueueUnit::group = %s", IONULL(g));
#endif
  
  return(g);
}

List *QueueUnit::hosts()
{
  // Obtain the list of hosts that are Queued, if set.

  // Returns: A pointer to a List object, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "QueueUnit::hosts()");
  dlog->log_exit(DEBUG_MINTRC, "QueueUnit::hosts = %d", h);
#endif
  
  return(h);
}

bool QueueUnit::mark(queueunit_t stat)
{
  // Mark this QueueUnit with the status <s> to indicate that the QueueUnit
  // is not currently assigned, is assigned, or is completed.

  // Returns: true if the status is updated, false otherwise.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "QueueUnit::mark(%d)", stat);
#endif
  
  s = stat;

  if(stat == assigned_unit)
  {
    // Track the time the unit was assigned so we can time it out if
    // necessary.

    if(gettimeofday(&ta, NULL) != 0)
      wlog->warn("gettimeofday failed in QueueUnit::mark");  
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "QueueUnit::mark = true");
#endif
  
  return(true);
}

queueunit_t QueueUnit::status()
{
  // Determine the current status of the QueueUnit.

  // Returns: The current status of the QueueUnit.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "QueueUnit::status()");
  dlog->log_exit(DEBUG_MINTRC, "QueueUnit::status = %d", s);
#endif
  
  return(s);
}

bool QueueUnit::timedout()
{
  // Determine if this QueueUnit has timed out.

  // Returns: true if the timeout has been reached, false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "QueueUnit::timedout()");
#endif

  struct timeval now;
  
  switch(s)
  {
  case assigned_unit:
    if(gettimeofday(&now, NULL) == 0)
    {
      if(ta.tv_sec + t < now.tv_sec)
	ret = true;
    }
    else
      wlog->warn("gettimeofday failed in QueueUnit::timedout");
    break;
  case timedout_unit:
    ret = true;
    break;
  default:
    break;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "QueueUnit::timedout = %s", IOTF(ret));
#endif
  
  return(ret);
}

int QueueUnit::timeout()
{
  // Determine the timeout of the QueueUnit.

  // Returns: The timeout in seconds, or 0.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "QueueUnit::timeout()");
  dlog->log_exit(DEBUG_MINTRC, "QueueUnit::timeout = %d", t);
#endif
  
  return(t);
}

QueueUnit::~QueueUnit()
{
  // Deallocate the QueueUnit object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "QueueUnit::~QueueUnit()");
#endif

  xdelete(a);
  xdelete(c);
  xdelete(g);
  xdelete(h);
  t = 0;

  s = unassigned_unit;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "QueueUnit::~QueueUnit()");
#endif
}

void QueueUnit::init(Alert *alert, char *check, char *group, List *hosts,
		     int timeout)
{
  // Initializer for QueueUnit constructor.

  // Returns: Nothing.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "QueueUnit::init(%d,%s,%s,%d,%d)",
		   alert, IONULL(check), IONULL(group), hosts, timeout);
#endif

  a = alert;
  c = xstrdup(check);
  g = xstrdup(group);
  h = hosts;
  s = unassigned_unit;
  t = timeout;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "QueueUnit::init()");
#endif
}
