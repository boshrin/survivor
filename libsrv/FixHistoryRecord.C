/*
 * FixHistoryRecord.C: Object to manage fix history records
 *
 * Version: $Revision: 0.1 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2003/11/08 15:36:39 $
 * MT-Level: Unsafe
 *
 * Copyright (c) 2003
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: FixHistoryRecord.C,v $
 * Revision 0.1  2003/11/08 15:36:39  benno
 * Initial revision
 *
 */

#include "survivor.H"

FixHistoryRecord::FixHistoryRecord()
{
  // Allocate and initialize a new FixHistoryRecord object.

  // Returns: A new FixHistoryRecord object.

  init(-1, NULL, NULL);
}

FixHistoryRecord::FixHistoryRecord(int fixrv, char *who, char *comment)
{
  // Allocate and initialize a new FixHistoryRecord object, holding
  // <fixrv>, <who>, <comment>.

  // Returns: A new FixHistoryRecord object.

  init(fixrv, who, comment);
}

char *FixHistoryRecord::comment()
{
  // Obtain the fix comment stored within.

  // Returns: A pointer to a string containing the comment, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "FixHistoryRecord::comment()");
  dlog->log_exit(DEBUG_MINTRC, "FixHistoryRecord::comment = %s", IONULL(cmt));
#endif
  
  return(cmt);
}

int FixHistoryRecord::fixrv()
{
  // Obtain the fix return value stored within.

  // Returns: The fix return value, or -1.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "FixHistoryRecord::fixrv()");
  dlog->log_exit(DEBUG_MINTRC, "FixHistoryRecord::fixrv = %d", frv);
#endif
  
  return(frv);
}

char *FixHistoryRecord::who()
{
  // Obtain the who identity stored within.

  // Returns: A pointer to a string containing the identity, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "FixHistoryRecord::who()");
  dlog->log_exit(DEBUG_MINTRC, "FixHistoryRecord::who = %s", IONULL(w));
#endif
  
  return(w);
}

FixHistoryRecord::~FixHistoryRecord()
{
  // Deallocate the FixHistoryRecord.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "FixHistoryRecord::~FixHistoryRecord()");
#endif
  
  frv = -1;
  xdelete(cmt);
  xdelete(m);
  xdelete(w);
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "FixHistoryRecord::~FixHistoryRecord()");
#endif
}

char *FixHistoryRecord::make_string()
{
  // Generate a string containing the data portion of the record entry.

  // Returns: A pointer to a string that should not be modified and
  // contains the data portion of the record, or NULL on error.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "FixHistoryRecord::make_string()");
#endif

  // Toss any old data
  xdelete(m);

  CharBuffer *cbuf = new CharBuffer();

  if(cbuf)
  {
    cbuf->append(frv);
    cbuf->append(":0:");
    cbuf->append(w);
    cbuf->append(':');
    cbuf->append(cmt);

    m = xstrdup(cbuf->str());
    xdelete(cbuf);
  }
  else
    wlog->warn("FixHistoryRecord::make_string failed to allocate cbuf");
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "FixHistoryRecord::make_string = %s",
		 IONULL(m));
#endif

  return(m);
}

bool FixHistoryRecord::parse_string(char *s)
{
  // Parse the contents of <s>.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "FixHistoryRecord::parse_string(%s)",
		  IONULL(s));
#endif

  if(s)
  {
    // Toss any previous data

    xdelete(cmt);
    xdelete(w);
    
    // Make a copy of s and run it through strtok

    char *sx = xstrdup(s);

    if(sx)
    {
      toss_eol(sx);
      
      char *last;
      char *r = strtok_r(sx, ":", &last);

      if(r)
      {
	frv = atoi(r);

	r = strtok_r(NULL, ":", &last);

	if(r)
	{
	  // We now have the 0, which is just a place holder.  Keep scanning.

	  r = strtok_r(NULL, ":", &last);

	  if(r)
	  {	  
	    w = xstrdup(r);
	    
	    r = strtok_r(NULL, "\n", &last);
	    
	    if(r)
	    {
	      cmt = xstrdup(r);
	      ret = true;
	    }
	    else
	      wlog->warn("FixHistoryRecord::parse_string did not find comment");
	  }
	  else
	    wlog->warn("FixHistoryRecord::parse_string did not find identity");
	}
	else
	  wlog->warn("FixHistoryRecord::parse_string did not find place holder");
      }
      else
	wlog->warn("FixHistoryRecord::parse_string did not find fix return value");
      
      xdelete(sx);
    }
    else
      wlog->warn("FixHistoryRecord::parse_string failed to copy s");
  }

#if defined(debug)
  dlog->log_exit(DEBUG_MINTRC, "FixHistoryRecord::parse_string = %s",
		 IOTF(ret));
#endif
  
  return(ret);
}

void FixHistoryRecord::init(int fixrv, char *who, char *comment)
{
  // Initializer for FixHistoryRecord object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "FixHistoryRecord::init(%s,%s,%s)",
		  fixrv, IONULL(who), IONULL(comment));
#endif

  frv = fixrv;
  cmt = xstrdup(comment);
  m = NULL;
  w = xstrdup(who);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "FixHistoryRecord::init()");
#endif
}
