/*
 * Configuration.C: Survivor configuration object.
 *
 * Version: $Revision: 0.32 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2006/10/17 14:03:51 $
 * MT-Level: Unsafe
 *
 * Copyright (c) 2002 - 2006
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: Configuration.C,v $
 * Revision 0.32  2006/10/17 14:03:51  benno
 * New CheckStateData args
 *
 * Revision 0.31  2005/10/20 02:13:09  benno
 * Add get_all_returngroups
 *
 * Revision 0.30  2005/09/26 13:44:49  benno
 * Start rotating calllist rotation when they are configured
 *
 * Revision 0.29  2005/04/09 02:36:17  benno
 * Use CheckStateData to generate initial checkstatus
 *
 * Revision 0.28  2004/10/06 21:33:44  benno
 * Fix memory allocation error causing problems on some linux boxen
 *
 * Revision 0.27  2004/09/13 14:23:02  benno
 * Add runs_on_host
 *
 * Revision 0.26  2004/04/24 15:13:09  benno
 * Fix minor memory leak
 *
 * Revision 0.25  2003/11/29 05:22:47  benno
 * Add history_consistency, remove_history
 *
 * Revision 0.24  2003/10/06 23:09:59  benno
 * Update documentation for appending calllists in alertplan aliases
 * add_dependency for dependency overhaul
 * get_all_fixes for config.l
 *
 * Revision 0.23  2003/05/29 00:35:09  benno
 * Add AlertModule support
 * Use xhdelete
 *
 * Revision 0.22  2003/05/04 21:29:05  benno
 * Fix typo
 *
 * Revision 0.21  2003/04/09 20:23:46  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.20  2003/04/04 22:17:54  benno
 * Use Debugger
 *
 * Revision 0.19  2003/02/28 04:12:58  benno
 * Fix incorrect usage of list->find
 *
 * Revision 0.18  2003/01/24 16:12:30  benno
 * Add IONULL and IOTF
 *
 * Revision 0.17  2003/01/01 02:04:06  benno
 * lastcheck is now part of checkstatus
 *
 * Revision 0.16  2002/12/31 04:30:21  benno
 * Add fix support
 *
 * Revision 0.15  2002/12/22 17:18:59  benno
 * Delete global noc and timeout
 * Add Transport support
 * Switch to xdelete
 *
 * Revision 0.14  2002/12/16 00:44:28  benno
 * Convert to try based AlertPlans
 *
 * Revision 0.13  2002/10/25 22:58:37  benno
 * add add_person and find_person
 *
 * Revision 0.12  2002/08/06 19:42:29  selsky
 * Remove embedded nulls in format
 *
 * Revision 0.11  2002/04/26 20:15:27  toor
 * lexerr++ missing in two places
 *
 * Revision 0.10  2002/04/04 20:09:56  benno
 * copyright
 *
 * Revision 0.9  2002/04/03 18:09:48  benno
 * rcsify date
 *
 * Revision 0.8  2002/04/03 18:09:38  benno
 * Move allocate_dirent to utils.C
 * Add get_all_calllists
 *
 * Revision 0.7  2002/04/03 18:09:05  benno
 * Add alert shift
 *
 * Revision 0.6  2002/04/03 18:08:31  benno
 * Support schedule for global notify on clear
 * Create running state directory
 *
 * Revision 0.5  2002/04/03 18:07:56  benno
 * Add add_alertplan_alias
 *
 * Revision 0.4  2002/04/03 18:07:27  benno
 * Add alert throttling
 *
 * Revision 0.3  2002/04/03 18:06:58  benno
 * Change state_consistency for state/service
 *
 * Revision 0.2  2002/04/03 18:06:29  benno
 * Reorganize state_consistency
 *
 * Revision 0.1  2002/04/03 18:05:51  benno
 * initial revision
 *
 */

#include "survivor.H"

Configuration::Configuration()
{
  // Allocate and initialize a new Configuration object.

  // Returns: A new Configuration object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "Configuration::Configuration()");
#endif

  calllistarray = NULL;
  checkarray = NULL;
  deparray = NULL;
  fixarray = NULL;
  alertmodhash = NULL;
  alerthash = NULL;
  calllisthash = NULL;
  checkhash = NULL;
  classhash = NULL;
  classbymember = NULL;
  fixhash = NULL;
  grouphash = NULL;
  groupbymember = NULL;
  personhash = NULL;
  returngrouphash = NULL;
  schedhash = NULL;
  transporthash = NULL;
  classlist = NULL;
  hostlist = NULL;
  returngrouplist = NULL;
  as = -1;
  at = -1;
  m = -1;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Configuration::Configuration()");
#endif
}

bool Configuration::add_alertmodule(AlertModule *module)
{
  // Add <alertmodule> to the hash of AlertModules.

  // Returns: true if successful, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Configuration::add_alertmodule(%d)",
		  module);
#endif
  
  if(module && alertmodhash)
  {
    char *key = module->name();

    if(key)
    {
      if(alertmodhash->insert(key, module))
	ret = true;
    }
#if defined(DEBUG)
    else
      dlog->log_progress(DEBUG_CONFIG, "AlertModule specified without name");
#endif
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Configuration::add_alertmodule = %s",
		 IOTF(ret));
#endif

  return(ret);
}

bool Configuration::add_alertplan(AlertPlan *alertplan)
{
  // Add <alertplan> to the hash of AlertPlans.

  // Returns: true if successful, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Configuration::add_alertplan(%d)",
		   alertplan);
#endif
  
  if(alertplan && alerthash)
  {
    char *key = alertplan->name();

    if(key)
    {
      if(alerthash->insert(key, alertplan))
	ret = true;
    }
#if defined(DEBUG)
    else
      dlog->log_progress(DEBUG_CONFIG, "AlertPlan specified without name");
#endif
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Configuration::add_alertplan = %s",
		  IOTF(ret));
#endif

  return(ret);
}

bool Configuration::add_alertplan_alias(AlertPlan *original,
					char *alias, CallList *target,
					CallList *replacement)
{
  // Create a new AlertPlan named <alias> with identical characteristics to
  // <original>, except that where the CallList <target> is found, replace it
  // with the CallList <replacement> instead.  If <target> is NULL,
  // <replacement> will be added to the existing CallList set.

  // Returns: true if successful, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC,
		   "Configuration::add_alertplan_alias(%d,%s,%d,%d)",
		   original, IONULL(alias), target, replacement);
#endif

  if(alerthash && original && alias && replacement)
  {
    // Allocate a new AlertPlan.  The constructors will perform all the
    // dirty work for us.

    AlertPlan *aliasplan = new AlertPlan(alias, original, target,
					 replacement);

    if(aliasplan)
    {
      char *key = aliasplan->name();

      if(key)
      {
	if(alerthash->insert(key, aliasplan))
	  ret = true;
      }
#if defined(DEBUG)
      else
	dlog->log_progress(DEBUG_CONFIG,
			    "AlertPlan specified without name");
#endif

      if(!ret)
      {
	xdelete(aliasplan);
      }
    }
    else
      wlog->warn("Configuration::add_alertplan_alias failed to allocate AlertPlan");
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Configuration::add_alertplan_alias = %s",
		  IOTF(ret));
#endif

  return(ret);
}

bool Configuration::add_calllist(CallList *calllist)
{
  // Add <calllist> to the hash of CallList objects.

  // Returns: true if successful, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Configuration::add_calllist(%d)",
		   calllist);
#endif
  
  if(calllist && calllisthash && calllistarray)
  {
    char *key = calllist->name();

    if(key)
    {
      if(calllisthash->insert(key, calllist) && calllistarray->add(calllist))
	ret = true;
    }
#if defined(DEBUG)
    else
      dlog->log_progress(DEBUG_CONFIG, "Call List specified without name");
#endif
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Configuration::add_calllist = %s",
		  IOTF(ret));
