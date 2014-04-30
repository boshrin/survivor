/*
 * FileHandler.C: File Based Character Array Buffer Object
 *
 * Version: $Revision: 0.2 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2005/08/10 01:12:07 $
 * MT-Level: Safe
 *
 * Copyright (c) 2003
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: FileHandler.C,v $
 * Revision 0.2  2005/08/10 01:12:07  benno
 * Add support for read timeout
 *
 * Revision 0.1  2004/03/01 23:15:20  benno
 * Initial revision
 *
 */

#include "survivor.H"

FileHandler::FileHandler()
{
  // Allocate and initialize a new FileHandler object.  A file must
  // be opened using an open_X method, the file will be closed when
  // the FileHandler is deallocated.

  // Returns: A new FileHandler object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "FileHandler::FileHandler()");
#endif

  // Be sure to update the other constructor, below
  do_close = true;
  f = -1;
  to = 0;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "FileHandler::FileHandler()");
#endif
}

FileHandler::FileHandler(int fd)
{
  // Allocate and initialize a new FileHandler object that manipulates
  // the already open file descriptor <fd>.  With this constructor,
  // <fd> will NOT be closed when the FileHandler is deallocated.

  // Returns: A new FileHandler object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "FileHandler::FileHandler(%d)", fd);
#endif

  // Be sure to update the other constructor, above
  do_close = false;
  f = fd;
  to = 0;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "FileHandler::FileHandler()");
#endif
}

bool FileHandler::open_read(char *path)
{
  // Open the file <path> for reading.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "FileHandler::open_read(%s)", IONULL(path));
#endif

  if(path)
  {
    f = open(path, O_RDONLY);

    if(f > -1)
      ret = true;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "FileHandler::open_read = %s", IOTF(ret));
#endif

  return(ret);
}

bool FileHandler::open_write(char *path, mode_t mode)
{
  // Open the file <path> for writing, creating it with <mode> if it
  // doesn't exist and truncating it if it does.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "FileHandler::open_write(%s,%d)",
		  IONULL(path), mode);
#endif

  if(path)
  {
    f = open(path, O_WRONLY | O_CREAT | O_TRUNC, mode);

    if(f > -1)
      ret = true;
    else
      wlog->warn("FileHandler::open_write failed to open %s (%s)",
		 path, strerror(errno));
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "FileHandler::open_write = %s", IOTF(ret));
#endif

  return(ret);
}

bool FileHandler::set_read_timeout(int t)
{
  // Set the read timeout to <t> seconds.  If <t> is 0, no timeout is set.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "FileHandler::set_read_timeout(%d)", t);
#endif

  if(t >= 0)
    to = t;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "FileHandler::set_read_timeout = %s",
		 IOTF(ret));
#endif
  
  return(ret);
}

FileHandler::~FileHandler()
{
  // Deallocate the FileHandler.

  // Returns: Nothing.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "FileHandler::FileHandler()");
#endif

  if(f > -1 && do_close)
    close(f);
  
  f = -1;
  to = 0;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "FileHandler::~FileHandler()");
#endif
}

bool FileHandler::do_append(char *s)
{
  // Append the string <s> to the FileHandler.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "FileHandler::do_append(%s)", IONULL(s));
#endif

  if(f > -1 && s && strlen(s) > 0 && write(f, s, strlen(s))==strlen(s))
    ret = true;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "FileHandler::do_append = %s", IOTF(ret));
#endif
  
  return(ret);
}

char FileHandler::do_getchar()
{
  // Obtain the next character from the FileHandler.

  // Returns: The next character, or NULL (0) if there are no more
  // characters to return.

  char ret[1] = { 0 };

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "FileHandler::do_getchar()");
#endif

  // If something goes wrong in the read, ret[0] will still be 0.
  
  if(f > -1)
  {
    if(to > 0)
    {
      // Make sure we have data to read before blocking.

      fd_set fdset;
      int fdmax = f+1;
      struct timeval timeout;

      timeout.tv_sec = to;
      timeout.tv_usec = 0;

      FD_ZERO(&fdset);
      FD_SET(f, &fdset);
      
      int n = select(fdmax, &fdset, NULL, NULL, &timeout);

      if(n > 0)
	read(f, &ret, 1);
      // else timed out
    }
    else  
      read(f, &ret, 1);
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "FileHandler::do_getchar = %c",
		 (ret[0] == 0 ? '0' : ret[0]));
#endif
  
  return(ret[0]);
}
