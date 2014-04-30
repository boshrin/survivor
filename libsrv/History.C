/*
 * History.C: Object to manage history information
 *
 * Version: $Revision: 0.5 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2006/02/09 13:21:14 $
 * MT-Level: Unsafe
 *
 * Copyright (c) 2003 - 2006
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: History.C,v $
 * Revision 0.5  2006/02/09 13:21:14  benno
 * Fix file descriptor leak in prune()
 *
 * Revision 0.4  2004/08/25 00:05:26  benno
 * Add iterate_begin_backwards, iterate_previous_entry
 *
 * Revision 0.3  2004/04/24 15:16:16  toor
 * Fix memory leaks
 *
 * Revision 0.2  2003/11/29 05:24:06  benno
 * Add History(char,char) for stale service@host pairs
 * Add prune methods
 *
 * Revision 0.1  2003/11/08 15:36:11  benno
 * Initial revision
 *
 */

#include "survivor.H"

History::History(Check *check, char *host)
{
  // Allocate and initialize a new History object for <check> and <host>.

  // Returns: A new History object.

  init(check ? check->name() : NULL, host);
}

History::History(char *check, char *host)
{
  // Allocate and initialize a new History object for <check> and <host>.
  // This constructor is intended for stale history, those for which no
  // corresponding check@host is defined.

  // Returns: A new History object.

  init(check, host);
}

bool History::iterate_begin()
{
  // Begin iterating through the history file from the beginning.

  // Returns: true if fully successful, false otherwise.
  
  return(iterate_begin(0, 0, false));
}

bool History::iterate_begin(time_t from, time_t until)
{
  // Begin iterating through the history file from the first entry at
  // or after time <from> through any entry before (but not including)
  // time <until>.

  // Returns: true if fully successful, false otherwise, including if the
  // requested history file doesn't (yet) exist.

  return(iterate_begin(from, until, false));
}

bool History::iterate_begin_backwards()
{
  // Begin iterating through the history file from the end.

  // Returns: true if fully successful, false otherwise.
  
  return(iterate_begin(0, 0, true));
}

bool History::iterate_begin_backwards(time_t from, time_t until)
{
  // Begin iterating through the history file from the last entry
  // before (but not including) time <until> through any entry
  // beginning at time <from>.

  // Returns: true if fully successful, false otherwise, including if the
  // requested history file doesn't (yet) exist.
  
  return(iterate_begin(from, until, true));
}

void History::iterate_end()
{
  // Discontinue iterating through the history file.

  // Returns: Nothing.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "History::iterate_end()");
#endif

  if(fd)
  {
    close(fd);
    fd = -1;
  }

  unlock_history();
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "History::iterate_end()");
#endif
}

bool History::nolock()
{
  // Set this History object to not lock state files when performing read
  // operations.

  // Returns: true if locking is disabled, false otherwise.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "History::nolock()");
#endif
  
  lock = false;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "History::nolock = true");
#endif

  return(true);
}

bool History::prune(char *destination, bool desttree)
{
  // Remove all entries from the history file.  If <destination> is
  // NULL, all entries are simply removed.  If <desttree> is false, the
  // entries are appended to <destination>, prefixed with
  //  historyfilename():check@host:
  // If <desttee> is true, the entries are appended to the file
  //  destination/host/check/historyfilename.
  // Note that the historyfile is truncated, not removed.

  // Returns: true if fully successful, false otherwise.

  return(prune(0, 0, destination, desttree));
}

bool History::prune(time_t from, time_t until, char *destination,
		    bool desttree)
{
  // Remove any history entries beginning at time <from> through (but
  // not including) time <until>.  If <destination> is NULL, the
  // entries are simply deleted.  If <desttree> is false, the entries
  // are appended to <destination>, prefixed with
  //  historyfilename():check@host:
  // If <desttree> is true, the entries are appended to the file
  //  destination/host/check/historyfilename.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "History::prune(%d,%d,%s,%s)",
		  from, until, IONULL(destination), IOTF(desttree));
