/*
 * Args.C: Survivor argument object.
 *
 * Version: $Revision: 0.26 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2007/01/25 13:05:51 $
 * MT-Level: Safe
 *  MT Safe only if parse_args and parse_instcf are not called after threaded
 *  operation begins.
 *
 * Copyright (c) 2002 - 2005
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: Args.C,v $
 * Revision 0.26  2007/01/25 13:05:51  benno
 * Build correctly with debugging disabled
 *
 * Revision 0.25  2005/11/11 03:59:53  benno
 * Fix bug where parse_instance didn't return true
 *
 * Revision 0.24  2004/08/24 23:49:40  benno
 * Add tmpdir
 *
 * Revision 0.23  2004/04/24 15:11:30  toor
 * Fix minor memory leak
 *
 * Revision 0.22  2004/03/01 23:25:58  benno
 * Add smartscheduling (-S)
 *
 * Revision 0.21  2003/11/29 05:19:06  benno
 * Add arg_o(), cli_allow_archive(), cli_allow_root(), cli_require_comment(),
 * fromtime(), untiltime(), and parse_args with array for -o
 *
 * Revision 0.20  2003/05/04 21:28:37  benno
 * Don't use string type
 *
 * Revision 0.19  2003/04/13 20:01:03  benno
 * Add -L logger
 * Call enable_timestamp for Debuggers
 *
 * Revision 0.18  2003/04/09 20:23:45  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.17  2003/04/07 20:58:23  benno
 * Fix typos
 *
 * Revision 0.16  2003/04/04 03:52:55  benno
 * Use Debugger
 *
 * Revision 0.15  2003/03/04 17:56:30  benno
 * Add --help detection
 * Bump copyright
 *
 * Revision 0.14  2003/01/23 22:38:10  benno
 * Use IONULL, IOTF, xdelete, and xadelete
 *
 * Revision 0.13  2002/12/31 04:26:40  benno
 * Add instuser
 *
 * Revision 0.12  2002/12/16 00:41:17  benno
 * Add comment and pkgdir
 *
 * Revision 0.11  2002/05/24 20:55:56  benno
 * add version stuff
 *
 * Revision 0.10  2002/05/13 18:07:36  benno
 * add check_instance to be called from both versions of parse_args
 *
 * Revision 0.9  2002/05/10 20:06:17  benno
 * add runningasinst
 *
 * Revision 0.8  2002/04/26 20:14:42  toor
 * don't call parse_instcf automatically, let interfaces do it so they
 * can get better error information
 * if only one instance is defined, use it by default
 *
 * Revision 0.7  2002/04/12 15:04:00  benno
 * use BUFSIZE
 *
 * Revision 0.6  2002/04/04 20:06:32  benno
 * copyright
 *
 * Revision 0.5  2002/04/03 17:38:00  benno
 * rcsify date
 *
 * Revision 0.4  2002/04/03 17:37:37  benno
 * helpdir
 *
 * Revision 0.3  2002/04/03 17:36:38  benno
 * trueargs
 * instgid
 * -s (timestamp)
 *
 * Revision 0.2  2002/04/03 17:35:54  benno
 * Use Instances
 *
 * Revision 0.1  2002/04/03 17:34:55  benno
 * initial revision
 *
 */

#include "survivor.H"

Args::Args()
{
  // Allocate and initialize a new Args object.

  // Returns: A new Args object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "Args::Args()");
#endif

  inst = NULL;
  oopthash = NULL;
  bg = true;
  ka = false;
  smartsched = true;
  tstamp = false;
  vers = false;
  cmt = NULL;
  instance = NULL;
  instancebuf = NULL;
  instcf = NULL;
  mdir = NULL;
  pfile = NULL;
  trueargs = NULL;
  gid = -1;
  athreads = DEFAULT_ATHREADS;
  cthreads = DEFAULT_CTHREADS;
  runuid = -1;
  ft = 0;
  ut = 0;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "Args::Args()");
#endif
}

int Args::alert_threads()
{
  // Determine the number of alert threads that should be created.

  // Returns: The number of alert threads, which is DEFAULT_ATHREADS if not
  // otherwise set.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Args::alert_threads()");
  dlog->log_exit(DEBUG_MINTRC, "Args::alert_threads = %d", athreads);
#endif
  
  return(athreads);
}

