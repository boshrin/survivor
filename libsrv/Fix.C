/*
 * Fix.C: An object to hold Fix definitions.
 *
 * Version: $Revision: 0.8 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2005/04/07 01:25:10 $
 * MT-Level: Safe
 *
 * Copyright (c) 2002 - 2003
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: Fix.C,v $
 * Revision 0.8  2005/04/07 01:25:10  benno
 * Use Array<Argument> instead of List
 *
 * Revision 0.7  2003/10/06 23:13:43  benno
 * Add support for Fix dependencies
 *
 * Revision 0.6  2003/04/09 20:23:47  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.5  2003/04/04 22:39:00  benno
 * Use Debugger
 *
 * Revision 0.4  2003/03/04 17:59:42  benno
 * Bump copyright
 *
 * Revision 0.3  2003/01/24 16:51:59  benno
 * Add IONULL and IOTF
 *
 * Revision 0.2  2002/12/31 04:31:50  benno
 * Use fix_lock_t
 *
 * Revision 0.1  2002/12/27 03:16:51  benno
 * Initial revision
 *
 */

#include "survivor.H"

Fix::Fix(char *name, char *module, Array<Argument> *modargs,
	 Transport *transport, int timeout, int lockexpiry,
	 fix_lock_t locktype)
{
  // Allocate and initialize a new Fix object, identified by <name>.
  // This object runs <module> with optional arguments <modargs>, and
  // times out in <timeout> seconds.  <lockexpiry> is the amount of
  // time in seconds after which the lock that protects this Fix
  // should be considered stale and be removed.  <transport>, if
  // specified, (it must remain valid for the duration of this
  // object), indicates the Transport module that should be used to
  // execute this Fix.  If not specified, the Fix will be executed
  // directly on the scheduler host which may not be particularly
  // useful.  <locktype> may be used to indicate what type of locking
  // is required.  <modargs> will be maintained by this object and
  // deleted when no longer required.

  // Returns: A new Fix object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Fix::Fix(%s,%s,%d,%d,%d,%d,%d)",
		   IONULL(name), IONULL(module), modargs, transport,
		   timeout, lockexpiry, locktype);
#endif

  depends = NULL;
  tr = transport;
  modarg = modargs;
  ltype = locktype;
  fixname = xstrdup(name);
  modname = xstrdup(module);
  ft = timeout;
  lt = lockexpiry;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Fix::Fix()");
#endif
}

bool Fix::add_dependency(Dependency *d)
{
  // Add <d> as a Dependency this Fix depends on.  Dependencies are processed
  // in the order added.

  // Returns: true if the Dependency was successfully added, false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Fix::add_dependency(%d)", d);
#endif
  
  if(d)
  {
    if(!depends)
    {
      // Allocate a new Array

      depends = new Array<Dependency>();
    }

    if(depends)
      ret = depends->add(d);
    else
      wlog->warn("Fix unable to allocate Dependency Array");
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Fix::add_dependency = %s", IOTF(ret));
#endif
  
  return(ret);
}

Array<Dependency> *Fix::dependencies()
{
  // Obtain the Dependencies that this Fix depends on, if any.

  // Returns: A pointer to an Array of Dependencies that should not be
  // modified.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Fix::dependencies()");
  dlog->log_exit(DEBUG_MINTRC, "Fix::dependencies = %d", depends);
#endif
  
  return(depends);
}

int Fix::fix_timeout()
{
  // Obtain the timeout for the Fix module.

  // Returns: The timeout.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Fix::fix_timeout()");
  dlog->log_exit(DEBUG_MINTRC, "Fix::fix_timeout = %d", ft);
#endif
  
  return(ft);
}

int Fix::lock_timeout()
{
  // Obtain the timeout for the lock file that protects this Fix.

  // Returns: The timeout.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Fix::lock_timeout()");
  dlog->log_exit(DEBUG_MINTRC, "Fix::lock_timeout = %d", lt);
#endif
  
  return(lt);
}

fix_lock_t Fix::lock_type()
{
  // Determine the locking type required for this Fix.

  // Returns: The lock type.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Fix::lock_type()");
  dlog->log_exit(DEBUG_MINTRC, "Fix::lock_type = %d", ltype);
#endif
  
  return(ltype);
}

Array<Argument> *Fix::modargs()
{
  // Obtain the array of arguments for the module for this Fix, if provided.

  // Returns: The array of arguments, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Fix::modargs()");
  dlog->log_exit(DEBUG_MINTRC, "Fix::modargs = %d", modarg);
#endif
  
  return(modarg);
}

char *Fix::module()
{
  // Obtain the name of the module for this Fix.

  // Returns: The module name, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Fix::module()");
  dlog->log_exit(DEBUG_MINTRC, "Fix::module = %s", IONULL(modname));
#endif
  
  return(modname);
}

char *Fix::name()
{
  // Obtain the name of this Fix.

  // Returns: The Fix name, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Fix::name()");
  dlog->log_exit(DEBUG_MINTRC, "Fix::name = %s", IONULL(fixname));
#endif
  
  return(fixname);
}

Transport *Fix::transport()
{
  // Obtain the Transport module this fix should use, if provided.

  // Returns: The Transport module to use, or NULL if the scheduler
  // should execute this Fix directly (which may be of limited utility).

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Fix::transport()");
  dlog->log_exit(DEBUG_MINTRC, "Fix::transport = %d", tr);
#endif
  
  return(tr);
}

Fix::~Fix()
{
  // Deallocate the Fix object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Fix::~Fix()");
#endif

  // Delete the Dependency Array, but not the Dependencies, which are
  // stored in cf.
  xdelete(depends);
  
  tr = NULL;
  xadelete(modarg, Argument);
  ltype = standard_lock;
  xdelete(fixname);
  xdelete(modname);
  ft = -1;
  lt = -1;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Fix::~Fix()");
#endif
}
