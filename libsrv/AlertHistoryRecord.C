/*
 * AlertHistoryRecord.C: Object to manage alert history records
 *
 * Version: $Revision: 0.2 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2003/11/29 05:16:23 $
 * MT-Level: Unsafe
 *
 * Copyright (c) 2003
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: AlertHistoryRecord.C,v $
 * Revision 0.2  2003/11/29 05:16:23  benno
 * "via" may be missing in old records
 *
 * Revision 0.1  2003/11/08 15:36:16  benno
 * Initial revision
 *
 */

#include "survivor.H"

AlertHistoryRecord::AlertHistoryRecord()
{
  // Allocate and initialize a new AlertHistoryRecord object.

  // Returns: A new AlertHistoryRecord object.

  init(-1, -1, NULL, NULL);
}

AlertHistoryRecord::AlertHistoryRecord(int alertrv, int checkrv, char *who,
				       char *via)
{
  // Allocate and initialize a new AlertHistoryRecord object, holding
  // <alertrv>, <checkrv>, <who>, <via>.

  // Returns: A new AlertHistoryRecord object.

  init(alertrv, checkrv, who, via);
}

int AlertHistoryRecord::alertrv()
{
  // Obtain the alert return value stored within.

  // Returns: The alert return value, or -1.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertHistoryRecord::alertrv()");
  dlog->log_exit(DEBUG_MINTRC, "AlertHistoryRecord::alertrv = %d", arv);
#endif
  
  return(arv);
}

int AlertHistoryRecord::checkrv()
{
  // Obtain the check return value stored within.

  // Returns: The check return value, or -1.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertHistoryRecord::checkrv()");
  dlog->log_exit(DEBUG_MINTRC, "AlertHistoryRecord::checkrv = %d", crv);
#endif
  
  return(crv);
}

char *AlertHistoryRecord::who()
{
  // Obtain the notification address stored within.

  // Returns: A pointer to a string containing the address, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertHistoryRecord::who()");
  dlog->log_exit(DEBUG_MINTRC, "AlertHistoryRecord::who = %s", IONULL(w));
#endif
  
  return(w);
}

char *AlertHistoryRecord::via()
{
  // Obtain the alert module name stored within.

  // Returns: A pointer to a string containing the module name, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertHistoryRecord::via()");
  dlog->log_exit(DEBUG_MINTRC, "AlertHistoryRecord::via = %s", IONULL(v));
#endif
  
  return(v);
}

AlertHistoryRecord::~AlertHistoryRecord()
{
  // Deallocate the AlertHistoryRecord.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertHistoryRecord::~AlertHistoryRecord()");
#endif
  
  arv = -1;
  crv = -1;
  xdelete(m);
  xdelete(w);
  xdelete(v);
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertHistoryRecord::~AlertHistoryRecord()");
#endif
}

char *AlertHistoryRecord::make_string()
{
  // Generate a string containing the data portion of the record entry.

  // Returns: A pointer to a string that should not be modified and
  // contains the data portion of the record, or NULL on error.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertHistoryRecord::make_string()");
#endif

  // Toss any old data
  xdelete(m);

  CharBuffer *cbuf = new CharBuffer();

  if(cbuf)
  {
    cbuf->append(arv);
    cbuf->append(':');
    cbuf->append(crv);
    cbuf->append(':');
    cbuf->append(w);
    cbuf->append(" via ");
    cbuf->append(v);

    m = xstrdup(cbuf->str());
    xdelete(cbuf);
  }
  else
    wlog->warn("AlertHistoryRecord::make_string failed to allocate cbuf");
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertHistoryRecord::make_string = %s",
		 IONULL(m));
#endif

  return(m);
}

bool AlertHistoryRecord::parse_string(char *s)
{
  // Parse the contents of <s>.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertHistoryRecord::parse_string(%s)",
		  IONULL(s));
#endif

  if(s)
  {
    // Toss any previous data

    xdelete(w);
    xdelete(v);
    
    // Make a copy of s and run it through strtok

    char *sx = xstrdup(s);

    if(sx)
    {
      toss_eol(sx);
      
      char *last;
      char *r = strtok_r(sx, ":", &last);

      if(r)
      {
	arv = atoi(r);

	r = strtok_r(NULL, ":", &last);

	if(r)
	{
	  crv = atoi(r);

	  r = strtok_r(NULL, " ", &last);

	  if(r)
	  {
	    w = xstrdup(r);

	    r = strtok_r(NULL, " ", &last);

	    if(r)
	    {
	      // This one moves us past "via"

	      r = strtok_r(NULL, " ", &last);

	      if(r)
	      {
		v = xstrdup(r);
		ret = true;
	      }
	      else
		wlog->warn("AlertHistoryRecord::parse_string did not find module name");
	    }
	    else
	    {
	      // We didn't find "via".  Most likely, this is because
	      // the history record was written prior to v0.9 (or maybe
	      // it was 0.9.2), so we silently set via to "unknown".
	      // This will have a sort of benefit in converting old
	      // history records to the newer format.

	      v = xstrdup("unknown");
	      ret = true;
	    }
	  }
	  else
	    wlog->warn("AlertHistoryRecord::parse_string did not find recipient addresses");
	}
	else
	  wlog->warn("AlertHistoryRecord::parse_string did not find check return value");
      }
      else
	wlog->warn("AlertHistoryRecord::parse_string did not find alert return value");
      
      xdelete(sx);
    }
    else
      wlog->warn("AlertHistoryRecord::parse_string failed to copy s");
  }

#if defined(debug)
  dlog->log_exit(DEBUG_MINTRC, "AlertHistoryRecord::parse_string = %s",
		 IOTF(ret));
#endif
  
  return(ret);
}

void AlertHistoryRecord::init(int alertrv, int checkrv, char *who, char *via)
{
  // Initializer for AlertHistoryRecord object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "AlertHistoryRecord::init(%d,%d,%s,%s)",
		  alertrv, checkrv, IONULL(who), IONULL(via));
#endif

  arv = alertrv;
  crv = checkrv;
  m = NULL;
  w = xstrdup(who);
  v = xstrdup(via);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "AlertHistoryRecord::init()");
#endif
}
