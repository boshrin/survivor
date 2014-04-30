/*
 * CallListState.C: Object to manage state information for Alerts
 *
 * Version: $Revision: 0.21 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2006/10/15 13:51:58 $
 * MT-Level: Unsafe
 *
 * Copyright (c) 2002 - 2006
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: CallListState.C,v $
 * Revision 0.21  2006/10/15 13:51:58  benno
 * Unlink file before rewriting
 *
 * Revision 0.20  2006/09/23 02:46:23  benno
 * Truncate when writing out substitutions
 *
 * Revision 0.19  2005/09/26 13:46:30  benno
 * Add checksubs(asof)
 * Add notenotify(person,via,address)
 * Add oncall()
 *
 * Revision 0.18  2004/06/20 01:05:07  benno
 * prunesubs supports time specification
 * toss unusued removesub
 *
 * Revision 0.17  2004/06/12 00:59:21  benno
 * State is XML based
 *
 * Revision 0.16  2003/04/09 20:23:45  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.15  2003/04/04 04:33:29  benno
 * Use Debugger
 *
 * Revision 0.14  2003/03/04 17:57:23  benno
 * Bump copyright
 * Use toss_eol
 *
 * Revision 0.13  2003/01/23 22:46:24  benno
 * Use IONULL, IOTF
 * Replace deletesubarray with xadelete
 *
 * Revision 0.12  2003/01/01 02:05:18  benno
 * Use xdelete
 * Clean up some stuff
 *
 * Revision 0.11  2002/10/25 22:54:35  benno
 * new state file format for person based call lists
 *
 * Revision 0.10  2002/09/05 21:57:12  benno
 * use try_fopen
 *
 * Revision 0.9  2002/08/06 19:42:29  selsky
 * Remove embedded nulls in format
 *
 * Revision 0.8  2002/05/31 21:37:32  benno
 * unlink before verify_file
 *
 * Revision 0.7  2002/05/22 19:03:11  selsky
 * fix format string for time_t
 *
 * Revision 0.6  2002/04/12 15:04:14  benno
 * use BUFSIZE
 *
 * Revision 0.5  2002/04/04 20:07:04  benno
 * copyright
 *
 * Revision 0.4  2002/04/03 17:49:46  benno
 * rcsify date
 *
 * Revision 0.3  2002/04/03 17:49:33  benno
 * Add support for substitutions
 *
 * Revision 0.2  2002/04/03 17:48:42  benno
 * Use verify_file
 *
 * Revision 0.1  2002/04/03 17:44:21  benno
 * initial revision
 *
 */

#include "survivor.H"

CallListState::CallListState(CallList *calllist)
{
  // Allocate a new CallListState object, which manages state information for
  // the CallList <calllist>.

  // Returns: A new, initialized CallListState.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CallListState::CallListState(%d)", calllist);
#endif

  cl = calllist;
  stf = NULL;
  sf = NULL;
  uncache(true);
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CallListState::CallListState()");
#endif
}

bool CallListState::addsub(Substitution *sub)
{
  // Append <sub> to the substitution file for this calllist.

  // Returns: true if fully successful, false otherwise.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "CallListState::addsub(%d)", sub);
#endif
  
  bool ret = false;
  
  if(cl && sub && sub->newname() && sub->oldname() && substitutionfile())
  {
    int locknum = lock_calllist_state(cl->name());

    if(locknum > -1)
    {
      Array<Substitution> *subs = read_substitution_status(false);

      if(subs)
      {
        // Duplicate Substitution to make cleanup easier

	Substitution *newsub = new Substitution(sub->begins(),
						sub->ends(),
						sub->newname(),
						sub->oldname());

	if(newsub)
	{
	  if(subs->add(newsub))
	  {
	    // Write out the new set

	    ret = write_substitutions(subs);
	  }
	  else
	  {
	    xdelete(newsub);
	  }
	}
	else
	  wlog->warn("CallListState::addsub failed to duplicate Substitution");

	// Toss the array
	xadelete(subs, Substitution);
      }

      unlock_state(locknum);
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "CallListState::addsub = %s", IOTF(ret));
#endif
  
  return(ret);
}

char *CallListState::checksubs(char *name, time_t asof)
{
  // Check to see if there are any substitutions for <name> in the
  // substitution list as of time <asof>.  If more than one matching
  // substitution is present, only the first one is returned.

  // Returns: The replacement name, if a replacement is found, or NULL.
  // The returned name is newly allocated and must be delete'd when no
  // longer required.

  char *r = NULL;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "CallListState::checksubs(%s,%d)",
		  IONULL(name), asof);
#endif

  // Read the substitution list
  
  Array<Substitution> *sarray = NULL;
    
  if(cl)
  {
    int locknum = lock_calllist_state(cl->name());
      
    if(locknum > -1)
    {
      sarray = read_substitution_status(false);
	
      if(sarray)
      {
	for(int i = 0;i < sarray->entries();i++)
	{
	  Substitution *s = sarray->retrieve(i);
	  
	  if(s && asof >= s->begins() && asof <= s->ends()
	     && s->oldname() && strcmp(s->oldname(), name)==0)
	  {
	    // Match, return this name

	    r = xstrdup(s->newname());
	    break;
	  }
	}

	xadelete(sarray, Substitution);
      }
	
      // Unlock the state after we determine our answer to maintain
      // consistency as long as possible.
      unlock_state(locknum);
    }
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "CallListState::checksubs = %s", IONULL(r));
#endif

  return(r);
}

