/*
 * utils.C: Utilities.
 *
 * Version: $Revision: 0.34 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2007/01/25 13:10:55 $
 * MT-Level: Safe
 *
 * Copyright (c) 2002 - 2007
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: utils.C,v $
 * Revision 0.34  2007/01/25 13:10:55  benno
 * Add _HAVE_LIBINTL_H
 *
 * Revision 0.33  2005/12/22 04:06:00  benno
 * Add next_schedule_time
 *
 * Revision 0.32  2005/11/30 01:38:44  benno
 * Add module serialization
 * Fix bug in mutex_init sequence
 *
 * Revision 0.31  2005/08/10 01:43:32  benno
 * read_xml_document takes timeout
 *
 * Revision 0.30  2005/04/27 01:56:18  benno
 * Add read_xml_document
 *
 * Revision 0.29  2005/04/16 21:15:08  benno
 * Fix compose_rc for sw composition
 *
 * Revision 0.28  2005/04/07 01:28:00  benno
 * Add escape_html, escape_xml, xstrnchr
 * Use INTBUFSIZE
 *
 * Revision 0.27  2004/11/26 22:14:26  benno
 * Add gateway.cf to config files
 * Add get_localhost_fqdn
 * Add tokenize(s,c,max)
 *
 * Revision 0.26  2004/08/25 00:48:28  benno
 * Add read_previous_line
 *
 * Revision 0.25  2004/06/12 01:00:10  benno
 * Add try_open
 *
 * Revision 0.24  2004/03/01 23:33:27  benno
 * Add cgi.cf to _config_files
 * Move compose_rc here
 * Add tokenize()
 *
 * Revision 0.23  2003/11/29 05:29:52  benno
 * Move try_lock.C here from State.C
 *
 * Revision 0.22  2003/10/06 23:17:37  benno
 * Add read_line
 *
 * Revision 0.21  2003/06/27 01:17:54  benno
 * Fix incorrect truncation on concatenation in xstrncat
 *
 * Revision 0.20  2003/05/29 00:32:11  benno
 * Add xstrncat
 *
 * Revision 0.19  2003/05/05 01:30:11  benno
 * Fix wrong type bugs
 *
 * Revision 0.18  2003/05/04 21:33:00  benno
 * Don't use string type
 * Add xstrcat, xstrchop, xstrreplace
 *
 * Revision 0.17  2003/04/13 20:02:58  benno
 * Add generate_csid
 *
 * Revision 0.16  2003/04/09 20:22:00  benno
 * dlog and wlog Debuggers
 * Define global warn Debugger (oops)
 *
 * Revision 0.15  2003/04/07 20:59:21  benno
 * Use Debugger instead of hacky debugging
 * Toss xtimestamp
 *
 * Revision 0.14  2003/03/04 20:51:42  benno
 * Add _config_files
 *
 * Revision 0.13  2003/01/24 18:43:02  benno
 * Reentrant gethostbyX
 * Use IONULL, xdelete
 *
 * Revision 0.12  2002/12/31 04:35:41  benno
 * Add expire_file, toss_eol
 *
 * Revision 0.11  2002/10/21 20:45:06  benno
 * add xnumdup
 *
 * Revision 0.10  2002/08/21 21:36:32  benno
 * add try_fopen
 *
 * Revision 0.9  2002/08/06 19:42:29  selsky
 * Remove embedded nulls in format
 *
 * Revision 0.8  2002/05/10 20:07:57  benno
 * finish implementing process_kill for solaris
 *
 * Revision 0.7  2002/05/02 22:58:06  toor
 * add process_kill and process_kill_target
 *
 * Revision 0.6  2002/04/19 20:10:36  benno
 * add compare_regex
 *
 * Revision 0.5  2002/04/04 20:12:00  benno
 * copyright
 *
 * Revision 0.4  2002/04/03 18:55:23  benno
 * rcsify date
 *
 * Revision 0.3  2002/04/03 18:55:13  benno
 * Move global requisite defines here
 * Move allocate_dirent here
 *
 * Revision 0.2  2002/04/03 18:54:35  benno
 * Define debuglevel here
 * xtimestamp
 *
 * Revision 0.1  2002/04/03 18:53:58  benno
 * initial revision
 *
 */

#include "survivor.H"

#if defined(DEBUG)
// Define dlog for use throughout.  We can't set it, though, since
// there are different types of debuggers.

Debugger *dlog = NULL;
#endif

// wlog is always enabled, regardless of DEBUG
Debugger *wlog = NULL;

// Global objects
Args *args = NULL;                  // Global argument object
Configuration *cf = NULL;           // Global configuration object
PRNG *prng = NULL;                  // Global pseudo random number generator
char *progname = NULL;              // argv[0]
#if defined(_MISSING_REENTRANT_GETHOST)
pthread_mutex_t nremutex;           // Non-reentrant function mutex
#endif

#if defined(ENABLE_MODULE_SERIALIZATION)
pthread_mutex_t msmutex;            // Module execution serialization mutex
#endif

// Have to define this somewhere... localization would make this
// less hacky

char *_config_files[9] = {
  "unknown",
  "instance.cf",
  "calllist.cf",
  "schedule.cf",
  "host.cf",
  "check.cf",
  "dependency.cf",
  "cgi.cf",
  "gateway.cf"
};

struct dirent *allocate_dirent(char *path)
{
  // Allocate a struct dirent suitable for traversing the directory <path>.
  // This is required to allocate buffers suitable for readdir_r.

  // Returns: A newly malloc'd dirent that should be free'd when no longer
  // required, or NULL on error.

  struct dirent *d = NULL;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "allocate_dirent(%s)", IONULL(path));
#endif

  if(path)
  {
    long l = pathconf(path, _PC_NAME_MAX);

    if(l > -1)
      d = (struct dirent *)malloc(sizeof(struct dirent) + l);
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "allocate_dirent = %d", d);
#endif

  return(d);
}

bool compare_regex(char *s, char *exp)
{
  // Determine if the string <s> matches the (extended) regular
  // expression <exp>.

  // Returns: true if <s> matches <exp>, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "compare_regex(%s,%s)",
		   IONULL(s), IONULL(exp));
#endif

  if(s && exp)
  {
    regex_t re;

    if(regcomp(&re, exp, REG_EXTENDED|REG_NOSUB)==0)
    {
      if(regexec(&re, s, 0, NULL, 0)==0)
	ret = true;
      
      regfree(&re);
    }
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "compare_regex = %s", IOTF(ret));
#endif
  
  return(ret);
}

