/*
 * CallList.C: An object to hold call list definitions.
 *
 * Version: $Revision: 0.19 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2006/10/10 01:43:00 $
 * MT-Level: Unsafe
 *
 * Copyright (c) 2002 - 2006
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: CallList.C,v $
 * Revision 0.19  2006/10/10 01:43:00  benno
 * Fix aliases (bug #194)
 *
 * Revision 0.18  2005/09/26 13:52:12  benno
 * Add notify with dosubs option, make all notify() based on lastoncall
 * rather than last notify
 * Calculate substitutions based on asof
 *
 * Revision 0.17  2005/08/02 17:41:24  benno
 * Fix bug computing substitutions
 *
 * Revision 0.16  2005/04/09 02:35:05  benno
 * Fix bug where rotating calllist starts at second position if
 * active person is removed
 *
 * Revision 0.15  2003/05/29 00:35:29  benno
 * Changes for AlertModule
 *
 * Revision 0.14  2003/04/13 20:01:19  benno
 * Fix incorrect debug levels
 *
 * Revision 0.13  2003/04/09 20:23:45  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.12  2003/04/04 04:14:31  benno
 * Use Debugger
 *
 * Revision 0.11  2003/03/04 17:57:07  benno
 * Bump copyright
 *
 * Revision 0.10  2003/01/23 22:42:21  benno
 * Use IONULL, IOTF, xdelete, and xadelete
 *
 * Revision 0.9  2002/10/25 22:56:08  benno
 * use persons
 *
 * Revision 0.8  2002/10/21 20:47:52  benno
 * add support for named args
 *
 * Revision 0.7  2002/04/04 20:06:53  benno
 * copyright
 *
 * Revision 0.6  2002/04/03 17:43:05  benno
 * rcsify date
 *
 * Revision 0.5  2002/04/03 17:42:27  benno
 * Add members()
 *
 * Revision 0.4  2002/04/03 17:41:47  benno
 * Add support for substitutions
 *
 * Revision 0.3  2002/04/03 17:41:11  benno
 * Move gettime and getday to utils.C
 *
 * Revision 0.2  2002/04/03 17:40:40  benno
 * Specify alert module here
 * Alias CallLists
 *
 * Revision 0.1  2002/04/03 17:39:45  benno
 * initial revision
 *
 */

#include "survivor.H"

CallList::CallList(char *name, Array<Person> *members, bool broadcast,
		   AlertModule *via)
{
  // Allocate and initialize a new CallList object named <name>.  <members>
  // is an Array of Persons in the CallList, which is maintained and deleted by
  // this object.  If <broadcast> is true, this is a Broadcast CallList,
  // otherwise it is a Simple CallList.  <via> indicates the alert module
  // to use for notification.

  // Returns: A new CallList object.

  init(name, NULL, members, broadcast, NULL, via);
}

CallList::CallList(char *name, Array<Person> *members, char *rotates,
		   AlertModule *via)
{
  // Allocate and initialize a new CallList object named <name>.  <members>
  // is an Array of Persons in the CallList, which is maintained and deleted by
  // this object.  <rotates> indicates the name of the Schedule (not yet
  // defined when this is typically called) that specifies when to rotate.
  // <via> indicates the alert module to use for notification.

  // Returns: A new CallList object.

  init(name, NULL, members, false, rotates, via);
}

CallList::CallList(char *name, CallList *cl, AlertModule *via)
{
  // Allocate and initialize a new CallList object named <name> that is an
  // alias for the CallList <cl>.  <via> indicates the alert module to use
  // for notification.

  // Returns: A new CallList object.

  init(name, cl, NULL, false, NULL, via);
}

calllist_t CallList::listtype()
{
  // Determine what type of CallList this is, as established when the CallList
  // was created.  If this is an alias list, the list type of the underlying
  // CallList will be returned.

  // Returns: The type of CallList.

  calllist_t r = ltype;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CallList::listtype()");
#endif

  if(alias)
    r = alias->listtype();
      
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CallList::listtype = %d", r);
#endif
  
  return(r);
}

