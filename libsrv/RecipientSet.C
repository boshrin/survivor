/*
 * RecipientSet.C: An object to hold recipient information.
 *
 * Version: $Revision: 0.6 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2003/05/29 00:33:26 $
 * MT-Level: Safe
 *
 * Copyright (c) 2002 - 2003
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: RecipientSet.C,v $
 * Revision 0.6  2003/05/29 00:33:26  benno
 * Changes for AlertModule and survivor.dtd
 *
 * Revision 0.5  2003/04/09 20:23:50  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.4  2003/04/05 23:57:25  benno
 * Use Debugger
 *
 * Revision 0.3  2003/03/04 20:30:03  benno
 * Fix typos
 *
 * Revision 0.2  2003/01/24 18:05:58  benno
 * Use IONULL, IOTF, xdelete, and xadelete
 *
 * Revision 0.1  2002/12/16 00:31:45  benno
 * Initial revision
 *
 */

#include "survivor.H"

RecipientSet::RecipientSet()
{
  // Allocate and initialize a new RecipientSet object.

  // Returns: A new RecipientSet object.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "RecipientSet::RecipientSet()");
#endif
  
  rcps = NULL;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "RecipientSet::RecipientSet()");
#endif
}

RecipientSet::RecipientSet(RecipientSet *rset)
{
  // Allocate and initialize a new RecipientSet object, duplicating the
  // contents of <rset>.

  // Returns: A new RecipientSet object.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "RecipientSet::RecipientSet(%d)", rset);
#endif
  
  rcps = new Array<RecipientMethod>();

  if(rcps)
  {
    if(rset && rset->rcps)
    {
      for(int i = 0;i < rset->rcps->entries();i++)
      {
	RecipientMethod *orig = rset->rcps->retrieve(i);

	if(orig)
	{
	  RecipientMethod *newrm = new RecipientMethod(orig);

	  if(newrm)
	  {
	    if(!rcps->add(newrm))
	      delete newrm;
	  }
	  else
	    wlog->warn("RecipientSet::RecipientSet failed to allocate RecipientMethod");
	}
      }
    }
  }
  else
    wlog->warn("RecipientSet::RecipientSet failed to allocate Array");
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "RecipientSet::RecipientSet()");
#endif
}

bool RecipientSet::add(char *address, char *calllist, char *module)
{
  // Add the <address>,<calllist>,<module> tuple to the RecipientSet.
  // <address> may be a single address, or a comma separated set of
  // addresses.  <calllist> will not remain strictly correlated to
  // <address>.  Only one add() method should be used per
  // RecipientSet.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "RecipientSet::add(%s,%s,%s)",
		  IONULL(address), IONULL(calllist), IONULL(module));
#endif

  if(address && calllist && module)
  {
    RecipientMethod *found = find_rm(module);
    
    if(found)
    {
      // We have a valid RecipientMethod for the module, so add the address
      // and calllist.
      
      if(found->add(address, calllist))
	ret = true;
    }
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "RecipientSet::add = %s", IOTF(ret));
#endif

  return(ret);
}

bool RecipientSet::add(char *address, char *module, int rc)
{
  // Add the <address>,<module>,<rc> tuple to the RecipientSet.
  // <address> may be a single address, or a comma separated set of
  // addresses.  If this method is called multiple times for the same
  // module, only the highest <rc> will be retained for that module.
  // Only one add() method should be used per RecipientSet.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "RecipientSet::add(%s,%s,%d)",
		  IONULL(address), IONULL(module), rc);
#endif

  if(address && module)
  {
    RecipientMethod *found = find_rm(module);
    
    if(found)
    {
      // We have a valid RecipientMethod for the module, so add the address.
      
      if(found->add(address, rc))
	ret = true;
    }
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "RecipientSet::add = %s", IOTF(ret));
#endif

  return(ret);
}

bool RecipientSet::add_address(char *address, char *module)
{
  // Add the <address>,<module> pair to the RecipientSet.  Addresses
  // and CallLists do not remain strictly correlated.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "RecipientSet::add_address(%s,%s)",
		  IONULL(address), IONULL(module));
#endif

  if(address && module)
  {
    RecipientMethod *found = find_rm(module);
    
    if(found)
    {
      // We have a valid RecipientMethod for the module, so add the address.
      
      if(found->add_address(address))
	ret = true;
    }
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "RecipientSet::add_address = %s", IOTF(ret));
#endif

  return(ret);
}

bool RecipientSet::add_calllist(char *calllist, char *module)
{
  // Add the <calllist>,<module> pair to the RecipientSet.  Addresses
  // and CallLists do not remain strictly correlated.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "RecipientSet::add_calllist(%s,%s)",
		  IONULL(calllist), IONULL(module));
#endif

  if(calllist && module)
  {
    RecipientMethod *found = find_rm(module);
    
    if(found)
    {
      // We have a valid RecipientMethod for the module, so add the calllist.
      
      if(found->add_calllist(calllist))
	ret = true;
    }
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "RecipientSet::add_calllist = %s", IOTF(ret));
#endif

  return(ret);
}