int compose_rc(int current, int additional, bool required)
{
  // Compose <additional> into <current> according to whether this is
  // for a <required> check or an optional check.  For generating a
  // composite value for a service (as the web interface does), set
  // <required> to true.

  // Returns: The composed return code.

  int ret = current;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "compose_rc(%d,%d,%s)",
		  current, additional, IOTF(required));
#endif

  // Required checks and optional checks have inverse logic.  This is
  // mainly for simplicity in explanation, one could argue a different
  // logic for optional checks.  There's no point doing anything if
  // current and additional are the same, or if additional is NOTYET
  // (which indicates something hasn't run).

  if(current != additional && additional != MODEXEC_NOTYET)
  {
    if(required)
    {
      // We do some extra comparisons here for general composition.
      
      switch(current)
      {
      case MODEXEC_MISCONFIG:
	// This trumps all, so don't do anything
	break;
      case MODEXEC_PROBLEM:
      case MODEXEC_TIMEDOUT:
	if(additional == MODEXEC_MISCONFIG)
	  ret = additional;
	break;
      case MODEXEC_WARNING:
      case MODEXEC_NOTICE:
      case MODEXEC_DEPEND:
	if(additional == MODEXEC_MISCONFIG ||
	   additional == MODEXEC_PROBLEM || additional == MODEXEC_TIMEDOUT)
	  ret = additional;
	break;
      case MODEXEC_OK:
	// Favor additional regardless
	ret = additional;
	break;
      default:
	// Go in simple numeric order
	if(additional > current)
	  ret = additional;
	break;
      }
    }
    else
    {
      switch(current)
      {
      case MODEXEC_OK:
	// This trumps all, so don't do anything
	break;
      case MODEXEC_WARNING:
	if(additional == MODEXEC_OK)
	  ret = additional;
	break;
      case MODEXEC_PROBLEM:
	if(additional == MODEXEC_OK || additional == MODEXEC_WARNING)
	  ret = additional;
	break;
      case MODEXEC_MISCONFIG:
	// Favor additional regardless
	ret = additional;
	break;
      default:
	// Go in simple numeric order
	if(additional < current)
	  ret = additional;
	break;
      }
    }
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "compose_rc = %d", ret);
#endif

  return(ret);
}

time_t convtime(char *time)
{
  // Convert a string of the form HHMMMMDDYYYY or MMDDYYYY into a time_t
  // value that represents the first second described by that time.

  // Returns: The converted time, or -1 in error.

  time_t r = -1;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "convtime(%s)", IONULL(time));
#endif

  if(time && (strlen(time)==8 || strlen(time)==12))
  {
    char *p = NULL;
    char buf[5];
    struct tm tm;

    // Set some static values

    tm.tm_sec = 0;
    tm.tm_isdst = -1;  // Let mktime figure it out
    memset(buf, 0, 5);
    
    // First get hour and minute

    if(strlen(time)==12)
    {
      strncpy(buf, time, 2);
      tm.tm_hour = atoi(buf);

      strncpy(buf, time+2, 2);
      tm.tm_min = atoi(buf);

      p = time + 4;
    }
    else
    {
      // Use midnight
      
      tm.tm_min = 0;
      tm.tm_hour = 0;

      p = time;
    }

    // Month

    strncpy(buf, p, 2);
    tm.tm_mon = atoi(buf) - 1;
    p += 2;

    // Day

    strncpy(buf, p, 2);
    tm.tm_mday = atoi(buf);
    p += 2;
    
    // Year

    strncpy(buf, p, 4);
    tm.tm_year = atoi(buf) - 1900;
    
    // Now convert

    r = mktime(&tm);
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "convtime = %d", r);
#endif

  return(r);
}

char *escape_html(char *s)
{
  // Escape <s> for XML, replacing '&', '#', '<', '>', '(', ')'.
  // <s> is not modified.

  // Returns: A newly allocated string that should be deleted when no
  // longer required, or NULL on error.

  char *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "escape_html(%s)", IONULL(s));
#endif

  int len = (s ? strlen(s) : 0);
  
  if(len > 0)
  {
    // The fastest implementation would be to allocate 5x memory and
    // then do a simple copy with switch.  The most memory efficient
    // would be to reallocate for each replacement.  A reasonable
    // compromise is to use a CharBuffer with a quick sanity check
    // to make sure we have work to do.

    if(xstrnchr(s, '&', len) || xstrnchr(s, '#', len)
       || xstrnchr(s, '<', len) || xstrnchr(s, '>', len)
       || xstrnchr(s, '(', len) || xstrnchr(s, ')', len))
    {
      CharBuffer *cb = new CharBuffer();

      if(cb)
      {
	for(int i = 0;i < len;i++)
	{
	  switch(s[i])
	  {
	  case '&':
	    cb->append("&amp;");
	    break;
	  case '#':
	    cb->append("&#35;");
	    break;
	  case '<':
	    cb->append("&lt;");
	    break;
	  case '>':
	    cb->append("&gt;");
	    break;
	  case '(':
	    cb->append("&#40;");
	    break;
	  case ')':
	    cb->append("&#41;");
	    break;
	  default:
	    cb->append(s[i]);
	    break;
	  }
	}

	ret = xstrdup(cb->str());
      }
      else
	wlog->warn("escape_html failed to allocate CharBuffer");
    }
    else
      ret = xstrncat(NULL, s, len);
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "escape_html = %s", IONULL(ret));
#endif

  return(ret);
}

char *escape_xml(char *s, int len)
{
  // Escape <len> characters in <s> for XML, replace '&', '<', and '>'.
  // <s> is not modified.

  // Returns: A newly allocated string that should be deleted when no
  // longer required, or NULL on error.

  char *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "escape_xml(%s,%d)", IONULL(s), len);
#endif

  if(s && len > 0)
  {
    // The fastest implementation would be to allocate 5x memory and
    // then do a simple copy with switch.  The most memory efficient
    // would be to reallocate for each replacement.  A reasonable
    // compromise is to use a CharBuffer with a quick sanity check
    // to make sure we have work to do.

    if(xstrnchr(s, '&', len) || xstrnchr(s, '<', len) || xstrnchr(s, '>', len))
    {
      CharBuffer *cb = new CharBuffer();

      if(cb)
      {
	for(int i = 0;i < len;i++)
	{
	  switch(s[i])
	  {
	  case '&':
	    cb->append("&amp;");
	    break;
	  case '<':
	    cb->append("&lt;");
	    break;
	  case '>':
	    cb->append("&gt;");
	    break;
	  default:
	    cb->append(s[i]);
	    break;
	  }
	}

	ret = xstrdup(cb->str());
      }
      else
	wlog->warn("escape_xml failed to allocate CharBuffer");
    }
    else
      ret = xstrncat(NULL, s, len);
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "escape_xml = %s", IONULL(ret));
#endif

  return(ret);
}

