/*
 * TwoWayReply.C: survivor object for holding information from two way message
 *
 * Version: $Revision: 0.11 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2006/02/14 03:20:40 $
 * MT-Level: Safe, if parse is only called once.
 *
 * Copyright (c) 2002 - 2006
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: TwoWayReply.C,v $
 * Revision 0.11  2006/02/14 03:20:40  benno
 * Fix backwards service/host parsing
 *
 * Revision 0.10  2004/11/27 00:48:33  benno
 * Use RFC822Message, better parsing, DEBUG_PARSER
 *
 * Revision 0.9  2003/04/09 20:15:39  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.8  2003/03/04 20:59:36  benno
 * Move most functionality to Reply object
 *
 * Revision 0.7  2003/03/03 03:49:10  benno
 * Use toss_eol
 *
 * Revision 0.6  2003/01/29 00:50:31  benno
 * Use IONULL, IOTF, xdelete, xadelete
 *
 * Revision 0.5  2002/04/12 15:02:53  benno
 * use BUFSIZE
 *
 * Revision 0.4  2002/04/04 19:57:34  benno
 * copyright
 *
 * Revision 0.3  2002/04/03 19:23:09  benno
 * add MT-Level
 *
 * Revision 0.2  2002/04/02 21:17:02  benno
 * rcsify date
 *
 * Revision 0.1  2002/04/02 21:16:27  benno
 * initial revision
 *
 */

#include "gateway.H"

bool TwoWayReply::parse(RFC822Message *msg)
{
  // Parse the message in <msg>.  Parsing continues until all data has
  // been read.  If an item is specified multiple times, only the
  // first will be used.  If parsing is not successful, the individual
  // Reply methods may be used to see what information was
  // successfully read.

  // Returns: true if all items were successfully parsed from the message,
  // false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "TwoWayReply::parse(%d)", msg);
  dlog->log_progress(DEBUG_PARSER, "Trying to parse message as Nextel reply");
#endif

  if(msg)
  {
    // First, get stuff from the headers

    char *mf = msg->header_entry("From", 0);

    if(mf)
    {
#if defined(DEBUG)
      dlog->log_progress(DEBUG_PARSER, "Message from '%s'", mf);
#endif
      
      f = xstrdup(mf);
    }

    char *ms = msg->header_entry("Subject", 0);

    if(ms)
    {
      // Pull out hostname and service

      char *p = strstr(ms, "ALERT: ");

      if(p)
      {
	p += 7;

	List *ts = tokenize(p, "@");

	if(ts)
	{
	  // This was backwards, bug #635
	  // Patched by Olivier Calle
	  svc = xstrdup(ts->retrieve(0));
	  h = xstrdup(ts->retrieve(1));
	  
#if defined(DEBUG)
	  dlog->log_progress(DEBUG_PARSER, "Host '%s', service '%s'", h, svc);
#endif
      
	  xdelete(ts);
	}
      }
    }

    // Now get the rest of the info from the body

    CharBuffer *body = msg->body();

    if(body)
    {
      for(char *s = body->read_line();s != NULL;s = body->read_line())
      {
	if(!t && strncmp(s, "Token=", 6)==0)
	{
	  // Pull out token

	  t = xstrdup(s+6);

#if defined(DEBUG)
	  dlog->log_progress(DEBUG_PARSER, "Token '%s'", t);
#endif
	}
	else if(!inst && strncmp(s, "Instance=", 9)==0)
	{
	  // Pull out instance
	  
	  inst = xstrdup(s+9);

#if defined(DEBUG)
	  dlog->log_progress(DEBUG_PARSER, "Instance '%s'", inst);
#endif
	}
	else if(ry == unknown_reply && strcmp(s, "Acknowledge")==0)
	{
	  ry = acknowledge_reply;

#if defined(DEBUG)
	  dlog->log_progress(DEBUG_PARSER, "Reply command '%s'", s);
#endif
	}
	else if(ry == unknown_reply && strcmp(s, "Escalate")==0)
	{
	  ry = escalate_reply;

#if defined(DEBUG)
	  dlog->log_progress(DEBUG_PARSER, "Reply command '%s'", s);
#endif
	}
	else if(ry == unknown_reply && strcmp(s, "Inhibit")==0)
	{
	  ry = inhibit_reply;

#if defined(DEBUG)
	  dlog->log_progress(DEBUG_PARSER, "Reply command '%s'", s);
#endif
	}
      }
    }

    if(f && h && inst && svc && t && (ry != unknown_reply))
      ret = true;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "TwoWayReply::parse = %s", IOTF(ret));
#endif

  return(ret);
}
