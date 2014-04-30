/*
 * StdDebugger.C: StdIO Debugging Object
 *
 * Version: $Revision: 0.2 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2003/04/09 20:02:44 $
 * MT-Level: Safe
 *
 * Copyright (c) 2002 - 2003
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: StdDebugger.C,v $
 * Revision 0.2  2003/04/09 20:02:44  benno
 * Offer stdout option
 *
 * Revision 0.1  2003/04/07 21:00:51  benno
 * Initial revision
 *
 */

#include "survivor.H"

bool StdDebugger::enable_stdout()
{
  // Use stdout instead of stderr for debugging output.

  // Returns: true if enabled, false otherwise, including if already
  // enabled.

  if(!so)
  {
    so = true;
    return(true);
  }

  return(false);
}

bool StdDebugger::do_log(bool warn, char *s)
{
  // Output <s>.  If <warn> is true, treat <s> as a warning, otherwise
  // as a log message.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;

  if(s)
  {
    if(warn)
    {
      if(fprintf((so ? stdout : stderr), "%s: WARNING: %s\n", progname, s))
	ret = true;
    }
    else
    {
      if(fprintf((so ? stdout : stderr), "%s\n", s))
	ret = true;
    }
  }
  
  return(ret);
}