char *RecipientSet::addresses(int module)
{
  // Obtain the addresses to be contacted via the module at index <module>,
  // numbered from 0 through modules()-1.

  // Returns: The comma separated list of addresses of the module at
  // position <module> that should not be modified, or NULL if not found.

  char *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "RecipientSet::addresses(%d)", module);
#endif
  
  if(rcps)
  {
    RecipientMethod *m = rcps->retrieve(module);

    if(m)
      ret = m->addresses();
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "RecipientSet::addresses = %s", IONULL(ret));
#endif
  
  return(ret);
}

List *RecipientSet::address_list(int module)
{
  // Obtain the list of addresses to be contacted via the module at
  // index <module>, numbered from 0 through modules()-1.

  // Returns: The List of addresses of the module at position <module>
  // that should not be modified, or NULL if not found.

  List *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "RecipientSet::address_list(%d)", module);
#endif
  
  if(rcps)
  {
    RecipientMethod *m = rcps->retrieve(module);

    if(m)
      ret = m->address_list();
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "RecipientSet::addresses_list = %d", ret);
#endif
  
  return(ret);
}

char *RecipientSet::calllists(int module)
{
  // Obtain the calllists used to generate the contacts via the module
  // at index <module>, numbered from 0 through modules()-1.

  // Returns: The comma separated list of calllists at position
  // <module> that should not be modified, or NULL if not found.

  char *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "RecipientSet::calllists(%d)", module);
#endif
  
  if(rcps)
  {
    RecipientMethod *m = rcps->retrieve(module);

    if(m)
      ret = m->calllists();
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "RecipientSet::calllists = %s", IONULL(ret));
#endif
  
  return(ret);
}

List *RecipientSet::calllist_list(int module)
{
  // Obtain the list of calllists used to generate the contacts via
  // the module at index <module>, numbered from 0 through
  // modules()-1.

  // Returns: The List of calllists of the module at position <module>
  // that should not be modified, or NULL if not found.

  List *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "RecipientSet::calllist_list(%d)", module);
#endif
  
  if(rcps)
  {
    RecipientMethod *m = rcps->retrieve(module);

    if(m)
      ret = m->calllist_list();
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "RecipientSet::calllist_list = %d", ret);
#endif
  
  return(ret);
}

char *RecipientSet::module(int module)
{
  // Obtain the name of the module at index <module>, numbered from 0
  // through modules()-1.

  // Returns: The name of the module at position <module>, or NULL if
  // not found.

  char *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "RecipientSet::module(%d)", module);
#endif
  
  if(rcps)
  {
    RecipientMethod *m = rcps->retrieve(module);

    if(m)
      ret = m->module();
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "RecipientSet::module = %s", IONULL(ret));
#endif
  
  return(ret);
}

int RecipientSet::modules()
{
  // Determine how many modules are known to this RecipientSet.

  // Returns: The number of modules.

  int ret = 0;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "RecipientSet::modules()");
#endif
  
  if(rcps)
    ret = rcps->entries();
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "RecipientSet::modules = %d", ret);
#endif
  
  return(ret);
}

int RecipientSet::rc(int module)
{
  // Obtain the return code for the module at index <module>,
  // numbered from 0 through modules()-1.

  // Returns: The return code.

  int ret = MODEXEC_MAXRETURN;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "RecipientSet::rc(%d)", module);
#endif
  
  if(rcps)
  {
    RecipientMethod *m = rcps->retrieve(module);

    if(m)
      ret = m->rc();
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "RecipientSet::rc = %d", ret);
#endif
  
  return(ret);
}

RecipientSet::~RecipientSet()
{
  // Deallocate the RecipientSet object.

  // Returns: Nothing.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "RecipientSet::~RecipientSet()");
#endif

  xadelete(rcps, RecipientMethod);
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "RecipientSet::~RecipientSet()");
#endif
}

RecipientMethod *RecipientSet::find_rm(char *module)
{
  // Find the RecipientMethod for <module>, creating one if necessary.

  // Returns: A pointer to a RecipientMethod object if successful, NULL
  // otherwise.

  RecipientMethod *ret = NULL;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "RecipientSet::find_rm(%s)", IONULL(module));
#endif
  
  if(module)
  {
    // We use an Array rather than a Hashtable because there should be
    // relatively few modules, so the Hashtable will add more overhead.
    
    if(!rcps)
      rcps = new Array<RecipientMethod>();
    
    if(rcps)
    {
      // First look for a RecipientMethod for the requested module.
      // If none is found, create one.

      for(int i = 0;i < rcps->entries();i++)
      {
	RecipientMethod *m = rcps->retrieve(i);

	if(m && m->module() && strcmp(m->module(), module)==0)
	{
	  ret = m;
	  break;
	}
      }

      if(!ret)
      {
	// Allocate a new RecipientMethod, we haven't seen this module before.
	// Stuff it into the Array.
	  
	ret = new RecipientMethod(module);

	if(ret)
	{
	  if(!rcps->add(ret))
	  {
	    xdelete(ret);
	  }
	}
	else
	  wlog->warn("RecipientSet::find_rm failed to allocate RecipientMethod");
      }
    }
    else
      wlog->warn("RecipientSet::add failed to allocate Array");
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "RecipientSet::find_rm = %d", ret);
#endif
  
  return(ret);
}
