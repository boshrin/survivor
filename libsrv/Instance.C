/*
 * Instance.C: survivor Instance object
 *
 * Version: $Revision: 0.9 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2004/08/25 00:05:46 $
 * MT-Level: Unsafe
 *  This object may be used in a Safe fashion if the set methods are only
 *  called before the object is made available to multiple threads.
 *
 * Copyright (c) 2002 - 2003
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: Instance.C,v $
 * Revision 0.9  2004/08/25 00:05:46  benno
 * Add set_tmp
 *
 * Revision 0.8  2003/11/29 05:25:42  benno
 * Add cli_allow_archive, cli_allow_root, cli_require_comment, and set_X
 *
 * Revision 0.7  2003/04/09 20:23:48  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.6  2003/04/05 23:18:06  benno
 * Use Debugger
 *
 * Revision 0.5  2003/01/24 17:55:32  benno
 * Use IONULL, IOTF, xdelete, and xadelete
 *
 * Revision 0.4  2002/04/04 20:10:43  benno
 * copyright
 *
 * Revision 0.3  2002/04/03 18:23:28  benno
 * rcsify date
 *
 * Revision 0.2  2002/04/03 18:20:15  benno
 * Added helpdir
 *
 * Revision 0.1  2002/04/03 18:19:36  benno
 * initial revision
 *
 */

#include "survivor.H"

Instance::Instance(char *name)
{
  // Allocate and initialize a new Instance object.

  // Returns: A new Instance object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Instance::Instance(%s)", IONULL(name));
#endif

  cliarc = false;
  cliroot = false;
  clicmt = false;
  cfdir = NULL;
  hdir = NULL;
  hpdir = NULL;
  instname = xstrdup(name);
  sdir = NULL;
  tdir = NULL;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Instance::Instance()");
#endif
}

bool Instance::cli_allow_archive()
{
  // Determine if this instance permits the command line interface to
  // run the history archive command.

  // Returns: true if the command is permitted, false otherwise.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Instance::cli_allow_archive()");
  dlog->log_exit(DEBUG_MINTRC, "Instance::cli_allow_archive = %s",
		 IOTF(cliarc));
#endif
  
  return(cliarc);
}

bool Instance::cli_allow_root()
{
  // Determine if this instance permits the command line interface to
  // be run by the root user.

  // Returns: true if the root user is permitted, false otherwise.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Instance::cli_allow_root()");
  dlog->log_exit(DEBUG_MINTRC, "Instance::cli_allow_root = %s",
		 IOTF(cliroot));
#endif
  
  return(cliroot);
}

bool Instance::cli_require_comment()
{
  // Determine if this instance requires comments for commands that
  // accept them. when run via the command line interface.

  // Returns: true if comments are required, false otherwise.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Instance::cli_require_comment()");
  dlog->log_exit(DEBUG_MINTRC, "Instance::cli_require_comment = %s",
		 IOTF(clicmt));
#endif
  
  return(clicmt);
}

char *Instance::configdir()
{
  // Obtain the configuration directory of this Instance.

  // Returns: The configuration directory, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Instance::configdir()");
  dlog->log_exit(DEBUG_MINTRC, "Instance::configdir = %s", IONULL(cfdir));
#endif

  return(cfdir);
}

char *Instance::helpdir()
{
  // Obtain the help directory of this Instance

  // Returns: The help directory, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Instance::helpdir()");
  dlog->log_exit(DEBUG_MINTRC, "Instance::helpdir = %s", IONULL(hpdir));
#endif

  return(hpdir);
}

char *Instance::historydir()
{
  // Obtain the history directory of this Instance.

  // Returns: The history directory, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Instance::historydir()");
  dlog->log_exit(DEBUG_MINTRC, "Instance::historydir = %s", IONULL(hdir));
#endif

  return(hdir);
}

char *Instance::name()
{
  // Obtain the name of this Instance.

  // Returns: The name of this Instance, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Instance::name()");
  dlog->log_exit(DEBUG_MINTRC, "Instance::name = %s", IONULL(instname));
#endif

  return(instname);
}

