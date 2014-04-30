/*
 * StateCache.C: An object to cache state.
 *
 * Version: $Revision: 0.6 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2003/07/23 16:19:56 $
 * MT-Level: Safe
 *
 * Copyright (c) 2003
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: StateCache.C,v $
 * Revision 0.6  2003/07/23 16:19:56  benno
 * Fix text order bug in warning
 *
 * Revision 0.5  2003/05/04 21:11:58  benno
 * No more string type
 *
 * Revision 0.4  2003/04/13 19:57:13  benno
 * Add Fix and AlertState caching
 *
 * Revision 0.3  2003/04/09 19:50:03  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.2  2003/03/31 12:49:11  benno
 * Second approach
 *
 * Revision 0.1  2003/03/25 13:09:31  paul
 * Initial revision
 *
 */

#include "scheduler.H"

StateCache::StateCache()
{
  // Allocate and initialize a new StateCache object.  Caching is
  // disabled until enabled.

  // Returns: A new StateCache object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "StateCache::StateCache()");
  dlog->log_exit(DEBUG_MAJTRC, "StateCache::StateCache()");
#endif
  
  caching = false;
}

bool StateCache::disable()
{
  // Disable the StateCache.  Any outstanding State objects obtained
  // via the StateCache object should be released.

  // Returns: true if fully successful, false otherwise.
  
  bool ret = false;

#if defined(_DEBUG)
  dlog->log_entry(DEBUG_MAJTRC | DEBUG_CACHES, "StateCache::disable()");
#endif

  if(caching)
  {
    caching = false;
    locks->sc_unprotect_all();
    
    ret = true;
  }

#if defined(_DEBUG)
  dlog->log_exit(DEBUG_MAJTRC | DEBUG_CACHES, "StateCache::disable = %s",
		 IOTF(ret));
#endif

  return(ret);
}

bool StateCache::enable()
{
  // Enable the StateCache.  This method should not be called while
  // there are any outstanding State objects obtained via the StateCache
  // object, and should only be called when the Configuration is locked
  // or otherwise protected against changes.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;

#if defined(_DEBUG)
  dlog->log_entry(DEBUG_MAJTRC | DEBUG_CACHES, "StateCache::enable()");
#endif

  if(!caching)
  {
    // Success unless otherwise noted

    ret = true;
    
    // Iterate through the Configuration, adding entries for every
    // service@host.

    Array<Check> *a = cf->get_all_checks();

    if(a)
    {
      for(int i = 0;i < a->entries();i++)
      {
	Check *c = a->retrieve(i);

	if(c)
	{
	  List *g = cf->find_group(c->name());

	  if(g)
	  {
	    // Each host in g is checked for c

	    for(int j = 0;j < g->entries();j++)
	    {
	      if(g->retrieve(j))
	      {
		AlertState *as = new AlertState(c, g->retrieve(j));
		CheckState *cs = new CheckState(c, g->retrieve(j));
		FixState *fs = new FixState(c, g->retrieve(j));

		if(!as || !cs || !fs)
		{
		  ret = false;

		  wlog->warn("StateCache::enable failed to allocate State");

		  xdelete(as);
		  xdelete(cs);
		  xdelete(fs);
		}
		else
		{
		  if(!locks->sc_protect(as->id(), as))
		  {
		    ret = false;
		    xdelete(as);
		  }

		  if(ret && !locks->sc_protect(cs->id(), cs))
		  {
		    ret = false;
		    xdelete(cs);
		  }

		  if(ret && !locks->sc_protect(fs->id(), fs))
		  {
		    ret = false;
		    xdelete(fs);
		  }

		  if(!ret)
		    wlog->warn("StateCache::enable failed to protect %s@%s State",
			       IONULL(c->name()), g->retrieve(j));
		}
	      }
	    }
	  }
	}
      }
    }

    if(ret)
      caching = true;
    else
      locks->sc_unprotect_all();
  }
  
#if defined(_DEBUG)
  dlog->log_exit(DEBUG_MAJTRC | DEBUG_CACHES, "StateCache::enable = %s",
		 IOTF(ret));
#endif
  
  return(ret);
}

AlertState *StateCache::get_alert_state(Check *c, char *h)
{
  // Obtain an AlertState for the Check <c> and host <h> for read only
  // access.

  // Returns: A pointer to a AlertState object that should be returned
  // via release() when no longer required, or NULL on error.

  AlertState *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "StateCache::get_alert_state(%d,%s)",
		  c, IONULL(h));
