/*
 * survivor web interface clipboard object
 *
 * Version: $Revision: 0.11 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2004/10/10 02:22:10 $
 * MT-Level: Safe, except simultaneous I/O to the storage file is not protected
 *
 * Copyright (c) 2002 - 2003
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: Clipboard.C,v $
 * Revision 0.11  2004/10/10 02:22:10  benno
 * Toss vestigal "Subject:" when sending clipboard
 *
 * Revision 0.10  2004/03/02 17:31:25  benno
 * Don't use INSTUSER
 *
 * Revision 0.9  2003/07/30 21:38:13  benno
 * Use FormattedAlert and send via transmit module
 *
 * Revision 0.8  2003/04/09 20:14:27  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.7  2003/01/26 17:58:41  benno
 * Use IONULL, IOTF, xdelete, xadelete
 *
 * Revision 0.6  2002/08/21 21:34:03  benno
 * use name in outgoing subject
 *
 * Revision 0.5  2002/08/06 16:17:12  selsky
 * Remove embedded nulls in format
 *
 * Revision 0.4  2002/05/22 15:57:41  benno
 * fix cmd bufsize
 *
 * Revision 0.3  2002/04/04 19:53:40  benno
 * copyright
 *
 * Revision 0.2  2002/04/02 19:49:09  benno
 * rcsify date
 *
 * Revision 0.1  2002/04/02 19:48:28  benno
 * initial revision
 *
 */

#include "cgi.H"

Clipboard::Clipboard(char *path, char *name)
{
  // Allocate and initialize a new Clipboard named <name>, whose data is
  // stored at <path>.

  // Returns: A new Clipboard object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Clipboard::Clipboard(%s)",
		  IONULL(path), IONULL(name));
#endif
  
  clippath = xstrdup(path);
  clipname = xstrdup(name);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Clipboard::Clipboard()");
#endif
}

bool Clipboard::append(char *contents)
{
  // Append <contents> to the existing contents of the Clipboard.

  // Returns: true if fully successful, false otherwise.

  return(output(contents, "a"));
}

char *Clipboard::read()
{
  // Obtain the contents of the Clipboard.

  // Returns: A newly allocated string that should be deleted when no
  // longer required, or NULL.

  char *r = NULL;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "Clipboard::read()");
#endif

  if(clippath)
  {
    struct stat sb;
    
    if(stat(clippath, &sb)==0)
    {
      r = new char[sb.st_size + 1];

      if(r)
      {
	memset(r, 0, sb.st_size + 1);

	FILE *in = fopen(clippath, "r");

	if(in)
	{
	  // We could check to make sure we read the right amount...
	  fread(r, 1, sb.st_size, in);
	  
	  fclose(in);
	}
	else
	  wlog->warn("Clipboard::read unable to open %s", clippath);
      }
      else
	wlog->warn("Clipboard::read failed to allocate char string");
    }
    else
      wlog->warn("Clipboard::read failed to stat %s", clippath);
  }
  
#if defined(DEBUG)
  // Output the length of what was read rather than what actually was read
  // since it could be quite lengthy.
  
  dlog->log_exit(DEBUG_MAJTRC, "Clipboard::read = %d", (r ? strlen(r) : 0));
#endif

  return(r);
}

bool Clipboard::send(char *address, char *from, char *mail, char *phone,
		     char *via)
{
  // Send the contents of the Clipboard to <address> via the transmit
  // module <via>.  <from> contains the login tag of who sent the
  // Clipboard.  <mail> and <phone> are optional identifiers.

  // Returns: true if fully successful, false otherwise.

  bool r = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "Clipboard::send(%s,%s,%s,%s,%s)",
		  IONULL(address), IONULL(from), IONULL(mail),
		  IONULL(phone), IONULL(via));
#endif

  // To send a Clipboard, we need to create a FormattedAlert object,
  // and pass it and a transmit module name to an Executor object.

  char *xcontent = read();

  if(address && from && xcontent)
  {
    List *addrlist = new List();

    if(addrlist)
    {
      addrlist->add(address);

      // Don't use mail or phone if they're empty 

      char *xmail = mail;
      char *xphone = phone;
  
      if(xmail && strlen(xmail)==0)
	xmail = NULL;
      if(xphone && strlen(xphone)==0)
	xphone = NULL;

      // Build a subject line

      char *xsubject = new char[(clipname ? strlen(clipname) : 7) + 40];
      char *xmessage = new char[strlen(xcontent) + strlen(from) + 40
			       + (xmail ? strlen(xmail) : 0)
			       + (xphone ? strlen(xphone) : 0)];

      if(xsubject && xmessage)
      {
	sprintf(xsubject, "Contents of %s Clipboard",
		(clipname ? clipname : "Unnamed"));

	sprintf(xmessage, "Clipboard sent from login %s", from);

	if(xmail)
	  sprintf(xmessage, "%s (%s)", xmessage, xmail);

	if(xphone)
	  sprintf(xmessage, "%s (%s)", xmessage, xphone);

	sprintf(xmessage, "%s:\n\n%s\n", xmessage, xcontent);

	FormattedAlert *fa = new FormattedAlert(addrlist,
						(char *)
						(xmail ? xmail :
						 (args ? args->instuser()
						  : "")),
						xsubject,
						xmessage);

	if(fa)
	{
	  Executor *e = new Executor();

	  if(e)
	  {
	    pid_t pid = e->exec_formatted_alert(fa, via);

	    if(pid > -1)
	    {
	      int rc = e->result();
	      
	      if(rc == MODEXEC_OK)
		r = true;
	      else
		wlog->warn("Clipboard::send received %d from %s module",
			   rc, via);
	    }
	    else
	      wlog->warn("Clipboard::send failed to exec transmit module");

	    xdelete(e);
	  }
	  else
	    wlog->warn("Clipboard::send failed to allocate Executor");
	  
	  xdelete(fa);
	}
	else
	  wlog->warn("Clipboard::send failed to allocate FormattedAlert");
      }
      else
	wlog->warn("Clipboard::send failed to allocate strings");
      
      xdelete(xsubject);
      xdelete(xmessage);
      
      xdelete(addrlist);
    }
    else
      wlog->warn("Clipboard::send failed to allocate List");
  }
  
  xdelete(xcontent);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "Clipboard::send = %s", IOTF(r));
#endif
  
  return(r);
}

bool Clipboard::write(char *contents)
{
  // Output <contents> to be the new content of the Clipboard.

  // Returns: true if fully successful, false otherwise.

  return(output(contents, "w"));
}

Clipboard::~Clipboard()
{
  // Deallocate the Clipboard.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Clipboard::~Clipboard()");
#endif

  xdelete(clipname);
  xdelete(clippath);
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Clipboard::~Clipboard()");
#endif
}

bool Clipboard::output(char *contents, char *mode)
{
  // Output <contents> to the Clipboard, where <mode> is "a" for append
  // or "w" for truncate.

  // Returns: true if fully successful, false otherwise.

  bool r = false;

#if defined(DEBUG)
  // Output the length of the contents since it could be quite lengthy

  dlog->log_entry(DEBUG_MAJTRC, "Clipboard::output(%d,%s)",
		  (contents ? strlen(contents) : 0), IONULL(mode));
#endif

  if(clippath && contents && mode)
  {
    FILE *out = fopen(clippath, mode);

    if(out)
    {
      // We could check to make sure the contents were actually written
      fwrite(contents, 1, strlen(contents), out);
      fclose(out);

      // The permissions on the file should have been properly set
      // when the ClipboardManager created it.
      r = true;
    }
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "Clipboard::output = %s", IOTF(r));
#endif
  
  return(r);
}
