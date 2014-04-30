/*
 * Contact.C: survivor Contact object
 *
 * Version: $Revision: 0.6 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2003/05/29 00:34:40 $
 * MT-Level: Safe
 *
 * Copyright (c) 2002 - 2003
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: Contact.C,v $
 * Revision 0.6  2003/05/29 00:34:40  benno
 * Changes for AlertModule
 *
 * Revision 0.5  2003/04/09 20:23:47  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.4  2003/04/04 22:20:09  benno
 * Use Debugger
 *
 * Revision 0.3  2003/03/04 17:58:06  benno
 * Bump copyright
 *
 * Revision 0.2  2003/01/24 16:46:06  benno
 * Use IONULL, IOTF, xdelete, and xadelete
 *
 * Revision 0.1  2002/10/25 22:58:54  benno
 * Initial revision
 *
 */

#include "survivor.H"

Contact::Contact(char *address)
{
  // Allocate and initialize a new Contact object, used to store <address>.
  // This <address> is considered a default address, suitable for use
  // when no matching module would otherwise be found.

  // Returns: A new Contact object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Contact::Contact(%s)", IONULL(address));
  dlog->log_exit(DEBUG_MINTRC, "Contact::Contact()");
#endif

  a = xstrdup(address);
  m = NULL;
}

Contact::Contact(char *address, AlertModule *module)
{
  // Allocate and initialize a new Contact object, used to store <address>
  // and <module>.

  // Returns: A new Contact object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Contact::Contact(%s,%d)",
		   IONULL(address), module);
  dlog->log_exit(DEBUG_MINTRC, "Contact::Contact()");
#endif

  a = xstrdup(address);
  m = module;
}

char *Contact::address()
{
  // Obtain the address for this Contact.

  // Returns: The address.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Contact::address()");
  dlog->log_exit(DEBUG_MINTRC, "Contact::address = %s", IONULL(a));
#endif
  
  return(a);
}

AlertModule *Contact::module()
{
  // Obtain the AlertModule for this Contact.

  // Returns: A pointer to the AlertModule.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Contact::module()");
  dlog->log_exit(DEBUG_MINTRC, "Contact::module = %d", m);
#endif
  
  return(m);
}

Contact::~Contact()
{
  // Delete the Contact object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Contact::~Contact()");
#endif

  xdelete(a);
  m = NULL;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Contact::~Contact()");
#endif
}