#endif

  return(ret);
}

bool Configuration::add_check(Check *check)
{
  // Add <check> to the hash of Check objects.

  // Returns: true if successful, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Check::add_check(%d)", check);
#endif
  
  if(check && checkhash && checkarray)
  {
    char *key = check->name();

    if(key)
    {
      // If we insert in the hash but not the array, we are inconsistent
      // and return false, although the Scheduler could still run (just
      // not scheduling this check).
      
      if(checkhash->insert(key, check) && checkarray->add(check))
      {
	if(check->all_hosts())
	{
	  // Create a group consisting of all known hosts.  If a group
	  // named for this check has already been created, add all other
	  // defined hosts to it.

	  List *l = find_group(check->name());

	  if(l)
	    ret = add_all_hosts(l);
	  else
	  {
	    l = find_all_hosts(check->name());

	    if(l)
	    {
	      ret = add_group(l);

	      if(!ret)
	      {
		xdelete(l);
	      }
	    }
	  }
	}
	
	ret = true;
      }
    }
#if defined(DEBUG)
    else
      dlog->log_progress(DEBUG_CONFIG, "Check specified without name");
#endif
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Configuration::add_check = %s", IOTF(ret));
#endif

  return(ret);
}

bool Configuration::add_class(HostClass *hostclass)
{
  // Add <hostclass> to the hash of HostClass objects.

  // Returns: true if successful, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Configuration::add_class(%d)", hostclass);
#endif

  if(hostclass && classhash && classbymember && classlist && hostlist)
  {
    // First insert the hostclass into the class hash
    
    char *key = hostclass->name();

    if(key)
    {
      if(strcmp(key, "all")!=0)
      {
	if(classhash->insert(key, hostclass))
	{
	  // Insert was successful, so now insert one entry for each member
	  // name into the classbymember hash and into the hostlist.

	  List *hosts = hostclass->hosts();

	  if(hosts)
	  {
	    char *key2;

	    for(int i = 0;i < hosts->entries();i++)
	    {
	      key2 = hosts->retrieve(i);

	      if(key2)
	      {
		// Insert without checking for errors

		classbymember->insert(key2, hostclass);
		hostlist->add(key2);
	      }
	    }
	  }

	  // Insert into list of HostClasses without checking for error

	  classlist->add(key);
	  
	  ret = true;
	}
      }
#if defined(DEBUG)
      else
	dlog->log_progress(DEBUG_CONFIG,
			    "HostClass specified with reserved name 'all'");
#endif
    }
#if defined(DEBUG)
    else
      dlog->log_progress(DEBUG_CONFIG, "HostClass specified without name");
#endif
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Configuration::add_class = %s", IOTF(ret));
#endif

  return(ret);
}

bool Configuration::add_dependency(Dependency *dependency)
{
  // Add <dependency> to the array of Dependency objects.  There are
  // no retrieval methods because Dependencys are stored here primarily
  // to as a convenience.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Configuration::add_dependency(%d)",
		  dependency);
#endif

  if(dependency && deparray && deparray->add(dependency))
    ret = true;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Configuration::add_dependency = %s",
		  IOTF(ret));
#endif
  
  return(ret);
}

bool Configuration::add_fix(Fix *fix)
{
  // Add <fix> to the hash of Fix objects.

  // Returns: true if successful, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Configuration::add_fix(%d)", fix);
#endif
  
  if(fix && fixhash && fixarray)
  {
    char *key = fix->name();

    if(key)
    {
      if(fixhash->insert(key, fix) && fixarray->add(fix))
	ret = true;
    }
#if defined(DEBUG)
    else
      dlog->log_progress(DEBUG_CONFIG, "Fix specified without name");
#endif
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Configuration::add_fix = %s", IOTF(ret));
#endif

  return(ret);
}

bool Configuration::add_group(List *group)
{
  // Add <group> to the hash of List objects.

  // Returns: true if successful, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Configuration::add_group(%d)", group);
#endif
  
  if(group && grouphash && groupbymember)
  {
    // First, add the Group to the Group hash
    
    char *key = group->name();

    if(key)
    {
      if(strcmp(key, "all"))
      {
	if(grouphash->insert(key, group))
	  ret = true;
      }
#if defined(DEBUG)
      else
	dlog->log_progress(DEBUG_CONFIG,
			    "Group specified with reserved name 'all'");
#endif
    }
#if defined(DEBUG)
    else
      dlog->log_progress(DEBUG_CONFIG, "Group specified without name");
#endif

    // Next, for each member of the Group, add an entry to groupbymember,
    // allocating new List objects as needed.

    if(ret)
    {
      for(int i = 0;i < group->entries();i++)
      {
	char *host = group->retrieve(i);

	if(host)
	{
	  List *l = groupbymember->retrieve(host);

	  if(l)
	  {
	    // l is a list of groups <host> is already a member of

	    l->add(key);
	  }
	  else
	  {
	    // Allocate a new list named for this host and consisting
	    // of this group name

	    l = new List(host);

	    if(l)
	    {
	      l->add(key);

	      if(!groupbymember->insert(host, l))
	      {
		xdelete(l);

		ret = false;
		wlog->warn("Configuration::add_group failed to insert into groupbymember");
	      }
	    }
	    else
	      wlog->warn("Configuration::add_group failed to allocate List");
	  }
	}
      }
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Configuration::add_group = %s", IOTF(ret));
#endif

  return(ret);
}

bool Configuration::add_named_schedule(Array<Schedule> *schedule)
{
  // Add <schedule> to the hash of Schedule Arrays.

  // Returns: true if successful, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Configuration::add_named_schedule(%d)",
		   schedule);
#endif
  
  if(schedule && schedhash)
  {
    char *key = schedule->name();

    if(key)
    {
      if(schedhash->insert(key, schedule))
	ret = true;
    }
#if defined(DEBUG)
    else
      dlog->log_progress(DEBUG_CONFIG, "Schedule specified without name");
#endif
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Configuration::add_named_schedule = %s",
		  IOTF(ret));
#endif

  return(ret);
}

bool Configuration::add_named_returngroup(Array<int> *returngroup)
{
  // Add <returngroup> to the hash of Return Groups.

  // Returns: true if successful, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Configuration::add_named_returngroup(%d)",
		   returngroup);
#endif
  
  if(returngroup && returngrouphash)
  {
    char *key = returngroup->name();

    if(key)
    {
      if(returngrouphash->insert(key, returngroup))
      {
	// Add the name of the returngroup to the list of returngroups

	returngrouplist->add(key);
	
	ret = true;
      }
    }
#if defined(DEBUG)
    else
      dlog->log_progress(DEBUG_CONFIG,
			  "Return Group specified without name");
#endif
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Configuration::add_named_returngroup = %s",
		  IOTF(ret));
#endif

  return(ret);
}

bool Configuration::add_person(Person *person)
{
  // Add <person> to the hash of Persons.

  // Returns: true if successful, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Configuration::add_person(%d)", person);
#endif
  
  if(person && personhash)
  {
    char *key = person->name();

    if(key)
    {
      if(personhash->insert(key, person))
	ret = true;
    }
#if defined(DEBUG)
    else
      dlog->log_progress(DEBUG_CONFIG, "Person specified without name");
#endif
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Configuration::add_person = %s", IOTF(ret));
#endif

  return(ret);
}

bool Configuration::add_transport(Transport *transport)
{
  // Add <transport> to the hash of Transports.

  // Returns: true if successful, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Configuration::add_transport(%d)",
		   transport);
#endif

  if(transport && transporthash)
  {
    char *key = transport->name();

    if(key)
    {
      if(transporthash->insert(key, transport))
	ret = true;
    }
#if defined(DEBUG)
    else
      dlog->log_progress(DEBUG_CONFIG, "Transport specified without name");
#endif
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Configuration::add_transport = %s",
		  IOTF(ret));
#endif

  return(ret);
}

int Configuration::alert_shift()
{
  // Obtain the alert shift value.

  // Returns: The alert shift value in seconds, or 0.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Configuration::alert_shift()");
  dlog->log_exit(DEBUG_MINTRC, "Configuration::alert_shift = %d",
		  ((as > -1) ? as : 0));
#endif

  if(as > -1)
    return(as);
  else
    return(0);
}

int Configuration::alert_throttle()
{
  // Obtain the throttle value for alert transmissions.  If no throttle
  // value has been set, then the default value will be returned.

  // Returns: The alert throttle value.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Configuration::alert_throttle()");
#endif
  
  if(at > -1)
  {
#if defined(DEBUG)
    dlog->log_exit(DEBUG_MINTRC, "Configuration::alert_throttle = %d", at);
#endif
  
    return(at);
  }
  else
  {
#if defined(DEBUG)
    dlog->log_exit(DEBUG_MINTRC, "Configuration::alert_throttle = %d",
		    DEFAULT_ALERT_THROTTLE);
#endif
      
    return(DEFAULT_ALERT_THROTTLE);
  }
}

AlertModule *Configuration::find_alertmodule(char *name)
{
  // Find the AlertModule named <name>.

  // Returns: A pointer to an AlertModule object if found, NULL
  // otherwise.

  AlertModule *a = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Configuration::find_alertmodule(%s)",
		  IONULL(name));
#endif
  
  if(name && alertmodhash)
    a = alertmodhash->retrieve(name);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Configuration::find_alertmodule = %d", a);
#endif
  
  return(a);
}