bool expire_file(char *filedir, char *filename, int age)
{
  // Remove the file <filedir>/<filename> if it is older than <age>
  // seconds.  The time for comparison is the time of last data
  // modification (mtime).

  // Returns: true if the file was removed, false otherwise, including if
  // it has not yet expired.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "expire_file(%s,%s,%d)",
		   IONULL(filedir), IONULL(filename), age);
#endif

  if(filedir && filename && age >= 0)
  {
    struct timeval tp;

    if(gettimeofday(&tp, NULL)==0)
    {
      char *fbuf = new char[strlen(filedir) + strlen(filename) + 2];
      
      if(fbuf)
      {
	struct stat sb;
	
	sprintf(fbuf, "%s/%s", filedir, filename);
	
	if(stat(fbuf, &sb)==0 && (sb.st_mtime + age < tp.tv_sec)
	   && unlink(fbuf)==0)
	  ret = true;
	// else ignore errors, the file may have disappeared
      
	xdelete(fbuf);
      }
      else
	wlog->warn("expire_file failed to allocate fbuf");
    }
    else
      wlog->warn("gettimeofday failed in expire_file");
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "expire_file = %s", IOTF(ret));
#endif

  return(ret);
}

char *generate_csid(char *stype, char *host, char *service)
{
  // Generate an ID tag for a State object using state type <stype>,
  // <host>, and <service>.  (The c in csid used to mean "check".)

  // Returns: A newly allocated string containing the csid, or NULL
  // on error.

  char *ret = new char[(stype ? strlen(stype) : 0)
		      + (host ? strlen(host) : 0)
		      + (service ? strlen(service) : 0)
		      + 3];

  if(ret)
    sprintf(ret, "%s:%s@%s", (stype ? stype : ""), (host ? host : ""),
	    (service ? service : ""));

  return(ret);
}

int getday(char *day)
{
  // Determine the day of the week specified in <day>, where 0 is
  // Sunday, 7 is "Weekdays", and 8 is "Weekends".
  
  // Returns: 0-8, or -1 on error.

  int r = -1;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "getday(%s)", IONULL(day));
#endif

  if(day)
  {
    switch(tolower(day[0]))
    {
    case 's':
      switch(tolower(day[1]))
      {
      case 'u':
	r = 0;
	break;
      case 'a':
	r = 6;
	break;
      default:
	r = -1;
	break;
      }
      break;
    case 'm':
      r = 1;
      break;
    case 't':
      switch(tolower(day[1]))
      {
      case 'u':
	r = 2;
	break;
      case 'h':
	r = 4;
	break;
      default:
	r = -1;
	break;
      }
      break;
    case 'w':
      if(strlen(day) > 5)
      {
	switch(tolower(day[2]))
	{
	case 'd':
	  r = 3;
	  break;
	case 'e':
	  switch(tolower(day[4]))
	  {
	  case 'd':
	    r = 7;
	    break;
	  case 'e':
	    r = 8;
	    break;
	  default:
	    r = -1;
	    break;
	  }
	  break;
	default:
	  r = -1;
	  break;
	}
      }
      else
	r = -1;
      break;
    case 'f':
      r = 5;
      break;
    default:
      r = -1;
      break;
    }
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "getday = %d", r);
#endif

  return(r);
}

#if defined(_MISSING_REENTRANT_GETHOST)
struct hostent *gethostbyaddr_r(const char *addr, int len, int type,
				struct hostent *hostent, char *buf,
				int bufsize, int *herrno)
{
  // Perform a gethostbyaddr lookup for <addr> (of length <len> and
  // type <type>) using the static storage <hostent> and <buf>, which
  // is of size <bufsize>.  On error, the value of h_errno will be
  // stored in <herrno> if provided.  Only a limited set of
  // information is stored in <hostent>.

  // Returns: A pointer to <hostent> if successful, NULL on error.

  struct hostent *ret = NULL;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "gethostbyaddr_r(%s,%d,%d,%d,%s,%d,%d)",
		   IONULL(addr), len, type, hostent,
		   (buf ? "<buf>" : "(null)"), bufsize, h_errno);
#endif
  
  if(addr && hostent && buf)
  {
    if(pthread_mutex_lock(&nremutex)==0)
    {
      struct hostent *hp = gethostbyaddr(addr, len, type);

      if(hp)
      {
	// Store a copy of h_name, the first address in h_addr_list
	// (h_addr = h_addr_list[0], this might be a Darwin thing),
	// and the length of h_addr.  Make sure we have enough space.
	// This is an exact duplicate of what we do in gethostbyname_r.

	if(hp && hp->h_name && hp->h_addr &&
	   (strlen(hp->h_name) + strlen(hp->h_addr) + 8 < bufsize))
	{
	  // Copy the name, though we don't really need it for this function
	  char *p = buf;
	  memcpy(p, hp->h_name, strlen(hp->h_name));
	  p[strlen(hp->h_name)] = '\0';
	  hostent->h_name = p;

	  // We don't use aliases, so ignore them
	  hostent->h_aliases = NULL;

	  // Note the address type
	  hostent->h_addrtype = hp->h_addrtype;

	  // Copy the address list, although we're really only copying the
	  // first address since that is all we use
	  p += strlen(hp->h_name) + 1;
	  hostent->h_addr_list = (char **)p;
	  p++;
	  *p = NULL;
	  p++;
	  memcpy(p, hp->h_addr, strlen(hp->h_addr));
	  p[strlen(hp->h_addr)] = '\0';
	  hostent->h_addr_list[0] = p;

	  ret = hostent;
	}
	else
	{
	  if(herrno)
	    *herrno = NO_RECOVERY;
	}
      }
      else
      {
	if(herrno)
	  *herrno = h_errno;
      }
      
      pthread_mutex_unlock(&nremutex);
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "gethostbyaddr_r = %d", ret);
#endif
  
  return(ret);
}