bool Args::arg(char c)
{
  // Determine if the option <c> was passed in argv.

  // Returns: true if <c> was passed, false otherwise.

  bool r = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Args::arg(%c)", c);
#endif
  
  if(trueargs && strchr(trueargs, c))
    r = true;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Args::arg = %s", IOTF(r));
#endif

  return(r);
}

char *Args::arg_o(char *option)
{
  // Determine if <option> was provided as a suboption to a -o flag.

  // Returns: The value to <option> if provided, an empty string
  // if <option> was provided without a value, or NULL if <option>
  // was not provided.  The string returned should not be modified.

  char *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Args::arg_o(%s)", IONULL(option));
#endif
  
  if(oopthash && option)
    ret = oopthash->retrieve(option);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Args::arg_o = %s", IONULL(ret));
#endif
  
  return(ret);
}

bool Args::background()
{
  // Determine if the Scheduler should background.

  // Returns: true if the Scheduler should background, false otherwise.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Args::background()");
  dlog->log_exit(DEBUG_MINTRC, "Args::background = %s", IOTF(bg));
#endif
  
  return(bg);
}

int Args::check_threads()
{
  // Determine the number of check threads that should be created.

  // Returns: The number of check threads, which is DEFAULT_CTHREADS if not
  // otherwise set.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Args::check_threads()");
  dlog->log_exit(DEBUG_MINTRC, "Args::check_threads = %d", cthreads);
#endif
  
  return(cthreads);
}

char *Args::comment()
{
  // Determine the comment provided on the command line, if any.

  // Returns: The comment, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Args::comment()");
  dlog->log_exit(DEBUG_MINTRC, "Args::comment = %s", IOTF(cmt));
#endif
  
  return(cmt);
}

bool Args::cli_allow_archive()
{
  // Determine if the cli archiving routines should be enabled.

  // Returns: true if the routines should be enabled, false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Args::cli_allow_archive()");
#endif
  
  if(inst)
    ret = inst->cli_allow_archive();
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Args::cli_allow_archive = %s", IOTF(ret));
#endif
  
  return(ret);
}

bool Args::cli_allow_root()
{
  // Determine if the root user may use the cli.

  // Returns: true if the root user may use the cli, false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Args::cli_allow_root()");
#endif
  
  if(inst)
    ret = inst->cli_allow_root();
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Args::cli_allow_root = %s", IOTF(ret));
#endif
  
  return(ret);
}

bool Args::cli_require_comment()
{
  // Determine if cli operations that accept comments should require
  // them.

  // Returns: true if comments are required where available, false
  // otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Args::cli_require_comment()");
#endif
  
  if(inst)
    ret = inst->cli_require_comment();
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Args::cli_require_comment = %s", IOTF(ret));
#endif
  
  return(ret);
}

char *Args::configdir()
{
  // Determine the directory in which the configuration files can be found.

  // Returns: The state directory, using DEFAULT_CFDIR if no other was
  // provided.

  char *r = DEFAULT_CFDIR;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Args::configdir()");
#endif

  if(inst)
    r = inst->configdir();

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Args::configdir = %s", IONULL(r));
#endif
  
  return(r);
}

time_t Args::fromtime()
{
  // Determine the fromtime (specifier for beginning of an interval).

  // Returns: The time in epoch seconds, or 0 if not provided.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Args::fromtime()");
  dlog->log_exit(DEBUG_MINTRC, "Args::fromtime = %d", ft);
#endif
  
  return(ft);
}

char *Args::helpdir()
{
  // Determine the directory where help files can be found.

  // Returns: The help directory, using DEFAULT_HPDIR if no other was
  // provided.

  char *r = DEFAULT_HPDIR;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Args::helpdir()");
#endif

  if(inst)
    r = inst->helpdir();

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Args::helpdir = %s", IONULL(r));
#endif
  
  return(r);
}

char *Args::histdir()
{
  // Determine the directory into which history should be written.

  // Returns: The history directory, using DEFAULT_HDIR if no other was
  // provided.

  char *r = DEFAULT_HDIR;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Args::histdir()");
#endif

  if(inst)
    r = inst->historydir();

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Args::histdir = %s", IONULL(r));
#endif
  
  return(r);
}

List *Args::instances()
{
  // Generate a List containing all defined instances.

  // Returns: A List that should be deleted when no longer required, or NULL
  // on error.

  List *l = NULL;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "Args::instances()");
