/*
 * CheckResult.C: Object to manage check result information
 *
 * Version: $Revision: 0.13 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2006/01/23 02:08:40 $
 * MT-Level: Unsafe, unless set_X methods are not called
 *
 * Copyright (c) 2002 - 2006
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: CheckResult.C,v $
 * Revision 0.13  2006/01/23 02:08:40  benno
 * Add duration
 *
 * Revision 0.12  2005/04/27 01:57:31  benno
 * Replace read() with parse()
 *
 * Revision 0.11  2005/04/06 14:06:52  benno
 * Read XML documents
 *
 * Revision 0.10  2003/10/06 23:07:55  benno
 * Use read_line and unbuffered I/O
 *
 * Revision 0.9  2003/04/09 20:23:46  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.8  2003/04/04 21:44:16  benno
 * Use Debugger
 *
 * Revision 0.7  2003/02/23 15:27:09  benno
 * Add constructor with hostname
 *
 * Revision 0.6  2003/01/23 22:50:24  benno
 * Add IONULL and IOTF
 *
 * Revision 0.5  2002/12/31 04:29:31  benno
 * Add hostname support
 *
 * Revision 0.4  2002/12/16 00:42:59  benno
 * Add read(FILE *)
 *
 * Revision 0.3  2002/04/04 20:09:14  benno
 * copyright
 *
 * Revision 0.2  2002/04/03 17:53:12  benno
 * rcsify date
 *
 * Revision 0.1  2002/04/03 17:52:35  benno
 * initial revision
 *
 */

#include "survivor.H"

CheckResult::CheckResult()
{
  // Allocate a new CheckResult object.

  // Returns: A new CheckResult object.
  
  init(NULL, 0, 0, NULL, -1);
}

CheckResult::CheckResult(int rc, int scalar, char *comment)
{
  // Allocate a new CheckResult object, containing <rc>, <scalar>,
  // and <comment> (which is duplicated and need not remain valid).

  // Returns: A new CheckResult object.
  
  init(NULL, rc, scalar, comment, -1);
}

CheckResult::CheckResult(char *hostname, int rc, int scalar, char *comment)
{
  // Allocate a new CheckResult object, containing <hostname>, <rc>,
  // <scalar>, and <comment> (<hostname> and <comment> are duplicated
  // and need not remain valid).

  // Returns: A new CheckResult object.
  
  init(hostname, rc, scalar, comment, -1);
}

CheckResult::CheckResult(char *hostname, int rc, int scalar, char *comment,
			 int duration)
{
  // Allocate a new CheckResult object, containing <hostname>, <rc>,
  // <scalar>, <comment>, and <duration> (<hostname> and <comment> are
  // duplicated and need not remain valid).

  // Returns: A new CheckResult object.
  
  init(hostname, rc, scalar, comment, duration);
}

char *CheckResult::comment()
{
  // Determine the comment of the CheckResult.

  // Returns: A pointer that should not be manipulated, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CheckResult::comment()");
  dlog->log_exit(DEBUG_MINTRC, "CheckResult::comment = %s", IONULL(c));
#endif
  
  return(c);
}

int CheckResult::duration()
{
  // Determine the duration for the CheckResult.

  // Returns: The duration, in milliseconds, or -1 if no duration is available.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CheckResult::duration()");
  dlog->log_exit(DEBUG_MINTRC, "CheckResult::duration = %d", d);
#endif
  
  return(d);
}

char *CheckResult::hostname()
{
  // Determine the hostname of the CheckResult.

  // Returns: A pointer that should not be maniuplated, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CheckResult::hostname()");
  dlog->log_exit(DEBUG_MINTRC, "CheckResult::hostname = %s", IONULL(h));
#endif
  
  return(h);
}

bool CheckResult::parse(char *xmldoc)
{
  // Parse check result information from the SurvivorCheckResult
  // document <xmldoc>.

  // Returns: true if fully successful, false otherwise.  On error,
  // an appropriate message will be stored in the comment field and
  // the return code will be set to MODEXEC_PROBLEM.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CheckResult::parse(%d)",
		  xmldoc ? strlen(xmldoc) : 0);