struct hostent *gethostbyname_r(const char *name, struct hostent *hostent,
				char *buf, int bufsize, int *herrno)
{
  // Perform a gethostbyname lookup for <name> using the static storage
  // <hostent> and <buf>, which is of size <bufsize>.  On error, the
  // value of h_errno will be stored in <herrno> if provided.  Only a
  // limited set of information is stored in <hostent>.

  // Returns: A pointer to <hostent> if successful, NULL on error.

  struct hostent *ret = NULL;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "gethostbyname_r(%s,%d,%s,%d,%d)",
		   IONULL(name), hostent, (buf ? "<buf>" : "(null)"),
		   bufsize, h_errno);
#endif
  
  if(name && hostent && buf)
  {
    if(pthread_mutex_lock(&nremutex)==0)
    {
      struct hostent *hp = gethostbyname(name);

      if(hp)
      {
	// Store a copy of h_name, the first address in h_addr_list
	// (h_addr = h_addr_list[0], this might be a Darwin thing),
	// and the length of h_addr.  Make sure we have enough space.
	// This is an exact duplicate of what we do in gethostbyaddr_r.

	if(hp && hp->h_name && hp->h_addr &&
	   (strlen(hp->h_name) + strlen(hp->h_addr) + 8 < bufsize))
	{
	  // Copy the name, though we don't really need it for this function
	  char *p = buf;
	  memcpy(p, hp->h_name, strlen(hp->h_name));
	  p[strlen(hp->h_name)] = '\0';
	  hostent->h_name = p;

	  // We don't use aliases, so ignore them
	  hostent->h_aliases = NULL;

	  // Note the address type
	  hostent->h_addrtype = hp->h_addrtype;

	  // Copy the address list, although we're really only copying the
	  // first address since that is all we use
	  p += strlen(hp->h_name) + 1;
	  hostent->h_addr_list = (char **)p;
	  p++;
	  *p = NULL;
	  p++;
	  memcpy(p, hp->h_addr, strlen(hp->h_addr));
	  p[strlen(hp->h_addr)] = '\0';
	  hostent->h_addr_list[0] = p;

	  ret = hostent;
	}
	else
	{
	  if(herrno)
	    *herrno = NO_RECOVERY;
	}
      }
      else
      {
	if(herrno)
	  *herrno = h_errno;
      }
      
      pthread_mutex_unlock(&nremutex);
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "gethostbyname_r = %d", ret);
#endif
  
  return(ret);
}
#endif // _MISSING_REENTRANT_GETHOST

char *get_localhost_fqdn()
{
  // Obtain the fully qualified domainname of the current (local) host.

  // Returns: A pointer to a string that should be deleted when no
  // longer required, or NULL on error.

  char *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "get_localhost_fqdn()");
#endif
  
  // First we obtain the current hostname.

  char hostname[MAXHOSTNAMELEN+1];

  memset(hostname, 0, MAXHOSTNAMELEN+1);

  if(gethostname(hostname, MAXHOSTNAMELEN)==0)
  {
    // If there is a dot (.) in the name, we assume we already have a FQDN.

    char *h = strchr(hostname, '.');

    if(h)
      ret = xstrdup(hostname);
    else
    {
      // hostname is just the short form, we need to get a domain to append

      char domainname[MAXHOSTNAMELEN+1];

      memset(domainname, 0, MAXHOSTNAMELEN+1);

#if defined(HAVE_GETDOMAINNAME)
      // Chances are we're not going to get anything useful (ie: "" or
      // "(none)"), but we'll take a look anyway.

      if(getdomainname(domainname, MAXHOSTNAMELEN)==0
	 || strcmp(domainname, "")==0 || strcmp(domainname, "(none)")==0)
	memset(domainname, 0, MAXHOSTNAMELEN+1);	
#endif

      if(domainname[0] == '\0')
      {
	// First see if domain is set in resolv.conf

	FileHandler *fin = new FileHandler();

	if(fin)
	{
	  if(fin->open_read(_PATH_RESCONF))
	  {
	    for(char *l = fin->read_line();l != NULL;l = fin->read_line())
	    {
	      // We're looking for a line with a domain keyword

	      if(strlen(l) > 6 && strncmp(l, "domain", 6)==0)
	      {
		// Match

		char *p = l+6;

		while(isspace(*p) && *p != '\0')
		  p++;

		strncpy(domainname, p, MAXHOSTNAMELEN);
		break;
	      }
	    }
	  }
	  
	  xdelete(fin);
	}
	else
	  wlog->warn("get_localhost_fqdn failed to allocate FileHandler");
      }

      if(domainname[0] == '\0')
      {
	// Try defaultdomain

	FileHandler *fin = new FileHandler();

	if(fin)
	{
	  if(fin->open_read("/etc/defaultdomain"))
	  {
	    char *l = fin->read_line();

	    if(strlen(l) > 0)
	      strncpy(domainname, l, MAXHOSTNAMELEN);
	  }
	  
	  xdelete(fin);
	}
	else
	  wlog->warn("get_localhost_fqdn failed to allocate FileHandler");
      }

      // Take what we've got and assemble it

      ret = xstrdup(hostname);

      if(domainname[0] != '\0')
      {
	ret = xstrcat(ret, ".");
	ret = xstrcat(ret, domainname);
      }
    }
  }
  else
    wlog->warn("gethostname() failed in get_localhost_fqdn");

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "get_localhost_fqdn = %s", IONULL(ret));
#endif
  
  return(ret);
}

int gettime(char *time, bool hour)
{
  // Determine the hour portion of <time> (HH:MM) if <hour> is true, or
  // the minute portion if false.

  // Returns: An hour from 0 through 23, a minute from 0 through 59, or
  // -1 on error.

  char buf[6];
  int r = -1;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "gettime(%s,%s)", IONULL(time), IOTF(hour));
#endif
  
  if(time && strlen(time)==5)
  {
    strcpy(buf,time);
    buf[2] = '\0';

    if(hour)
    {
      r = atoi(buf);

      if(r < 0)
	r = 0;
      if(r > 23)
	r = 23;
    }
    else
    {
      r = atoi(buf+3);

      if(r < 0)
	r = 0;
      if(r > 59)
	r = 59;
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "gettime = %d", r);
#endif
  
  return(r);
}

void libsrvexit()
{
  // Clean up global variables required by libsrv.  This method clears
  // progname and prng, as well as args and cf if not NULL.  nremutex
  // is also cleared if appropriate.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "libsrvexit()");
#endif

  progname = NULL;

  xdelete(prng);
  xdelete(cf);
  xdelete(args);

#if defined(_MISSING_REENTRANT_GETHOST)
  pthread_mutex_destroy(&nremutex);
#endif
#if defined(ENABLE_MODULE_SERIALIZATION)
  pthread_mutex_destroy(&msmutex);
#endif

#if defined(DEBUG)
  dlog->log_progress(DEBUG_MAJTRC, "Discontinuing debugging");
  dlog->log_exit(DEBUG_MAJTRC, "libsrvexit()");

  xdelete(dlog);
#endif

  xdelete(wlog);
}