#endif

  l = new List();

  if(l)
  {
    char *xinstcf = (char *)(instcf ? instcf : DEFAULT_INSTCF);

    FILE *in = fopen(xinstcf, "r");

    if(in)
    {
      lexiastart(in, l);
      yylex();

      if(lexerr || lexincomplete())
      {
	lexerr += lexincomplete();

#if defined(DEBUG)
	if(lexincomplete())
	  dlog->log_progress(DEBUG_CONFIG, "Unexpected EOF (unclosed brace)");
	
	dlog->log_progress(DEBUG_CONFIG, "%d %s encountered while parsing %s",
			    lexerr, ((lexerr == 1) ? " error" : " errors"),
			    xinstcf);
#endif
      }
#if defined(DEBUG)
      else
	dlog->log_progress(DEBUG_CONFIG, "Parse of %s successful", xinstcf);
#endif
      
      fclose(in);
    }
    else
      wlog->warn("Args::instances failed to open %s", xinstcf);
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "Args::instances()");
#endif

  return(l);
}

gid_t Args::instgid()
{
  // Obtain the gid of INSTGROUP.

  // Returns: The gid.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Args::instgid()");
#endif
  
  if(gid == -1)
  {
    group *g, gs;
    char buf[BUFSIZE];

    if(getgrnam_r(INSTGROUP, &gs, buf, BUFSIZE, &g)==0 && g)
      gid = g->gr_gid;
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Args::instgid = %d", gid);
#endif
  
  return(gid);
}

char *Args::instname()
{
  // Obtain the instance in use.

  // Returns: The instance if set, or the default instance otherwise.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Args::instname()");
  dlog->log_exit(DEBUG_MINTRC, "Args::instname = %s", 
		  (char *)(instance ? instance : DEFAULT_INSTANCE));
#endif
  
  return((char *)(instance ? instance : DEFAULT_INSTANCE));
}

char *Args::instuser()
{
  // Obtain the installation username.

  // Returns: The installation username.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Args::instuser()");
  dlog->log_exit(DEBUG_MINTRC, "Args::instuser = %s", INSTUSER);
#endif
  
  return(INSTUSER);
}

bool Args::keepalive()
{
  // Determine if the Scheduler should watch itself and restart on unexpected
  // exit.

  // Returns: true if the Scheduler should keepalive itself, false otherwise.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Args::keepalive()");
  dlog->log_exit(DEBUG_MINTRC, "Args::keepalive = %s", IOTF(ka));
#endif
  
  return(ka);
}

char *Args::moddir()
{
  // Determine the directory in which modules can be found.

  // Returns: The module directory, using DEFAULT_MDIR if no other was
  // provided.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Args::moddir()");
#endif
  
  if(!mdir)
    mdir = DEFAULT_MDIR;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Args::moddir = %s", mdir);
#endif
  
  return(mdir);
}

bool Args::parse_args(char *inst, char *instconfig, char *moddir)
{
  // For survivor executables that cannot accept command line arguments
  // (eg: web interface), this method allows overriding the defaults
  // for <inst>, <instconfig>, and <moddir>.  Any values passed MUST
  // remain valid for the duration of the Args object.

  // Returns: true if the values provided are accepted, false otherwise.

  bool r = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "Args::parse_args(%s,%s,%s)",
		   IONULL(inst), IONULL(instconfig), IONULL(moddir));
#endif

  // None of these are required.  They are not strdup'd because the real
  // parse_args doesn't strdup them.

  instance = inst;
  instcf = instconfig;
  mdir = moddir;

  check_instance();

  r = true;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "Args::parse_args = %s", IOTF(r));
#endif
  
  return(r);
}

bool Args::parse_args(char *opts, int argc, char **argv)
{
  // Parse the args provided on the command line, using <opts> as the
  // formatting string to pass to getopt.  All survivor executables
  // must have the same semantics for any given option, and all must
  // provide minimally i:I:m: (d:i:I:m: if DEBUG is defined).

  // Returns: true if fully successful, false otherwise, including for
  // usage errors.
  
  return(parse_args(opts, NULL, argc, argv));
}