char *CallListState::lastnotify()
{
  // Determine the name of the last Person notified from a non-broadcast
  // calllist.

  // Returns: A character string that remains valid until the
  // CallListState is deleted, or NULL if no address is found or if
  // unable to access the state information.

  char *ret = NULL;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CallListState::lastnotify()");
#endif

  if(!dcache)
    read_status();

  if(dcache)
    ret = dcache->last_notified_person();

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CallListState::lastnotify = %s", IONULL(ret));
#endif
  
  return(ret);
}

char *CallListState::lastnotifyaddress()
{
  // Determine the last address notified from a non-broadcast calllist.

  // Returns: A character string that remains valid until the
  // CallListState is deleted, or NULL if no address is found or if
  // unable to access the state information.

  char *ret = NULL;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CallListState::lastnotifyaddress()");
#endif

  if(!dcache)
    read_status();

  if(dcache)
    ret = dcache->last_notified_address();

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CallListState::lastnotifyaddress = %s",
		 IONULL(ret));
#endif
  
  return(ret);
}

char *CallListState::lastnotifyvia()
{
  // Determine the module used to notify the last address notified from a
  // non-broadcast calllist.

  // Returns: A character string that remains valid until the
  // CallListState is deleted, or NULL if no address is found or if
  // unable to access the state information.

  char *ret = NULL;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CallListState::lastnotifyvia()");
#endif

  if(!dcache)
    read_status();

  if(dcache)
    ret = dcache->last_notified_via();

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CallListState::lastnotifyvia = %s",
		 IONULL(ret));
#endif
  
  return(ret);
}

time_t CallListState::lastrotate()
{
  // Determine the last time the (rotating) CallList was rotated.

  // Returns: The unix time of the last rotation, or 0 if the list has not
  // rotated or if unable to access the state information.

  time_t ret = 0;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CallListState::lastrotate()");
#endif

  if(!dcache)
    read_status();

  if(dcache)
    ret = dcache->last_rotated();
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CallListState::lastrotate = %d", ret);
#endif
  
  return(ret);
}

bool CallListState::notenotify(char *person, char *via, char *address)
{
  // Update the last notified information for the CallList to <person>,
  // <via>, and <address>.  This method is intended for simple calllists.

  // Returns: true if fully successful, false otherwise.

  return(notenotify(person, via, address, NULL));
}

bool CallListState::notenotify(char *person, char *via, char *address,
			       char *oncall)
{
  // Update the last notified information for the CallList to <person>,
  // <via>, <address>, and <oncall>, where <oncall> is the person now
  // oncall for rotating calllists or NULL for simple calllists.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CallListState::notenotify(%s,%s,%s,%s)",
		  IONULL(person), IONULL(via), IONULL(address),
		  IONULL(oncall));