AlertPlan *Configuration::find_alertplan(char *name)
{
  // Find the AlertPlan named <name>.

  // Returns: A pointer to an AlertPlan object if found, NULL
  // otherwise.

  AlertPlan *a = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Configuration::find_alertplan(%s)",
		   IONULL(name));
#endif
  
  if(name && alerthash)
    a = alerthash->retrieve(name);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Configuration::find_alertplan = %d", a);
#endif
  
  return(a);
}

CallList *Configuration::find_calllist(char *name)
{
  // Find the CallList named <name>.

  // Returns: A Pointer to a CallList object if found, NULL otherwise.

  CallList *c = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Configuration::find_calllist(%s)",
		   IONULL(name));
#endif

  if(name && calllisthash)
    c = calllisthash->retrieve(name);
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Configuration::find_calllist = %d", c);
#endif
  
  return(c);
}

HostClass *Configuration::find_class(char *hostname)
{
  // Find the HostClass of which <hostname> is a member.

  // Returns: A pointer to a HostClass object if found, NULL otherwise.

  HostClass *h = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Configuration::find_class(%s)",
		   IONULL(hostname));
#endif
  
  if(hostname && classbymember)
    h = classbymember->retrieve(hostname);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Configuration::find_class = %d", h);
#endif
  
  return(h);
}

Check *Configuration::find_check(char *name)
{
  // Find the Check of which <name> is a member.

  // Returns: A pointer to a Check object if found, NULL otherwise.

  Check *c = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Configuration::find_check(%s)",
		   IONULL(name));
#endif
  
  if(name && checkhash)
    c = checkhash->retrieve(name);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Configuration::find_check = %d", c);
#endif
  
  return(c);
}

Fix *Configuration::find_fix(char *name)
{
  // Find the Fix named <name>.

  // Returns: A pointer to the Fix if found, NULL otherwise.

  Fix *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Configuration::find_fix(%s)", IONULL(name));
#endif
  
  if(name && fixhash)
    ret = fixhash->retrieve(name);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Configuration::find_fix = %d", ret);
#endif
  
  return(ret);
}

List *Configuration::find_group(char *name)
{
  // Find the group named <name>.

  // Returns: A pointer to a List for the group if found, NULL otherwise.

  List *l = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Configuration::find_group(%s)",
		   IONULL(name));
#endif

  if(name && grouphash)
    l = grouphash->retrieve(name);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Configuration::find_group = %d", l);
#endif
  
  return(l);
}

List *Configuration::find_groups(char *hostname)
{
  // Find the groups containing <name>.

  // Returns: A pointer to a List of the groups if found, NULL otherwise.

  List *l = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Configuration::find_groups(%s)",
		   IONULL(hostname));
#endif

  if(hostname && groupbymember)
    l = groupbymember->retrieve(hostname);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Configuration::find_groups = %d", l);
#endif
  
  return(l);
}

HostClass *Configuration::find_hostclass(char *classname)
{
  // Find the HostClass of which <hostname> is a member.

  // Returns: A pointer to a HostClass object if found, NULL otherwise.

  HostClass *h = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Configuration::find_hostclass(%s)",
		   IONULL(classname));
#endif
  
  if(classname && classhash)
    h = classhash->retrieve(classname);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Configuration::find_hostclass = %d", h);
#endif
  
  return(h);
}

Person *Configuration::find_person(char *name)
{
  // Find the Person named <name>.

  // Returns: A pointer to the Person if found, NULL otherwise.

  Person *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Configuration::find_person(%s)",
		   IONULL(name));
#endif
  
  if(name && personhash)
    ret = personhash->retrieve(name);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Configuration::find_person = %d", ret);
#endif
  
  return(ret);
}

Array<int> *Configuration::find_returngroup(char *name)
{
  // Find the Return Group named <name>.

  // Returns: A pointer to an Array of return codes if found, NULL
  // otherwise.

  Array<int> *r = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Configuration::find_returngroup(%s)",
		   IONULL(name));
#endif
  
  if(name && returngrouphash)
    r = returngrouphash->retrieve(name);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Configuration::find_returngroup = %d", r);
#endif
  
  return(r);
}

Array<Schedule> *Configuration::find_schedule(char *name)
{
  // Find the Schedule named <name>.

  // Returns: A pointer to an Array of Schedule objects if found, NULL
  // otherwise.

  Array<Schedule> *s = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Configuration::find_schedule(%s)",
		   IONULL(name));
#endif
  
  if(name && schedhash)
    s = schedhash->retrieve(name);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Configuration::find_schedule = %d", s);
#endif
  
  return(s);
}

Transport *Configuration::find_transport(char *name)
{
  // Find the Transport named <name>.

  // Returns: A pointer to the Transport if found, NULL otherwise.

  Transport *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Configuration::find_transport(%s)",
		   IONULL(name));
#endif
  
  if(name && transporthash)
    ret = transporthash->retrieve(name);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Configuration::find_transport = %d", ret);
#endif
  
  return(ret);
}

Array<CallList> *Configuration::get_all_calllists()
{
  // Obtain an Array of all CallLists.  This method is intended for use only
  // by the CGI.

  // Returns: A pointer to an Array of CallLists, which should not be modified
  // in any way.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Configuration::get_all_calllists()");
  dlog->log_exit(DEBUG_MINTRC, "Configuration::get_all_calllists = %d",
		  calllistarray);
#endif

  return(calllistarray);
}

Array<Check> *Configuration::get_all_checks()
{
  // Obtain an Array of all Checks.  This method is intended for use only
  // by the Scheduler and config.l.

  // Returns: A pointer to an Array of Checks, which should not be modified
  // in any way.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Configuration::get_all_checks()");
  dlog->log_exit(DEBUG_MINTRC, "Configuration::get_all_checks = %d",
		 checkarray);
#endif

  return(checkarray);
}

Array<Fix> *Configuration::get_all_fixes()
{
  // Obtain an Array of all Fixes.  This method is intended for use only
  // by config.l.

  // Returns: A pointer to an Array of Fixes, which should not be modified
  // in any way.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Configuration::get_all_fixes()");
  dlog->log_exit(DEBUG_MINTRC, "Configuration::get_all_fixes = %d", fixarray);
#endif

  return(fixarray);
}