bool Args::parse_args(char *opts, char **oopts, int argc, char **argv)
{
  // Parse the args provided on the command line, using <opts> as the
  // formatting string to pass to getopt.  All survivor executables
  // must have the same semantics for any given option, and all must
  // provide minimally i:I:m: (d:i:I:m: if DEBUG is defined).
  // <oopts>, if provided, is a NULL terminated array of char strings
  // containing values that may be passed to the -o flag. <oopts> must
  // remain valid for the duration of the Args object.

  // Returns: true if fully successful, false otherwise, including for
  // usage errors.

  bool r = true;
  int c;
  extern char *optarg;
  extern int optind;

  // Make a check for any arguments of the form '--', eg: --help.
  // getopt() may interpret this as simple -- and thus stop parsing,
  // instead of complaining.

  for(int i = 0;i < argc;i++)
  {
    if(strlen(argv[i]) > 2 && strncmp(argv[i], "--", 2)==0)
    {
#if defined(DEBUG)
      char *s = NULL;

      for(int i = 0;i < argc;i++)
      {
	if(i > 0)
	  s = xstrcat(s, ",");

	s = xstrcat(s, argv[i]);
      }
      
      s = xstrcat(s, ")");

      dlog->log_entry(DEBUG_MAJTRC, "main(%s)", s);
      dlog->log_entry(DEBUG_MAJTRC, "parse_args(%s)", s);
      dlog->log_exit(DEBUG_MAJTRC, "Args::parse_args = false");

      xdelete(s);
#endif
      
      return(false);
    }
  }

  // Note that at this point, we haven't seen if debugging should be enabled.
  
  while((c = getopt(argc, argv, opts)) != EOF)
  {
    switch(c)
    {
    case 'c':
      cmt = optarg;
      break;
#if defined(DEBUG)
    case 'd':
      dlog->set_level(atoi(optarg));

      {
	// We do this here because obviously if the debug level hasn't been
	// set yet (ie: at main's beginning), no debugging will take place.

	// We need the stanza for scoping of the string.

	char *s = NULL;
	
	for(int i = 0;i < argc;i++)
	{
	  if(i > 0)
	    s = xstrcat(s, ",");

	  s = xstrcat(s, argv[i]);
	}

	s = xstrcat(s, ")");
	
	dlog->log_entry(DEBUG_MAJTRC, "main(%s)", s);
	dlog->log_entry(DEBUG_MAJTRC, "parse_args(%s)", s);

	xdelete(s);
      }
      break;
#endif
    case 'D':
      bg = false;
      break;
    case 'f':
      ft = convtime(optarg);
      break;
    case 'i':
      instance = optarg;
      break;
    case 'I':
      instcf = optarg;
      break;
    case 'k':
      ka = true;
      break;
    case 'L':
      if(strcmp(optarg, "syslog")==0 || strcmp(optarg, "none")==0)
      {
	// Toss the existing Debuggers and replace them

#if defined(DEBUG)
	Debugger *newd = NULL;

	if(strcmp(optarg, "syslog")==0)
	  newd = new SyslogDebugger();
	else
	  newd = new NullDebugger();
#endif
	Debugger *neww = NULL;

	if(strcmp(optarg,"syslog")==0)
	  neww = new SyslogDebugger();
	else
	  neww = new NullDebugger();

	if(neww
#if defined(DEBUG)
	   && newd
#endif
	   )
	{
#if defined(DEBUG)
	  xdelete(dlog);
	  dlog = newd;
#endif
	  xdelete(wlog);
	  wlog = neww;

	  // Enable timestamps if already requested

	  if(tstamp)
	  {
#if defined(DEBUG)
	    dlog->enable_timestamp();
#endif
	    wlog->enable_timestamp();
	  }
	}
	else
	{
#if defined(DEBUG)
	  xdelete(newd);
#endif
	  xdelete(neww);

	  r = false;
	}
      }
      else if(strcmp(optarg, "std")==0)
      {
	// Nothing to do since std is default
      }
      else
      {
	wlog->warn("Unknown debugging method: %s", optarg);
	r = false;
      }
      break;
    case 'm':
      mdir = optarg;
      break;
    case 'o':
      if(oopts)
      {
	// Hashtables are a bit much, but store name=value pairs easily
	
	if(!oopthash)
	  oopthash = new Hashtable<char>();

	if(oopthash)
	{
	  char *options = optarg;
	  char *value;

	  while(*options != '\0')
	  {
	    value = NULL;
	    int sub = getsubopt(&options, oopts, &value);

	    if(sub > -1)
	      oopthash->insert(oopts[sub],
			       xstrdup(value ? value : (char *)""));
	    else
	      r = false;  // unknown suboption
	  }
	}
	else
	{
	  wlog->warn("Args::parse_args failed to allocate Hashtable");
	  r = false;
	}
      }
      else
	r = false;
      break;
    case 'p':
      pfile = optarg;
      break;
    case 's':
      tstamp = true;
#if defined(DEBUG)
      dlog->enable_timestamp();
#endif
      wlog->enable_timestamp();
      break;
    case 'S':
      smartsched = false;
      break;
    case 't':
      cthreads = atoi(optarg);
      break;
    case 'T':
      athreads = atoi(optarg);
      break;
    case 'u':
      ut = convtime(optarg);
      break;
    case 'V':
      vers = true;
      break;
    default:
      if(strchr(opts, c))
      {
	if(trueargs)
	{
	  int l = strlen(trueargs);
	  
	  char *newtrueargs = new char[l+2];
	  
	  if(newtrueargs)
	  {
	    strcpy(newtrueargs, trueargs);
	    newtrueargs[l] = c;
	    newtrueargs[l+1] = '\0';
	    
	    delete trueargs;
	    trueargs = newtrueargs;
	    newtrueargs = NULL;
	  }
	}
	else
	{
	  trueargs = new char[2];
	  
	  if(trueargs)
	  {
	    trueargs[0] = c;
	    trueargs[1] = '\0';
	  }
	}
      }
      else
	r = false;
      break;
    }
  }

  check_instance();
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "Args::parse_args() = %s", IOTF(r));
#endif

  return(r);
}

