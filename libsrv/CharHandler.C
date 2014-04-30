/*
 * CharHandler.C: Character Handler Object
 *
 * Version: $Revision: 0.3 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2005/11/11 04:00:35 $
 * MT-Level: Safe
 *
 * Copyright (c) 2003 - 2005
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: CharHandler.C,v $
 * Revision 0.3  2005/11/11 04:00:35  benno
 * Add append(double)
 *
 * Revision 0.2  2005/04/06 14:05:45  benno
 * Add read_until(char *)
 * Move increase_bsize here
 *
 * Revision 0.1  2004/03/01 23:14:50  benno
 * Initial revision
 *
 */

#include "survivor.H"

CharHandler::CharHandler()
{
  // Allocate and initialize a new CharHandler object.

  // Returns: A new CharHandler object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CharHandler::CharHandler()");
#endif

  rbuf = NULL;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CharHandler::CharHandler()");
#endif
}

bool CharHandler::append(char c)
{
  // Append the single character <c> to the CharHandler.

  // Returns: true if fully successful, false otherwise.

  // Stuff c into a fixed width string
  
  char s[2];

  s[0] = c;
  s[1] = '\0';

  return(do_append(s));
}

bool CharHandler::append(char *s)
{
  // Append the string <s> to the CharHandler.

  // Returns: true if fully successful, false otherwise.

  return(do_append(s));
}

bool CharHandler::append(double d)
{
  // Append the double float <d> to the CharHandler, using the default
  // precision.

  // Returns: true if fully successful, false otherwise.

  // All we do is sprintf it into an internal buffer and then append
  // as if it were a string.

  char lbuf[INTBUFSIZE * 2];

  memset(lbuf, 0, INTBUFSIZE * 2);
  snprintf(lbuf, INTBUFSIZE * 2, "%f", d);

  return(do_append(lbuf));
}

bool CharHandler::append(int i)
{
  // Append the integer <i> to the CharHandler.

  // Returns: true if fully successful, false otherwise.

  // All we do is sprintf it into an internal buffer and then append
  // as if it were a string.

  char ibuf[INTBUFSIZE];

  memset(ibuf, 0, INTBUFSIZE);
  snprintf(ibuf, INTBUFSIZE, "%d", i);

  return(do_append(ibuf));
}

bool CharHandler::append(long l)
{
  // Append the long integer <l> to the CharHandler.

  // Returns: true if fully successful, false otherwise.

  // All we do is sprintf it into an internal buffer and then append
  // as if it were a string.

  char lbuf[INTBUFSIZE];

  memset(lbuf, 0, INTBUFSIZE);
  snprintf(lbuf, INTBUFSIZE, "%d", l);

  return(do_append(lbuf));
}

char CharHandler::read_char()
{
  // Obtain the next character from the CharHandler.

  // Returns: The next character, or NULL (0) if there are no more
  // characters to return.
  
  return(do_getchar());
}

char *CharHandler::read_line()
{
  // Obtain the next line from the CharHandler, not including the
  // newline.

  // Returns: A pointer to a string that should not be modified,
  // and that remains valid until the next read_ call, or NULL
  // on error or when no more data was read.

  return(read_until('\n'));
}

char *CharHandler::read_until(char c)
{
  // Obtain data from the CharHandler, until the character <c> is seen
  // or until no further input is available.  <c> is not transferred
  // to the return string.

  // Returns: A pointer to a string that should not be modified,
  // and that remains valid until the next read_ call, or NULL
  // on error or when no more data was read.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CharHandler::read_until(%c)",
		  (c == 0 ? '0' : c));
#endif

  xdelete(rbuf);

  bool done = false;
  int rindex = 0;
  int rlen = 0;

  while(!done)
  {
    char gc = do_getchar();

    if(gc != c && gc != 0)
    {
      if(rindex + 2 >= rlen)
      {
	// Time to increase the return buffer

	if(!increase_bsize(&rbuf, &rlen, BUFSIZE))
	{
	  done = true;
	  break;
	}
      }

      // If we make it here, append gc

      rbuf[rindex] = gc;
      rindex++;
    }
    else
    {
      done = true;

      // If <c> is the first character we read, we want to return an
      // empty string, as distinct from NULL.

      if(gc == c && !rbuf)
      {
	rbuf = new char[1];

	if(rbuf)
	{
	  rbuf[0] = '\0';
	  rlen = 1;
	  rindex++;
	}
      }
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CharHandler::read_until = %s", IOTF(rbuf));
#endif
  
  return(rbuf);
}

char *CharHandler::read_until(char *s, bool casesen)
{
  // Obtain data from the CharHandler, until the string <s> is seen or
  // until no further input is available.  If <casesen> is true, a
  // case sensitive match is performed.  <s> (with original case) is
  // transferred to the return string.

  // Returns: A pointer to a string that should not be modified,
  // and that remains valid until the next read_ call, or NULL
  // on error or when no more data was read.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CharHandler::read_until(%s,%s)",
		  s, IOTF(casesen));
#endif

  xdelete(rbuf);
  
  if(s && strlen(s) > 0)
  {
    bool done = false;
    int matched = 0;  // # of consecutive chars of <s> we've matched so far
    int rindex = 0;
    int rlen = 0;

    while(!done)
    {
      char gc = do_getchar();

      if(gc != 0)
      {
	// Append, then check

	if(rindex + 2 >= rlen)
	{
	  // Time to increase the return buffer

	  if(!increase_bsize(&rbuf, &rlen, BUFSIZE))
	  {
	    done = true;
	    break;
	  }
	}

	// Store the new char

	rbuf[rindex] = gc;
	rindex++;

	if(gc == s[matched] ||            // Have we matched the next char?
	   (!casesen && tolower(gc)==tolower(s[matched])))
	  matched++;
	else
	  matched = 0;  // Reset

	if(matched == strlen(s))  // We're done if we've matched the whole 
	  done = true;            // string
      }
      else
	done = true;
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CharHandler::read_until = %s", IOTF(rbuf));
#endif
  
  return(rbuf);
}

char *CharHandler::read_until_eof()
{
  // Obtain all remaining data from the CharHandler.

  // Returns: A pointer to a string that should not be modified,
  // and that remains valid until the next read_ call, or NULL
  // on error or when no more data was read.

  return(read_until(0));
}

CharHandler::~CharHandler()
{
  // Deallocate the CharHandler.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CharHandler::~CharHandler()");
#endif
  
  xdelete(rbuf);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CharHandler::~CharHandler()");
#endif
}

bool CharHandler::increase_bsize(char **buf, int *curlen, int addlen)
{
  // Copy the contents of *<buf> to a new buffer where <curlen>
  // initially contains the size of the current buffer (which is
  // to be increased by <addlen>), and upon completion of this
  // method will contain the size of the new buffer.  *<buf> will
  // be deleted on success and <buf> will point to the new buffer.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CharHandler::increase_bsize(%d,%d,%d)",
		  buf, *curlen, addlen);
#endif

  if(buf && addlen > 0 && *curlen >= 0)
  {
    char *newbuf = new char[*curlen + addlen];

    if(newbuf)
    {
      *curlen += addlen;
      memset(newbuf, 0, *curlen);

      // Copy the old string if there was one and swap pointers
      if(*buf)
	strcpy(newbuf, *buf);

      xdelete(*buf);
      *buf = newbuf;

      ret = true;
    }
    else
      wlog->warn("CharHandler::increase_bsize failed to allocate new buffer");
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CharHandler::increase_bsize = %d", IOTF(ret));
#endif
  
  return(ret);
}