#endif

  // If from and until are both 0, we could optimize the performance
  // here by simply blindly copying files (or, at least not parsing
  // the dates out), but for now that will just make the code more
  // complicated.
  
  if(c && h && from >= 0 && until >= 0 && historyfile() && fd == -1)
  {
    // To begin with, we lock the history

    if(lock_history())
    {
      // Generate the name of the temporary copy of the history file.
      // We'll need it later.

      CharBuffer *thf = new CharBuffer(historyfile());
      
      // Make sure the append succeeds, too, so we don't clobber
      // the old file
      
      if(thf && thf->append(".tmp"))
      {      
	// We open the existing file for reading only, we'll copy
	// everything that needs to be saved to a new file, then
	// move it into place.
      
	fd = open(historyfile(), O_RDONLY);
	
	if(fd > -1)
	{
	  // Next, try to open the new copy of the history file

	  int tmpfd = open(thf->str(), O_WRONLY|O_CREAT, FILE_GRP_WR);

	  if(tmpfd > -1)
	  {
	    // Open our archive, if a location was provided

	    int destfd = -1;
	    
	    if(destination)
	    {
	      // We open the destination file with the same GRP_WR
	      // permissions as the regular history file to be
	      // consistent.

	      if(desttree)
	      {
		// Verify each component of the directory tree before
		// opening the file for writing.  Set umask to 002 for
		// mkdir operations.

		struct stat sb;
		mode_t u = umask(002);

		CharBuffer *cbuf = new CharBuffer(destination);

		if(cbuf)
		{
		  if(verify_directory(cbuf->str(), &sb, DIR_GRP_WR))
		  {
		    if(cbuf->append('/') && cbuf->append(h)
		       && verify_directory(cbuf->str(), &sb, DIR_GRP_WR))
		    {
		      if(cbuf->append('/') && cbuf->append(c)
			 && verify_directory(cbuf->str(), &sb, DIR_GRP_WR))
		      {
			// Finally, all directories check out OK,
			// open the archival file
			
			if(cbuf->append('/')
			   && cbuf->append(historyfilename()))
			{
			  destfd = open(cbuf->str(),
					O_WRONLY|O_APPEND|O_CREAT,
					FILE_GRP_WR);

			  if(destfd == -1)
			  {
			    wlog->warn("History::prune failed to open %s for writing (%d)",
				       destination, errno);
			  }

			}
		      }
		      else
			wlog->warn("History::prune failed to create %s",
				   cbuf->str());
		    }
		    else
		      wlog->warn("History::prune failed to create %s",
				 cbuf->str());
		  }
		  else
		    wlog->warn("History::prune failed to create %s",
			       cbuf->str());
		  
		  xdelete(cbuf);
		}
		else
		  wlog->warn("History::prune failed to allocate CharBuffer");
		
		// Restore the old umask

		umask(u);
	      }
	      else
	      {
		destfd = open(destination, O_WRONLY|O_APPEND|O_CREAT,
			      FILE_GRP_WR);

		if(destfd == -1)
		{
		  wlog->warn("History::prune failed to open %s for writing (%d)",
			     destination, errno);
		}
	      }
	    }

	    if(destfd > -1 || !destination)
	    {
	      // We're good to go.

	      HistoryRecord *hr = new HistoryRecord();

	      if(hr)
	      {
		bool done = false;

		// At this point, we assume success unless a failure
		// is noted.  Success allows the new file to be
		// swapped into place.
			
		ret = true;
	
		while(ret && !done)
		{
		  char *rl = read_line(fd);
		  
		  if(rl && strlen(rl) > 0)
		  {
		    // Determine where to write the record based on the
		    // timestamp.

		    if(hr->parse_entry(rl, true))
		    {
		      int t = hr->timestamp();

		      if(t >= from && (until == 0 || t < until))
		      {
			// This record is in the prune range, write it
			// to destfd, if it exists.

			if(destfd > -1)
			{
			  if(!desttree)
			  {
			    // Write the prefix information

			    CharBuffer *cbuf =
			      new CharBuffer(historyfilename());

			    if(cbuf)
			    {
			      cbuf->append(':');
			      cbuf->append(c);
			      cbuf->append('@');
			      cbuf->append(h);
			      cbuf->append(':');

			      if(write(destfd,
				       cbuf->str(),
				       strlen(cbuf->str()))
				 != strlen(cbuf->str()))
			      {
				wlog->warn("History::prune failed to archive entry");
				ret = false;
			      }
			      
			      xdelete(cbuf);
			    }
			    else
			      wlog->warn("History::prune failed to allocate CharBuffer");
			  }

			  // Write out the (rest of the) record.

			  if(write(destfd, rl, strlen(rl)) != strlen(rl)
			     || write(destfd, "\n", 1) != 1)
			  {
			    wlog->warn("History::prune failed to archive entry");
			    ret = false;
			  }
			}
		      }
		      else
		      {
			// Save this record.

			if(write(tmpfd, rl, strlen(rl)) != strlen(rl)
			   || write(tmpfd, "\n", 1) != 1)
			{
			  wlog->warn("History::prune failed to rewrite entry");
			  ret = false;
			}
		      }
		    }
		    else
		      ret = false;
		  }
		  else
		    done = true;  // No more records to read

		  xdelete(rl);
		}

		xdelete(hr);
	      }
	      else
		wlog->warn("History::prune failed to allocate HistoryRecord");
	    }
	    else
	      wlog->warn("History::prune could not write archive records to %s",
			 destination);

	    if(destfd != -1)
	    {
	      close(destfd);
	      destfd = -1;
	    }
	    
	    close(tmpfd);
	    tmpfd = -1;
	  }
	
	  close(fd);
	  fd = -1;

	  // Swap the new file into place if we were successful

	  if(ret)
	  {
	    if(unlink(historyfile()) == 0 &&
	       rename(thf->str(), historyfile()) == 0)
	    {
	      // Make sure the new historyfile() has the right ownerships.
	      
	      verify_file(historyfile(), FILE_GRP_WR);
	    }
	    else
	      wlog->warn("History::prune failed to rename %s to %s",
			 thf->str(), historyfile());
	  }
	  else
	  {
	    // On failure, we only clean up the temp file since we might
	    // have been appending to a preexisting destination.
	    
	    unlink(thf->str());
	  }
	}
	else
	{
	  // If the file doesn't exist at all, we return success,
	  // since there wasn't any history to prune.

	  if(access(historyfile(), F_OK)==0)
	    wlog->warn("History::prune failed to open %s", historyfile());
	  else
	    ret = true;
	}
      }
      else
	wlog->warn("History::prune failed to allocate CharBuffer");
      
      xdelete(thf);

      unlock_history();
    }
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "History::prune = %s", IOTF(ret));
#endif
  
  return(ret);
}