bool libsrvinit(char *argv0)
{
  // Initialize global variables required by libsrv.  <argv0> is
  // argv[0] as received by the invoking program.  This method
  // establishes progname and prng for use, and sets args and cf to
  // NULL.  The invoking program is responsible for establishing args
  // and cf.  For relevant platforms, the non-reentrant mutex is
  // also initialized.

  // Returns: true if fully successful, false otherwise.

  return(libsrvinit(argv0, NULL, NULL));
}

bool libsrvinit(char *argv0, Debugger *d, Debugger *w)
{
  // Initialize global variables required by libsrv.  <argv0> is
  // argv[0] as received by the invoking program.  This method
  // establishes progname and prng for use, and sets args and cf to
  // NULL.  The invoking program is responsible for establishing args
  // and cf.  If <d> or <w> are specified, they are used for debug and
  // warning logging.  Otherwise, the default debuggers are used. For
  // relevant platforms, the non-reentrant mutex is also initialized.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;

#if defined(_HAVE_LIBINTL_H)
  // We set up localization here, since any component of the package
  // may be localized.

  textdomain("survivor");
#endif
  
  if(w)
    wlog = w;
  else
    wlog = new StdDebugger();

  if(!wlog)
    return(false);
  
#if defined(DEBUG)
  if(d)
    dlog = d;
  else
    dlog = new StdDebugger();

  if(!dlog)
  {
    xdelete(wlog);
    return(false);
  }
  
  dlog->log_entry(DEBUG_MAJTRC, "libsrvinit(%s,%d,%d)", argv0, d, w);
#endif

  args = NULL;
  cf = NULL;

  if(argv0)
  {
    if(strrchr(argv0, '/'))
      progname = strrchr(argv0, '/')+1;
    else
      progname = argv0;
  }

  prng = new PRNG();

  if(prng)
    prng->init();
  
  if(progname && prng)
    ret = true;

#if defined(_MISSING_REENTRANT_GETHOST)
  if(ret && pthread_mutex_init(&nremutex, NULL) != 0)
    ret = false;
#endif
#if defined(ENABLE_MODULE_SERIALIZATION)
  // We always initialize this, even though only the scheduler requires it.
  // The overhead of calling the mutex in non-threaded components should be
  // pretty low.
  if(ret && pthread_mutex_init(&msmutex, NULL) != 0)
    ret = false;
#endif

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "libsrvinit = %s", IOTF(ret));
#endif
  
  return(ret);
}

time_t next_schedule_time(Array<Schedule> *sched, time_t last)
{
  // Compute the next time the Schedules <sched> is in effect, based on
  // <last>.  Note that when using this to estimate the next time a Check
  // will execute, the scheduler uses a more sophisticated algorithm and
  // may defer until later than the time returned here.

  // Returns: The next time in epoch seconds, or 0.

  time_t ret = 0;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "next_schedule_time(%d,%d)", sched, last);
#endif

  if(sched)
  {
    // Get current time, we may need it later

    struct timeval tv;

    if(gettimeofday(&tv, NULL)==0)
    {
      // Iterate through the non-never Schedules, determining when
      // each is next in effect.
      
      for(int i = 0;i < sched->entries();i++)
      {
	Schedule *s = sched->retrieve(i);

	if(s && (s->frequency() > 0 || s->at()))
	{
	  int nextdiff = s->next(last);

	  if(nextdiff > -1)
	  {
	    // See if any of the previous schedules are in effect during
	    // last+nextdiff.  If so, ignore this entry.

	    bool skip = false;
	    
	    for(int j = 0;j < i;j++)
	    {
	      Schedule *sn = sched->retrieve(j);

	      if(sn && sn->now(nextdiff - (tv.tv_sec - last)))
	      {
		skip = true;
		break;
	      }
	    }

	    if(!skip)
	      ret = last + nextdiff;
	  }
	}
      }
    }
    else
      wlog->warn("gettimeofday failed in next_schedule_time()");
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "next_schedule_time = %d", ret);
#endif
  
  return(ret);
}

#if defined(USE_PROCESS_KILL)
void process_kill(pid_t pid, int signal)
{
  // Send the process <pid> the signal <signal>.  If supported, all
  // child processes will also be sent signal.  (If that behavior is
  // not desired, just use kill() instead.)

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "process_kill()");
#endif

#if defined(_HAVE_PROC)
  Array<PidPair> *pidarray = new Array<PidPair>();

  if(pidarray)
  {
    DIR *dir = opendir(_PROC_DIR);

    if(dir)
    {
      struct dirent *entry = allocate_dirent(_PROC_DIR);
      struct dirent *dp;

      if(entry)
      {
	while(readdir_r(dir, entry, &dp) == 0 && dp)
	{
	  // Only look at directory entries that begin with a number (pids)

	  if(dp && dp->d_name[0] >= '0' && dp->d_name[0] <= '9')
	  {
#if defined(LINUX)
	    const char *psname = "stat";
#elif defined(SOLARIS)
	    const char *psname = "psinfo";
#else
#error OS defined as having procfs but procfs type is unknown
#endif // /proc OS
	    
	    char *pspath = new char[strlen(_PROC_DIR) + strlen(dp->d_name)
				   + strlen(psname) + 3];
	    
	    if(pspath)
	    {
	      sprintf(pspath, "%s/%s/%s", _PROC_DIR, dp->d_name, psname);
	      
	      FILE *psin = fopen(pspath, "r");

	      if(psin)
	      {
		int pid = -1, ppid = -1;
		
#if defined(LINUX)
		char junk[PATH_MAX * 2];
		
		if(fscanf(psin, "%d %s %s %d", &pid, junk, junk, &ppid) > 0)
		{
#elif defined(SOLARIS)
	        psinfo_t p;
		
		if(fread(&p, sizeof(psinfo_t), 1, psin) > 0)
		{
		  pid = p.pr_pid;
		  ppid = p.pr_ppid;
#else
#error OS defined as having procfs but procfs type is unknown
#endif // /proc OS
		  // Don't kill init, in case we're running as root
		  
		  if(pid > 1)
		  {
		    PidPair *pp = new PidPair(pid, ppid);
		    
		    if(pp)
		    {
		      // Add this pid/ppid pair to the array
		      
		      if(!pidarray->add(pp))
		      {
			xdelete(pp);
		      }
		    }
		    else
		      wlog->warn("process_kill failed to allocate pp");
		  }
		}
		fclose(psin);
	      }
	      // else we probably don't have read permission on the process
	      
	      xdelete(pspath);
	    }
	    else
	      wlog->warn("process_kill failed to allocate pspath");
	  }
	}

	// Now that we have our list of arrays, start sending out the signals

	process_kill_target(pidarray, pid, signal);

	free(entry);
	entry = NULL;
      }

      closedir(dir);
    }
    else
      wlog->warn("process_kill failed to opendir %s", _PROC_DIR);

    // Clean up the pidarray by tossing the PidPairs stored within.

    for(int i = 0;i < pidarray->entries();i++)
    {
      PidPair *pp = pidarray->retrieve(i);

      if(pp)
	delete pp;
    }

    xdelete(pidarray);
  }
  else
    wlog->warn("process_kill failed to allocate pidarray");
#else // _HAVE_PROC
  kill(pid, signal);
#endif

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "process_kill()");
#endif
}
  