bool Args::parse_instcf()
{
  // Parse the instance information.  Either parse_args must be
  // successfully completed before this method may be called.

  // Returns: true if the parse is successful, false otherwise.

  bool r = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "Args::parse_instcf()");
#endif
  
  char *xinstance = (char *)(instance ? instance : DEFAULT_INSTANCE);
  char *xinstcf = (char *)(instcf ? instcf : DEFAULT_INSTCF);
  
  FILE *in = fopen(xinstcf, "r");
  
  if(in)
  {
    inst = new Instance(xinstance);
    
    if(inst)
    {
      lexistart(in, inst);
      yylex();
      
      if(lexerr || lexincomplete())
      {
	lexerr += lexincomplete();
	
#if defined(DEBUG)
	if(lexincomplete())
	  dlog->log_progress(DEBUG_CONFIG, "Unexpected EOF (unclosed brace)");

	dlog->log_progress(DEBUG_CONFIG, "%d %s encountered while parsing %s",
			    lexerr, ((lexerr == 1) ? " error" : " errors"),
			    xinstcf);
#endif
      }
      else
      {
#if defined(DEBUG)
	dlog->log_progress(DEBUG_CONFIG, "Parse of %s successful", xinstcf);
#endif
	
	// Make sure we have an useful stuff
	
	if(inst->configdir())
	  r = true;
	else
	{
	  if(!instance)
	    wlog->warn("No instance specified, and there is not exactly one instance defined in %s", xinstcf);
	  else
	    wlog->warn("Instance %s not found in %s", xinstance, xinstcf);
	}
      }
    }
    else
      wlog->warn("Args::parse_instcf failed to allocate new Instance");
    
    fclose(in);
  }
  else
    wlog->warn("Args::parse_instcf failed to open %s", xinstcf);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "Args::parse_instcf = %s", IOTF(r));
#endif
  
  return(r);
}

char *Args::pidfile()
{
  // Determine the name of the file that should be used to store the pid
  // of the Scheduler, if one was specified.

  // Returns: The name of the file, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Args::pidfile()");
  dlog->log_exit(DEBUG_MINTRC, "Args::pidfile = %s", IONULL(pfile));
#endif
  
  return(pfile);
}

char *Args::pkgdir()
{
  // Determine the top level package directory.  This in an advisory
  // location only, as some or all of the package could have been
  // moved out of this directory after compilation.

  // Returns: The package directory, as configured at build time.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Args::pkgdir()");
  dlog->log_exit(DEBUG_MINTRC, "Args::pkgdir = %s", DEFAULT_DIR);
#endif
  
  return(DEFAULT_DIR);
}

