/*
 * CGIAuthResult.C: survivor CGIAuthResult object
 *
 * Version: $Revision: 0.1 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2004/03/01 23:13:46 $
 * MT-Level: Safe
 *
 * Copyright (c) 2004
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: CGIAuthResult.C,v $
 * Revision 0.1  2004/03/01 23:13:46  benno
 * Initial revision
 *
 */

#include "survivor.H"

CGIAuthResult::CGIAuthResult(authn_result_t authok, char *username,
			     List *groups, char *deferral, char *error,
			     List *skipflags)
{
  // Allocate and initialize a new CGIAuthResult object, holding
  // <authok>, for <username> and the user's <groups>. <deferral> is a
  // document to be delivered on deferral, <error> indicates any
  // encountered error, and <skipflags> indicates which flags should
  // not be included when new URIs are generated.  <groups> and
  // <skipflags> will be maintained by the CGIAuthResult object and
  // deleted when no longer required.

  // Returns: A new CGIAuthResult object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC,
		  "CGIAuthResult::CGIAuthResult(%d,%s,%d,%s,%s,%d)",
		  authok, IONULL(username), groups, IONULL(deferral),
		  IONULL(error), skipflags);
#endif

  a = authok;
  g = groups;
  sf = skipflags;
  d = xstrdup(deferral);
  e = xstrdup(error);
  u = xstrdup(username);
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CGIAuthResult::CGIAuthResult()");
#endif
}

authn_result_t CGIAuthResult::authok()
{
  // Obtain the authentication result for this CGIAuthResult.

  // Returns: The authentication result.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CGIAuthResult::authok()");
  dlog->log_exit(DEBUG_MINTRC, "CGIAuthResult::authok = %d", a);
#endif
  
  return(a);
}

char *CGIAuthResult::deferral()
{
  // Obtain the deferral document for this CGIAuthResult, if set.

  // Returns: The deferral document, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CGIAuthResult::deferral()");
  dlog->log_exit(DEBUG_MINTRC, "CGIAuthResult::deferral = %s", IONULL(d));
#endif
  
  return(d);
}

char *CGIAuthResult::error()
{
  // Obtain the error for this CGIAuthResult, if set.

  // Returns: The error, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CGIAuthResult::error()");
  dlog->log_exit(DEBUG_MINTRC, "CGIAuthResult::error = %s", IONULL(e));
#endif
  
  return(e);
}

List *CGIAuthResult::groups()
{
  // Obtain the groups for this CGIAuthResult, if set.

  // Returns: The list of groups, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CGIAuthResult::groups()");
  dlog->log_exit(DEBUG_MINTRC, "CGIAuthResult::groups = %d", g);
#endif
  
  return(g);
}

List *CGIAuthResult::skipflags()
{
  // Obtain the set of flags to skip in uris for this CGIAuthResult,
  // if set.

  // Returns: The list of groups, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CGIAuthResult::skipflags()");
  dlog->log_exit(DEBUG_MINTRC, "CGIAuthResult::skipflags = %d", sf);
#endif
  
  return(sf);
}

char *CGIAuthResult::username()
{
  // Obtain the username for this CGIAuthResult, if set.

  // Returns: The username, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CGIAuthResult::username()");
  dlog->log_exit(DEBUG_MINTRC, "CGIAuthResult::username = %s", IONULL(u));
#endif
  
  return(u);
}

CGIAuthResult::~CGIAuthResult()
{
  // Delete the CGIAuthResult object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CGIAuthResult::~CGIAuthResult()");
#endif

  a = no_authn;
  xdelete(d);
  xdelete(e);
  xdelete(g);
  xdelete(sf);
  xdelete(u);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CGIAuthResult::~CGIAuthResult()");
#endif
}