bool History::iterate_next_entry(HistoryRecord *hr)
{
  // Obtain the next line from the history file.  The line will be
  // passed to <hr>, which is responsible for parsing it.  On a
  // false return, <hr> may have data in it, such data should be
  // ignored.  On a false return, iteration should stop.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "History::iterate_next_entry(%d)", hr);
#endif

  if(fd > -1 && hr)
  {
    // We loop until either we find a record in the appropriate time
    // frame or we get back a 0 length line.

    bool done = false;
    
    while(!done)
    {
      char *rl = read_line(fd);

      if(rl && strlen(rl) > 0)
      {
	if(hr->parse_entry(rl))
	{
	  int t = hr->timestamp();

	  if(u == 0 || t < u)
	  {
	    if(t >= f)
	    {
	      // We're within the time range
	      
	      done = true;
	      ret = true;
	    }
	    // else we'll loop through and read another entry
	  }
	  else
	    done = true;  // We've passed the until time, so stop
	}
	else
	  done = true;  // Stop on parse error
      }
      else
	done = true;  // No more records to read

      xdelete(rl);
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "History::iterate_next_entry = %s",
		 IOTF(ret));
#endif
  
  return(ret);
}

bool History::iterate_previous_entry(HistoryRecord *hr)
{
  // Obtain the previous line from the history file.  The line will be
  // passed to <hr>, which is responsible for parsing it.  On a
  // false return, <hr> may have data in it, such data should be
  // ignored.  On a false return, iteration should stop.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "History::iterate_previous_entry(%d)", hr);
#endif

  if(fd > -1 && hr)
  {
    // We loop until either we find a record in the appropriate time
    // frame or we get back a 0 length line.

    bool done = false;

    while(!done)
    {
      char *rl = read_previous_line(fd);

      if(rl && strlen(rl) > 0)
      {
	if(hr->parse_entry(rl))
	{
	  int t = hr->timestamp();

	  if(t >= f)
	  {
	    if(u == 0 || t < u)
	    {
	      // We're within the time range

	      done = true;
	      ret = true;
	    }
	    // else we'll loop through and read another entry
	  }
	  else
	    done = true;  // We've passed the from time, so stop
	}
	else
	  done = true;  // Stop on parse error
      }
      else
	done = true;  // No more records to read

      xdelete(rl);
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "History::iterate_next_entry = %s",
		 IOTF(ret));
#endif
  
  return(ret);
}

bool History::do_record(HistoryRecord *hr)
{
  // Write the HistoryRecord <hr> to the end of the history file.
  // Locking must be enabled for this method to succeed.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "History::do_record(%d)", hr);
#endif

  if(hr && lock && historyfile())
  {
    // Obtain a lock on the file
  
    if(lock_history())
    {
      int wfd = open(historyfile(), O_WRONLY|O_APPEND|O_CREAT, FILE_GRP_WR);

      if(wfd > -1)
      {
	// Grab the string to write and write it.  hrx does not need
	// to be deleted when we're done.

	char *hrx = hr->generate_entry();
  
	if(hrx && strlen(hrx) > 0
	   && write(wfd, hrx, strlen(hrx))==strlen(hrx))
	  ret = true;
	else
	  wlog->warn("History::record failed to write record to %s",
		     historyfile());
	
	close(wfd);

	// Make sure historyfile() has the right ownerships before we
	// unlock it.  We don't report errors since in some cases
	// history files are written by regular users (eg: via sc)
	// who wouldn't have permission to change ownerships.
  
	verify_file(historyfile(), FILE_GRP_WR);
      }
      else
	wlog->warn("History::record failed to open %s", historyfile());
  
      unlock_history();
    }
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "History::do_record = %s", IOTF(ret));
#endif
  
  return(ret);
}

History::~History()
{
  // Deallocate the History object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "History::~History()");
#endif

  c = NULL;
  xdelete(h);
  xdelete(hf);
  xdelete(lf);
  lock = true;
  if(fd > -1)
    iterate_end();
  if(lockfd > -1)
    unlock_history();
  f = 0;
  u = 0;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "History::~History()");
#endif
}

char *History::historyfile()
{
  // Obtain the path to the historyfile to use.

  // Returns: A pointer to a string that should not be modified, or NULL
  // on error.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "History::historyfile()");
#endif

  if(!hf && c && h)
  {
    CharBuffer *cbuf = new CharBuffer(args->histdir());

    if(cbuf)
    {
      cbuf->append("/host/");
      cbuf->append(h);
      cbuf->append("/");
      cbuf->append(c);
      cbuf->append("/");
      cbuf->append(historyfilename());

      hf = xstrdup(cbuf->str());

      xdelete(cbuf);
    }
    else
      wlog->warn("History::historyfile failed to allocate cbuf");
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "History::historyfile = %s", IONULL(hf));
#endif
  
  return(hf);
}

void History::init(char *check, char *host)
{
  // Initializer for History constructors.

  // Returns: Nothing.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "History::init(%s,%s)",
		  IONULL(check), IONULL(host));
#endif

  c = xstrdup(check);
  h = xstrdup(host);
  hf = NULL;
  lf = NULL;
  lock = true;
  fd = -1;
  lockfd = -1;
  f = 0;
  u = 0;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "History::init()");