#endif

  if(cl && person && via && address)
  {
    SurvivorXML *sxml = new SurvivorXML();
    
    if(sxml)
    {
      int locknum = lock_calllist_state(cl->name());
      
      if(locknum > -1)
      {
	// We need the old state to determine if we've rotated

	if(!dcache)
	  read_status();
	
	CallListStateData *oldcache = dcache;
	
	// Clear dcache, we're going to recreate it

	dcache = NULL;

	// Unlink in case the user calling this method isn't the same
	// as the owner of the file.

	unlink(statusfile());
	
	int fdout = try_open(statusfile(), O_WRONLY | O_CREAT, FILE_GRP_WR);

	if(fdout > -1)
	{
	  if(oncall)
	  {
	    struct timeval tv;

	    tv.tv_sec = 0;

	    char *prevoc = (oldcache ? oldcache->oncall_person() : NULL);

	    if(prevoc && strcmp(oncall, prevoc)==0)
	    {
	      // Either there was no rotation or we looped a full interval
	      // (ie: skipping exactly all other people on the list), which
	      // if effectively the same as no rotation
	      
	      tv.tv_sec = oldcache->last_rotated();
	    }
	    else
	    {
	      if(gettimeofday(&tv, NULL) != 0)
		wlog->warn("CallListState::notenotify failed to gettimeofday");
	    }

	    dcache = new CallListStateData(person, address, via, tv.tv_sec,
					   oncall);
	  }
	  else
	    dcache = new CallListStateData(person, address, via);

	  if(dcache)
	  {
	    if(!sxml->generate(fdout, dcache))
	    {
	      xdelete(dcache);
			    
	      wlog->warn("CallListState::notenotify failed to write state for %s", cl->name());
	    }
	    else
	      ret = true;
	  }
	  else
	    wlog->warn("CallListState::notenotify failed to allocate CallListStateData");

	  close(fdout);

	  if(!verify_file(statusfile(), FILE_GRP_WR))
	    wlog->warn("CallListState::notenotify failed to reset permissions on %s",
		       statusfile());
	}
	else
	  wlog->warn("CallListState::notenotify failed to open %s",
		     statusfile());

	// We can toss oldcache now
	xdelete(oldcache);
	
	unlock_state(locknum);
      }

      xdelete(sxml);
    }
    else
      wlog->warn("CallListState::notenotify failed to allocate SurvivorXML");
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CallListState::notenotify = %s", IOTF(ret));
#endif
  
  return(ret);
}

char *CallListState::oncall()
{
  // Determine who is currently on call for a rotating calllist.

  // Returns: A character string that remains valid until the
  // CallListState is deleted, or NULL if no address is found or if
  // unable to access the state information.

  char *ret = NULL;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CallListState::oncall()");
#endif

  if(!dcache)
    read_status();

  if(dcache)
    ret = dcache->oncall_person();

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CallListState::oncall = %s",
		 IONULL(ret));
#endif
  
  return(ret);
}

bool CallListState::prunesubs(time_t fromtime, time_t untiltime)
{
  // Remove any substitution whose times fall fully within <fromtime>
  // and <untiltime>.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "CallListState::prunesubs(%d,%d)",
		  fromtime, untiltime);
#endif

  // First, read the substitution list
  
  Array<Substitution> *sarray = NULL;
  
  if(cl)
  {
    int locknum = lock_calllist_state(cl->name());
    
    if(locknum > -1)
    {
      sarray = read_substitution_status(false);
      
      if(sarray)
      {
	// Iterate backwards through the array so as not to mess with
	// the indexing as we delete entries.  Since order is not
	// important here, this is OK.

	for(int i = sarray->entries() - 1;i >= 0;i--)
	{
	  Substitution *s = sarray->retrieve(i);

	  // We use <= and >= so it is possible to use the same times
	  // in the -f -u arguments for clsub and clunsub.
	  
	  if(s && (fromtime <= s->begins()) && (untiltime >= s->ends()))
	  {
	    // Toss this entry

	    delete s;
	    sarray->remove(i);
	  }
	}

	// Now write out the list
	
	if(write_substitutions(sarray))
	  ret = true;

	xadelete(sarray, Substitution);
      }

      // Unlock the state after we are finished
      unlock_state(locknum);
    }
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "CallListState::prunesubs = %s", IOTF(ret));
#endif

  return(ret);
}

Array<Substitution> *CallListState::readsubs()
{
  // Obtain an Array of all Substitutions for this CallList.  The List
  // may change by the time it has been returned, and should be considered
  // advisory only.  It will be an accurate representation of the list
  // of substitutions at the time the list was read.

  // Returns: An Array of Substitutions, which may be deleted with
  // xadelete, or NULL on error.

  return(read_substitution_status(true));
}

CallListState::~CallListState()
{
  // Deallocate the CallListState object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CallListState::~CallListState()");
#endif
  
  cl = NULL;
  xdelete(stf);
  xdelete(sf);
  uncache(false);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CallListState::~CallListState()");
#endif
}