List *Configuration::get_all_hostclasses()
{
  // Obtain a List containing the names of all defined HostClass objects.
  // This method is intended for use only by the Scheduler.

  // Returns: A pointer to a List of names, which should not be modified in
  // any way.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Configuration::get_all_hostclasses()");
  dlog->log_exit(DEBUG_MINTRC, "Configuration::get_all_hostclasses = %d",
		  classlist);
#endif
  
  return(classlist);
}

List *Configuration::get_all_returngroups()
{
  // Obtain a List containing the names of all defined ReturnGroup objects.

  // Returns: A pointer to a List of names, which should not be modified in
  // any way.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Configuration::get_all_returngroups()");
  dlog->log_exit(DEBUG_MINTRC, "Configuration::get_all_returngroups = %d",
		  returngrouplist);
#endif
  
  return(returngrouplist);
}

Array<SHPair> *Configuration::history_consistency()
{
  // Verify the consistency of the history directory by comparing the
  // existing directories with the currently defined services and
  // hosts in the configuration.  Unlike state_consistency, no changes
  // are made.  Instead, an Array of the stale directories (as
  // referenced by service/host pairs) is returned.

  // Returns: An Array of SHPair objects, containing the service and
  // host names of stale history directories, which should be deleted
  // (both the Array and its contents) when no longer required, or
  // NULL on error.

  Array<SHPair> *ret = NULL;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "Configuration::history_consistency()");
#endif

  ret = new Array<SHPair>();

  if(ret)
  {
    CharBuffer *hdir = new CharBuffer(args->histdir());

    if(hdir)
    {
      if(hdir->append("/host"))
      {
	struct dirent *hentry = allocate_dirent(hdir->str());
	struct dirent *hdp;
	
	if(hentry)
	{
	  DIR *hdirp = opendir(hdir->str());
	  
	  if(hdirp)
	  {
	    while(readdir_r(hdirp, hentry, &hdp) == 0 && hdp)
	    {
	      // Loop through the set of hosts.  If the current host is
	      // not defined, then any service directory underneath is
	      // automatically invalid.  Ignore any entry that begins
	      // with a .
	      
	      if(hdp->d_name[0] != '.')
	      {
		bool hostok = true;
		
		if(!find_class(hdp->d_name))
		  hostok = false;

		CharBuffer *sdir = new CharBuffer(hdir);

		if(sdir)
		{
		  if(sdir->append('/') && sdir->append(hdp->d_name))
		  {
		    struct dirent *sentry = allocate_dirent(sdir->str());
		    struct dirent *sdp;
		    
		    if(sentry)
		    {
		      DIR *sdirp = opendir(sdir->str());
		      
		      if(sdirp)
		      {
			while(readdir_r(sdirp, sentry, &sdp) == 0 && sdp)
			{
			  // Loop through the set of services.  If either
			  // the host or the service is invalid, add an
			  // SHPair to the Array for this service@host.
			  
			  if(sdp->d_name[0] != '.')
			  {
			    if(!hostok  // host is no longer valid
			       // host is no longer in group
			       || !find_group(sdp->d_name)
			       // check is no longer valid
			       || !find_check(sdp->d_name))
			    {
			      SHPair *badpair = new SHPair(sdp->d_name,
							   hdp->d_name);

			      if(!badpair || !ret->add(badpair))
			      {
				wlog->warn("Configuration::history_consistency failed to add %s@%s to list",
					   sdp->d_name, hdp->d_name);
				
				xdelete(badpair);
			      }
			    }
			  }
			}
			
			closedir(sdirp);
		      }
		      else
			wlog->warn("Configuration::history_consistency failed to open %s",
				   sdir->str());
		      
		      free(sentry);
		      sentry = NULL;
		    }
		    else
		      wlog->warn("Configuration::history_consistency failed to allocate dirent");
		  }
		  
		  xdelete(sdir);
		}
		else
		  wlog->warn("Configuration::history_consistency failed to allocate CharBuffer");
	      }
	    }
	    
	    closedir(hdirp);
	  }
	  else
	    wlog->warn("Configuration::history_consistency failed to open %s",
		       hdir->str());
	  
	  free(hentry);
	  hentry = NULL;
	}
	else
	  wlog->warn("Configuration::history_consistency failed to allocate dirent");
      }

      xdelete(hdir);
    }
    else
      wlog->warn("Configuration::history_consistency failed to allocate CharBuffer");

  }
  else
    wlog->warn("Configuration::history_consistency failed to allocate Array");

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "Configuration::history_consistency = %d",
		 ret);
#endif
  
  return(ret);
}

int Configuration::max_dependencies()
{
  // Obtain the maximum number of dependencies.  If no value has been set,
  // then the default value is returned.

  // Returns: The maximum number of dependencies.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Configuration::max_dependencies()");
#endif
  
  if(m > -1)
  {
#if defined(DEBUG)
    dlog->log_exit(DEBUG_MINTRC, "Configuration::max_dependencies = %d", m);
#endif
  
    return(m);
  }
  else
  {
#if defined(DEBUG)
    dlog->log_exit(DEBUG_MINTRC, "Configuration::max_dependencies = %d",
		    DEFAULT_MAXDEPEND);
#endif
      
    return(DEFAULT_MAXDEPEND);
  }
}

bool Configuration::parse_cfs()
{
  // Parse the configuration files.  If the configuration files have
  // already been parsed, delete and create a new Configuration
  // object rather than invoke this method again.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "Configuration::parse_cfs()");
#endif

  char *cdir = args->configdir();
  
  // First allocate our hashtables and arrays and lists

  calllistarray = new Array<CallList>();
  checkarray = new Array<Check>();
  deparray = new Array<Dependency>();
  fixarray = new Array<Fix>();
  alertmodhash = new Hashtable<AlertModule>();
  alerthash = new Hashtable<AlertPlan>();
  calllisthash = new Hashtable<CallList>();
  checkhash = new Hashtable<Check>();
  classhash = new Hashtable<HostClass>();
  classbymember = new Hashtable<HostClass>();
  fixhash = new Hashtable<Fix>();
  grouphash = new Hashtable<List>();
  groupbymember = new Hashtable<List>();
  personhash = new Hashtable<Person>();
  returngrouphash = new Hashtable< Array<int> >();
  schedhash = new Hashtable< Array<Schedule> >();
  transporthash = new Hashtable<Transport>();
  classlist = new List();
  hostlist = new List();
  returngrouplist = new List();

  if(cdir && calllistarray && checkarray && deparray && fixarray
     && alertmodhash && alerthash && checkhash && calllisthash && classhash
     && classbymember && fixhash && grouphash && groupbymember
     && personhash && returngrouphash && schedhash && transporthash
     && classlist && hostlist && returngrouplist)
  {
    // Call lex once per config file
    
    for(int i = 0;i < 5;i++)
    {
      char *cfname = NULL;
      FILE *cf = NULL;

      cfname = new char[strlen(cdir) + 15];

      if(cfname)
      {
	config_file_t cftype = host_cf;
	
	switch(i)
	{
	case 0:
	  sprintf(cfname, "%s/calllist.cf", cdir);
	  cftype = calllist_cf;
	  break;
	case 1:
	  sprintf(cfname, "%s/schedule.cf", cdir);
	  cftype = sched_cf;
	  break;
	case 2:
	  sprintf(cfname, "%s/host.cf", cdir);
	  cftype = host_cf;
	  break;
	case 3:
	  sprintf(cfname, "%s/check.cf", cdir);
	  cftype = check_cf;
	  break;
	case 4:
	  sprintf(cfname, "%s/dependency.cf", cdir);
	  cftype = dependency_cf;
	  break;
	default:
	  // If you make it here, it's because of programmer stupidity
	  return(false);
	  break;
	}

	cf = fopen(cfname, "r");

	if(cf)
	{
	  // Ooo! A use of this!
	  lexstart(cftype, cf, this);
	  yylex();
	  lexfinish();

	  if(lexerr || lexincomplete())
	  {
	    lexerr += lexincomplete();

#if defined(DEBUG)
	    if(lexincomplete())
	      dlog->log_progress(DEBUG_CONFIG,
				  "Unexpected EOF (unclosed brace or otherwise unfinished entry)");

	    dlog->log_progress(DEBUG_CONFIG,
				"%d %s encountered while parsing %s",
				lexerr,
				((lexerr == 1) ? "error" : "errors"),
				cfname);
#endif
	  }
#if defined(DEBUG)
	  else
	    dlog->log_progress(DEBUG_CONFIG, "Parse of %s successful",
				cfname);
#endif
	  fclose(cf);
	}
	else
	{
	  wlog->warn("Unable to open %s", cfname);
	  lexerr++;
	}

	xdelete(cfname);
      }
      else
      {
	wlog->warn("Configuration failed to allocate cfname");
	lexerr++;
      }

      // Don't bother continuing if we hit any errors
      
      if(lexerr)
	break;
    }

    if(!lexerr)
      ret = true;
  }
  else
  {
    wlog->warn("Configuration failed to allocate all Hashtables");

    // We don't clean up here because that will happen when the destructor
    // is called.
  }

  // Don't perform state_consistency check here because only the scheduler
  // should do so.

  /*
  if(ret)
  {
    // Perform consistency check

    if(!state_consistency())
    {
      ret = false;
      WARN << "State consistency check failed" << endl;
    }
  }
  */
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "Configuration::parse_cfs = %s", IOTF(ret));
#endif

  return(ret);
}

