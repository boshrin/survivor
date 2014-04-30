/*
 * Cookie Managing object
 *
 * Version: $Revision: 0.3 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2004/10/04 21:28:27 $
 * MT-Level: Unsafe
 *
 * Copyright (c) 2004
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: Cookies.C,v $
 * Revision 0.3  2004/10/04 21:28:27  benno
 * Fix cookie string parsing bug
 *
 * Revision 0.2  2004/04/24 14:52:27  benno
 * Add ; before newline
 *
 * Revision 0.1  2004/03/02 03:19:42  benno
 * Initial revision
 *
 */

#include "cgi-parser.H"

Cookies::Cookies()
{
  // Create a new, empty, Cookies object.

  // Returns: A new Cookies object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Cookies::Cookies()");
#endif

  ckin = NULL;
  ckout = NULL;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Cookies::Cookies()");
#endif
}

bool Cookies::add(Cookie *cookie)
{
  // Add the <cookie> to the set of Cookies.  <cookie> will be managed
  // by the Cookies object, and be deleted when no longer required.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Cookies::add(%d)", cookie);
#endif

  if(cookie)
  {
    if(!ckout)
      ckout = new Array<Cookie>();

    if(ckout)
    {
      if(ckout->add(cookie))
	ret = true;
    }
    else
      wlog->warn("Cookies::add failed to allocate new Array");
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Cookies::add = %s", IOTF(ret));
#endif

  return(ret);
}

Cookie *Cookies::cookie(char *name)
{
  // Find the cookie <name> from the set of parsed Cookies.

  // Returns: A pointer to the cookie if found, NULL otherwise.

  Cookie *ret = NULL;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Cookies::cookie(%s)", name);
#endif

  if(ckin)
  {
    for(int i = 0;i < ckin->entries();i++)
    {
      Cookie *c = ckin->retrieve(i);

      if(c && c->name() && strcmp(c->name(), name)==0)
      {
	ret = c;
	break;
      }
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Cookies::cookie = %d", ret);
#endif

  return(ret);
}

bool Cookies::dump(int fd)
{
  // Output the stored (not parsed) Cookies to <fd>.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Cookies::dump(%d)", fd);
#endif

  if(ckout)
  {
    FileHandler *fout = new FileHandler(fd);

    if(fout)
    {    
      for(int i = 0;i < ckout->entries();i++)
      {
	Cookie *c = ckout->retrieve(i);
	
	if(c && c->name() && c->value() && c->path())
	{
	  fout->append("Set-Cookie: ");
	  fout->append(c->name());
	  fout->append('=');
	  fout->append(c->value());
	  fout->append("; path=");
	  fout->append(c->path());
	  fout->append(";\n");
	}
      }

      xdelete(fout);

      ret = true;
    }
    else
      wlog->warn("Cookies::dump failed to allocate FileHandler");
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Cookies::dump = %s", IOTF(ret));
#endif

  return(ret);
}

bool Cookies::parse()
{
  // Parse the cookies passed via the web server and store them
  // so they are accessible via cookie().  This method may only be
  // called once per Cookies object.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Cookies::parse()");
#endif

  if(!ckin)
  {
    if(getenv("HTTP_COOKIE"))
    {
      ckin = new Array<Cookie>();

      if(ckin)
      {
	char *ck = xstrdup(getenv("HTTP_COOKIE"));

	if(ck)
	{
	  // Cookies are of the form name=value[; name=value[...]]

	  char *lasts;

	  for(char *c = strtok_r(ck, ";", &lasts);
	      c != NULL;
	      c = strtok_r(NULL, ";", &lasts))
	  {
	    // Move past any initial spaces in c

	    for(int i = 0;i < strlen(c);i++)
	    {
	      if(isspace(*c))
		c++;
	      else
		break;
	    }
	      
	    char *v = strchr(c, '=');

	    if(v)
	    {
	      *v = '\0';
	      v++;

	      Cookie *cookie = new Cookie(c, v);

	      if(cookie)
	      {
		if(!ckin->add(cookie))
		  xdelete(cookie);
	      }
	      else
		wlog->warn("Cookies::parse failed to allocate Cookie");
	    }
	  }
	  
	  xdelete(ck);

	  ret = true;
	}
	else
	  wlog->warn("Cookies::parse failed to copy string");
      }
      else
	wlog->warn("Cookies::parse failed to allocate new Array");
    }
    else
      ret = true;  // Nothing to parse
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Cookies::parse = %d", IOTF(ret));
#endif

  return(ret);
}

Cookies::~Cookies()
{
  // Delete the Cookie Manager, also deleting any Cookie stored within.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Cookies::~Cookies()");
#endif

  xadelete(ckin, Cookie);
  xadelete(ckout, Cookie);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Cookies::~Cookies()");
#endif
}