#endif

  if(c && c->name() && h)
  {
    // If caching is enabled, try to retrieve from cache.  If not found,
    // return NULL since enable() should have set up for every service
    // @host pair.

    if(caching)
    {
      char *csid = generate_csid("alert", h, c->name());

      if(csid)
      {
	ret = (AlertState *)locks->sc_lock(csid);
	
	// Make sure the AlertState is current
	
	if(ret)
	  ret->verify_freshness();

	xdelete(csid);
      }
    }
    else // No caching
      ret = new AlertState(c, h);
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "StateCache::get_alert_state = %d", ret);
#endif
  
  return(ret);
}

CheckState *StateCache::get_check_state(Check *c, char *h)
{
  // Obtain a Type II CheckState for the Check <c> and host <h>
  // for read only access.

  // Returns: A pointer to a CheckState object that should be returned
  // via release() when no longer required, or NULL on error.

  CheckState *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "StateCache::get_check_state(%d,%s)",
		  c, IONULL(h));
#endif

  if(c && c->name() && h)
  {
    // If caching is enabled, try to retrieve from cache.  If not found,
    // return NULL since enable() should have set up for every service
    // @host pair.

    if(caching)
    {
      char *csid = generate_csid("check", h, c->name());

      if(csid)
      {
	ret = (CheckState *)locks->sc_lock(csid);

	// Make sure the CheckState is current
      
	if(ret)
	  ret->verify_freshness();

	xdelete(csid);
      }
    }
    else // No caching
      ret = new CheckState(c, h);
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "StateCache::get_check_state = %d", ret);
#endif
  
  return(ret);
}

FixState *StateCache::get_fix_state(Check *c, char *h)
{
  // Obtain FixState for the Check <c> and host <h> for read only
  // access.

  // Returns: A pointer to a FixState object that should be returned
  // via release() when no longer required, or NULL on error.

  FixState *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "StateCache::get_fix_state(%d,%s)",
		  c, IONULL(h));
#endif

  if(c && c->name() && h)
  {
    // If caching is enabled, try to retrieve from cache.  If not found,
    // return NULL since enable() should have set up for every service
    // @host pair.

    if(caching)
    {
      char *csid = generate_csid("fix", h, c->name());

      if(csid)
      {
	ret = (FixState *)locks->sc_lock(csid);

	// Make sure the FixState is current
	
	if(ret)
	  ret->verify_freshness();

	xdelete(csid);
      }
    }
    else // No caching
      ret = new FixState(c, h);
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "StateCache::get_fix_state = %d", ret);
#endif
  
  return(ret);
}

AlertState *StateCache::release(AlertState *as)
{
  // Release the AlertState <as> obtained via get_alert_state().

  // Returns: NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "StateCache::release(%d)", as);
#endif

  if(as)
  {
    if(caching)
    {
      // Release our lock
      
      locks->sc_unlock(as->id());
    }
    else
    {
      // <as> was new'd, so toss it.
      
      xdelete(as);
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "StateCache::release = NULL");
#endif

  return(NULL);
}

CheckState *StateCache::release(CheckState *cs)
{
  // Release the CheckState <cs> obtained via get_check_state().

  // Returns: NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "StateCache::release(%d)", cs);
#endif

  if(cs)
  {
    if(caching)
    {
      // Release our lock
      
      locks->sc_unlock(cs->id());
    }
    else
    {
      // <cs> was new'd, so toss it.
      
      xdelete(cs);
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "StateCache::release = NULL");
#endif

  return(NULL);
}

FixState *StateCache::release(FixState *fs)
{
  // Release the FixState <fs> obtained via get_fix_state().

  // Returns: NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "StateCache::release(%d)", fs);
#endif

  if(fs)
  {
    if(caching)
    {
      // Release our lock
      
      locks->sc_unlock(fs->id());
    }
    else
    {
      // <fs> was new'd, so toss it.
      
      xdelete(fs);
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "StateCache::release = NULL");
#endif

  return(NULL);
}

StateCache::~StateCache()
{
  // Deallocate the StateCache object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "StateCache::~StateCache()");
#endif

  disable();
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "StateCache::StateCache()");
#endif
}
