/*
 * RecipientMethod.C: An object to hold recipient information.
 *
 * Version: $Revision: 0.7 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2005/04/07 01:26:57 $
 * MT-Level: Safe
 *
 * Copyright (c) 2002 - 2005
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: RecipientMethod.C,v $
 * Revision 0.7  2005/04/07 01:26:57  benno
 * Fix typo
 *
 * Revision 0.6  2003/05/29 00:33:42  benno
 * Changes for survivor.dtd
 *
 * Revision 0.5  2003/04/09 20:23:49  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.4  2003/04/05 23:46:58  benno
 * Use Debugger
 *
 * Revision 0.3  2003/03/04 20:29:46  benno
 * Bump copyright
 *
 * Revision 0.2  2003/01/24 18:03:34  benno
 * Use IONULL, IOTF, xdelete, and xadelete
 *
 * Revision 0.1  2002/12/16 00:31:28  benno
 * Initial revision
 *
 */

#include "survivor.H"

RecipientMethod::RecipientMethod(char *module)
{
  // Allocate and initialize a new RecipientMethod object for the
  // module <module>.

  // Returns: A new RecipientMethod object.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "RecipientMethod::RecipientMethod(%s)",
		   IONULL(module));
#endif

  mod = xstrdup(module);
  addrs = new List();
  cls = new List();
  maxrc = MODEXEC_OK;

  if(!addrs || !cls)
    wlog->warn("RecipientMethod::RecipientMethod failed to initialize");
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "RecipientMethod::RecipientMethod()");
#endif  
}

RecipientMethod::RecipientMethod(RecipientMethod *r)
{
  // Allocate and initialize a new RecipientMethod object, duplicating
  // the contents of <r>.

  // Returns: A new RecipientMethod object.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "RecipientMethod::RecipientMethod(%d)", r);
#endif

  mod = NULL;
  addrs = NULL;
  cls = NULL;
  maxrc = MODEXEC_OK;

  if(r)
  {
    mod = xstrdup(r->mod);

    if(r->addrs)
    {
      addrs = new List(r->addrs);

      if(!addrs)
	wlog->warn("RecipientMethod::RecipientMethod failed to allocate List");
    }

    if(r->cls)
    {
      cls = new List(r->cls);

      if(!cls)
	wlog->warn("RecipientMethod::RecipientMethod failed to allocate List");
    }

    maxrc = r->maxrc;
  }

  if(!addrs)
    addrs = new List();

  if(!cls)
    cls = new List();
  
  if(!addrs || !cls)
    wlog->warn("RecipientMethod::RecipientMethod failed to initialize");
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "RecipientMethod::RecipientMethod()");
#endif  
}

bool RecipientMethod::add(char *address, char *calllist)
{
  // Add the <address> (which may be a comma separated list of addresses)
  // from the <calllist> to this RecipientMethod.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "RecipientMethod::add(%s,%s)",
		   IONULL(address), IONULL(calllist));
#endif

  if(address && calllist && addrs && cls)
  {
    // We don't try to keep address and calllist correlated, since it
    // is somewhat difficult and of little benefit.  We also don't try
    // to see if either is already stored within.

    if(addrs->add(address) && cls->add(calllist))
      ret = true;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "RecipientMethod::add = %s", IOTF(ret));
#endif
  
  return(ret);
}

bool RecipientMethod::add(char *address, int rc)
{
  // Add the <address> (which may be a comma separated list of addresses)
  // to this RecipientMethod.  Only the highest value of <rc> added,
  // if this method is called multiple times, will be remembered.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "RecipientMethod::add(%s,%d)",
		  IONULL(address), rc);
#endif

  if(address && addrs)
  {
    if(addrs->add(address))
    {
      if(rc > maxrc)
	maxrc = rc;
      
      ret = true;
    }
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "RecipientMethod::add = %s", IOTF(ret));
#endif
  
  return(ret);
}

bool RecipientMethod::add_address(char *address)
{
  // Add <address> to the addresses stored within.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "RecipientMethod::add_address(%s)",
		  IONULL(address));
#endif
  
  if(address && addrs && addrs->add(address))
    ret = true;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "RecipientMethod::add_address = %s", IOTF(ret));
#endif
  
  return(ret);
}

bool RecipientMethod::add_calllist(char *calllist)
{
  // Add <calllist> to the calllists stored within.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "RecipientMethod::add_calllist(%s)",
		  IONULL(calllist));
#endif
  
  if(calllist && cls && cls->add(calllist))
    ret = true;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "RecipientMethod::add_calllist = %s",
		 IOTF(ret));
#endif
  
  return(ret);
}

char *RecipientMethod::addresses()
{
  // Obtain the comma separated list of addresses stored within.

  // Returns: A comma separated list of addresses that should not be
  // modified, or NULL.

  char *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "RecipientMethod::addresses()");
#endif

  if(addrs)
    ret = addrs->comma_list();
 
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC,
		  "RecipientMethod::addresses = %s", IONULL(ret));
#endif
  
  return(ret);
}

List *RecipientMethod::address_list()
{
  // Obtain the list of addresses stored within.

  // Returns: A List of addresses that should not be modified, or
  // NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "RecipientMethod::address_list()");
  dlog->log_exit(DEBUG_MINTRC, "RecipientMethod::address_list = %d", addrs);
#endif
  
  return(addrs);
}

char *RecipientMethod::calllists()
{
  // Obtain the comma separated list of calllists stored within.

  // Returns: A comma separated list of calllists that should not be
  // modified, or NULL.

  char *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "RecipientMethod::calllists()");
#endif

  if(cls)
    ret = cls->comma_list();
 
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "RecipientMethod::calllists = %s",
		  IONULL(ret));
#endif
  
  return(ret);
}

List *RecipientMethod::calllist_list()
{
  // Obtain the list of calllists stored within.

  // Returns: A List of calllists that should not be modified, or
  // NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "RecipientMethod::calllist_list()");
  dlog->log_exit(DEBUG_MINTRC, "RecipientMethod::calllist_list = %d", cls);
#endif
  
  return(cls);
}

char *RecipientMethod::module()
{
  // Obtain the name of the module this data is for.

  // Returns: The name of the module, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "RecipientMethod::module()");
  dlog->log_exit(DEBUG_MINTRC, "RecipientMethod::module = %s", IONULL(mod));
#endif
  
  return(mod);
}

int RecipientMethod::rc()
{
  // Obtain the maximum rc found for this module.

  // Returns: The maximum return code found.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "RecipientMethod::rc()");
  dlog->log_exit(DEBUG_MINTRC, "RecipientMethod::rc = %s", maxrc);
#endif
  
  return(maxrc);
}

RecipientMethod::~RecipientMethod()
{
  // Delete the RecipientMethod object.

  // Returns: Nothing.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "RecipientMethod::~RecipientMethod()");
#endif

  xdelete(mod);
  xdelete(addrs);
  xdelete(cls);
  
  maxrc = MODEXEC_OK;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "RecipientMethod::~RecipientMethod()");
#endif  
}
