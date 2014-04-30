/*
 * HistoryRecord.C: Object to manage history records
 *
 * Version: $Revision: 0.4 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2006/01/23 02:09:52 $
 * MT-Level: Unsafe
 *
 * Copyright (c) 2003 - 2006
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: HistoryRecord.C,v $
 * Revision 0.4  2006/01/23 02:09:52  benno
 * Add duration
 *
 * Revision 0.3  2005/04/07 01:26:09  benno
 * Handle comments with newlines
 *
 * Revision 0.2  2003/11/29 05:25:11  benno
 * parse_entry offers split only to support History::prune
 *
 * Revision 0.1  2003/11/08 15:36:45  benno
 * Initial revision
 *
 */

#include "survivor.H"

HistoryRecord::HistoryRecord()
{
  // Allocate and initialize a new HistoryRecord object.

  // Returns: A new HistoryRecord object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "HistoryRecord::HistoryRecord()");
#endif

  dur = -1;
  tp.tv_sec = 0;
  tp.tv_usec = 0;
  e = NULL;
  tl = NULL;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "HistoryRecord::HistoryRecord()");
#endif
}

int HistoryRecord::duration()
{
  // Obtain the duration for this history record.

  // Returns: The duration, in milliseconds, or -1 if no duration is
  // available.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "HistoryRecord::duration()");
  dlog->log_exit(DEBUG_MINTRC, "HistoryRecord::duration = %d", dur);
#endif

  return(dur);
}

char *HistoryRecord::generate_entry()
{
  // Generate a single line history record suitable for writing to
  // a file.

  // Returns: A pointer to a string that should not be modified and
  // contains the history record, or NULL on error.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "HistoryRecord::generate_entry()");
#endif
  
  // Before we begin, clear out any old record.
  xdelete(e);
  
  CharBuffer *cbuf = new CharBuffer(timestamp_local());

  if(cbuf)
  {
    cbuf->append('/');
    cbuf->append(timestamp());
    
    if(dur > -1)
    {
      // Duration is optional

      cbuf->append('/');
      cbuf->append(dur);
    }
    
    cbuf->append(':');
    cbuf->append(make_string());

    // It's possible the make_string() added a newline since, as of
    // v0.9.6, comments can have more than one line.  Since history
    // records are all currently specified to have exactly one line,
    // we'll change any newlines to spaces.

    cbuf->replace('\n', ' ');

    // And now add our newline.
    
    cbuf->append('\n');

    e = xstrdup(cbuf->str());
    xdelete(cbuf);
  }
  else
    wlog->warn("HistoryRecord::generate_entry failed to allocate buffer");

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "HistoryRecord::generate_entry = %s",
		 IONULL(e));
#endif
  
  return(e);
}

bool HistoryRecord::parse_entry(char *s)
{
  // Parse the record <s> into its components.

  // Returns: true if fully successful, false otherwise.

  return(parse_entry(s, false));
}

bool HistoryRecord::parse_entry(char *s, bool splitonly)
{
  // Parse the record <s> into its components.  If <splitonly> is
  // true, then the data portion is not parsed.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "History::parse_entry(%s,%s)",
		  IONULL(s), IOTF(splitonly));
#endif

  if(s)
  {
    // We expect a line of the format
    //  timestamplocal/timestamp[/duration]:somestuff\n

    // Although we always write a local timestamp of 24 characters, we
    // shouldn't expect to find that there, especially as the size of
    // timestamp could change in the future.  Whatever we read we store,
    // we don't try to make sure the two timestamps mean the same thing.

    // Clean up any previous timestamp

    xdelete(tl);

    tl = xstrdup(s);

    if(tl)
    {
      toss_eol(tl);
      
      char *ts = strchr(tl, '/');
      
      if(ts)
      {
	*ts = '\0';
	ts++;

	// We now point to the numerical timestamp.  Find the colon.

	char *p = strchr(ts, ':');

	if(p)
	{
	  *p = '\0';
	  p++;

	  // Look to see if there's a /, indicating a duration

	  char *d = strchr(ts, '/');

	  if(d)
	  {
	    *d = '\0';
	    d++;

	    dur = atol(d);
	  }

	  // Convert ts to a number

	  tp.tv_sec = atol(ts);

	  if(splitonly)
	    ret = true;
	  else
	    ret = parse_string(p);
	}
	else
	  wlog->warn("HistoryRecord::parse_entry did not find data separator in entry [%s]", ts);
      }
      else
	wlog->warn("HistoryRecord::parse_entry did not find timestamp separator in entry [%s]", tl);
    }
    else
      wlog->warn("HistoryRecord::parse_entry failed to allocate s");
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "History::parse_entry = %s", IOTF(ret));
#endif
  
  return(ret);
}

int HistoryRecord::timestamp()
{
  // Obtain the timestamp for the record read by parse_entry() (if any),
  // or the current time (if none).

  // Returns: The timestamp, as seconds since the Unix Epoch, or 0 on
  // error.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "HistoryRecord::timestamp()");
#endif
  
  if(tp.tv_sec == 0)
  {
    if(gettimeofday(&tp, NULL) != 0)
      wlog->warn("gettimeofday() failed in HistoryRecord::timestamp()");
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "HistoryRecord::timestamp = %d", tp.tv_sec);
#endif
  
  return(tp.tv_sec);
}

char *HistoryRecord::timestamp_local()
{
  // Obtain the localized (human readable) timestamp for the record
  // read by parse_entry() (if any), or the current time (if none).

  // Returns: A pointer to a string that should not be modified and
  // describes the time or NULL on error.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "HistoryRecord::timestamp_local()");
#endif

  if(!tl)
  {
    if(tp.tv_sec > 0 || timestamp() > 0)
    {
      tl = new char[26];

      if(tl)
      {
	if(ctime_r((time_t *)&tp.tv_sec, tl))
	  tl[24] = '\0';  // Chop off the newline
	else
	{
	  xdelete(tl);
	}
      }
      else
	wlog->warn("HistoryRecord::timestamp_local failed to allocate tl");
    }
  }

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "HistoryRecord::timestamp_local = %s",
		  IONULL(tl));
#endif

  return(tl);
}

HistoryRecord::~HistoryRecord()
{
  // Deallocate the HistoryRecord object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "HistoryRecord::~HistoryRecord()");
#endif

  dur = -1;
  tp.tv_sec = 0;
  tp.tv_usec = 0;
  xdelete(e);
  xdelete(tl);
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "HistoryRecord::~HistoryRecord()");
#endif
}