#endif

  if(xmldoc)
  {
    // The XML parser will do all the work, but then return a
    // CheckResult.  We'll copy that and then delete it.  This is a
    // bit strange, but that's what you get with code that has evolved
    // over a few years.

    SurvivorXML *sxml = new SurvivorXML();

    if(sxml)
    {
      CheckResult *cr = sxml->parse_checkresult(xmldoc, strlen(xmldoc));

      if(cr)
      {
	set_comment(cr->comment());
	set_rc(cr->rc());
	set_scalar(cr->scalar());
	h = xstrdup(cr->hostname());
	d = cr->duration();
	
	xdelete(cr);

	ret = true;
      }
      else
      {
	set_comment("Check result did not parse");
	set_rc(MODEXEC_PROBLEM);
      }

      xdelete(sxml);
    }
    else
    {
      set_comment("Failed to allocate SurvivorXML");
      set_rc(MODEXEC_PROBLEM);
    }
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CheckResult::parse = %s", IOTF(ret));
#endif
  
  return(ret);
}

int CheckResult::rc()
{
  // Determine the return code of the CheckResult.

  // Returns: The return code.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CheckResult::rc()");
  dlog->log_exit(DEBUG_MINTRC, "CheckResult::rc = %d", r);
#endif
  
  return(r);
}

void CheckResult::reset()
{
  // Clear the contents of the CheckResult.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CheckResult::reset()");
#endif

  xdelete(c);
  xdelete(h);
  d = -1;
  r = 0;
  s = 0;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CheckResult::reset()");
#endif
}

int CheckResult::scalar()
{
  // Determine the scalar value of the CheckResult.

  // Returns: The scalar value.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CheckResult::scalar()");
  dlog->log_exit(DEBUG_MINTRC, "CheckResult::scalar = %d", s);
#endif
  
  return(s);
}

bool CheckResult::set_comment(char *comment)
{
  // Set the comment of this CheckResult to <comment>.  If a comment
  // is already set, it will be replaced.

  // Returns: true if fully successful, false otherwise.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CheckResult::set_comment(%s)",
		   IONULL(comment));
#endif

  xdelete(c);
  c = xstrdup(comment);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CheckResult::set_comment = true");
#endif
  
  return(true);
}

bool CheckResult::set_duration(int duration)
{
  // Set the duration of this CheckResult to <duration>.

  // Returns: true if fully successful, false otherwise.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CheckResult::set_duration(%d)", duration);
#endif

  d = duration;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CheckResult::set_duration = true");
#endif
  
  return(true);
}

bool CheckResult::set_rc(int rc)
{
  // Set the return code of this CheckResult to <rc>.

  // Returns: true if fully successful, false otherwise.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CheckResult::set_rc(%d)", rc);
#endif
  
  r = rc;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CheckResult::set_rc = true");
#endif
  
  return(true);
}

bool CheckResult::set_scalar(int scalar)
{
  // Set the scalar value of this CheckResult to <scalar>.

  // Returns: true if fully successful, false otherwise.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CheckResult::set_scalar(%d)", scalar);
#endif
  
  s = scalar;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CheckResult::set_scalar = true");
#endif
  
  return(true);
}

CheckResult::~CheckResult()
{
  // Deallocate the CheckResult object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CheckResult::~CheckResult()");
#endif

  reset();
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CheckResult::~CheckResult()");
#endif
}

void CheckResult::init(char *hostname, int rc, int scalar, char *comment,
		       int duration)
{
  // Initializer for Constructors.

  // Returns: Nothing.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CheckResult::init(%s,%d,%d,%s,%d)",
		   IONULL(hostname), rc, scalar, IONULL(comment), duration);
#endif

  c = xstrdup(comment);
  h = xstrdup(hostname);
  d = duration;
  r = rc;
  s = scalar;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CheckResult::init()");
#endif
}

bool CheckResult::set_hostname(char *host)
{
  // Set the hostname of this CheckResult to <host>.  If a host
  // is already set, it will be replaced.

  // Returns: true if fully successful, false otherwise.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CheckResult::set_hostname(%s)",
		   IONULL(host));
#endif

  xdelete(h);
  h = xstrdup(host);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CheckResult::set_hostname = true");
#endif
  
  return(true);
}