bool Configuration::remove_history(char *service, char *host)
{
  // Remove the history directory for <service>@<host>.  If this is
  // the last service archive for host, the parent host directory will
  // also be removed.  This method cannot be called for <service>@<host>
  // pairs that are still valid, and should generally only follow an
  // archival of the history records.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Configuration::remove_history(%s,%s)",
		 IONULL(service), IONULL(host));
#endif

  if(service && host)
  {
    if(!find_class(host)           // host is no longer valid
       || !find_group(service)     // host is no longer in group
       || !find_check(service))    // check is no longer defined
    {
      CharBuffer *cbuf = new CharBuffer(args->histdir());

      if(cbuf && cbuf->append("/host/") && cbuf->append(host))
      {
	if(remove_directory(cbuf->str(), service))
	{
	  // If parent is empty, remove that, too.  To do this, we
	  // just try to rmdir it.  If it's not empty, the rmdir will
	  // fail.  So, ignore the return value and always set ret
	  // to true.

	  rmdir(cbuf->str());
	  ret = true;
	}
	else
	  wlog->warn("Configuration::remove_history failed to remove %s/%s",
		     cbuf->str(), service);
      }
      else
	wlog->warn("Configuration::remove_history failed to set up cbuf");
      
      xdelete(cbuf);
    }
    else
      wlog->warn("%s@%s still valid in Configuration::remove_history",
		 service, host);
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Configuration::remove_history = %s",
		 IOTF(ret));
#endif
  
  return(ret);
}

bool Configuration::runs_on_host(char *service, char *host)
{
  // Determine if <service> is configured to be run for <host>.

  // Returns: true if <service> runs on <host>, false otherwise.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Configuration::runs_on_host(%s,%s)",
		  IONULL(service), IONULL(host));
#endif

  if(service && host)
  {
    List *g = find_group(service);

    if(g)
    {  
      for(int i = 0;i < g->entries();i++)
	if(g->retrieve(i) && strcmp(g->retrieve(i), host)==0)
	{
#if defined(DEBUG)
	  dlog->log_exit(DEBUG_MINTRC, "Configuration::runs_on_host = %s",
			 IOTF(true));
#endif
  
	  return(true);
	}
    }
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Configuration::runs_on_host = %s",
		 IOTF(false));
#endif
  
  return(false);
}

bool Configuration::set_alert_shift(int seconds)
{
  // Set the alert shift value to <seconds>.  Once set, this value
  // cannot be changed (to prevent multiple definitions in schedule.cf).

  // Returns: true if the value was set, false otherwise, including if it
  // was previously set.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Configuration::set_alert_shift(%d)",
		   seconds);
#endif

  if(as == -1 && seconds >= 0)
  {
    as = seconds;
    ret = true;
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Configuration::set_alert_shift = %s",
		  IOTF(ret));
#endif
  
  return(ret);
}

bool Configuration::set_alert_throttle(int value)
{
  // Set the alert transmission throttle to <value>.  Once set, this value
  // cannot be changed (to prevent multiple definitions in schedule.cf).

  // Returns: true if the value was set, false otherwise, including if it
  // was previously set.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Configuration::set_alert_throttle(%d)",
		   value);
#endif
  
  if(at == -1 && value >= 0)
  {
    at = value;
    ret = true;
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Configuration::set_alert_throttle = %s",
		  IOTF(ret));
#endif
  
  return(ret);
}

bool Configuration::set_max_dependencies(int max)
{
  // Set the maximum dependencies to <max>.  Once set, this value cannot be
  // changed (to prevent multiple definitions in check.cf).

  // Returns: true if the value was set, false otherwise, including if it
  // was previously set.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Configuration::set_max_dependencies(%d)",
		   max);
#endif

  if(m == -1 && max >= 0)
  {
    m = max;
    ret = true;
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Configuration::set_max_dependencies = %s",
		  IOTF(ret));
#endif
  
  return(ret);
}

bool Configuration::state_consistency()
{
  // Verify that the state directory matches the Configuration, and
  // fix any inconsistencies.  This method should be called whenever
  // the Configuration is successfully read, before the write lock is
  // released.  The top level state directory must already exist, but
  // nothing underneath need be.  This method should only be called by
  // the scheduler.

  // Returns: true if the state directory is consistent, false otherwise.

  bool r = true;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "Configuration::state_consistency()");
