/*
 * Person.C: survivor Person object
 *
 * Version: $Revision: 0.6 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2003/05/29 00:33:53 $
 * MT-Level: Safe
 *
 * Copyright (c) 2002 - 2003
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: Person.C,v $
 * Revision 0.6  2003/05/29 00:33:53  benno
 * Changes for AlertModule
 *
 * Revision 0.5  2003/04/09 20:23:49  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.4  2003/04/05 23:29:46  benno
 * Use Debugger
 *
 * Revision 0.3  2003/01/24 17:58:41  benno
 * Add IONULL and IOTF
 *
 * Revision 0.2  2002/12/31 04:33:17  benno
 * Use xdelete
 *
 * Revision 0.1  2002/10/25 22:59:28  benno
 * Initial revision
 *
 */

#include "survivor.H"

Person::Person(char *name)
{
  // Allocate and initialize a new Person object, for the Person
  // identified by <name>.

  // Returns: A new Person object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Person::Person(%s)", IONULL(name));
  dlog->log_exit(DEBUG_MINTRC, "Person::Person()");
#endif

  n = xstrdup(name);
  cs = NULL;
}

bool Person::add_contact(Contact *c)
{
  // Add the Contact <c> to this Person.  <c> will be maintained by this
  // object.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Person::add_contact(%d)", c);
#endif

  if(c)
  {
    // Make sure our Array exists

    if(!cs)
      cs = new Array<Contact>();

    if(cs)
    {
      if(cs->add(c))
	ret = true;
    }
    else
      wlog->warn("Person::add_contact failed to allocate Array");
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Person::add_contact = %s", IOTF(ret));
#endif
  
  return(ret);
}

char *Person::find_address(char *module)
{
  // Find an address for this Person for use with <module>.

  // Returns: A pointer to a string containing the address and that
  // should not be modified if found, NULL otherwise.

  char *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Person::find_address(%s)", IONULL(module));
#endif

  if(module && cs)
  {
    // Iterate through all the Contacts.  If we find an exact match
    // on the module name, return that one.  If we find a NULL
    // module name, return that only if we reach the end of the array.
    
    char *def = NULL;

    for(int i = 0;i < cs->entries();i++)
    {
      Contact *c = cs->retrieve(i);

      if(c)
      {
	if(c->module())
	{
	  if(c->module()->name() && strcmp(module, c->module()->name())==0)
	  {
	    ret = c->address();
	    break;
	  }
	}
	else
	{
	  // Only take the first default we see.
	  
	  if(!def)
	    def = c->address();
	}
      }
    }

    // Use the default only if we haven't matched something else.

    if(!ret && def)
      ret = def;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Person::find_address = %s", IONULL(ret));
#endif

  return(ret);
}

char *Person::name()
{
  // Obtain the name of this Person.

  // Returns: The name.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Person::name()");
  dlog->log_exit(DEBUG_MINTRC, "Person::name = %s", IONULL(n));
#endif
  
  return(n);
}

Person::~Person()
{
  // Delete the Person object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Person::~Person()");
#endif

  xdelete(n);

  // Iterate through and delete all the Contacts
  xadelete(cs, Contact);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Person::~Person()");
#endif
}
