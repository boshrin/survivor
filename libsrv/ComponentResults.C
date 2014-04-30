/*
 * ComponentResults.C: Object to manage component check result information
 * 
 * Version: $Revision: 0.9 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2006/01/23 02:09:13 $
 * MT-Level: Unsafe, unless set_X methods are not called
 *
 * Copyright (c) 2002 - 2006
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: ComponentResults.C,v $
 * Revision 0.9  2006/01/23 02:09:13  benno
 * Add duration
 *
 * Revision 0.8  2004/04/24 15:14:30  toor
 * Fix memory leak
 *
 * Revision 0.7  2004/03/01 23:28:17  benno
 * Move compose_rc to utils
 *
 * Revision 0.6  2003/06/25 21:33:13  benno
 * Use ComponentCheckResult instead of MODEXEC_INVALID
 *
 * Revision 0.5  2003/06/17 15:09:05  benno
 * Modify ready() to track results already seen
 *
 * Revision 0.4  2003/05/04 21:28:51  benno
 * Don't use string type
 *
 * Revision 0.3  2003/04/09 20:23:46  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.2  2003/04/04 21:44:31  benno
 * Use Debugger
 *
 * Revision 0.1  2003/03/04 17:53:32  benno
 * Initial revision
 *
 */

#include "survivor.H"

#define _CRVALID ocrs && rcrs && crbysvc && onames && rnames

ComponentResults::ComponentResults(Array<Check> *required,
				   Array<Check> *optional, int hosts)
{
  // Allocate a new ComponentResults object, which will hold the results
  // for the <required> and <optional> Checks, on <hosts> number of hosts.
  // neither Array need remain valid after the constructor completes.

  // Returns: A new ComponentResults object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC,
		   "ComponentResults::ComponentResults(%d,%d,%d)",
		   required, optional, hosts);
#endif
  
  ocrs = new Hashtable<ComponentCheckResult>();
  rcrs = new Hashtable<ComponentCheckResult>();
  crbysvc = new Hashtable< Array<ComponentCheckResult> >();
  onames = new List();
  rnames = new List();

  if(_CRVALID)
  {
    if(optional)
    {
      for(int i = 0;i < optional->entries();i++)
      {
	Check *c = optional->retrieve(i);

	if(c && c->name())
	  onames->add(c->name());
      }
    }
    
    if(required)
    {
      for(int i = 0;i < required->entries();i++)
      {
	Check *c = required->retrieve(i);
	
	if(c && c->name())
	  rnames->add(c->name());
      }
    }
  }

  h = hosts;
  opt = (optional ? optional->entries() : 0);
  req = (required ? required->entries() : 0);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "ComponentResults::ComponentResults()");
#endif
}

bool ComponentResults::add(ComponentCheckResult *cr, char *service)
{
  // Add the information stored in <cr> to the ComponentResults
  // structure for the Check named <service>.  <cr> need not remain
  // valid, its contents are copied, but <service> must remain valid.
  // No validation of data is performed, if the same results are
  // provided twice then they will be added twice, until the
  // appropriate number of required or optional check results have
  // been provided.

  // Returns: true if the results have been successfully assimilated,
  // false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "ComponentResults::add(%d,%s)",
		   cr, IONULL(service));
#endif

  if(_CRVALID && cr && cr->hostname() && service)
  {
    // Set up a pointer to the appropriate table and note our max value
    Hashtable<ComponentCheckResult> *crs = ocrs;
    int max = opt;
    bool required = false;
    
    if(rnames->find(service) > -1)
    {
      // This is a required service
      crs = rcrs;
      max = req;
      required = true;
    }

    // See if we already have any results for this host.  If so,
    // retrieve the existing ComponentCheckResult, otherwise create a
    // new one and stuff it in.

    ComponentCheckResult *hashcr = crs->retrieve(cr->hostname());

    if(hashcr)
    {
      // We've already added one result, make sure we haven't exceeded
      // the expected total and then append the comment and adjust the
      // rc values.
      
      if(hashcr->scalar() + 1 <= max)
      {
	char *newcomment = new char[strlen(IONULL(hashcr->comment()))
				   + strlen(IONULL(cr->comment())) + 2];

	if(newcomment)
	  sprintf(newcomment, "%s;%s", IONULL(hashcr->comment()),
		  IONULL(cr->comment()));
	
	hashcr->set_comment(newcomment);
	hashcr->set_rc(compose_rc(hashcr->rc(), cr->rc(), required));
	hashcr->set_scalar(hashcr->scalar() + 1);

	// To compose duration, we add together all valid component
	// durations.  This may or may not be the right thing to do,
	// and could change.

	if(cr->duration() != -1)
	{
	  if(hashcr->duration() == -1)
	    hashcr->set_duration(cr->duration());
	  else
	    hashcr->set_duration(hashcr->duration() + cr->duration());
	}
	
	xdelete(newcomment);
	
	ret = true;
      }
      else
	wlog->warn("ComponentResults::add reached maximum expected %d checks for %s with %s (%s+%s)",
		   max, hashcr->hostname(), service, hashcr->comment(),
		   cr->comment());
    }
    else
    {
      if(max > 0)
      {
	// Allocate a new structure, set scalar to 1 since this is
	// the first result.
	  
	hashcr = new ComponentCheckResult(cr->hostname(), cr->rc(), 1,
					  cr->comment(), cr->duration());

	if(hashcr)
	{
	  if(crs->insert(hashcr->hostname(), hashcr))
	    ret = true;
	  else
	  {
	    delete hashcr;
	    wlog->warn("ComponentResults::add failed to insert ComponentCheckResult");
	  }
	}
	else
	  wlog->warn("ComponentResults::add failed to allocate ComponentCheckResult");
      }
      else
	wlog->warn("ComponentResults::add cannot add unexpected result for %s",
		   cr->hostname());
    }

    if(ret && hashcr)
    {
      // Add an entry into crbysvc

      Array<ComponentCheckResult> *acr = crbysvc->retrieve(service);

      if(acr)
      {
	// Just append hashcr

	acr->add(hashcr);
      }
      else
      {
	// Create a new Array and stuff it in

	acr = new Array<ComponentCheckResult>();

	if(acr)
	{
	  acr->add(hashcr);

	  if(!crbysvc->insert(service, acr))
	  {
	    delete acr;
	    wlog->warn("ComponentResults::add failed to insert new Array");
	    ret = false;
	  }
	}
	else
	{
	  wlog->warn("ComponentResults::add failed to allocate new ComponentCheckResult Array");
	  ret = false;
	}
      }
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "ComponentResults::add = %s", IOTF(ret));
#endif

  return(ret);
}

