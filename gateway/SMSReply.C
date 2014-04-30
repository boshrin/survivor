/*
 * SMSReply.C: survivor object for holding information from SMS reply
 *
 * Version: $Revision: 0.4 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2004/11/27 00:47:29 $
 * MT-Level: Safe, if parse is only called once.
 *
 * Copyright (c) 2002 - 2004
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: SMSReply.C,v $
 * Revision 0.4  2004/11/27 00:47:29  benno
 * Use RFC822Message, add more robust parsing, DEBUG_PARSER
 *
 * Revision 0.3  2003/04/09 20:15:39  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.2  2003/03/05 03:27:24  benno
 * Change commands to be one character long to conserve message space
 *
 * Revision 0.1  2003/03/04 21:00:08  benno
 * Initial revision
 *
 */

#include "gateway.H"

bool SMSReply::parse(RFC822Message *msg)
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
  dlog->log_entry(DEBUG_MAJTRC, "SMSReply::parse(%d)", msg);
  dlog->log_progress(DEBUG_PARSER, "Trying to parse message as SMS reply");
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

    char *mt = msg->header_entry("To", 0);

    if(mt)
    {
      // Extract the command from the to address

      char *p = strchr(mt, '+');

      if(p)
      {
	p++;
	
#if defined(DEBUG)
	dlog->log_progress(DEBUG_PARSER, "Reply command '%c'", *p);
#endif
	
	switch(*p)
	{
	case 'a':
	  ry = acknowledge_reply;
	  break;
	case 'e':
	  ry = escalate_reply;
	  break;
	case 'i':
	  ry = inhibit_reply;
	  break;
	}
      }
    }

    char *ms = xstrdup(msg->header_entry("Subject", 0));

    if(ms)
    {
      // The usual reply will have ALERT: service@host in the subject,
      // so try that first

      char *p = strstr(ms, "ALERT: ");

      if(p)
      {
	p += 7;

	char *q = strchr(p, '@');
	
	if(q)
	{
	  // We've got the service
	  
	  *q = '\0';
	  
	  svc = xstrdup(p);

#if defined(DEBUG)
	  dlog->log_progress(DEBUG_PARSER, "Service '%s'", svc);
#endif
	    
	  // Now move on to host
	    
	  p = q + 1;

	  q = strchr(p, '/');  // There might be multiple "lines" in subject

	  if(q)
	    *q = '\0';
	  
	  h = xstrdup(p);
	    
#if defined(DEBUG)
	    dlog->log_progress(DEBUG_PARSER, "Host '%s'", h);
#endif
	}
      }

      xdelete(ms);
    }
    
    // Now get the rest of the info from the body

    CharBuffer *body = msg->body();

    if(body)
    {
      // Everything should be in the first line
      
      char *l = xstrdup(body->read_line());

      if(l)
      {
	char *p = l;
	char *q = NULL;
	
	if(!h)
	{
	  // Try to pull out hostname and service from message text
	
	  p = strstr(l, "ALERT: ");

	  if(p)
	  {
	    p += 7;
	    
	    q = strchr(p, '@');
	    
	    if(q)
	    {
	      // We've got the service
	      
	      *q = '\0';
	      
	      svc = xstrdup(p);
	      
#if defined(DEBUG)
	      dlog->log_progress(DEBUG_PARSER, "Service '%s'", svc);
#endif
	      
	      // Now move on to host

	      cout << "[" << p << "]" << endl;
	      
	      p = q + 1;
	      q = strchr(p, '/');
	      
	      if(q)
	      {
		*q = '\0';
		
		h = xstrdup(p);
		
#if defined(DEBUG)
		dlog->log_progress(DEBUG_PARSER, "Host '%s'", h);
#endif
		p = q + 1;
	      }
	    }
	  }
	}

	// Now look for token and instance

	if(p)
	{
	  q = strstr(p, "T=");

	  if(q && strlen(q) > 6)
	  {
	    // The form is T=x,I=y where x is the token and y is
	    // the instance.
	    
	    char *s = q + 2;
	    p = strchr(s, ',');
	    
	    if(p)
	    {
	      *p = '\0';
	      p++;
	      
	      // s now points to the token, find the instance but make
	      // sure we have enough string left

	      if(strlen(p) > 2)
	      {
		p += 2;
		
		q = p;
		
		while(!isspace(*q) && *q != '\0')
		  q++;

		*q = '\0';

		// Now that the pointers are set up, copy the strings

		t = xstrdup(s);
		inst = xstrdup(p);

#if defined(DEBUG)
		dlog->log_progress(DEBUG_PARSER, "Token '%s', instance '%s'",
				   t, inst);
#endif	    
	      }
	    }
	  }
	}
	
	xdelete(l);
      }
    }    

    if(f && h && inst && svc && t && (ry != unknown_reply))
      ret = true;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "SMSReply::parse = %s", IOTF(ret));
#endif

  return(ret);
}