bool Args::runningasinst()
{
  // Determine if the currently executing program is running as INSTUSER.

  // Returns: true if the current uid is that of the INSTUSER, false
  // otherwise.

  bool r = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Args::runningasinst()");
#endif
  
  if(runuid == -1)
  {
    // Lookup INSTUSER and compare uid, then cache answer.

    char buf[BUFSIZE];
    struct passwd pw, *pwp;

    if(getpwnam_r(INSTUSER, &pw, buf, BUFSIZE, &pwp)==0 && pwp)
    {
      if(pwp->pw_uid == getuid())
	runuid = 1;
      else
	runuid = 0;
    }
    else
      runuid = 0;  // Getting here may be a bad thing (INSTUSER doesn't exist)
  }

  if(runuid == 1)
    r = true;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Args::runningasinst = %s", IOTF(r));
#endif
  
  return(r);
}

bool Args::smartscheduling()
{
  // Determine whether smart scheduling should be enabled.

  // Returns: true if smart scheduling should be enabled, false otherwise.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Args::smartscheduling()");
  dlog->log_exit(DEBUG_MINTRC, "Args::smartscheduling = %s", IOTF(smartsched));
#endif

  return(smartsched);
}

char *Args::statedir()
{
  // Determine the directory into which state should be written.

  // Returns: The state directory, using DEFAULT_SDIR if no other was
  // provided.

  char *r = DEFAULT_SDIR;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Args::statedir()");
#endif
  
  if(inst)
    r = inst->statedir();

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Args::statedir = %s", IONULL(r));
#endif
  
  return(r);
}

bool Args::timestamp()
{
  // Whether or not to timestamp debugging output.

  // Returns: true if debugging output should be timestamped, false otherwise.

  // Best not to debug this for fear of circular dependencies
  
  return(tstamp);
}

char *Args::tmpdir()
{
  // Determine the directory into which temporary files should be written.

  // Returns: The temporary directory, using DEFAULT_TDIR if no other was
  // provided.

  char *r = DEFAULT_TDIR;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Args::tmpdir()");
#endif
  
  if(inst && inst->tmpdir())
    r = inst->tmpdir();

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Args::tmpdir = %s", IONULL(r));
#endif
  
  return(r);
}

time_t Args::untiltime()
{
  // Determine the untiltime (specifier for end of an interval).

  // Returns: The time in epoch seconds, or 0 if not provided.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Args::untiltime()");
  dlog->log_exit(DEBUG_MINTRC, "Args::untiltime = %d", ut);
#endif
  
  return(ut);
}

bool Args::version()
{
  // Whether or not to display the version number of the package.

  // Returns: true if the version number should be displayed, false otherwise.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Args::version()");
  dlog->log_exit(DEBUG_MINTRC, "Args::version = %s", IOTF(vers));
#endif
  
  return(vers);
}

char *Args::version(version_t info)
{
  // Obtain version information about <info>.

  // Returns: A pointer to a string that should not be modified and that
  // contains the requested information if successful, or NULL.

  char *r = NULL;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Args::version(%d)", info);
#endif

  switch(info)
  {
  case build_version:
    r = BUILDVERSION;
    break;
  case package_version:
    r = PKGVERSION;
    break;
  default:
    break;
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Args::version = %s", IONULL(r));
#endif

  return(r);
}

Args::~Args()
{
  // Deallocate the Args object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "Args::~Args()");
#endif

  xdelete(inst);
  // args are not allocated, but point to argv[] stack
  xhdelete(oopthash, char);
  bg = false;
  ka = false;
  smartsched = false;
  tstamp = false;
  vers = false;
  inst = NULL;
  cmt = NULL;
  instance = NULL;
  instcf = NULL;
  mdir = NULL;
  pfile = NULL;
  athreads = 0;
  cthreads = 0;
  gid = -1;
  runuid = -1;
  ft = 0;
  ut = 0;

  xdelete(instancebuf);
  xdelete(trueargs);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "Args::~Args()");
#endif
}

bool Args::check_instance()
{
  // If there is not already an instance set, look to see if there is
  // exactly one instance.  If so, use it.

  // Returns: true if exactly one instance is defined (and selected),
  // false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "Args::check_instance()");
#endif
  
  if(!instance)
  {
    // If exactly one instance is defined, set it as the one to be used.
    
    List *l = instances();

    if(l)
    {
      if(l->entries()==1)
      {
	// Point to a buffer so we remember to deallocate it on exit.
	instancebuf = xstrdup(l->retrieve(0));
	instance = instancebuf;
	ret = true;
      }
      
      delete l;
      l = NULL;
    }
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "Args::check_instance = %s", IOTF(ret));
#endif
  
  return(ret);
}