void process_kill_target(Array<PidPair> *pidarray, int target, int signal)
{
  // Recursively send <target> and its children <signal>, using <pidarray>
  // as the list of pids to examine.  Children are sent the signal before
  // parents.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "process_kill_target(%d,%d,%d)",
		   pidarray, target, signal);
#endif

  if(pidarray && target > 1 && signal)
  {
    // Examine the list of pids.  Anytime we find a child of <target>,
    // recurse over the child pid.
    
    for(int i = 0;i < pidarray->entries();i++)
    {
      PidPair *pp = pidarray->retrieve(i);

      if(pp && pp->ppid()==target)
	process_kill_target(pidarray, pp->pid(), signal);
    }

    // Done recursing, signal the target.

    kill(target, signal);
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "process_kill_target()");
#endif
}
#endif // USE_PROCESS_KILL 

char *read_line(int fd)
{
  // Read from the file descriptor <fd> until an end of line character
  // is seen (\n) or until no more data is available to read.

  // Returns: A newly allocated string that should be delete'd when
  // no longer required, or NULL on error or if no data was read.

  char *ret = NULL;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "read_line(%d)", fd);
#endif

  if(fd > -1)
  {
    bool done = false;
    int retsize = 0;
    int rindex = 0;

    while(!done)
    {
      if(rindex + 2 >= retsize)
      {
	// We allocate ret in intervals of BUFSIZE.

	char *newret = new char[retsize + BUFSIZE];

	if(newret)
	{
	  // Copy the previous string
	  if(ret)
	    strncpy(newret, ret, rindex);

	  // And blank out the new area
	  memset(newret+retsize, 0, BUFSIZE);

	  ret = newret;
	  newret = NULL;
	  retsize += BUFSIZE;
	}
	else
	{
	  wlog->warn("read_line failed to allocate newret");	  
	  done = true;
	}
      }

      if(!done)
      {
	// We have to read one character at a time, since we can't
	// put back characters we've read after we see the new line

	char c[1];

	if(read(fd, c, 1))
	{
	  // Store c, unless it is \n
	  
	  if(c[0] == '\n')
	    done = true;
	  else
	  {
	    ret[rindex] = c[0];
	    rindex++;
	  }
	}
	else
	{
	  // No data ready to read.  If we haven't read anything yet,
	  // return NULL.

	  if(rindex == 0)
	  {
	    xdelete(ret);
	  }
	  
	  done = true;
	}
      }
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "read_line = %s", IONULL(ret));
#endif

  return(ret);
}

char *read_previous_line(int fd)
{
  // Read backwards on the file descriptor <fd> until an end of line
  // character is seen (\n) or until the beginning of the file is
  // encountered.

  // Returns: A newly allocated string that should be delete'd when no
  // longer required, or NULL on error or if no data was read.  The
  // data returned will be in the correct order (sequential from the
  // newline forward).

  char *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "read_previous_line(%d)", fd);
#endif

  if(fd > -1)
  {
    // Initial move backwards

    if(lseek(fd, -1, SEEK_CUR) != -1)
    {    
      // First scan backwards from the current position until we see the
      // beginning

      int offset = 0;
      bool done = false;
      char buf[1] = { '\0' };

      while(!done)
      {
	// read() seems to be more portable than pread()
	
	if(read(fd, buf, 1) == 1 && buf[0] != '\n')
	{
	  offset--;
	  
	  // Move to the previous character (and move back past the
	  // character we just read)
	  
	  if(lseek(fd, -2, SEEK_CUR) == -1)
	  {
	    // We found the beginning of the file, move 1 char instead
	    // (the lseek(-2) should fail and leave fd where it was)

	    lseek(fd, -1, SEEK_CUR);
	    done++;
	  }
	}
	else
	{
	  // We found the beginning of the line, move back over the
	  // char we just read
	  
	  lseek(fd, -1, SEEK_CUR);
	  done++;
	}
      }
      
      // Next, read the line, but move forward if we're sitting on a newline
      
      ret = new char[abs(offset) + 1];
      
      if(ret)
      {
	memset(ret, 0, abs(offset) + 1);
	
	if(buf[0] == '\n')
	  lseek(fd, 1, SEEK_CUR);   
	
	if(read(fd, ret, abs(offset)) != abs(offset))
	  wlog->warn("read() failed in read_previous_line");
	else
	{
	  // Reposition the fd to where we started reading
	  
	  lseek(fd,
		(buf[0] == '\n' ? offset - 1 : offset),
		SEEK_CUR);
	}
      }
      else
	wlog->warn("read_previous_line failed to allocate buf");
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "read_previous_line = %s", IONULL(ret));
#endif

  return(ret);
}

char *read_xml_document(int fd, char *endtag, int timeout)
{
  // Read an XML document from <fd> up to <endtag> and store it.
  // No processing or formatting is performed, initial blanks or
  // garbage are ignored. If <timeout> is non-zero, do not block
  // more than the specified number of seconds.  (This may result
  // in incomplete documents being returned.)

  // Returns: A newly allocated string that should be delete'd when
  // no longer required, or NULL on error.

  char *ret = NULL;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "read_xml_document(%d,%s,%d)", fd,
		  IONULL(endtag), timeout);