bool CallList::member(char *name)
{
  // Determine if <name> is a member of this CallList.  This method applies
  // to simple and rotating CallLists only.

  // Returns: true if <name> is a member, false otherwise.

  bool r = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CallList::member(%s)", IONULL(name));
#endif

  if(alias)
    return(alias->member(name));
  else
  {
    if(name && memberlist)
    {
      for(int i = 0;i < memberlist->entries();i++)
      {
	Person *p = memberlist->retrieve(i);
      
	if(p && p->name() && strcmp(name, p->name())==0)
	{  
#if defined(DEBUG)
	  dlog->log_exit(DEBUG_MINTRC, "CallList::member = true");
#endif

	  return(true);
	}
      }
    }
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CallList::member = false");
#endif

  return(false);
}

Array<Person> *CallList::members()
{
  // Obtain the list of members of the CallList.  This method is intended
  // for the use of the CGI only.

  // Returns: A pointer to an Array that should not be modified in
  // any way, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CallList::members()");
  dlog->log_exit(DEBUG_MINTRC, "CallList::members = %d", memberlist);
#endif

  if(alias)
    return(alias->members());
  else
    return(memberlist);
}

AlertModule *CallList::module()
{
  // Obtain the AlertModule used to notify this CallList.

  // Returns: A point to an AlertModule object that should not be
  // modified, or NULL.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CallList::module()");
  dlog->log_exit(DEBUG_MINTRC, "CallList::module = %d", alertvia);
#endif
  
  return(alertvia);
}

char *CallList::name()
{
  // Obtain the name of this CallList.  If this is an alias, the name of
  // the alias is returned.

  // Returns: The CallList name, or NULL.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CallList::name()");
  dlog->log_exit(DEBUG_MINTRC, "CallList::name = %s", IONULL(listname));
#endif
  
  return(listname);
}

char *CallList::notify()
{
  // For broadcast CallLists, obtain a string consisting of comma
  // separated addresses to be notified.

  // Returns: An internally maintained string which should not be modified,
  // or NULL.

  char *r = NULL;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CallList::notify()");
#endif

  if(alias)
    r = alias->notify();
  else
  {
    if(memberlist && (ltype == broadcast_list) && alertvia && alertvia->name())
    {
      if(bcache.empty())
      {
	// Look up all the addresses and store them in a comma separated
	// string
	
	for(int i = 0;i < memberlist->entries();i++)
	{
	  Person *p = memberlist->retrieve(i);

	  if(p && p->find_address(alertvia->name()))
	  {
	    if(!bcache.empty())
	      bcache += ",";

	    bcache += p->find_address(alertvia->name());
	  }
	}
      }
      
      r = (char *)bcache.c_str();
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CallList::notify = %s", IONULL(r));
#endif
  
  return(r);
}

Person *CallList::notify(char *lastnotify)
{
  // For simple CallLists, determine the next address to be notified,
  // given that <lastnotify> is the name of the most recent person
  // notified.  If <lastnotify> is not found in this CallList or is
  // null, the first person on this list will be returned.

  // Returns: A Person that should not be modified or NULL.

  Person *r = NULL;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CallList::notify(%s)", IONULL(lastnotify));
#endif

  if(alias)
    r = alias->notify(lastnotify);
  else
  {
    if(memberlist && memberlist->entries() > 0)
    {
      // First, find the Person who should be notified.  Don't verify
      // the module against the last notified person, since the
      // configuration may have changed since the Person was notified.

      if(lastnotify)
      {
	// If lastindex is the last item in addresslist, r will be NULL when
	// we try to retrieve the one after it.

	for(int i = 0;i < memberlist->entries();i++)
	{
	  Person *px = memberlist->retrieve(i);

	  if(px && px->name() && strcmp(px->name(), lastnotify)==0)
	  {
	    // px is the last notified, retrieve the next Person
	    
	    r = memberlist->retrieve(i+1);
	    break;
	  }
	}
      }
      
      if(!r)
      {
	// Get the first entry because either lastnotify wasn't specified,
	// wasn't found, or was the last item in the list.
	
	r = memberlist->retrieve(0);
      }

      if(r)
      {
	// Check for calllist substitutions.  The configuration file
	// parser should prevent us from trying to retrieve an address
	// for a person via a module that isn't configured.

	struct timeval tv;
 
	if(gettimeofday(&tv, NULL)==0)
	{
	  Person *sub = substitute(r, tv.tv_sec);
 
	  if(sub)
	    r = sub;
	}
	else
	  wlog->warn("CallList::notify unable to get time of day");
      }
    }
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CallList::notify = %d", r);
#endif

  return(r);
}

Person *CallList::notify(char *lastoncall, time_t lastrotate, bool dosubs)
{
  // For rotating CallLists, determine the Person to be notified,
  // given that <lastrotate> was the last time the last rotated, and
  // <lastoncall> was the last person rotated to.  If <dosubs> is
  // true, calculate substitutions.

  // Returns: A Person that should not be modified or NULL.

  struct timeval tv;

  if(alias)
    return(alias->notify(lastoncall, lastrotate, dosubs));
  else
  {
    if(gettimeofday(&tv, NULL)==0)
      return(notify(lastoncall, lastrotate, tv.tv_sec, dosubs));
    else
      wlog->warn("CallList::notify unable to get time of day");
  }
}

Person *CallList::notify(char *lastoncall, time_t lastrotate, time_t asof)
{
  // For rotating CallLists, determine the Person to be notified,
  // given that <lastrotate> was the last time the list rotated, and
  // <lastoncall> was the last person rotated to.  Use <asof> as the
  // time to compute the rotation at, instead of "now".

  // Returns: A Person that should not be modified or NULL.

  return(notify(lastoncall, lastrotate, asof, true));
}

Person *CallList::notify(char *lastoncall, time_t lastrotate, time_t asof,
			 bool dosubs)
{
  // For rotating CallLists, determine the Person to be notified,
  // given that <lastrotate> was the last time the list rotated, and
  // <lastoncall> was the last person rotated to.  Use <asof> as the
  // time to compute the rotation at, instead of "now".  If <dosubs>
  // is true, calculate substitutions.

  // Returns: A Person that should not be modified or NULL.

  Person *r = NULL;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CallList::notify(%s,%d,%d,%s)",
		   IONULL(lastoncall), lastrotate, asof, IOTF(dosubs));
#endif

  if(alias)
    r = alias->notify(lastoncall, lastrotate, asof, dosubs);
  else
  {
    if(memberlist && memberlist->entries() > 0)
    {
      int lastindex = -1;  // -1 indicates last notified not found
      
      // If lastoncall is empty, or lastoncall can't be found in the list,
      // we start at position 0 and continue from there.  Note that if an
      // address is removed while it is active, this will force the list to
      // start over since the last position cannot be found.
      
      if(lastoncall)
      {
	for(int i = 0;i < memberlist->entries();i++)
	{
	  Person *px = memberlist->retrieve(i);

	  if(px && px->name() && strcmp(px->name(), lastoncall)==0)
	  {
	    lastindex = i;
	    break;
	  }
	}
      }

      if(lastrotate > 0 && rotsched && lastindex > -1)
      {
	// To compute who is on call, we determine how many rotation periods
	// have passed since the last rotation, and skip forward that many
	// entries in the list.  For example, if we rotate daily at 12:30,
	// the last rotation was two days ago at 13:30, and it is now 11:30,
	// then we move forward only one because all we missed was
	// yesterday's rotation.

	// If we rotate on more than one schedule, we simply count the
	// number of periods missed on each schedule and add them together.
	// For example, if we rotate on Mondays and Thursdays at 12:00,
	// the last rotation was last Thursday (eight days ago) at 12:00,
	// and it is now Friday at 13:45, then we move forward once for
	// Monday and once more for Thursday.

	// We skip this entirely if we were unable to find the current
	// person on call.  In that case, we want to start from the
	// beginning regardless of how much time has passed since the
	// last rotation.  (We don't have an anchor to compute the
	// number of entries to skip.)

	int advances = 0;

	for(int i = 0;i < rotsched->entries();i++)
	{
	  Schedule *s = rotsched->retrieve(i);

	  if(s && s->at())
	    advances += s->intervals(lastrotate, asof);
	  else
	    wlog->warn("Bad retrieval from call list rotation schedule");
	}
      
	// We now have the number of positions to advance in addresslist.
	    
	int newindex = lastindex + advances;

	// In case we have not sent an alert in quite some time, do a mod
	    
	while(newindex - memberlist->entries() >= 0)
	  newindex -= memberlist->entries();
	  
	// And, finally, we have the new Person

	r = memberlist->retrieve(newindex);
      }
    
      if(!r)
      {
	// Get the first entry because lastrotate wasn't specified or some
	// other error was encountered.
	
	r = memberlist->retrieve(0);
      }
      
      if(r && dosubs)
      {
	// Check for calllist substitutions.  The configuration
	// file parser should prevent us from trying to retrieve an
	// address for a person via a module that isn't configured.
	
	Person *sub = substitute(r, asof);

	if(sub)
	  r = sub;
      }
    }
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CallList::notify = %d", r);
#endif

  return(r);
}

char *CallList::rotatename()
{
  // Obtain the name of the Schedule this CallList should use to rotate.

  // Returns: The Schedule name, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CallList::rotname()");
  dlog->log_exit(DEBUG_MINTRC, "CallList::rotname = %s", IONULL(rotname));
#endif

  if(alias)
    return(alias->rotname);
  else
    return(rotname);
}

bool CallList::set_rotatesched(Array<Schedule> *sched)
{
  // Set the Schedule this CallList should use to rotate to <sched>,
  // a pointer to an Array of Schedules.  This is what the CallList
  // object will actually use, rotatename() is just a placeholder
  // until schedule.cf is parsed.  This method can not be used on aliases.

  bool ret = false;

#if defined(DEBUG)  
  dlog->log_entry(DEBUG_MINTRC, "CallList::set_rotatesched(%d)", sched);
#endif

  if(!alias)
  {
    rotsched = sched;
    ret = true;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CallList::set_rotatesched = %s", IOTF(ret));
#endif
  
  return(ret);
}

CallList::~CallList()
{
  // Deallocate the CallList object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CallList::~CallList()");
#endif

  alias = NULL;  // Don't delete it
  
  // Just delete the memberlist, not the Persons, which are pointers
  // to objects stored in the Configuration object.
    
  xdelete(memberlist);
  alertvia = NULL;
  xdelete(listname);
  xdelete(rotname);

  if(rotsched)
    rotsched = NULL;  // We just maintain a pointer

  bcache = "";
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CallList::~CallList");
#endif
}

void CallList::init(char *name, CallList *cl, Array<Person> *members,
		    bool broadcast, char *rotates, AlertModule *via)
{
  // Initializer for constructors.

  // Returns: Nothing.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CallList::init(%s,%d,%d,%s,%s,%d)",
		   IONULL(name), cl, members, IOTF(broadcast),
		   IONULL(rotates), via);
#endif

  alias = cl;
  memberlist = members;
  alertvia = via;
  listname = xstrdup(name);
  rotname = NULL;
  rotsched = NULL;
  bcache = "";

  if(alias)
    ltype = alias_list;
  else
  {
    if(rotates)
    {
      ltype = rotating_list;
      rotname = xstrdup(rotates);
    }
    else
    {
      if(broadcast)
	ltype = broadcast_list;
      else
	ltype = simple_list;
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CallList::init()");
#endif
}

Person *CallList::substitute(Person *p, time_t asof)
{
  // Determine if a substitution exists for <p> within the current
  // CallList as of time <asof>.  Substitutions are only supported for
  // simple and rotating CallLists, as no state is maintained for
  // broadcast lists.

  // Returns: A Person that should not be modified or NULL.

  Person *r = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CallList::substitute(%d, %d)", p, asof);
#endif
  
  if(p && p->name() && (ltype == simple_list || ltype == rotating_list))
  {
    // Create a new CallListState to access the substitution list with.
    // As an added bonus, we have a rare sighting of the ever elusive 'this'.

    CallListState *cls = new CallListState(this);

    if(cls)
    {
      char *x = cls->checksubs(p->name(), asof);

      if(x)
      {
	// Now, make sure x is part of this calllist by iterating through
	// memberlist.

	if(memberlist)
	{
	  for(int i = 0;i < memberlist->entries();i++)
	  {
	    Person *y = memberlist->retrieve(i);

	    if(y && strcmp(x, y->name())==0)
	    {
	      r = y;
	      break;
	    }
	  }
	}
	
	xdelete(x);
      }
      
      xdelete(cls);
    }
    else
      wlog->warn("CallList::substitute failed to allocate CallListState");
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CallList::substitute = %d", r);
#endif

  return(r);
}
