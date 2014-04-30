/*
 * Debugger.C: Debugging Object
 *
 * Version: $Revision: 0.2 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2005/09/26 13:56:11 $
 * MT-Level: Safe
 *
 * Copyright (c) 2002 - 2003
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: Debugger.C,v $
 * Revision 0.2  2005/09/26 13:56:11  benno
 * Replace ctime() call with ctime_r()
 *
 * Revision 0.1  2003/04/07 21:00:14  benno
 * Initial revision
 *
 */

#include "survivor.H"

Debugger::Debugger()
{
  // Allocate and initialize a new Debugger object.

  // Returns: A new Debugger object.
  
  tid = false;
  ts = false;
#if defined(_BROKEN_VSNPRINTF)
  use_vsp = false;
#endif
  dl = 0;
}

bool Debugger::enable_tid()
{
  // Enable inclusing of thread IDs in debugging output.

  // Returns: true if enabled, false otherwise, including if already
  // enabled.

  if(!tid)
  {
    tid = true;
    return(true);
  }

  return(false);
}

bool Debugger::enable_timestamp()
{
  // Enable inclusing of timestamps in debugging output.

  // Returns: true if enabled, false otherwise, including if already
  // enabled.

  if(!ts)
  {
    ts = true;
    return(true);
  }

  return(false);
}

void Debugger::log_entry(int level, char *fmt, ...)
{
  // Log entry into a method, if <level> debugging is in effect.

  // Returns: Nothing.

  va_list ap;

  va_start(ap, fmt);
  
  if(level & dl)
  {
    char msgbuf[BUFSIZE];

    int index = prepare_entry(msgbuf, BUFSIZE, "-> ");
#if defined(_BROKEN_VSNPRINTF)
    if(use_vsp)
      vsprintf(msgbuf+index, fmt, ap);
    else
#endif
    vsnprintf(msgbuf+index, BUFSIZE - index - 1, fmt, ap);
    do_log(false, msgbuf);
  }

  va_end(ap);
}

void Debugger::log_exit(int level, char *fmt, ...)
{
  // Log exit from a method, if <level> debugging is in effect.

  // Returns: Nothing.

  va_list ap;

  va_start(ap, fmt);
  
  if(level & dl)
  {
    char msgbuf[BUFSIZE];

    int index = prepare_entry(msgbuf, BUFSIZE, "<- ");
#if defined(_BROKEN_VSNPRINTF)
    if(use_vsp)
      vsprintf(msgbuf+index, fmt, ap);
    else
#endif
    vsnprintf(msgbuf+index, BUFSIZE - index - 1, fmt, ap);
    do_log(false, msgbuf);
  }

  va_end(ap);  
}

void Debugger::log_lex(int level, bool err, char *cf, int line, char *fmt,
		       ...)
{
  // Log lex progress, if <level> debugging is in effect.  If <err> is
  // true, the log entry is flagged as an error, otherwise progress.
  // <cf> is the configuration file being parsed, <line> is the line
  // number of the error.

  // Returns: Nothing.

  va_list ap;

  va_start(ap, fmt);
  
  if(level & dl)
  {
    char msgbuf[BUFSIZE];
    
    int index = prepare_entry(msgbuf, BUFSIZE,
			      (char *)(err ? "*lex* " : "|lex| "));
#if defined(_BROKEN_VSNPRINTF)
    if(use_vsp)
      vsprintf(msgbuf+index, fmt, ap);
    else
#endif
    vsnprintf(msgbuf+index, BUFSIZE - index - 1, fmt, ap);
    index = strlen(msgbuf);
    if(index + 1 < BUFSIZE)
      snprintf(msgbuf+index, BUFSIZE - index - 1, " at %s line %d",
	       IONULL(cf), line);
    do_log(false, msgbuf);
  }

  va_end(ap);  
}

void Debugger::log_progress(int level, char *fmt, ...)
{
  // Log progress within a method, if <level> debugging is in effect.

  // Returns: Nothing.

  va_list ap;

  va_start(ap, fmt);
  
  if(level & dl)
  {
    char msgbuf[BUFSIZE];

    int index = prepare_entry(msgbuf, BUFSIZE, "|| ");
#if defined(_BROKEN_VSNPRINTF)
    if(use_vsp)
      vsprintf(msgbuf+index, fmt, ap);
    else
#endif
    vsnprintf(msgbuf+index, BUFSIZE - index - 1, fmt, ap);
    do_log(false, msgbuf);
  }

  va_end(ap);  
}

bool Debugger::set_level(int level)
{
  // Set the level of debugging to <level>.

  // Returns: true if fully successful, false otherwise.

  dl = level;
  return(true);
}

void Debugger::warn(char *fmt, ...)
{
  // Log a warning.

  // Returns: Nothing.

  va_list ap;
  char msgbuf[BUFSIZE];
  
  va_start(ap, fmt);
  
  int index = prepare_entry(msgbuf, BUFSIZE, NULL);
#if defined(_BROKEN_VSNPRINTF)
  if(use_vsp)
    vsprintf(msgbuf+index, fmt, ap);
  else
#endif
  vsnprintf(msgbuf+index, BUFSIZE - index - 1, fmt, ap);
  do_log(true, msgbuf);

  va_end(ap);  
}

Debugger::~Debugger()
{
  // Deallocate the Debugger object.

  // Returns: Nothing.

  tid = false;
  ts = false;
  dl = 0;
}

int Debugger::prepare_entry(char *buf, int bufsize, char *prefix)
{
  // Prepare a log entry in <buf> (of size <bufsize>), prefixed with
  // <prefix>.

  // Returns: The number of characters stored in <buf>.

  int r = 0;
  struct timeval tp;
  
  if(buf && bufsize > 100)
  {
    if(tid)
    {
      sprintf(buf, "%d ", pthread_self());
      r += strlen(buf);
    }

    if(ts)
    {
      char cbuf[26];
      
      gettimeofday(&tp, NULL);
      sprintf(buf+r, "%24.24s ", ctime_r((time_t *)&(tp.tv_sec), cbuf));
      r += 25;
    }

    if(prefix)
    {
      sprintf(buf+r, prefix);
      r += strlen(prefix);
    }
  }

  return(r);
}

