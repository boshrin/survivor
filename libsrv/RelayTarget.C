/*
 * RelayTarget.C: survivor RelayTarget object
 *
 * Version: $Revision: 0.1 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2004/11/26 22:01:40 $
 * MT-Level: Safe
 *
 * Copyright (c) 2004
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: RelayTarget.C,v $
 * Revision 0.1  2004/11/26 22:01:40  benno
 * Initial revision
 *
 */

#include "survivor.H"

RelayTarget::RelayTarget(char *name, char *instance, char *calllist)
{
  // Allocate and initialize a new RelayTarget object named <name>,
  // used to identify <calllist> within <instance>.

  // Returns: A new RelayTarget object.

  init(name, instance, calllist, NULL, NULL);
}

RelayTarget::RelayTarget(char *name, char *instance, char *person, char *via)
{
  // Allocate and initialize a new RelayTarget object named <name>,
  // used to identify <person> and <via> within <instance>.

  // Returns: A new RelayTarget object.

  init(name, instance, NULL, person, via);
}

RelayTarget::RelayTarget(char *name, RelayTarget *rt)
{
  // Allocate and initialize a new RelayTarget object named <name>,
  // with the same contents as <rt>.

  // Returns: A new RelayTarget object.
  
  if(rt)
    init(name, rt->instance(), rt->calllist(), rt->person(), rt->via());
  else
    init(name, NULL, NULL, NULL, NULL);
}

char *RelayTarget::calllist()
{
  // Obtain the name of the CallList for this RelayTarget.

  // Returns: The CallList name, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "RelayTarget::calllist()");
  dlog->log_exit(DEBUG_MINTRC, "RelayTarget::calllist = %s", IONULL(cl));
#endif
  
  return(cl);
}

char *RelayTarget::instance()
{
  // Obtain the name of the instance for this RelayTarget.

  // Returns: The instance name, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "RelayTarget::instance()");
  dlog->log_exit(DEBUG_MINTRC, "RelayTarget::instance = %s", IONULL(in));
#endif
  
  return(in);
}

char *RelayTarget::name()
{
  // Obtain the name of this RelayTarget.

  // Returns: The name, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "RelayTarget::name()");
  dlog->log_exit(DEBUG_MINTRC, "RelayTarget::name = %s", IONULL(n));
#endif
  
  return(n);
}

char *RelayTarget::person()
{
  // Obtain the name of the Person of this RelayTarget.

  // Returns: The Person name, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "RelayTarget::person()");
  dlog->log_exit(DEBUG_MINTRC, "RelayTarget::person = %s", IONULL(p));
#endif
  
  return(p);
}

char *RelayTarget::via()
{
  // Obtain the relay method of this RelayTarget, if a Person was
  // specified.

  // Returns: The relay method, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "RelayTarget::via()");
  dlog->log_exit(DEBUG_MINTRC, "RelayTarget::via = %s", IONULL(v));
#endif
  
  return(v);
}

RelayTarget::~RelayTarget()
{
  // Delete the RelayTarget object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "RelayTarget::~RelayTarget()");
#endif
  
  xdelete(cl);
  xdelete(in);
  xdelete(n);
  xdelete(p);
  xdelete(v);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "RelayTarget::~RelayTarget()");
#endif
}

void RelayTarget::init(char *name, char *instance, char *calllist,
		       char *person, char *via)
{
  // Initialize RelayTarget object with <name>, <instance>,
  // <calllist>, <person>, and <via>.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "RelayTarget::init(%s,%s,%s,%s,%s)",
		  IONULL(name), IONULL(instance), IONULL(calllist),
		  IONULL(person), IONULL(via));
#endif

  cl = xstrdup(calllist);
  in = xstrdup(instance);
  p = xstrdup(person);
  n = xstrdup(name);
  v = xstrdup(via);
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "RelayTarget::init()");
#endif
}
