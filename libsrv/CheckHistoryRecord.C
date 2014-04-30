/*
 * CheckHistoryRecord.C: Object to manage check history records
 *
 * Version: $Revision: 0.2 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2006/01/23 02:08:20 $
 * MT-Level: Unsafe
 *
 * Copyright (c) 2003 - 2006
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: CheckHistoryRecord.C,v $
 * Revision 0.2  2006/01/23 02:08:20  benno
 * Add duration
 *
 * Revision 0.1  2003/11/08 15:36:25  benno
 * Initial revision
 *
 */

#include "survivor.H"

CheckHistoryRecord::CheckHistoryRecord()
{
  // Allocate and initialize a new CheckHistoryRecord object.

  // Returns: A new CheckHistoryRecord object.

  init(-1, -1, NULL);
}

CheckHistoryRecord::CheckHistoryRecord(int checkrv, int scalar, char *comment)
{
  // Allocate and initialize a new CheckHistoryRecord object, holding
  // <checkrv>, <scalar>, <comment>.

  // Returns: A new CheckHistoryRecord object.

  init(checkrv, scalar, comment);
}

CheckHistoryRecord::CheckHistoryRecord(int checkrv, int scalar, char *comment,
				       int duration)
{
  // Allocate and initialize a new CheckHistoryRecord object, holding
  // <checkrv>, <scalar>, <comment>, and <duration>.

  // Returns: A new CheckHistoryRecord object.

  dur = duration;
  init(checkrv, scalar, comment);
}

int CheckHistoryRecord::checkrv()
{
  // Obtain the check return value stored within.

  // Returns: The check return value, or -1.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CheckHistoryRecord::checkrv()");
  dlog->log_exit(DEBUG_MINTRC, "CheckHistoryRecord::checkrv = %d", crv);
#endif
  
  return(crv);
}

int CheckHistoryRecord::scalar()
{
  // Obtain the check scalar value stored within.

  // Returns: The check scalar value, or -1.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CheckHistoryRecord::scalar()");
  dlog->log_exit(DEBUG_MINTRC, "CheckHistoryRecord::scalar = %d", sc);
#endif
  
  return(sc);
}

char *CheckHistoryRecord::comment()
{
  // Obtain the check comment stored within.

  // Returns: A pointer to a string containing the comment, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CheckHistoryRecord::comment()");
  dlog->log_exit(DEBUG_MINTRC, "CheckHistoryRecord::comment = %s", IONULL(c));
#endif
  
  return(c);
}

CheckHistoryRecord::~CheckHistoryRecord()
{
  // Deallocate the CheckHistoryRecord.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CheckHistoryRecord::~CheckHistoryRecord()");
#endif
  
  crv = -1;
  sc = -1;
  xdelete(c);
  xdelete(m);
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CheckHistoryRecord::~CheckHistoryRecord()");
#endif
}

char *CheckHistoryRecord::make_string()
{
  // Generate a string containing the data portion of the record entry.

  // Returns: A pointer to a string that should not be modified and
  // contains the data portion of the record, or NULL on error.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CheckHistoryRecord::make_string()");
#endif

  // Toss any old data
  xdelete(m);

  CharBuffer *cbuf = new CharBuffer();

  if(cbuf)
  {
    cbuf->append(crv);
    cbuf->append(':');
    cbuf->append(sc);
    cbuf->append(':');
    cbuf->append(c);

    m = xstrdup(cbuf->str());
    xdelete(cbuf);
  }
  else
    wlog->warn("CheckHistoryRecord::make_string failed to allocate cbuf");
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CheckHistoryRecord::make_string = %s",
		 IONULL(m));
#endif

  return(m);
}

bool CheckHistoryRecord::parse_string(char *s)
{
  // Parse the contents of <s>.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CheckHistoryRecord::parse_string(%s)",
		  IONULL(s));
#endif

  if(s)
  {
    // Toss any previous data

    xdelete(c);
    
    // Make a copy of s and run it through strtok

    char *sx = xstrdup(s);

    if(sx)
    {
      toss_eol(sx);
      
      char *last;
      char *r = strtok_r(sx, ":", &last);

      if(r)
      {
	crv = atoi(r);

	r = strtok_r(NULL, ":", &last);

	if(r)
	{
	  sc = atoi(r);

	  r = strtok_r(NULL, "\n", &last);

	  if(r)
	  {
	    c = xstrdup(r);
	    ret = true;
	  }
	  else
	    wlog->warn("CheckHistoryRecord::parse_string did not find comment");
	}
	else
	  wlog->warn("CheckHistoryRecord::parse_string did not find check scalar value");
      }
      else
	wlog->warn("CheckHistoryRecord::parse_string did not find check return value");
      
      xdelete(sx);
    }
    else
      wlog->warn("CheckHistoryRecord::parse_string failed to copy s");
  }

#if defined(debug)
  dlog->log_exit(DEBUG_MINTRC, "CheckHistoryRecord::parse_string = %s",
		 IOTF(ret));
#endif
  
  return(ret);
}

void CheckHistoryRecord::init(int checkrv, int scalar, char *comment)
{
  // Initializer for CheckHistoryRecord object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CheckHistoryRecord::init(%d,%d,%s)",
		  checkrv, scalar, IONULL(comment));
#endif

  crv = checkrv;
  sc = scalar;
  c = xstrdup(comment);
  m = NULL;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CheckHistoryRecord::init()");
#endif
}