#endif
}

bool History::iterate_begin(time_t from, time_t until, bool backwards)
{
  // Begin iterating through the history file from the first entry at
  // or after time <from> through any entry before (but not including)
  // time <until>.  If <backwards> is true, move to the end of the file
  // to iterate via _previous_entry.

  // Returns: true if fully successful, false otherwise, including if the
  // requested history file doesn't (yet) exist.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "History::iterate_begin(%d,%d,%s)",
		  from, until, IOTF(backwards));
#endif

  if(historyfile() && fd == -1)
  {
    if(lock_history())
    {
      fd = open(historyfile(), O_RDONLY);

      if(fd > -1)
      {
	f = from;
	u = until;

	if(!backwards || lseek(fd, -1, SEEK_END) != -1)
	  ret = true;
      }
      else
	unlock_history();
    }
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "History::iterate_begin = %s", IOTF(ret));
#endif

  return(ret);
}

char *History::lockfile()
{
  // Obtain the path to the lockfile to use.

  // Returns: A pointer to a string that should not be modified, or NULL
  // on error.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "History::lockfile()");
#endif

  if(!lf && c && h)
  {
    CharBuffer *cbuf = new CharBuffer(args->histdir());

    if(cbuf)
    {
      cbuf->append("/host/");
      cbuf->append(h);
      cbuf->append("/");
      cbuf->append(c);
      cbuf->append("/lock");

      lf = xstrdup(cbuf->str());

      xdelete(cbuf);
    }
    else
      wlog->warn("History::lockfile failed to allocate cbuf");
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "History::lockfile = %s", IONULL(lf));
#endif
  
  return(lf);
}

bool History::lock_history()
{
  // Lock the history files, if locking is enabled.

  // Returns: true if a lock was established or if locking is disabled,
  // false otherwise.
  
  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "History::lock_history()");
#endif

  if(lockfd == -1)
  {
    if(lock)
    {
      lockfd = try_lock(lockfile());

      if(lockfd > -1)
	ret = true;
    }
    else
      ret = true;
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "History::lock_history = %s", IOTF(ret));
#endif
  
  return(ret);
}

void History::unlock_history()
{
  // Unlock the history files, if locking is enabled.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "History::unlock_history()");
#endif

  if(lock && lockfd > -1)
  {
    close(lockfd);
    lockfd = -1;
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "History::unlock_history()");
#endif
}
