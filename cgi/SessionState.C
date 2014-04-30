/*
 * survivor web interface session state object
 *
 * Version: $Revision: 0.10 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2004/03/02 17:34:42 $
 * MT-Level: Safe
 *
 * Copyright (c) 2002 - 2003
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: SessionState.C,v $
 * Revision 0.10  2004/03/02 17:34:42  benno
 * Use FileHandler
 * Support groups
 *
 * Revision 0.9  2003/04/09 20:14:29  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.8  2003/03/04 21:50:27  benno
 * Use toss_eol
 *
 * Revision 0.7  2003/01/27 02:56:16  benno
 * Use IONULL, IOTF, xdelete, xadelete
 *
 * Revision 0.6  2002/08/06 16:05:24  selsky
 * Remove embedded nulls in format
 *
 * Revision 0.5  2002/06/02 23:29:46  toor
 * change atoi to atol for time_t
 *
 * Revision 0.4  2002/05/22 19:04:03  selsky
 * fix format string for time_t
 *
 * Revision 0.3  2002/04/04 19:54:40  benno
 * copyright
 *
 * Revision 0.2  2002/04/02 20:14:37  benno
 * rcsify date
 *
 * Revision 0.1  2002/04/02 20:14:06  benno
 * initial revision
 *
 */

#include "cgi.H"

SessionState::SessionState()
{
  // Allocate and initialize a new SessionState object.

  // Returns: A new SessionState object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "SessionState::SessionState()");
#endif
  
  statefile = NULL;
  
  // Determine statefile, which is global to all session state, here.
  
  if(cgicf && cgicf->statedir())
  {
    statefile = new char[strlen(cgicf->statedir()) + 13];

    if(statefile)
      sprintf(statefile, "%s/session.sws", cgicf->statedir());
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "SessionState::SessionState()");
#endif  
}

bool SessionState::add(Session *s)
{
  // Add the Session <s> to the session records.  Additionally, this
  // method will clear out any expired sessions from the record.

  // Returns: true if the session state was successfully rewritten,
  // false otherwise.

  return(rewrite(s, NULL));
}

Session *SessionState::find(char *sid)
{
  // Find a Session record identified by <sid>.  Expired sessions will
  // not be returned.

  // Returns: A newly allocated Session object that should be deleted
  // when no longer required if found, NULL otherwise.

  Session *r = NULL;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "SessionState::find(%s)", IONULL(sid));
#endif
  
  if(sid && statefile)
  {
    int locknum = lock_misc_state(statefile);

    if(locknum > -1)
    {
      FileHandler *in = new FileHandler();

      if(in && in->open_read(statefile))
      {
	char *inl;
	
	while((inl = in->read_line()) != NULL)
	{
	  char *inbuf = xstrdup(inl);

	  if(inbuf)
	  {
	    char *sx = NULL;
	    char *ux = NULL;
	    char *hx = NULL;
	    char *ex = NULL;
	    char *gx = NULL;
	    char *lasts;
	    
	    sx = strtok_r(inbuf, ":", &lasts);
	  
	    if(sx && strcmp(sid,sx)==0)
	    {
	      ux = strtok_r(NULL, ":", &lasts);
	      
	      if(ux)
	      {
		hx = strtok_r(NULL, ":", &lasts);
		
		if(hx)
		{
		  // If <hx> is empty, consider that to mean no hostid.
		  
		  if(strlen(hx)==0)
		    hx = NULL;
		  
		  ex = strtok_r(NULL, ":", &lasts);
		  
		  if(ex)
		  {
		    List *groups = NULL;
		    
		    gx = strtok_r(NULL, "\n", &lasts);

		    if(gx)
		    {
		      // Process groups if there are any

		      groups = new List();

		      if(groups)
		      {
			for(char *gxx = strtok_r(gx, " ", &lasts);
			    gxx != NULL;
			    gxx = strtok_r(NULL, " ", &lasts))
			  groups->add(gxx);
		      }
		      else
			wlog->warn("SessionState::find failed to allocate List");
		    }

		    // We now have all the bits out of the line, allocate a
		    // session object and break the loop, but make sure the
		    // session is still valid.

		    struct timeval tv;

		    if(gettimeofday(&tv, NULL)==0 && (tv.tv_sec <= atol(ex)))
		      r = new Session(sx, ux, groups, hx, atol(ex));
			
		    // Session copies groups
		    xdelete(groups);
		  }
		}
	      }
	    }

	    xdelete(inbuf);
	  }
	  else
	    wlog->warn("SessionState::find failed to copy string");
		    
	  if(r)
	    break;
	}
      }
      // If we fail to read the file, it may simply be that there have
      // not yet been any sessions.  In any event, no session is found.

      if(!in)
	wlog->warn("SessionState::find failed to allocate FileHandler");
      else
	xdelete(in);
      
      unlock_state(locknum);
    }
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "SessionState::find = %d", r);
#endif

  return(r);
}

