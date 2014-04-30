/*
 * SyslogDebugger.C: Syslog Debugging Object
 *
 * Version: $Revision: 0.2 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2003/04/09 20:03:04 $
 * MT-Level: Safe
 *
 * Copyright (c) 2002 - 2003
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: SyslogDebugger.C,v $
 * Revision 0.2  2003/04/09 20:03:04  benno
 * Allow facility and priority to be specified
 *
 * Revision 0.1  2003/04/07 21:01:13  benno
 * Initial revision
 *
 */

#include "survivor.H"

SyslogDebugger::SyslogDebugger()
{
  // Allocate and initialize a new, standard SyslogDebugger object.

  // Returns: A new SyslogDebugger object.

  f = LOG_USER;
  p = -1;
}

SyslogDebugger::SyslogDebugger(int facility)
{
  // Allocate and initialize a new SyslogDebugger object that logs
  // using <facility>.

  // Returns: A new SyslogDebugger object.

  f = facility;
  p = -1;
}

bool SyslogDebugger::set_priority(int priority)
{
  // Set the log priority to <priority>.  Use -1 to restore default
  // behavior.

  // Returns: true if fully successful, false otherwise.

  p = priority;
  return(true);
}

bool SyslogDebugger::do_log(bool warn, char *s)
{
  // Output <s>.  If <warn> is true, treat <s> as a warning, otherwise
  // as a log message.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;

  if(s)
  {
    openlog(progname, LOG_PID, f);

    if(p == -1)
    {
      if(warn)
	syslog(LOG_WARNING, "%s", s);
      else
	syslog(LOG_DEBUG, "%s", s);
    }
    else
      syslog(p, "%s", s);

    closelog();
    
    ret = true;
  }
  
  return(ret);
}