char *Instance::statedir()
{
  // Obtain the state directory of this Instance.

  // Returns: The state directory, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Instance::statedir()");
  dlog->log_exit(DEBUG_MINTRC, "Instance::statedir = %s", IONULL(sdir));
#endif

  return(sdir);
}

bool Instance::set_cli_allow_archive()
{
  // Set this instance to allow the command line interface to run the
  // history archive command.

  // Returns: true if fully successful, false otherwise.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Instance::set_cli_allow_archive()");
#endif
  
  cliarc = true;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Instance::set_cli_allow_archive = %s",
		 IOTF(true));
#endif
  
  return(true);
}

bool Instance::set_cli_allow_root()
{
  // Set this instance to allow the command line interface to be run
  // by the root user.

  // Returns: true if fully successful, false otherwise.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Instance::set_cli_allow_root()");
#endif
  
  cliroot = true;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Instance::set_cli_allow_root = %s",
		 IOTF(true));
#endif
  
  return(true);
}

bool Instance::set_cli_require_comment()
{
  // Set this instance to require comments for commands that accept
  // them, when run via the command line interface.

  // Returns: true if fully successful, false otherwise.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Instance::set_cli_require_comment()");
#endif
  
  clicmt = true;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Instance::set_cli_require_comment = %s",
		 IOTF(true));
#endif
  
  return(true);
}

bool Instance::set_config(char *dir)
{
  // Set the configuration directory of this Instance to <dir>,
  // if not already set.

  // Returns: true if fully successful, false otherwise.

  bool r = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Instance::set_config(%s)", IONULL(dir));
#endif
  
  if(dir && !cfdir)
  {
    cfdir = xstrdup(dir);

    if(cfdir)
      r = true;
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Instance::set_config = %s", IOTF(r));
#endif

  return(r);
}

bool Instance::set_help(char *dir)
{
  // Set the help directory of this Instance to <dir>, if not
  // already set.

  // Returns: true if fully successful, false otherwise.

  bool r = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Instance::set_help(%s)", IONULL(dir));
#endif
  
  if(dir && !hpdir)
  {
    hpdir = xstrdup(dir);

    if(hpdir)
      r = true;
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Instance::set_help = %s", IOTF(r));
#endif

  return(r);
}

bool Instance::set_history(char *dir)
{
  // Set the history directory of this Instance to <dir>,
  // if not already set.

  // Returns: true if fully successful, false otherwise.

  bool r = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Instance::set_history(%s)", IONULL(dir));
#endif
  
  if(dir && !hdir)
  {
    hdir = xstrdup(dir);

    if(hdir)
      r = true;
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Instance::set_history = %s", IOTF(r));
#endif

  return(r);
}

bool Instance::set_state(char *dir)
{
  // Set the state directory of this Instance to <dir>,
  // if not already set.

  // Returns: true if fully successful, false otherwise.

  bool r = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Instance::set_state(%s)", IONULL(dir));
#endif
  
  if(dir && !sdir)
  {
    sdir = xstrdup(dir);

    if(sdir)
      r = true;
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Instance::set_state = %s", IOTF(r));
#endif

  return(r);
}

bool Instance::set_tmp(char *dir)
{
  // Set the temporary directory of this Instance to <dir>,
  // if not already set.

  // Returns: true if fully successful, false otherwise.

  bool r = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Instance::set_tmp(%s)", IONULL(dir));
#endif
  
  if(dir && !tdir)
  {
    tdir = xstrdup(dir);

    if(tdir)
      r = true;
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Instance::set_tmpdir = %s", IOTF(r));
#endif

  return(r);
}

char *Instance::tmpdir()
{
  // Obtain the temporary directory of this Instance.

  // Returns: The temporary directory, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Instance::tmpdir()");
  dlog->log_exit(DEBUG_MINTRC, "Instance::tmpdir = %s", IONULL(tdir));
#endif

  return(tdir);
}

Instance::~Instance()
{
  // Deallocate the Instance object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Instance::~Instance()");
#endif

  cliarc = false;
  cliroot = false;
  clicmt = false;
  xdelete(cfdir);
  xdelete(hdir);
  xdelete(hpdir);
  xdelete(instname);
  xdelete(sdir);
  xdelete(tdir);
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Instance::~Instance()");
#endif
}