bool SessionState::remove(char *sid)
{
  // Remove a session identified by <sid>, if present.  Additionally,
  // this method will remove any expired sessions.

  // Returns: true if the session state was successfully rewritten,
  // false otherwise.

  return(rewrite(NULL, sid));
}

SessionState::~SessionState()
{
  // Deallocate the SessionState object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "SessionState::~SessionState()");
#endif
  
  xdelete(statefile);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "SessionState::~SessionState()");
#endif
}

bool SessionState::rewrite(Session *add, char *remove)
{
  // Rewrite the session file, removing any expired sessions.
  // Optionally, add the Session <add> and remove the session
  // with session ID <remove>.

  // Returns: true if fully successful, false otherwise.
  
  bool r = false;
  struct timeval tv;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "SessionState::rewrite(%d,%s)",
		  add, IONULL(remove));
#endif

  // gettimeofday must succeed in order to proceed.

  if(statefile && gettimeofday(&tv, NULL)==0)
  {
    int locknum = lock_misc_state(statefile);

    if(locknum > -1)
    {
      // First read each entry from statefile and write valid entries
      // to a new statefile.  Append <s> to the new file, remove the
      // old file, move the new file into place, and finally release
      // the state lock.

      FileHandler *in = new FileHandler();

      if(in && (in->open_read(statefile)
		// We may not have written anything yet
		|| (access(statefile, F_OK)==-1 && errno == ENOENT)))
      {
	char *ofile = xstrdup(statefile);

	if(ofile)
	{
	  ofile = xstrcat(ofile, ".t");

	  if(ofile)
	  {
	    FileHandler *out = new FileHandler();

	    if(out && out->open_write(ofile, S_IRUSR | S_IWUSR))
	    {
	      for(char *inl = in->read_line();inl != NULL;
		  inl = in->read_line())
	      {
		// A "valid" record has four colons.  A v0.9.3 and
		// earlier valid record had only three, we'll simply
		// ignore those lines if we see them.

		char *p = inl;

		for(int i = 0;i < 3;i++)
		{
		  if(p)
		    p = strchr(p, ':');
		  
		  if(p)
		    p++;
		}

		if(p)
		{
		  // We now point to the expiration time, which we can
		  // pass directly to atol since the following : will
		  // cause the number parsing to stop.  It must be
		  // greater than the current time or we skip this
		  // line.  Additionally, if <remove> was provided and
		  // <remove> is the initial substring of inbuf, skip
		  // this line.  (This is easier and no less effective
		  // than strtok'ing on :, since session IDs are all
		  // the same length.)
		  
		  if(*p != '\0' && (atol(p) > tv.tv_sec)
		     && (!remove ||
			 (strlen(inl) > strlen(remove)
			  && strncmp(inl, remove, strlen(remove))!=0)))
		  {
		    out->append(inl);
		    out->append('\n');
		  }
		}
	      }

	      // Next, add the session if requested.  Don't bother if
	      // we're missing any information.  Groups cannot have
	      // spaces in them according to the cgi.cf parser, so we
	      // use spaces as group delimiters.

	      if(add && add->sid() && add->user() && add->expiry() > 0)
	      {
		out->append(add->sid());
		out->append(':');
		out->append(add->user());
		out->append(':');
		out->append((char *)(add->hostid() ? add->hostid() : ""));
		out->append(':');
		out->append(add->expiry());
		out->append(':');

		if(add->groups())
		{
		  for(int i = 0;i < add->groups()->entries();i++)
		  {
		    if(i > 0)
		      out->append(' ');
		    
		    out->append(add->groups()->retrieve(i));
		  }
		}

		out->append('\n');
	      }

	      // If we make it this far, rename the files.  The old
	      // files won't actually unlink until closed by the
	      // deletion of in and out.

	      unlink(statefile);
	      
	      if(rename(ofile, statefile)==0)
	      {
		verify_file(statefile, FILE_OTH_NO);
		r = true;
	      }
	      else
		unlink(ofile);  // Try to clean up after ourselves.
	    }
	    else
	      wlog->warn("SessionState::rewrite failed to open %s", ofile);

	    xdelete(out);
	  }
	  else
	    wlog->warn("SessionState::rewrite failed to cat string");
	}
	else
	  wlog->warn("SessionState::rewrite failed to dup string");

	xdelete(ofile);
      }
      else
	wlog->warn("SessionState::rewrite failed to open %s", statefile);

      xdelete(in);
      
      unlock_state(locknum);
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "SessionState::rewrite = %s", IOTF(r));
#endif
  
  return(r);
}