#endif

  struct stat sb;
  char *sdir = args->statedir();
  char *hdir = args->histdir();

  if(!sdir || !hdir)
  {
    wlog->warn("state_consistency encountered unexpected NULL value for statedir or histdir");
    r = false;
  }

  // Set umask to 002 for mkdir (verify_directory)

  mode_t u = umask(002);
  
  if(r)
  {
    // Make sure sdir and hdir already exist.

    if(stat(sdir, &sb) != 0 || stat(hdir, &sb) != 0)
    {
      wlog->warn("state_consistency failed to stat state and history directories.");
      wlog->warn("Both %s and %s must exist.", IONULL(sdir), IONULL(hdir));

      r = false;
    }
  }

  char *hhdir = NULL;   // host history
  char *scdir = NULL;   // calllist state
  char *shdir = NULL;   // host state
  char *ssdir = NULL;   // service state
  char *sfdir = NULL;   // fix state
  
  if(r)
  {
    // Make sure state subdirectories exist.  If not, create them.

    hhdir = new char[strlen(hdir) + 6];

    if(hhdir)
    {
      sprintf(hhdir, "%s/host", hdir);

      if(!verify_directory(hhdir, &sb, DIR_GRP_RD))
      {
	wlog->warn("state_consistency failed to create host history directory '%s'",
		   hhdir);
	r = false;
      }
    }
    else
    {
      wlog->warn("state_consistency failed to allocate hhdir");
      r = false;
    }

    scdir = new char[strlen(sdir) + 10];

    if(scdir)
    {
      sprintf(scdir, "%s/calllist", sdir);

      if(!verify_directory(scdir, &sb, DIR_GRP_WR))
      {
	wlog->warn("state_consistency failed to create calllist state directory '%s'",
		   scdir);
	r = false;
      }
    }
    else
    {
      wlog->warn("state_consistency failed to allocate scdir");
      r = false;
    }

    sfdir = new char[strlen(sdir) + 17];

    if(sfdir)
    {
      sprintf(sfdir, "%s/fix", sdir);

      if(verify_directory(sfdir, &sb, DIR_GRP_WR))
      {
	sprintf(sfdir, "%s/fix/locks", sdir);

	if(!verify_directory(sfdir, &sb, DIR_GRP_WR))
	{
	  wlog->warn("state_consistency failed to create locks directory '%s'",
		     sfdir);
	
	  r = false;
	}
      }
      else
      {
	wlog->warn("state_consistency failed to create fix state directory '%s'",
		   sfdir);
	
	r = false;
      }
    }
    else
    {
      wlog->warn("state_consistency failed to allocate sfdir");
      r = false;
    }
    
    shdir = new char[strlen(sdir) + 6];

    if(shdir)
    {
      sprintf(shdir, "%s/host", sdir);

      if(!verify_directory(shdir, &sb, DIR_GRP_WR))
      {
	wlog->warn("state_consistency failed to create host state directory '%s'",
		   shdir);
	r = false;
      }
    }
    else
    {
      wlog->warn("state_consistency failed to allocate shdir");
      r = false;
    }

    ssdir = new char[strlen(sdir) + 9];

    if(ssdir)
    {
      sprintf(ssdir, "%s/service", sdir);

      if(!verify_directory(ssdir, &sb, DIR_GRP_WR))
      {
	wlog->warn("state_consistency failed to create service state directory '%s'",
		   ssdir);
	r = false;
      }
    }
    else
    {
      wlog->warn("state_consistency failed to allocate ssdir");
      r = false;
    }

    // There are no subdirectories under srdir, so we don't need to
    // keep it around.
    
    char *srdir = new char[strlen(sdir) + 9];

    if(srdir)
    {
      sprintf(srdir, "%s/running", sdir);

      if(!verify_directory(srdir, &sb, DIR_GRP_WR))
      {
	wlog->warn("state_consistency failed to create running state directory '%s'",
		   srdir);
	r = false;
      }

      xdelete(srdir);
    }
    else
    {
      wlog->warn("state_consistency failed to allocate srdir");
      r = false;
    }
  }

  // Verify state (and history) directories

  if(r)
  {
    // For each directory under state/callist, make sure a corresponding
    // non-broadcast calllist exists.

    struct dirent *entry = allocate_dirent(scdir);
    struct dirent *dp;

    if(entry)
    {
      DIR *dirp = opendir(scdir);

      if(dirp)
      {
	while(readdir_r(dirp, entry, &dp) == 0 && dp)
	{
	  // Ignore any entry that begins with a .

	  if(dp->d_name[0] != '.')
	  {
	    // See if there is a corresponding CallList

	    CallList *cl = find_calllist(dp->d_name);

	    if(!cl || cl->listtype() == broadcast_list)
	    {
#if defined(DEBUG)
	      dlog->log_progress(DEBUG_SCHEDS | DEBUG_CONFIG,
				  "Deleting old state information for calllist '%s'",
				  dp->d_name);
#endif
	      
	      remove_directory(scdir, dp->d_name);
	    }

	    // If there is a simple CallList, unlink the lastrotate
	    // file (which might not exist).  This isn't really needed,
	    // but it is hygienic.
	    
	    if(cl && cl->listtype() == simple_list)
	    {
	      char *u = new char[strlen(scdir) + strlen(dp->d_name) + 13];
	      
	      if(u)
	      {
		sprintf(u, "%s/%s/lastrotate", scdir, dp->d_name);
		
		// Ignore errors from unlink
		unlink(u);

		xdelete(u);
	      }
	      else
		wlog->warn("state_consistency failed to allocate u");
	    }	    
	  }
	}

	closedir(dirp);
      }
      
      free(entry);
      entry = NULL;
    }
    else
    {
      wlog->warn("state_consistency failed to allocate struct dirent");
      r = false;
    }
  }
  
  if(r)
  {
    // For each non-broadcast calllist, make sure a corresponding
    // directory exists.

    if(calllistarray)
    {
      for(int i = 0;i < calllistarray->entries();i++)
      {
	CallList *cl = calllistarray->retrieve(i);
	
	if(cl)
	{

	  if(cl->listtype() != broadcast_list)
	  {
	    char *scxdir = new char[strlen(scdir) + strlen(cl->name()) + 2];
	  
	    if(scxdir)
	    {
	      sprintf(scxdir, "%s/%s", scdir, cl->name());
	      
	      if(!verify_directory(scxdir, &sb, DIR_GRP_WR))
		wlog->warn("state_consistency failed to create calllist state directory '%s'",
			   scxdir);
	      
	      xdelete(scxdir);
	    }
	    else
	      wlog->warn("state_consistency failed to allocate scxdir");
	  }

	  if(cl->listtype() == rotating_list)
	  {
	    // For rotating calllists, if the calllist has not yet
	    // rotated, start it now.

	    CallListState *cls = new CallListState(cl);

	    if(cls)
	    {
	      time_t lrot = cls->lastrotate();

	      if(lrot == 0)
	      {
		Array<Person> *ap = cl->members();

		if(ap)
		{
		  Person *p = ap->retrieve(0);

		  if(p)
		  {
		    if(!cls->notenotify("", "", "", p->name()))
		      wlog->warn("Configuration::state_consistency failed to initialize calllist '%s'", cl->name());
		  }
		  else
		    wlog->warn("Configuration::state_consistency retrieved no valid persons when initializing rotating calllist '%s'",
			       cl->name());
		}
	      }

	      xdelete(cls);
	    }
	    else
	      wlog->warn("Configuration::state_consistency failed to allocate CallListState");
	  }
	}
      }
    }
  }

  if(r)
  {
    // Remove any fix locks for hosts or services that no longer exist.
    // This is mostly hygienic.  Do not remove seemingly stale locks,
    // that it is not the purpose of state_consistency.

    struct dirent *entry = allocate_dirent(sfdir);
    struct dirent *dp;

    if(entry)
    {
      DIR *dirp = opendir(sfdir);

      if(dirp)
      {
	while(readdir_r(dirp, entry, &dp) == 0 && dp)
	{
	  if(strchr(dp->d_name, '@'))
	  {
	    // xbuf is the service
	    char *xbuf = xstrdup(dp->d_name);

	    if(xbuf)
	    {
	      // hx is the host
	      char *hx = strchr(xbuf, '@');

	      if(hx)
	      {
		*hx = '\0';
		hx++;

		// If either the service or the host no longer exists
		// (and is not the wildcard), toss the lockfile

		if(!find_check(xbuf) || !find_class(hx))
		  expire_file(sfdir, dp->d_name, 0);
	      }
	      
	      xdelete(xbuf);
	    }
	  }
	}
	
	closedir(dirp);
      }

      free(entry);
      entry = NULL;
    }
  }
  
  if(r)
  {
    // For each directory under state/service, make sure that service
    // still exists.
	      
    struct dirent *entry = allocate_dirent(ssdir);
    struct dirent *dp;

    if(entry)
    {
      DIR *dirp = opendir(ssdir);
	      
      if(dirp)
      {
	while(readdir_r(dirp, entry, &dp) == 0 && dp)
	{
	  // Ignore any entries that begin with a .
	  
	  if(dp->d_name[0] != '.')
	  {
	    // See if there is a corresponding Check object.
	    
	    Check *c = find_check(dp->d_name);
		    
	    if(!c)
	    {
#if defined(DEBUG)
	      dlog->log_progress(DEBUG_SCHEDS | DEBUG_CONFIG,
				  "Deleting old state information for service '%s'",
				  dp->d_name);
#endif
	      
	      remove_directory(ssdir, dp->d_name);
	    }
	  }
	}
	
	closedir(dirp);
      }
      
      free(entry);
      entry = NULL;
    }
    else
    {
      wlog->warn("state_consistency failed to allocate struct dirent");
      r = false;
    }
  }

  if(r)
  {
    // For each directory under state/host, make sure that host is
    // still defined
	      
    struct dirent *entry = allocate_dirent(shdir);
    struct dirent *dp;

    if(entry)
    {
      DIR *dirp = opendir(shdir);
	      
      if(dirp)
      {
	while(readdir_r(dirp, entry, &dp) == 0 && dp)
	{
	  // Ignore any entries that begin with a .
	  
	  if(dp->d_name[0] != '.')
	  {
	    // See if there is a HostClass for this hostname
	    
	    if(find_class(dp->d_name))
	    {
	      // For each directory under 'host/hostname', make sure
	      // that host is still in the group with that name

	      char *shxdir = new char[strlen(shdir) + strlen(dp->d_name) + 2];

	      if(shxdir)
	      {
		sprintf(shxdir, "%s/%s", shdir, dp->d_name);

		struct dirent *entry2 = allocate_dirent(shxdir);
		struct dirent *dp2;

		if(entry2)
		{
		  DIR *dirp2 = opendir(shxdir);
			
		  if(dirp2)
		  {
		    while(readdir_r(dirp2, entry2, &dp2) == 0 && dp2)
		    {
		      // Ignore any entries that begin with a .
			    
		      if(dp2->d_name[0] != '.')
		      {
			// See if there is a Group with this name, and
			// if so, if this host is a member of this Group
			
			List *g = find_group(dp2->d_name);
			
			if(!g || g->find(dp->d_name) == -1)
			{
			  // If not, see if there is a Check with this name,
			  // and if that Check checks all.

			  Check *c = find_check(dp->d_name);

			  if(!c || !c->all_hosts())
			  {
#if defined(DEBUG)
			    dlog->log_progress(DEBUG_SCHEDS | DEBUG_CONFIG,
						"Deleting old state information for %s@%s",
						dp2->d_name, dp->d_name);
#endif			  
			    remove_directory(shxdir, dp2->d_name);
			  }
			}
		      }
		    }
			  
		    closedir(dirp2);
		  }
		  else
		    wlog->warn("state_consistency failed to allocate dirent");

		  free(entry2);
		  entry2 = NULL;
		}

		xdelete(shxdir);
	      }
	      else
		wlog->warn("state_consistency failed to allocate shxdir");
	    }
	    else
	    {
#if defined(DEBUG)
	      dlog->log_progress(DEBUG_SCHEDS | DEBUG_CONFIG,
				  "Deleting old state information for host '%s'",
				  dp->d_name);
#endif
	      
	      remove_directory(shdir, dp->d_name);
	    }
	  }
	}
		
	closedir(dirp);
      }

      free(entry);
      entry = NULL;
    }
    else
    {
      wlog->warn("state_consistency failed to allocate struct dirent");
      r = false;
    }
  }

  if(r)
  {
    // For each Check object defined, make sure a corresponding
    // service directory exists.  Only services with an override
    // schedule keep any state here, but all services write here
    // for CheckState->tmp_state_filename().
	      
    if(checkarray)
    {
      for(int i = 0;i < checkarray->entries();i++)
      {
	Check *c = checkarray->retrieve(i);
	
	if(c && c->name())
	{
	  char *ssxdir = new char[strlen(ssdir) + strlen(c->name()) + 2];
		    
	  if(ssxdir)
	  {
	    sprintf(ssxdir, "%s/%s", ssdir, c->name());
		      
	    if(!verify_directory(ssxdir, &sb, DIR_GRP_WR))
	      wlog->warn("state_consistency failed to create service state directory '%s'",
			 ssxdir);

	    xdelete(ssxdir);
	  }
	  else
	    wlog->warn("state_consistency failed to allocate ssxdir");
	}
      }
    }
  }
  
  if(r)
  {
    // For each host, make sure a corresponding directory exists, both
    // under the state directory and under the history directory.
	      
    if(classlist && classhash)
    {
      for(int i = 0;i < classlist->entries();i++)
      {
	HostClass *hc = classhash->retrieve(classlist->retrieve(i));
	
	if(hc)
	{
	  List *hosts = hc->hosts();
	  
	  if(hosts)
	  {
	    for(int j = 0;j < hosts->entries();j++)
	    {
	      if(hosts->retrieve(j))
	      {
		char *shxdir = new char[strlen(shdir) +
				       strlen(hosts->retrieve(j)) + 2];
		
		if(shxdir)
		{
		  sprintf(shxdir, "%s/%s", shdir, hosts->retrieve(j));
			    
		  if(!verify_directory(shxdir, &sb, DIR_GRP_WR))
		    wlog->warn("Unable to create host state directory '%s'",
			       shxdir);

		  xdelete(shxdir);
		}
		else
		  wlog->warn("state_consistency failed to allocate shxdir");
			  
		char *hhxdir = new char[strlen(hhdir) +
				       strlen(hosts->retrieve(j)) + 2];
			  
		if(hhxdir)
		{
		  sprintf(hhxdir, "%s/%s", hhdir, hosts->retrieve(j));
			    
		  if(!verify_directory(hhxdir, &sb, DIR_GRP_RD))
		    wlog->warn("Unable to create host history directory '%s'",
			       hhxdir);

		  xdelete(hhxdir);
		}
		else
		  wlog->warn("state_consistency failed to allocate hhxdir");
	      }
	    }
	  }
	}
      }
    }
  }

  if(r)
  {      
    // For each defined Check, retrieve its corresponding group and
    // make sure a service directory exists for each host in that
    // group.  Only do this after the host directories are verified.
    // Do this for all Checks, even those with override schedules, for
    // both state and history directories.
	      
    if(checkarray)
    {
      for(int i = 0;i < checkarray->entries();i++)
      {
	Check *c = checkarray->retrieve(i);
	
	if(c && c->name())
	{
	  List *g = find_group(c->name());
	  
	  if(g)
	  {
	    for(int j = 0;j < g->entries();j++)
	    {
	      char *host = g->retrieve(j);
	      
	      if(host)
	      {
		CharBuffer *shxdir = new CharBuffer();
			  
		if(shxdir)
		{
		  shxdir->append(shdir);
		  shxdir->append("/");
		  shxdir->append(host);
		  shxdir->append("/");
		  shxdir->append(c->name());
			    
		  if(!verify_directory(shxdir->str(), &sb, DIR_GRP_WR))
		    wlog->warn("Unable to create host service state directory '%s'",
			       shxdir->str());

		  // Create a last check of 0 to avoid spurious errors
		  // before the check has a chance to run.  (The lock
		  // file will be created by the inquiring client.)

		  shxdir->append("/");
		  shxdir->append("checkstatus");
		  
		  int fd = open(shxdir->str(), O_WRONLY | O_CREAT | O_EXCL,
				FILE_GRP_WR);

		  if(fd > -1)
		  {
		    CheckStateData *newdata =
		      new CheckStateData(0, MODEXEC_NOTYET, "", 0, 0, -1);
		    SurvivorXML *sxml = new SurvivorXML();

		    if(newdata && sxml)
		    {
		      if(!sxml->generate(fd, newdata))
			wlog->warn("state_consistency failed to generate initial checkstatus for %s@%s",
				   c->name(), host);
		    }
		    else
		      wlog->warn("state_consistency failed to allocate CheckStateData or SurvivorXML");

		    xdelete(newdata);
		    xdelete(sxml);

		    close(fd);
		  }
		  // else checkstatus probably exists already.  We don't
		  // want to reset the contents, so ignore the error.
		  
		  xdelete(shxdir);
		}
		else
		  wlog->warn("state_consistency failed to allocate shxdir");

		char *hhxdir = new char[strlen(hhdir) + strlen(host)
				       + strlen(c->name()) + 3];
			    
		if(hhxdir)
		{
		  // This directory should really be DIR_GRP_RD, but
		  // sc currently needs group write permission in order
		  // to write fixhistory.
		  
		  sprintf(hhxdir, "%s/%s/%s", hhdir, host, c->name());
			      
		  if(!verify_directory(hhxdir, &sb, DIR_GRP_WR))
		    wlog->warn("Unable to create host service history directory '%s'",
			       hhxdir);

		  xdelete(hhxdir);
		}
		else
		  wlog->warn("state_consistency failed to allocate hhxdir");
	      }
	    }
	  }
	}
      }
    }
  }
  
  // Clean up dynamically allocated stuff.

  xdelete(hhdir);
  xdelete(scdir);
  xdelete(sfdir);
  xdelete(shdir);
  xdelete(ssdir);

  // Restore the old umask

  umask(u);
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "Configuration::state_consistency = %s",
		  IOTF(r));
