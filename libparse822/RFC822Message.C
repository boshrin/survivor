/*
 * RFC822Message.C: survivor object for holding information from an RFC 822
 *  compliant message
 *
 * Version: $Revision: 0.1 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2004/11/26 21:59:59 $
 * MT-Level: Safe, if parse is only called once.
 *
 * Copyright (c) 2004
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: RFC822Message.C,v $
 * Revision 0.1  2004/11/26 21:59:59  benno
 * Initial revision
 *
 */

#include "survivor.H"

RFC822Message::RFC822Message()
{
  // Allocate and initialize a new RFC822Message object.

  // Returns: A newly allocated RFC822Message object, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "RFC822Message()::RFC822Message()");
#endif
  
  cmsg = NULL;
  headers = NULL;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "RFC822Message()::RFC822Message()");
#endif
}

CharBuffer *RFC822Message::body()
{
  // Obtain the parsed message body.

  // Returns: A pointer to a CharBuffer, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "RFC822Message::body()");
  dlog->log_exit(DEBUG_MAJTRC, "RFC822Message::body = %d", cmsg);
#endif

  return(cmsg);
}

int RFC822Message::header_entries(char *header)
{
  // Determine the number of entries for the specified <header>.

  // Returns: The number of entries, available via header_entry(), or 0
  // if none found.

  int ret = 0;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "RFC822Message::header_entries(%s)",
		  IONULL(header));
#endif

  if(header && headers)
  {
    List *l = headers->retrieve(header);

    if(l)
      ret = l->entries();
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "RFC822Message::header_entries = %d", ret);
#endif
  
  return(ret);
}

char *RFC822Message::header_entry(char *header, int entry)
{
  // Obtain the contents of the <header> at position <entry>.

  // Returns: A pointer to a string that should not be modified, or NULL
  // if the requested heaer or entry could not be found.

  char *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "RFC822Message::header_entries(%s)",
		  IONULL(header));
#endif

  if(header && headers)
  {
    List *l = headers->retrieve(header);

    if(l)
      ret = l->retrieve(entry);
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "RFC822Message::header_entries = %d", ret);
#endif
  
  return(ret);
}

bool RFC822Message::parse(char *msg)
{
  // Parse the message provided on <msg>.

  // Returns: true if all items were successfully parsed from the message,
  // false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "RFC822Message::parse(size=%d)",
		  (msg ? strlen(msg) : 0));
#endif

  if(msg)
  {
    // Copy the message into a CharBuffer.  We could also tokenize()
    // on \n for much the same effect.

    cmsg = new CharBuffer();  // Message body
    CharBuffer *xmsg = new CharBuffer(msg);

    if(cmsg && xmsg)
    {
      bool body = false;  // Have we seen the last header?
      char *hn = NULL;    // Current header name
      char *hv = NULL;    // Current header value

      for(char *p = xmsg->read_line();p != NULL;p = xmsg->read_line())
      {
	if(!body)
	{
	  if(hn && hv && (p[0] == '\n' || !isspace(p[0])))
	  {
	    // We're done with whatever we were reading, insert it

	    if(!headers)
	      headers = new Hashtable<List>();

	    if(headers)
	    {
	      List *l = headers->retrieve(hn);

	      if(!l)
		l = new List(hn);

	      if(!l || !headers->insert(l->name(), l))
	      {
		xdelete(l);
		wlog->warn("RFC822Message::parse failed to insert List into Hashtable");
	      }

	      if(l)
	      {
		// If we are looking at a field with addresses, parse
		// out the individual addresses and add them separately

		if(strcasecmp(hn, "To")==0 || strcasecmp(hn, "Cc")==0
		   || strcasecmp(hn, "From")==0)
		{
		  // Use librfc822 to pull out the actual address contents
		  
		  string hvs = hv;
		  hvs += "\n";
		  
		  try {
		    // Run parser here
		    RFC822Committer committer(l);
		    
		    rfc822parser parser(lex(hvs), &committer);
		    parser.addresses();
		  }
		  catch(rfc822_syntax_error & e)
		  {
		    wlog->warn("RFC822Message::parse found address '%s' with syntax error: %s", IONULL(hvs.c_str()), e.what());
		  }
		  catch(exception & e)
		  {
		    wlog->warn("RFC822Message::parse caught exception: %s", e.what());
		  }
		  catch(...)
		  {
		    wlog->warn("RFC822Message::parse caught unknown exception");
		  }
		}
		else
		  l->add(hv);
	      }
	    }
	    else
	      wlog->warn("RFC822Message::parse failed to allocated Hashtable");

	    xdelete(hn);
	    xdelete(hv);
	  }
	  
	  if(strlen(p) == 0)
	  {
	    body = true;

	    // At this point, consider the parse successful

	    ret = true;
	  }
	  else if(isspace(p[0]))
	  {
	    // Fold this line into the previously seen header

	    char *q = p;
	    
	    while(isspace(*q) && *q != '\0')
	      q++;

	    // q now points to the beginning of the indented text

	    hv = xstrcat(hv, q);
	  }
	  else
	  {
	    // Split out the header name from the text

	    // Make sure hn and hv are NULL (they might not be in a
	    // badly formatted message).

	    xdelete(hn);
	    xdelete(hv);
	    
	    List *t = tokenize(p, ":", 2);

	    if(t && t->entries() == 2)
	    {
	      hn = xstrdup(t->retrieve(0));

	      char *v = t->retrieve(1);

	      // First char of v should be a space

	      if(strlen(v) > 1)
		hv = xstrdup(v+1);
	    }
	    else
	      wlog->warn("RFC822Message::parse found malformed header: '%s'",
			 p);

	    xdelete(t);
	  }
	}
	else
	{
	  // Just append to the message

	  cmsg->append(p);
	  cmsg->append('\n');
	}
      }

      // There shouldn't be anything in these, but let's make sure
      
      xdelete(hn);
      xdelete(hv);
    }
    else
      wlog->warn("RFC822Message::parse failed to allocate message buffers");
      
    xdelete(xmsg);
    // cmsg gets cleared in deconstructor
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "RFC822Message::parse = %s", IOTF(ret));
#endif

  return(ret);
}

RFC822Message::~RFC822Message()
{
  // Deallocate the RFC822Message object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "RFC822Message()::~RFC822Message()");
#endif

  xdelete(cmsg);
  xhdelete(headers, List);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "RFC822Message()::~RFC822Message()");
#endif
}
