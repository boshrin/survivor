/*
 * Alert.C: survivor Alert information object
 *
 * Version: $Revision: 0.9 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2003/05/29 00:35:59 $
 * MT-Level: Safe
 *
 * Copyright (c) 2002 - 2003
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: Alert.C,v $
 * Revision 0.9  2003/05/29 00:35:59  benno
 * Changes for survivor.dtd, format and transmit modules
 *
 * Revision 0.8  2003/04/09 20:23:05  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.7  2003/04/03 03:16:27  benno
 * Use Debugger
 *
 * Revision 0.6  2003/01/23 19:56:24  benno
 * Add IONULL and IOTF
 *
 * Revision 0.5  2002/12/31 04:26:29  benno
 * Add fix methods
 * Use xdelete
 *
 * Revision 0.4  2002/12/16 00:32:29  benno
 * Use RecipientSet object
 *
 * Revision 0.3  2002/04/04 20:05:55  benno
 * copyright
 *
 * Revision 0.2  2002/04/03 16:50:28  benno
 * rcsify date
 *
 * Revision 0.1  2002/04/03 16:50:03  benno
 * initial revision
 *
 */

#include "survivor.H"

Alert::Alert(RecipientSet *rset, time_t checktime, char *host, char *helpfile,
	     char *instance, int instances, int retval, char *service,
	     char *summary, char *token, bool fix)
{
  // Allocate and initialize a new Alert object, where <rset> contains
  // the addresses to notify, the modules to notify them with, and the
  // calllists from whence they came.  <checktime> is the time the
  // problem was noticed on <host>, <helpfile> is the path to a file
  // containing more information about this alert, <instance> is the
  // scheduler instance the Alert is associated with, <instances> is
  // the number of consecutive failures, <retval> is the return code
  // from the Check, <service> is the service (check module) that
  // failed, <summary> is a brief description of what happened, and
  // <token> is the Alert token.  If <fix> is true, the fix associated
  // with the service should be attempted.  <rset> will be maintained
  // by this Alert object.

  // Returns: A new, initialized Alert.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC,
		  "Alert::Alert(%d,%d,%s,%s,%s,%d,%d,%s,%s,%s,%s)",
		  rset, checktime, IONULL(host), IONULL(helpfile),
		  IONULL(instance), instances, retval, IONULL(service),
		  IONULL(summary), IONULL(token), IOTF(fix));
#endif

  rs = rset;
  frv = MODEXEC_MISCONFIG;
  inst = instances;
  rv = retval;
  f = fix;
  fs = NULL;
  h = xstrdup(host);
  hf = xstrdup(helpfile);
  in = xstrdup(instance);
  s = xstrdup(summary);
  svc = xstrdup(service);
  tok = xstrdup(token);
  ct = checktime;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Alert::Alert()");
#endif
}

time_t Alert::checktime()
{
  // Obtain the time the problem was noticed.

  // Returns: The time.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Alert::checktime()");
  dlog->log_exit(DEBUG_MINTRC, "Alert::checktime = %d", ct);
#endif

  return(ct);
}

bool Alert::fix()
{
  // Determine if a Fix should be attempted.

  // Returns: true if the fix should be attempted, false otherwise.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Alert::fix()");
  dlog->log_exit(DEBUG_MINTRC, "Alert::fix = %s", IOTF(f));
#endif

  return(f);
}

int Alert::fix_retval()
{
  // If a fix was run, obtain its return value.

  // Returns: The return value of the fix.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Alert::fix_retval()");
  dlog->log_exit(DEBUG_MINTRC, "Alert::fix_retval = %d", frv);
#endif

  return(frv);
}

char *Alert::fix_summary()
{
  // If a fix was run, obtain its summary.

  // Returns: The summary of the fix, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Alert::fix_summary()");
  dlog->log_exit(DEBUG_MINTRC, "Alert::fix_summary = %s", IONULL(fs));
#endif

  return(fs);
}

char *Alert::helpfile()
{
  // Obtain the path of the helpfile associated with this Alert.

  // Returns: The helpfile path, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Alert::helpfile()");
  dlog->log_exit(DEBUG_MINTRC, "Alert::helpfile = %s", IONULL(hf));
#endif

  return(hf);
}

char *Alert::host()
{
  // Obtain the host that generated the Alert.

  // Returns: The host, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Alert::host()");
  dlog->log_exit(DEBUG_MINTRC, "Alert::host = %s", IONULL(h));
#endif

  return(h);
}

char *Alert::instance()
{
  // Obtain the instance that generated the Alert.

  // Returns: The host, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Alert::instance()");
  dlog->log_exit(DEBUG_MINTRC, "Alert::instance = %s", IONULL(in));
#endif

  return(in);
}

int Alert::instances()
{
  // Obtain the number of consecutive instances of retval().

  // Returns: The number of instances, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Alert::instances()");
  dlog->log_exit(DEBUG_MINTRC, "Alert::instances = %d", inst);
#endif

  return(inst);
}

RecipientSet *Alert::recipients()
{
  // Obtain the RecipientSet to be used to transmit this Alert.
  
  // Returns: A pointer to a RecipientSet object, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Alert::recipients()");
  dlog->log_exit(DEBUG_MINTRC, "Alert::recipients = %d", rs);
#endif

  return(rs);
}

int Alert::retval()
{
  // Obtain the return value that generated this Alert.

  // Returns: The return value.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Alert::retval()");
  dlog->log_exit(DEBUG_MINTRC, "Alert::retval = %d", rv);
#endif

  return(rv);
}

char *Alert::service()
{
  // Obtain the service (check module) of this Alert.

  // Returns: The service name, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Alert::service()");
  dlog->log_exit(DEBUG_MINTRC, "Alert::service = %s", IONULL(svc));
#endif
  
  return(svc);
}

bool Alert::set_fix_status(int fixrv, char *fixsummary)
{
  // Set the fix status for this Alert to return value <fixrv> and
  // summary <fixsummary>.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Alert::set_fix_status(%d,%s)",
		   fixrv, IONULL(fixsummary));
#endif

  if(fixsummary)
  {
    xdelete(fs);
    
    frv = fixrv;
    fs = xstrdup(fixsummary);

    ret = true;
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Alert::set_fix_status = %s", IOTF(ret));
#endif

  return(ret);
}

char *Alert::summary()
{
  // Obtain the summary description of this Alert.

  // Returns: The summary, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Alert::summary()");
  dlog->log_exit(DEBUG_MINTRC, "Alert::summary = %s", IONULL(s));
#endif

  return(s);
}

char *Alert::token()
{
  // Obtain the token for this Alert.

  // Returns: The token, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Alert::token()");
  dlog->log_exit(DEBUG_MINTRC, "Alert::token = %s", IONULL(tok));
#endif

  return(tok);
}

Alert::~Alert()
{
  // Deallocate the Alert object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Alert::~Alert()");
#endif

  xdelete(rs);
  frv = 0;
  inst = 0;
  rv = 0;
  f = false;
  xdelete(fs);
  xdelete(h);
  xdelete(hf);
  xdelete(in);
  xdelete(s);
  xdelete(svc);
  xdelete(tok);
  ct = 0;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Alert::~Alert()");
#endif
}