#endif
  
  return(r);
}

Configuration::~Configuration()
{
  // Deallocate the Configuration object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "Configuration::~Configuration()");
#endif

#if defined(DEBUG)
  dlog->log_progress(DEBUG_MAJMEM, "Cleaning AlertModule Hashtable");
#endif

  xhdelete(alertmodhash, AlertModule);
  
#if defined(DEBUG)
  dlog->log_progress(DEBUG_MAJMEM, "Cleaning AlertPlan Hashtable");
#endif

  xhdelete(alerthash, AlertPlan);

  // calllistarray points to the same objects as calllisthash, but
  // calllisthash is inserted into first, so we delete from it (since an
  // add to calllisthash might have failed)

  xdelete(calllistarray);
  
#if defined(DEBUG)
  dlog->log_progress(DEBUG_MAJMEM, "Cleaning CallList Hashtable");
#endif

  xhdelete(calllisthash, CallList);

  // checkarray points to the same objects as checkhash, but checkhash is
  // inserted into first, so we delete from it (since an add to checkhash
  // might have failed)

  xdelete(checkarray);
  
#if defined(DEBUG)
  dlog->log_progress(DEBUG_MAJMEM, "Cleaning Check Hashtable");
#endif

  xhdelete(checkhash, Check);
    
  // classbymember uses the same HostClass objects as classhash, so it need
  // merely be deleted

  xdelete(classbymember);

#if defined(DEBUG)
  dlog->log_progress(DEBUG_MAJMEM, "Cleaning HostClass Hashtable");
#endif

  xhdelete(classhash, HostClass);

#if defined(DEBUG)
  dlog->log_progress(DEBUG_MAJMEM, "Cleaning Dependency Array");
#endif

  xadelete(deparray, Dependency);
  
#if defined(DEBUG)
  dlog->log_progress(DEBUG_MAJMEM, "Cleaning Fix Hashtable");
#endif
  
  // fixarray points to the same objects as fixhash, but fixhash is
  // inserted into first, so we delete from it (since an add to
  // fixhash might have failed)

  xdelete(fixarray);
  xhdelete(fixhash, Fix);

#if defined(DEBUG)
  dlog->log_progress(DEBUG_MAJMEM, "Cleaning Group List Hashtable");
#endif

  xhdelete(grouphash, List);

#if defined(DEBUG)
  dlog->log_progress(DEBUG_MAJMEM, "Cleaning Group by Member Hashtable");
#endif

  xhdelete(groupbymember, List);

#if defined(DEBUG)
  dlog->log_progress(DEBUG_MAJMEM, "Cleaning Person Hashtable");
#endif

  xhdelete(personhash, Person);

#if defined(DEBUG)
  dlog->log_progress(DEBUG_MAJMEM, "Cleaning Return Group Hashtable");
#endif

  xhdelete(returngrouphash, Array<int>);
  
  if(schedhash)
  {
#if defined(DEBUG)
    dlog->log_progress(DEBUG_MAJMEM, "Cleaning Schedule Hashtable");
#endif
    
    HashHandle *h = schedhash->iterate_begin();
    
    if(h)
    {
      Array<Schedule> *a;

      while((a = schedhash->iterate_next(h)) != NULL)
      {
	// Delete each Schedule from each Array.  We don't worry about
	// removing the now invalid Schedule pointer from the Array because
	// we're about to toss the Array anyway.

	xadelete(a, Schedule);
      }

      schedhash->iterate_end(h);
    }

    xdelete(schedhash);
  }

#if defined(DEBUG)
  dlog->log_progress(DEBUG_MAJMEM, "Cleaning Transport Hashtable");
#endif

  xhdelete(transporthash, Transport);

  xdelete(classlist);
  xdelete(hostlist);
  xdelete(returngrouplist);

  as = -1;
  at = -1;
  m = -1;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "Configuration::~Configuration()");
