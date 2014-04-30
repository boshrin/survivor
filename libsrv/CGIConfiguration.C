/*
 * survivor web interface CGI configuration object
 *
 * Version: $Revision: 0.2 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2005/04/06 14:04:08 $
 * MT-Level: Safe, if parse_cf is only called once
 *
 * Copyright (c) 2002 - 2004
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: CGIConfiguration.C,v $
 * Revision 0.2  2005/04/06 14:04:08  benno
 * Fix typo
 *
 * Revision 0.1  2004/03/01 23:14:19  benno
 * Initial revision
 *
 */

#include "cgi.H"

CGIConfiguration::CGIConfiguration()
{
  // Allocate and initialize a new CGIConfiguration object.

  // Returns: A new CGIConfiguration object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CGIConfiguration::CGIConfiguration()");
#endif

  authzs = NULL;
  ams = NULL;
  ce = NULL;
  cp = NULL;
  ps = NULL;
  psdir = NULL;
  sdir = NULL;
  stime = SESSIONTIME;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CGIConfiguration::CGIConfiguration()");
#endif
}

bool CGIConfiguration::add_authmodule(CGIAuthModule *authmod)
{
  // Add the WebAuth module <authmod>.  The CGIAuthModule object
  // will be managed by the CGIConfiguration object and deleted when
  // no longer required.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CGIConfiguration::add_authmodule(%d)",
		  authmod);
#endif

  if(authmod)
  {
    if(!ams)
      ams = new Array<CGIAuthModule>();

    if(ams)
    {
      if(ams->add(authmod))
	ret = true;
    }
    else
      wlog->warn("CGIConfiguration::add_authmodule failed to allocate Array");
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CGIConfiguration::add_authmodule = %s",
		 IOTF(ret));
#endif

  return(ret);
}

bool CGIConfiguration::add_authorization(CGIAuthorization *authz)
{
  // Add a new authorization <authz> to the CGIConfiguration.
  // The CGIAuthorization will be maintained by the CGIConfiguration
  // and will be deleted when no longer required.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CGIConfiguration::add_authorization(%d)",
		  authz);
#endif

  if(authz)
  {
    if(!authzs)
      authzs = new Array<CGIAuthorization>();

    if(authzs)
    {
      if(authzs->add(authz))
	ret = true;
    }
    else
      wlog->warn("CGIConfiguration::add_authorization failed to allocate Array");
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CGIConfiguration::add_authorization = %s",
		 IOTF(ret));
#endif

  return(ret);
}

authz_level_t CGIConfiguration::authorized(char *user, List *groups)
{
  // Determine what level of access the specified <user> (a member of
  // the optional <groups>) has.  Access is determined by the first
  // matching CGIAuthorization entry, using the name in the entry
  // as a regular expression to be compared against <user> and the
  // members of <groups>.

  // Returns: The first authorization level found, or read_only_authz if
  // no match is found.

  authz_level_t ret = read_only_authz;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CGIConfiguration::authorized(%s,%d)",
		  IONULL(user), groups);
#endif

  if(authzs && (user || groups))
  {
    bool matched = false;
    
    for(int i = 0;i < authzs->entries();i++)
    {
      CGIAuthorization *authz = authzs->retrieve(i);

      if(authz)
      {
	if(authz->authz_type() == user_authz_id)
	{
	  // Simple, just compare against user

	  if(user && compare_regex(user, authz->name()))
	  {
	    ret = authz->authz_level();
	    matched = true;
	  }
	}
	else if(authz->authz_type() == group_authz_id)
	{
	  // Cycle through all the entries in the list

	  if(groups)
	  {
	    for(int j = 0;j < groups->entries();j++)
	    {
	      if(compare_regex(groups->retrieve(j), authz->name()))
	      {
		ret = authz->authz_level();
		matched = true;
		break;
	      }
	    }
	  }
	}
	else
	  wlog->warn("CGIConfiguration::authorized found unknown authz_type");
      }

      if(matched)
	break;
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CGIConfiguration::authorized = %d", ret);
#endif

  return(ret);
}

Array<CGIAuthModule> *CGIConfiguration::authmodules()
{
  // Obtain the WebAuth modules in use..

  // Returns: A pointer to an Array of CGIAuthModules, or NULL on error.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CGIConfiguration::authmodule()");
  dlog->log_exit(DEBUG_MINTRC, "CGIConfiguration::authmodule = %d", ams);
#endif

  return(ams);
}

char *CGIConfiguration::clip_email()
{
  // Obtain the default clipboard email address.

  // Returns: The address if defined, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CGIConfiguration::clip_email()");
  dlog->log_exit(DEBUG_MINTRC, "CGIConfiguration::clip_email = %s",
		 IONULL(ce));
#endif

  return(ce);
}

char *CGIConfiguration::clip_phone()
{
  // Obtain the default clipboard phone number.

  // Returns: The phone number if defined, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CGIConfiguration::clip_phone()");
  dlog->log_exit(DEBUG_MINTRC, "CGIConfiguration::clip_phone = %s",
		 IONULL(cp));
#endif

  return(cp);
}

char *CGIConfiguration::pageset()
{
  // Obtain the default pageset.

  // Returns: A pointer to a string that remains valid for the duration
  // of the CGIConfiguration object, or NULL on error.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CGIConfiguration::pageset()");
#endif

  if(!ps)
    ps = xstrdup("en_US");
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CGIConfiguration::pageset = %s", IONULL(ps));
#endif

  return(ps);
}

char *CGIConfiguration::pageset_directory()
{
  // Obtain the pageset directory, either as set in cgi.cf or (if
  // not set) the default.

  // Returns: A pointer to a string that remains valid for the duration
  // of the CGIConfiguration object, or NULL on error.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CGIConfiguration::pageset_directory()");
#endif

  if(!psdir && args->pkgdir())
  {
    // Assemble the default directory.

    psdir = xstrdup(args->pkgdir());

    if(psdir)
      psdir = xstrcat(psdir, "/html/sw");
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CGIConfiguration::pageset_directory = %s",
		 IONULL(psdir));
#endif

  return(psdir);
}

bool CGIConfiguration::parse_cf()
{
  // Parse the cgi configuration file.  The location of this file is
  // DEFAULT_CFDIR/cgi.cf and cannot be overridden at run time.  This
  // method should only be called once for the duration of this object.
  // Upon a failed parse, a descriptive error string will be available
  // via the parse_error method.

  // Returns: True if the configuration file was successfully parsed,
  // false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "CGIConfiguration::parse_cf()");
#endif

  CharBuffer *cf = new CharBuffer(DEFAULT_CFDIR);

  if(cf)
  {
    cf->append("/cgi.cf");

    FILE *in = fopen(cf->str(), "r");

    if(in)
    {
      lexswstart(in, this);
      yylex();

      if(lexerr || lexincomplete())
      {
	lexerr += lexincomplete();
	
#if defined(DEBUG)
	if(lexincomplete())
	  dlog->log_progress(DEBUG_CONFIG, "Unexpected EOF (unclosed brace)");

	dlog->log_progress(DEBUG_CONFIG, "%d %s encountered while parsing %s",
			    lexerr, ((lexerr == 1) ? " error" : " errors"),
			    cf->str());
#endif
      }
      else
      {
#if defined(DEBUG)
	dlog->log_progress(DEBUG_CONFIG, "Parse of %s successful", cf->str());
#endif
	ret = true;
      }
      
      fclose(in);
    }
    
    xdelete(cf);
  }
  else
    wlog->warn("CGIConfiguration::parse_cf failed to allocate CharBuffer");

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "CGIConfiguration::parse_cf = %s", IOTF(ret));
#endif
  
  return(ret);
}

