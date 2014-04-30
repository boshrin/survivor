/*
 * CharBuffer.C: Character Array Buffer Object
 *
 * Version: $Revision: 0.5 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2005/04/06 14:05:06 $
 * MT-Level: Safe
 *
 * Copyright (c) 2002 - 2005
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: CharBuffer.C,v $
 * Revision 0.5  2005/04/06 14:05:06  benno
 * Add replace()
 * Move increase_bsize to CharHandler
 *
 * Revision 0.4  2004/11/26 22:16:02  benno
 * Add seek(int)
 *
 * Revision 0.3  2004/03/01 23:27:38  benno
 * Move append() to CharHandler
 * Use do_append and do_getchar
 * Add clear()
 *
 * Revision 0.2  2003/11/29 05:19:35  benno
 * Add constructor from existing CharBuffer
 *
 * Revision 0.1  2003/11/08 15:34:09  benno
 * Initial revision
 *
 */

#include "survivor.H"

CharBuffer::CharBuffer()
{
  // Allocate and initialize a new, empty CharBuffer object.

  // Returns: A new CharBuffer object.

  init(NULL, false);
}

CharBuffer::CharBuffer(char *s)
{
  // Allocate and initialize a new CharBuffer object, holding a copy of <s>.

  // Returns: A new CharBuffer object.

  init(s, false);
}

CharBuffer::CharBuffer(CharBuffer *cb)
{
  // Allocate and initialize a new CharBuffer object, holding a copy
  // of the contents of <cb>.

  // Returns: A new CharBuffer object.

  init(cb ? cb->str() : NULL, false);
}

bool CharBuffer::append_fixed_buffer()
{
  // Append the contents of the fixed buffer to the CharBuffer.
  // The fixed buffer becomes invalid after this method is invoked.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CharBuffer::append_fixed_buffer()");
#endif

  if(tbuf)
  {
    ret = do_append(tbuf);
    xdelete(tbuf);
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CharBuffer::append_fixed_buffer = %s",
		 IOTF(ret));
#endif
  
  return(ret);
}

bool CharBuffer::clear()
{
  // Clear the contents of the CharBuffer.

  // Returns: true if fully successful, false otherwise.

  init(NULL, true);  
  return(true);
}

char *CharBuffer::create_fixed_buffer(int bsize)
{
  // Allocate a fixed, initialized char buffer of size <bsize>.  The
  // buffer is suitable for passing to system routines such as fread()
  // that require a buffer of a known size.  The buffer is maintained
  // by the CharBuffer object, so care must be taken by the user to
  // not write beyond <bsize> bytes (not including terminating NULL).
  // After data has been written to the buffer, call append_fixed_buffer()
  // to add the data to the CharBuffer and delete the fixed buffer.

  // Returns: A pointer to a char buffer of size <bsize> + 1, or NULL.

  char *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CharBuffer::create_fixed_buffer(%d)", bsize);
#endif

  if(!tbuf)
  {
    tbuf = new char[bsize + 1];
    
    if(tbuf)
    {
      memset(tbuf, 0, bsize + 1);
      ret = tbuf;
    }
    else
      wlog->warn("CharBuffer::create_fixed_buffer failed to allocate tbuf");
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CharBuffer::create_fixed_buffer = %s",
		 IOTF(ret));
#endif

  return(ret);
}

int CharBuffer::replace(char o, char n)
{
  // Replace all occurrences of the character <o> with the character
  // <n>.

  // Returns: The number of characters replaced.

  int ret = 0;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CharBuffer::replace(%c,%c)", o, n);
#endif

  if(b && blen > 0)
  {
    for(int i = 0;i < blen;i++)
    {
      if(b[i] == o)
      {
	// Match, replace this char
	
	b[i] = n;
	ret++;
      }
    }
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CharBuffer::replace = %d", ret);
#endif

  return(ret);
}

bool CharBuffer::seek(int pos)
{
  // Move the current read index used by the read_* methods to
  // the position <pos>.  Setting <pos> to 0 indicates the beginning
  // of the buffer, -1 indicates the end of the buffer.

  // Returns: true if fully successful, false otherwise.

  bool ret = true;  // Note default successful return
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CharBuffer::seek(%d)", pos);
#endif
  
  if(pos == -1)
    gindex = blen;
  else if(pos > -1 && pos < blen)
    gindex = pos;
  else
    ret = false;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CharBuffer::seek = %s", IOTF(ret));
#endif
  
  return(ret);
}

char *CharBuffer::str()
{
  // Obtain the contents of the CharBuffer.

  // Returns: A pointer to a char array that should not be modified, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CharBuffer::str()");
  dlog->log_exit(DEBUG_MINTRC, "CharBuffer::str = %s", IONULL(b));
#endif
  
  return(b);
}

CharBuffer::~CharBuffer()
{
  // Deallocate the CharBuffer.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CharBuffer::~CharBuffer()");
#endif
  
  xdelete(b);
  xdelete(tbuf);
  gindex = 0;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CharBuffer::~CharBuffer()");
#endif
}

bool CharBuffer::do_append(char *s)
{
  // Append the string <s> to the CharBuffer.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CharBuffer::do_append(%s)", IONULL(s));
#endif

  if(s)
  {
    int slen = strlen(s);

    if(slen + 2 >= BUFSIZE)
    {
      // If <s> is larger than BUFSIZE, just append the whole thing
      // regardless of how much space may already be available.

      if(increase_bsize(&b, &bsize, slen + 1))
      {
	strcat(b, s);
	blen += slen;

	ret = true;
      }
    }
    else
    {
      // Increase bsize if we don't have enough space.
      
      if(blen + slen + 2 < bsize || increase_bsize(&b, &bsize, BUFSIZE))
      {
	strcat(b, s);
	blen += slen;

	ret = true;
      }
    }
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CharBuffer::do_append = %s", IOTF(ret));
#endif
  
  return(ret);
}

char CharBuffer::do_getchar()
{
  // Obtain the next character from the CharBuffer.

  // Returns: The next character, or NULL (0) if there are no more
  // characters to return.

  char ret = 0;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CharBuffer::do_getchar()");
#endif

  if(b && gindex < blen)
  {
    ret = b[gindex];
    gindex++;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CharBuffer::do_getchar = %c",
		 (ret == 0 ? '0' : ret));
#endif
  
  return(ret);
}

void CharBuffer::init(char *s, bool reset)
{
  // Initializer for constructors.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CharBuffer::init(%s,%s)", IONULL(s),
		  IOTF(reset));
#endif

  if(reset)
  {
    xdelete(b);
    xdelete(tbuf);
  }
  
  blen = 0;
  bsize = 0;
  b = xstrdup(s);
  tbuf = NULL;
  gindex = 0;

  if(b)
  {
    blen = strlen(s);
    bsize = blen + 1;
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CharBuffer::init()");
#endif
}
