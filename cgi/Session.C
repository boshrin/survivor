/*
 * survivor web interface session object
 *
 * Version: $Revision: 0.8 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2005/12/22 03:57:48 $
 * MT-Level: Safe
 *
 * Copyright (c) 2002 - 2005
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: Session.C,v $
 * Revision 0.8  2005/12/22 03:57:48  benno
 * Change Session constructor argument order for AIX
 *
 * Revision 0.7  2004/03/02 17:33:47  benno
 * Add group support
 *
 * Revision 0.6  2003/04/09 20:14:28  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.5  2003/01/27 02:54:21  benno
 * Use IONULL, IOTF, xdelete, xadelete
 *
 * Revision 0.4  2002/06/02 23:31:56  toor
 * add additional Session constructor and init()
 *
 * Revision 0.3  2002/04/04 19:54:29  benno
 * copyright
 *
 * Revision 0.2  2002/04/02 20:13:19  benno
 * rcsify date
 *
 * Revision 0.1  2002/04/02 20:12:47  benno
 * initial revision
 *
 */

#include "cgi.H"

Session::Session(char *sid, int validity, char *user, List *groups,
		 char *hostid)
{
  // Allocate and initialize a new Session object, identified <sid>,
  // valid for <validity> minutes, for <user>, and a member of
  // <groups>.  If <hostid> is provided, then the same value must be
  // provided to valid().  <groups> will be copied and need not remain
  // valid.

  // Returns: A new Session object.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Session::Session(%s,%d,%s,%s,%d)",
		  IONULL(sid), validity, IONULL(user), groups, IONULL(hostid));
#endif

  time_t expiry = 0;
  
  if(validity > 0)
  {
    struct timeval tv;

    if(gettimeofday(&tv, NULL)==0)
      expiry = tv.tv_sec + (long)(60 * validity);
  }

  init(sid, user, groups, hostid, expiry);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Session::Session()");
#endif
}

Session::Session(char *sid, char *user, List *groups, char *hostid,
		 time_t expiry)
{
  // Allocate and initialize a new Session object, identified <sid>,
  // for <user>, a member of <groups>, and valid until <expiry>.  If
  // <hostid> is provided, then the same value must be provided to
  // valid().  <groups> will be copied and need not remain valid.

  // Returns: A new Session object.

  init(sid, user, groups, hostid, expiry);
}

time_t Session::expiry()
{
  // Obtain the expiration time for this Session.

  // Returns: The expiration time, or 0.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Session::expiry()");
  dlog->log_exit(DEBUG_MINTRC, "Session::expiry = %d", exp);
#endif
  
  return(exp);
}

List *Session::groups()
{
  // Obtain the groups for this Session.

  // Returns: The groups, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Session::groups()");
  dlog->log_exit(DEBUG_MINTRC, "Session::groups = %d", g);
#endif
  
  return(g);
}

char *Session::hostid()
{
  // Obtain the host ID associated with this Session.

  // Returns: A pointer to a string that should not be modified and
  // contains the host ID, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Session::hostid()");
  dlog->log_exit(DEBUG_MINTRC, "Session::hostid = %s", IONULL(h));
#endif
  
  return(h);
}

char *Session::sid()
{
  // Obtain the Session ID associated with this Session.

  // Returns: A pointer to a string that should not be modified and
  // contains the Session ID, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Session::sid()");
  dlog->log_exit(DEBUG_MINTRC, "Session::sid = %s", IONULL(s));
#endif
  
  return(s);
}

char *Session::user()
{
  // Obtain the username associated with this Session.

  // Returns: A pointer to a string that should not be modified and
  // contains the username, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Session::user()");
  dlog->log_exit(DEBUG_MINTRC, "Session::user = %s", IONULL(u));
#endif
  
  return(u);
}

bool Session::valid(char *hostid)
{
  // Determine if this Session is still valid (has not expired).
  // If a hostid was provided to the constructor, then the same
  // <hostid> must be provided here (otherwise <hostid> is ignored).

  // Returns: true if the Session is still valid, false otherwise.

  bool r = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Session::valid(%s)", IONULL(hostid));
#endif
  
  if(exp > 0)
  {
    struct timeval tv;
    
    if(gettimeofday(&tv, NULL)==0 && (tv.tv_sec < exp)
       && (!h || (hostid && strcmp(hostid, h)==0)))
	r = true;
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Session::valid = %s", IOTF(r));
#endif
  
  return(r);
}

Session::~Session()
{
  // Deallocate the Session object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Session::~Session()");
#endif

  xdelete(g);
  xdelete(h);
  xdelete(s);
  xdelete(u);

  exp = 0;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Session::~Session()");
#endif
}

void Session::init(char *sid, char *user, List *groups, char *hostid,
		   time_t expiry)
{
  // Initialize a new Session object, identified <sid>, for <user>, a
  // member of <groups>, and valid for <validity> minutes.  If
  // <hostid> is provided, then the same value must be provided to
  // valid().  <groups> will be copied and need not remain valid.

  // Returns: Nothing.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Session::init(%s,%s,%d,%s,%d)",
		  IONULL(sid), IONULL(user), groups, IONULL(hostid), expiry);
#endif

  if(groups)
    g = new List(groups);
  else
    g = NULL;
  h = xstrdup(hostid);
  s = xstrdup(sid);
  u = xstrdup(user);
  exp = expiry;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Session::init()");
#endif
}
