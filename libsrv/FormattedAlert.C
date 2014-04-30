/*
 * FormattedAlert.C: survivor FormattedAlert information object
 *
 * Version: $Revision: 0.2 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2003/10/06 23:14:21 $
 * MT-Level: Safe
 *
 * Copyright (c) 2003
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: FormattedAlert.C,v $
 * Revision 0.2  2003/10/06 23:14:21  benno
 * Add support for replyok
 *
 * Revision 0.1  2003/07/30 21:43:46  benno
 * Initial revision
 *
 */

#include "survivor.H"

FormattedAlert::FormattedAlert(List *addresses, char *subject, char *message,
			       bool replyok)
{
  // Allocate and initialize a new FormattedAlert object, where <addresses>
  // contains the addresses to notify, using <subject> and <message>.
  // <addresses> will be maintained by this FormattedAlert object.
  // If <replyok> is true, then two-way replies are suitable for this
  // alert.

  // Returns: A new, initialized FormattedAlert.
  
  init(addresses, NULL, subject, message, replyok);
}

FormattedAlert::FormattedAlert(List *addresses, char *replyto, char *subject,
			       char *message)
{
  // Allocate and initialize a new FormattedAlert object, where
  // <addresses> contains the addresses to notify and <replyto> is the
  // return address, using <subject> and <message>.  <addresses> will
  // be maintained by this FormattedAlert object.  Two-way replies are
  // always suitable for this alert.

  // Returns: A new, initialized FormattedAlert.
  
  init(addresses, replyto, subject, message, true);
}

List *FormattedAlert::addresses()
{
  // Obtain the List of addresses for this FormattedAlert.

  // Returns: A pointer to a List, or NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "FormattedAlert::addresses()");
  dlog->log_exit(DEBUG_MINTRC, "FormattedAlert::addresses = %d", as);
#endif

  return(as);
}

char *FormattedAlert::message()
{
  // Obtain the message for this FormattedAlert.

  // Returns: The message, or NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "FormattedAlert::message()");
  dlog->log_exit(DEBUG_MINTRC, "FormattedAlert::message = %s", IONULL(m));
#endif

  return(m);
}

bool FormattedAlert::replyok()
{
  // Determine if two-way replies are suitable for this FormattedAlert
  // data.

  // Returns: true if two-way replies are suitable, false otherwise.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "FormattedAlert::replyok()");
  dlog->log_exit(DEBUG_MINTRC, "FormattedAlert::replyok = %s", IOTF(rok));
#endif

  return(rok);
}

char *FormattedAlert::replyto()
{
  // Obtain the reply-to address for this FormattedAlert.

  // Returns: The reply-to address, or NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "FormattedAlert::replyto()");
  dlog->log_exit(DEBUG_MINTRC, "FormattedAlert::replyto = %s", IONULL(r));
#endif

  return(r);
}

char *FormattedAlert::subject()
{
  // Obtain the subject for this FormattedAlert.

  // Returns: The subject, or NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "FormattedAlert::subject()");
  dlog->log_exit(DEBUG_MINTRC, "FormattedAlert::subject = %s", IONULL(s));
#endif

  return(s);
}

FormattedAlert::~FormattedAlert()
{
  // Deallocate the FormattedAlert object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "FormattedAlert::~FormattedAlert()");
#endif

  xdelete(as);
  rok = false;
  xdelete(m);
  xdelete(r);
  xdelete(s);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "FormattedAlert::~FormattedAlert()");
#endif
}

void FormattedAlert::init(List *addresses, char *replyto, char *subject,
			  char *message, bool replyok)
{
  // Initialize the FormattedAlert object, using <addresses>, <replyto>,
  // <subject>, <message>, and <replyok>.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "FormattedAlert::init(%d,%s,%s,%s,%d)",
		  addresses, IONULL(replyto), IONULL(subject),
		  IONULL(message), replyok);
#endif
  
  as = new List(addresses);
  rok = replyok;
  m = xstrdup(message);
  r = xstrdup(replyto);
  s = xstrdup(subject);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "FormattedAlert::init()");
#endif
}