bool CallListState::read_status()
{
  // Read the CallList status from lastnotifyfile() and lastrotatefile()
  // and cache the results.

  // Returns: true if fully successful, false otherwise, including if
  // there is no status to cache.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CallListState::read_status()");
#endif

  if(cl && cl->name() && statusfile())
  {
    int locknum = lock_calllist_state(cl->name());
    
    if(locknum > -1)
    {
      // Toss anything old
      uncache(false);

      // Let the XML Parser do the hard work, but we have to open the
      // file first

      int fdin = try_open(statusfile(), O_RDONLY, 0);

      if(fdin > -1)
      {
	SurvivorXML *sxml = new SurvivorXML();

	if(sxml)
	{
	  dcache = sxml->parse_callliststatedata(fdin);

	  if(dcache)
	    ret = true;
	  
	  xdelete(sxml);
	}
	else
	  wlog->warn("CallListState::read_status() failed to allocate SurvivorXML");

	close(fdin);
      }

      unlock_state(locknum);
    }
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CallListState::read_status() = %s", IOTF(ret));
#endif
  
  return(ret);
}

Array<Substitution> *CallListState::read_substitution_status(bool dolock)
{
  // Read the CallList substiution status from substitutionfile().

  // Returns: An Array of Substitutions that should be deleted
  // (both the Array and the Substitutions) when no longer required,
  // or NULL on error.

  Array<Substitution> *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CallListState::read_substitution_status(%s)",
		  IOTF(dolock));
#endif

  if(cl && cl->name() && substitutionfile())
  {
    int locknum = -1;

    if(dolock)
      locknum = lock_calllist_state(cl->name());

    if(!dolock || locknum > -1)
    {
      // Let the XML Parser do the hard work, but we have to open the
      // file first

      int fdin = try_open(substitutionfile(), O_RDONLY, 0);

      if(fdin > -1)
      {
	SurvivorXML *sxml = new SurvivorXML();

	if(sxml)
	{
	  ret = sxml->parse_substitutions(fdin);

	  xdelete(sxml);
	}
	else
	  wlog->warn("CallListState::read_substitution_status() failed to allocate SurvivorXML");

	close(fdin);
      }
      else
      {
	// If the file doesn't exist, there aren't any substitutions to
	// read, so return an empty set.

	if(access(substitutionfile(), F_OK)==-1 && errno == ENOENT)
	  ret = new Array<Substitution>();
      }

      unlock_state(locknum);
    }
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC,
		 "CallListState::read_substitution_status() = %d",
		 ret);
#endif
  
  return(ret);
}

char *CallListState::statusfile()
{
  // Obtain or generate the filename containing any the calllist state.

  // Returns: A pointer to the filename, or NULL on error.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CallListState::statusfile()");
#endif

  if(!stf && cl)
  {
    stf = new char[strlen(args->statedir()) + strlen(cl->name()) + 26];
    
    if(stf)
      sprintf(stf, "%s/calllist/%s/callliststatus", args->statedir(),
	      cl->name());
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CallListState::substitutionfile = %s",
		 IONULL(stf));
#endif

  return(stf);
}

char *CallListState::substitutionfile()
{
  // Obtain or generate the filename containing any substitutions for this
  // calllist.

  // Returns: A pointer to the filename, or NULL on error.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CallListState::substitutionfile()");
#endif

  if(!sf && cl)
  {
    sf = new char[strlen(args->statedir()) + strlen(cl->name()) + 25];
    
    if(sf)
      sprintf(sf, "%s/calllist/%s/substitutions", args->statedir(),
	      cl->name());
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CallListState::substitutionfile = %s",
		  IONULL(sf));
#endif

  return(sf);
}

void CallListState::uncache(bool init)
{
  // Clear any cached results.  <init> should only be true when called
  // by the constructor.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CallListState::uncache(%s)", IOTF(init));
#endif
  
  if(!init)
  {
    if(dcache)
      delete dcache;
  }

  dcache = NULL;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CallListState::uncache()");
#endif
}

bool CallListState::write_substitutions(Array<Substitution> *sarray)
{
  // Write the list of substitutions <sarray>.  Any NULL entries are
  // ignored.  It is assumed that a lock has already been obtained on
  // the state directory.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "CallListState::write_substitutions(%d)",
		  sarray);
#endif
  
  if(sarray && substitutionfile())
  {
    SurvivorXML *sxml = new SurvivorXML();
    
    if(sxml)
    {
      // Unlink statusfile to clear permissions and owner,
      // but don't bother checking for errors since it may
      // not exist.
      
      unlink(substitutionfile());
      
      int fdout = try_open(substitutionfile(), O_WRONLY | O_CREAT | O_TRUNC,
			   FILE_GRP_WR);

      if(fdout > -1)
      {      
	if(sxml->generate(fdout, sarray))
	  ret = true;

	close(fdout);

	if(!verify_file(substitutionfile(), FILE_GRP_WR))
	  wlog->warn("CallListState::write_substitutions failed to reset permissions on %s",
		     substitutionfile());
      }

      xdelete(sxml);
    }
    else
      wlog->warn("CallListState::write_substitutions failed to allocate SurvivorXML");
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "CallListState::write_substitutions = %s",
		 IOTF(ret));
#endif

  return(ret);
}