#endif
}

bool Configuration::add_all_hosts(List *l)
{
  // Add all hosts not already in <l> into it.

  // Returns: true if fully successful, false otherwise.

  bool r = true;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Configuration::add_all_hosts(%d)", l);
#endif

  if(l && hostlist)
  {
    for(int i = 0;i < hostlist->entries();i++)
    {
      // This is not the most efficient algorithm ever, since l->find
      // is linear, but it shouldn't be used often (or at all, really).
      
      char *h = hostlist->retrieve(i);

      if(h && l->find(h)==-1 && !l->add(h))
	r = false;
    }
  }
  else
    r = false;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Configuration::add_all_hosts = %s", IOTF(r));
#endif
  
  return(r);
}

List *Configuration::find_all_hosts(char *name)
{
  // Find all hosts and store them in a new List named <name>.
  // This method is much more efficient than add_all_hosts.

  // Returns: A newly allocated List if successful, NULL otherwise.

  List *r = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Configuration::find_all_hosts(%s)",
		   IONULL(name));
#endif
  
  if(name && hostlist)
  {
    r = new List(name);

    if(r)
    {
      for(int i = 0;i < hostlist->entries();i++)
	r->add(hostlist->retrieve(i));
    } 
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Configuration::find_all_hosts = %d", r);
#endif  
  
  return(r);
}

bool Configuration::remove_directory(char *parent, char *dir)
{
  // Recursively remove the directory <dir>, which is under the directory
  // <parent>.  This method performs little or no error checking.

  // Returns: true if successful, false otherwise.

  bool r = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Configuration::remove_directory(%s,%s)",
		   IONULL(parent), IONULL(dir));
#endif

  // As a sanity check, parent and dir must each be at least one character
  // in length
  
  if(parent && dir && strlen(parent) > 0 && strlen(dir) > 0)
  {    
    char *rdir = new char[strlen(parent) + strlen(dir) + 2];

    if(rdir)
    {
      sprintf(rdir, "%s/%s", parent, dir);

      struct dirent *entry = allocate_dirent(rdir);
      struct dirent *dp;

      if(entry)
      {
	DIR *dirp = opendir(rdir);
	
	if(dirp)
	{
	  // Call recursively
	
	  while(readdir_r(dirp, entry, &dp) == 0 && dp)
	  {
	    // Ignore . and ..
	    
	    if(strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
	      remove_directory(rdir, dp->d_name);
	  }
	  
	  closedir(dirp);
	}
	// else probably not a directory

	free(entry);
	entry = NULL;
      }

      // Rather than stat to figure out whether to use unlink or rmdir,
      // just run them both and ignore any errors
      unlink(rdir);
      rmdir(rdir);
      
      r = true;

      xdelete(rdir);
    }
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Configuration::remove_directory = %s",
		  IOTF(r));
#endif
  
  return(r);
}