int CGIConfiguration::session_length()
{
  // Obtain the session timeout.

  // Returns: The timeout, in minutes.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CGIConfiguration::session_length()");
  dlog->log_exit(DEBUG_MINTRC, "CGIConfiguration::session_length = %d",
		 stime);
#endif

  return(stime);
}

bool CGIConfiguration::set_clip_email(char *email)
{
  // Set the default clipboard email address to <email>.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CGIConfiguration::set_clip_email(%s)",
		  IONULL(email));
#endif

  if(email)
  {
    xdelete(ce);

    ce = xstrdup(email);
    ret = true;
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CGIConfiguration::set_clip_email = %s",
		 IOTF(ret));
#endif

  return(ret);
}

bool CGIConfiguration::set_clip_phone(char *phone)
{
  // Set the default clipboard phone number to <phone>.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CGIConfiguration::set_clip_phone(%s)",
		  IONULL(phone));
#endif

  if(phone)
  {
    xdelete(cp);

    cp = xstrdup(phone);
    ret = true;
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CGIConfiguration::set_clip_phone = %s",
		 IOTF(ret));
#endif

  return(ret);
}

bool CGIConfiguration::set_pageset(char *pageset)
{
  // Set the default pageset to <pageset>.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CGIConfiguration::set_pageset(%s)",
		  IONULL(pageset));
#endif

  if(pageset)
  {
    xdelete(ps);

    ps = xstrdup(pageset);
    ret = true;
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CGIConfiguration::set_pageset = %s",
		 IOTF(ret));
#endif

  return(ret);
}

bool CGIConfiguration::set_pageset_directory(char *srcdir)
{
  // Set the pageset source directory to <srcdir>.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CGIConfiguration::set_pageset_directory(%s)",
		  IONULL(srcdir));
#endif

  if(srcdir)
  {
    xdelete(psdir);

    psdir = xstrdup(srcdir);
    ret = true;
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CGIConfiguration::set_pageset_directory = %s",
		 IOTF(ret));
#endif

  return(ret);
}

bool CGIConfiguration::set_session_length(int len)
{
  // Set the session timeout to <len> minutes.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CGIConfiguration::set_session_length(%d)",
		  len);
#endif

  if(len > -1)
  {
    stime = len;
    ret = true;
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CGIConfiguration::set_session_length = %s",
		 IOTF(ret));
#endif
  
  return(ret);
}

bool CGIConfiguration::set_state_directory(char *statedir)
{
  // Set the state directory to <statedir>.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CGIConfiguration::set_state_directory(%s)",
		  IONULL(statedir));
#endif

  if(statedir)
  {
    xdelete(sdir);

    sdir = xstrdup(statedir);
    ret = true;
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CGIConfiguration::set_state_directory = %s",
		 IOTF(ret));
#endif

  return(ret);
}

char *CGIConfiguration::statedir()
{
  // Obtain the state directory.

  // Returns: The directory if defined, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CGIConfiguration::statedir()");
  dlog->log_exit(DEBUG_MINTRC, "CGIConfiguration::statedir = %s",
		 IONULL(sdir));
#endif

  return(sdir);
}

CGIConfiguration::~CGIConfiguration()
{
  // Deallocate the CGIConfiguration object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CGIConfiguration::~CGIConfiguration()");
#endif

  xadelete(authzs, CGIAuthorization);
  xadelete(ams, CGIAuthModule);
  xdelete(ce);
  xdelete(cp);
  xdelete(ps);
  xdelete(psdir);
  xdelete(sdir);
  stime = SESSIONTIME;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CGIConfiguration::~CGIConfiguration()");
#endif
}