bool ComponentResults::ready(char *hostname, bool invalidate)
{
  // Determine if the expected required and optional results have been
  // provided for <hostname>.  If <invalidate> is true and the results
  // are ready, mark the results as invalid, so they will not return
  // ready again.

  // Returns: true if all the expected results have been returned, false
  // otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "ComponentResults::ready(%s,%s)",
		  IONULL(hostname), IOTF(invalidate));
#endif

  if(_CRVALID && hostname)
  {
    int ofound = 0;
    int rfound = 0;
    bool invalid = false;

    ComponentCheckResult *ocr = ocrs->retrieve(hostname);
    ComponentCheckResult *rcr = rcrs->retrieve(hostname);

    if(ocr)
    {
      if(ocr->invalid())                // If previously marked invalid,
	invalid = true;                // act as if the results aren't there
      else
	ofound = ocr->scalar();
    }

    if(!invalid && rcr)
    {
      if(rcr->invalid())
	invalid = true;
      else
	rfound = rcr->scalar();
    }

    if(!invalid && ofound == opt && rfound == req)
    {
      if(invalidate)
      {
	// Invalidate after we know we are ready

	if(ocr)
	  ocr->invalidate();
	if(rcr)
	  rcr->invalidate();
      }
      
      ret = true;
    }
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "ComponentResults::ready = %s", IOTF(ret));
#endif

  return(ret);
}

bool ComponentResults::finished(char *service)
{
  // Determine if the expected number of hosts have been reported
  // for <service>.

  // Returns: true if the expected number of hosts have been reported,
  // false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "ComponentResults::finished(%s)",
		   IONULL(service));
#endif
  
  if(_CRVALID && service)
  {
    Array<ComponentCheckResult> *a = crbysvc->retrieve(service);

    if(a && a->entries() == h)
      ret = true;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "ComponentResults::finished = %s",
		  IOTF(ret));
#endif

  return(ret);
}

CheckResult *ComponentResults::result(char *hostname)
{
  // Obtain the result for <hostname>.  Even if <hostname> is not
  // ready(), any results found will be returned.

  // Returns: A newly allocated CheckResult that should be deleted
  // when no longer required, or NULL on error.

  CheckResult *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "ComponentResults::result(%s)",
		   IONULL(hostname));
#endif

  if(_CRVALID && hostname)
  {
    ComponentCheckResult *optcr = ocrs->retrieve(hostname);
    ComponentCheckResult *reqcr = rcrs->retrieve(hostname);

    if(optcr || reqcr)
    {
      // We assemble all collected results here, regardless of what was
      // expected.
      
      char *xcomment = NULL;
      int xrc = MODEXEC_OK;
      int xsc = 0;
      int xdur = -1;

      if(reqcr)
      {
	xcomment = xstrcat(xcomment, reqcr->comment());
	xrc = reqcr->rc();
	xsc = reqcr->scalar();

	if(reqcr->duration() != -1)
	  xdur = reqcr->duration();
      }

      if(optcr)
      {
	// We don't change xrc if reqcr exists.
	
	if(!reqcr)
	  xrc = optcr->rc();
	else
	  xcomment = xstrcat(xcomment, ";");

	xcomment = xstrcat(xcomment, optcr->comment());
	xsc += optcr->scalar();

	if(optcr->duration() != -1)
	{
	  if(xdur == -1)
	    xdur = optcr->duration();
	  else
	    xdur += optcr->duration();
	}
      }
    
      ret = new CheckResult(hostname, xrc, xsc, xcomment, xdur);
      
      if(!ret)
	wlog->warn("ComponentResults::result failed to allocate CheckResult");

      xdelete(xcomment);
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "ComponentResults::result = %d", ret);
#endif

  return(ret);
}

ComponentResults::~ComponentResults()
{
  // Deallocate the ComponentResults object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "ComponentResults::~ComponentResults()");
#endif

  // The Arrays within store pointers to CheckResults also stored within
  // ocrs and rcrs, so don't delete those twice.

  xhdelete(crbysvc, Array<ComponentCheckResult>);

  xhdelete(ocrs, ComponentCheckResult);
  xhdelete(rcrs, ComponentCheckResult);

  xdelete(onames);
  xdelete(rnames);

  h = 0;
  opt = 0;
  req = 0;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "ComponentResults::~ComponentResults()");
#endif
}