#endif

  if(fd > -1 && endtag)
  {
    // We'll read from a FileHandler and copy to a CharBuffer.
    // We can't start a document with whitespace, so we must
    // track whether we've seen the xml tag yet.

    FileHandler *xmlin = new FileHandler(fd);
    CharBuffer *xmlblock = new CharBuffer();
    bool started = false;

    if(xmlin && xmlblock)
    {
      if(timeout > 0)
	xmlin->set_read_timeout(timeout);
      
      for(char *l = xmlin->read_line();
	  l != NULL;
	  l = xmlin->read_line())
      {
	// Our test for the xml tag is pretty simplistic
	// but should suffice.
	      
	if(!started && strstr(l, "xml") || strstr(l, "XML"))
	  started = true;
		
	if(started)
	{
	  // Append the current line as long as we've seen the
	  // xml tag.  We could just xmlin->read_until(endtag) here...
	  
	  xmlblock->append(l);
	  xmlblock->append('\n');

	  if(strstr(l, endtag))
	  {
	    // We're done

	    ret = xstrdup(xmlblock->str());

	    if(!ret)
	      wlog->warn("read_xml_document failed to copy document");
	  }
	}

	if(ret)
	  break;
      }

      // If we make it here with ret = NULL, we probably ran out of
      // stuff to read.
    }
    else
      wlog->warn("read_xml_document failed to allocate CharHandlers");

    xdelete(xmlin);
    xdelete(xmlblock);
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "read_xml_document = %d",
		 ret ? strlen(ret) : 0);
#endif

  return(ret);  
}

List *tokenize(char *s, const char *c)
{
  // Tokenize <s>, separating on any character in <c>, properly
  // handling quotes and escaped characters (prefixed with '\').

  // Returns: The tokens, in a newly allocated List that should be
  // deleted when no longer required, or NULL on error.

  return(tokenize(s, c, 0));
}

List *tokenize(char *s, const char *c, int max)
{
  // Tokenize <s>, separating on any character in <c>, properly
  // handling quotes and escaped characters (prefixed with '\').
  // Up to <max> tokens will be generated.  If <max> is 0, then
  // no upper bound applies.

  // Returns: The tokens, in a newly allocated List that should be
  // deleted when no longer required, or NULL on error.

  List *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "tokenize(%s,%s,%d)", IONULL(s), c, max);
#endif

  if(s && c)
  {
    CharBuffer *cbuf = new CharBuffer();
    ret = new List();
    
    if(cbuf && ret)
    {
      bool esc = false;  // Did we just see a backslash?
      int quotes = 0;    // 0 = none, 1 = ', 2 = "
      
      for(int i = 0;i < strlen(s);i++)
      {
	// First check for escape
	
	if(!esc && s[i] == '\\')
	  esc = true;
	else
	{
	  if(!esc && (s[i] == '\'' || s[i] == '"'))
	  {
	    // Handle single and double quotes
	    
	    if(!quotes)
	    {
	      // Begin quote
	      
	      if(s[i] == '\'')
		quotes = 1;
	      else
		quotes = 2;
	    }
	    else
	    {
	      // End quote, if matching type

	      if((s[i] == '\'' && quotes == 1) ||
		 (s[i] == '"' && quotes == 2))
		quotes = 0;
	      else
		cbuf->append(s[i]);  // Easier to just append it here
	    }
	  }
	  else
	  {
	    // Compare the character against the tokenizers.  We don't
	    // break the outer loop when max-1 entries is reached
	    // because we want to continue removing escape characters.

	    bool tmatch = false;
	    
	    if(!esc && !quotes && ret->entries() != max - 1)
	    {
	      for(int j = 0;j < strlen(c);j++)
	      {
		if(s[i] == c[j])
		{
		  tmatch = true;
		  break;
		}
	      }
	    }

	    if(tmatch)
	    {
	      // Matching character.  If cbuf has any content, add it
	      // to ret and reset.
		
	      if(cbuf->str() && strlen(cbuf->str()) > 0)
	      {
		ret->add(cbuf->str());
		cbuf->clear();
	      }
	    }
	    else
	    {
	      // Copy the character to cbuf
		
	      cbuf->append(s[i]);
	    }
	  }

	  // Escape only lasts one iteration
	  
	  if(esc)
	    esc = false;
	}
      }

      // Add the last segment, if any

      if(cbuf->str() && strlen(cbuf->str()) > 0)
	ret->add(cbuf->str());
    }
    else
    {
      wlog->warn("tokenize failed to allocate List");
      xdelete(ret);
    }

    xdelete(cbuf);
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "tokenize = %d", ret);
#endif

  return(ret);
}

void toss_eol(char *s)
{
  // NULL out any newline end of line (\n, \r) characters in <s>.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "toss_eol(%s)", IONULL(s));
#endif

  if(s)
  {
    char *p = strchr(s, '\n');

    if(p)
      *p = '\0';

    p = strchr(s, '\r');

    if(p)
      *p = '\0';
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "toss_eol()");
#endif
}
 
FILE *try_fopen(const char *filename, const char *mode)
{
  // Attempt to open <filename> with <mode>.  This method will retry
  // if the open fails for transient reasons.

  // Returns: A FILE pointer, or NULL on non-transient error.
  
  FILE *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "try_fopen(%s,%s)", IONULL(filename),
		  IONULL(mode));
#endif

  if(filename && mode)
  {
    // Make up to 3 attempts at opening the file

    for(int retryl = 0;retryl < 3;retryl++)
    {
      ret = fopen(filename, mode);

      if(ret)
	break;  // Success
      else
      {
	// Note the same test is done in try_open
	
	if(errno == EINTR || errno == ENFILE ||
	   errno == EMFILE || errno == ENOMEM)
	{
	  wlog->warn("try_fopen failed to open %s (%s, %s)",
		     filename, strerror(errno),
		     (char *)(retryl < 2 ? "trying again" : "giving up"));
	  sleep(1);
	}
	else
	  break;  // Non-transient error
      }
    }
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "try_fopen = %d", ret);
#endif

  return(ret);
}

int try_lock(char *lockfile)
{
  // Attempt to obtain a lock on <lockfile>, creating <lockfile> if it
  // does not exist.  This method will retry if the lock fails for
  // transient reasons.

  // Returns: A number identifying the lock that must be passed to
  // close() to release the lock, or -1 on error.

  int ret = -1;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "try_lock(%s)", IONULL(lockfile));
#endif

  if(lockfile)
  {
    mode_t u = umask(007);
    
    // Make up to 3 attempts at locking the file
    
    for(int retryl = 0;retryl < 3;retryl++)
    {
      int fd = open(lockfile, O_WRONLY | O_CREAT, FILE_OTH_NO);

      if(fd > -1)
      {
	verify_file(lockfile, FILE_GRP_WR);
	
	if(lockf(fd, F_LOCK, 0)==0)
	{
	  ret = fd;
	  break;
	}
	else
	{
	  wlog->warn("try_lock failed to lock %s (%s), %s",
		     lockfile, strerror(errno),
		     (retryl < 2 ? "trying again" : "giving up"));
			   
	  close(fd);
	  sleep(1);
	}
      }
      else
      {
	wlog->warn("try_lock failed to open %s", lockfile);
	break;
      }
    }

    umask(u);
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "try_lock = %d", ret);
#endif
  
  return(ret);
}

int try_open(const char *filename, int flags, mode_t mode)
{
  // Attempt to open <filename> with <flags> and <mode>.  This method
  // will retry if the open fails for transient reasons.

  // Returns: A file descriptor, or -1 on error.

  int ret = -1;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "try_open(%s,%d,%d)", IONULL(filename),
		  flags, mode);
#endif

  if(filename)
  {
    // Make up to 3 attempts at opening the file

    for(int retryl = 0;retryl < 3;retryl++)
    {
      ret = open(filename, flags, mode);

      if(ret > -1)
	break;  // Success
      else
      {
	// Note the same test is done in try_fopen
	
	if(errno == EINTR || errno == ENFILE ||
	   errno == EMFILE || errno == ENOMEM)
	{
	  wlog->warn("try_open failed to open %s (%s, %s)",
		     filename, strerror(errno),
		     (char *)(retryl < 2 ? "trying again" : "giving up"));
	  sleep(1);
	}
	else
	  break;  // Non-transient error
      }
    }
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "try_open = %d", ret);
#endif

  return(ret);
}

char *xnumdup(int i)
{
  // Allocate a new char string containing <i>.

  // Returns: A new char string that should be delete'd when no longer
  // required, or NULL.

  char *r = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "xnumdup(%d)", i);
#endif
  
  r = new char[INTBUFSIZE];

  if(r)
  {
    memset(r, 0, INTBUFSIZE);
    sprintf(r, "%d", i);
  }
  else
    wlog->warn("xnumdup failed to allocate r");

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "xnumdup = %s", IONULL(r));
#endif
  
  return(r);
}

char *xstrcat(char *s1, char *s2)
{
  // Concatenate <s2> to the end of <s1>.  <s1> is deleted by this
  // function, <s2> is not.  If <s1> is NULL, this function behaves
  // as xstrdup(s2).

  // Returns: A new char string containing the concatenation that should
  // be deleted when no longer required, or NULL on error.

  return(xstrncat(s1, s2, (s2 ? strlen(s2) : 0)));
}

char *xstrncat(char *s1, char *s2, int len)
{
  // Concatenate <len> characters from <s2> to the end of <s1>.  <s1>
  // is deleted by this function, <s2> is not.  If <s1> is NULL, this
  // function behaves as xstrdup(s2), except with <len> length.

  // Returns: A new char string containing the concatenation that should
  // be deleted when no longer required, or NULL on error.

  char *r = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "xstrncat(%s,%s,%d)",
		  IONULL(s1), IONULL(s2), len);
#endif
  
  if(s1)
  {
    if(s2 && len > 0)
    {
      // We can't strlen(s2) because we might be called with a non-NULL
      // terminated string.
      
      r = new char[strlen(s1) + len + 1];

      if(r)
      {
	strcpy(r, s1);
	strncat(r, s2, len);
	r[strlen(s1) + len] = '\0';
      }

      xdelete(s1);
    }
    else
      r = s1;
  }
  else
  {
    r = new char[len + 1];

    if(r)
    {
      strncpy(r, s2, len);
      r[len] = '\0';
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "xstrncat = %s", IONULL(r));
#endif
  
  return(r);
}

char *xstrnchr(char *s, char c, int len)
{
  // Determine if char <c> occurs in the first <len> chars of <s>.

  // Returns: A pointer to the position where <c> first occurs, if
  // found, or NULL otherwise.

  char *ret = NULL;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "xstrnchr(%s,%c,%d)", IONULL(s), c, len);
#endif

  if(s && len > 0)
  {
    for(int i = 0;i < len;i++)
    {
      if(s[i] == c)
      {
	ret = s+i;
	break;
      }
    }
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "xstrnchr = %s", IONULL(ret));
#endif
  
  return(ret);
}

void xstrchop(char *s)
{
  // Truncate the last character from <s>.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "xstrchop(%s)", IONULL(s));
#endif

  if(s)
    s[strlen(s)-1] = '\0';

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "xstrchop()");
#endif
}

char *xstrdup(char *s)
{
  // Allocate a new char string containing <s>.

  // Returns: A new char string that should be delete'd when no longer
  // required, or NULL.

  char *r = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "xstrdup(%s)", IONULL(s));
#endif
  
  if(s)
  {
    r = new char[strlen(s) + 1];

    if(r)
      strcpy(r,s);
    else
      wlog->warn("xstrdup failed to allocate r");
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "xstrdup = %s", IONULL(r));
#endif
  
  return(r);
}

char *xstrreplace(char *s, char *sfind, char *sreplace)
{
  // Allocate a new char string in which occurrences of <sfind> in <s>
  // are replaced by <sreplace>.  None of the original strings are
  // modified.

  // Returns: A new char string that should be delete'd when no longer
  // required, or NULL.

  char *r = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "xstrreplace(%s,%s,%s)",
		  IONULL(s), IONULL(sfind), IONULL(sreplace));
#endif

  // This implementation loops through the string, replacing one instance
  // at a time.  This may not be the most efficient approach for large
  // numbers of replacements in a single string, but should work for the
  // expected typical case.
  
  if(s && sfind && sreplace)
  {
    char *p;

    // Duplicate s since we return a new copy regardless of any replacing.
    r = xstrdup(s);
    
    while((p = strstr(r, sfind)) != NULL)
    {
      // Allocate a new r that is sized for the replacement, then swap

      char *newr = new char[strlen(s) + strlen(sreplace) - strlen(sfind) + 1];

      if(newr)
      {
	memset(newr, 0, strlen(s) + strlen(sreplace) - strlen(sfind) + 1);
	
	// First, copy up to (but not including) the replacement string
	strncpy(newr, r, (p - r));

	// Now, append the replacement string
	strcat(newr, sreplace);

	// Next, append the rest of the original
	strcat(newr, p + strlen(sfind));

	// And finally swap
	delete r;
	r = newr;
      }
      else
	break;
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "xstrdup = %s", IONULL(r));
#endif
  
  return(r);
}
