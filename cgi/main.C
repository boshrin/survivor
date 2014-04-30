/*
 * survivor web interface
 *
 * Version: $Revision: 0.35 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2007/01/25 13:05:31 $
 * MT-Level: Safe
 *
 * Copyright (c) 2002 - 2007
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: main.C,v $
 * Revision 0.35  2007/01/25 13:05:31  benno
 * Build correctly with debugging disabled
 *
 * Revision 0.34  2006/11/24 04:31:21  benno
 * Specify to not encode ampersands
 *
 * Revision 0.33  2005/12/22 03:58:56  benno
 * process_status looks for stalled checks
 *
 * Revision 0.32  2005/11/28 00:33:47  benno
 * Don't run process_action if action is "none" (bug #263)
 *
 * Revision 0.31  2005/11/28 00:28:44  benno
 * Change Session() arg order for AIX portability
 *
 * Revision 0.30  2005/11/25 04:09:59  benno
 * Continuation of previous change
 *
 * Revision 0.29  2005/11/25 03:53:34  benno
 * Handle error tag when sending refresh to location specified in "ret"
 *
 * Revision 0.28  2005/11/24 01:55:22  benno
 * ui_execute_report takes FileHandler
 *
 * Revision 0.27  2005/06/09 02:52:06  benno
 * Add support for processing actions on multiple hosts, services,
 * or service@host pairs
 *
 * Revision 0.26  2004/11/26 22:10:41  benno
 * Use ui_calllist_notifies and ui_person_notifies
 *
 * Revision 0.25  2004/09/09 16:17:16  benno
 * Fix bug preventing uninhibit from working
 *
 * Revision 0.24  2004/09/09 12:45:55  benno
 * Better error generation for bad report module parameters
 *
 * Revision 0.23  2004/08/24 23:24:37  benno
 * Add support for report modules, including tmpfile processing
 *
 * Revision 0.22  2004/05/09 02:18:48  benno
 * Recreate Configuration on parse failure for better error reporting
 *
 * Revision 0.21  2004/04/26 17:26:18  benno
 * Add (back) process_status
 *
 * Revision 0.20  2004/04/06 21:42:58  benno
 * Yet another typo, this one preventing processing actions
 *
 * Revision 0.19  2004/04/03 19:54:42  benno
 * Allow multiple recipients when sending clipboards
 *
 * Revision 0.18  2004/04/03 02:22:52  benno
 * Toss autherr when a module down the stack succeeds
 *
 * Revision 0.17  2004/04/02 23:01:06  benno
 * Fix typos, including one preventing Clipboard postprocessing redirects
 *
 * Revision 0.16  2004/04/02 22:53:26  benno
 * Fix bug in processing multiple auth modules
 *
 * Revision 0.15  2004/03/02 17:35:49  benno
 * Overhaul for 0.9.4
 *
 * Revision 0.14  2003/10/06 23:33:33  benno
 * Add support for status tag
 *
 * Revision 0.13  2003/04/09 20:14:29  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.12  2003/03/02 05:09:01  benno
 * Use config_error
 *
 * Revision 0.11  2003/01/27 03:06:50  benno
 * Use xdelete
 *
 * Revision 0.10  2002/12/16 01:10:19  benno
 * Add comments for acks/inhibits
 *
 * Revision 0.9  2002/09/09 02:27:53  benno
 * establish auth object regardless of 'l' value
 *
 * Revision 0.8  2002/07/05 14:18:41  benno
 * fix typo
 *
 * Revision 0.7  2002/05/13 18:08:24  benno
 * adjust order of instance checking to work correctly with single instances
 *
 * Revision 0.6  2002/04/26 20:06:51  toor
 * call args->parse_instcf to get better error information
 *
 * Revision 0.5  2002/04/04 19:54:19  benno
 * copyright
 *
 * Revision 0.4  2002/04/02 20:08:42  benno
 * rcsify date
 *
 * Revision 0.3  2002/04/02 20:07:05  benno
 * Add auth support
 * Remove requisite global defines
 *
 * Revision 0.2  2002/04/02 20:06:19  benno
 * Add "error" view
 *
 * Revision 0.1  2002/04/02 20:05:07  benno
 * initial revision
 *
 */

#include "cgi.H"

/*
 * Global Definitions
 *
 */

CGIConfiguration *cgicf = NULL;    // cgicf can be global
CGIParser *parser = NULL;          // And CGIParser is similar
Cookies *cookies = NULL;           // Ditto
Session *session = NULL;           // Current session info

/*
 * main()
 *
 */

int main(int argc, char **argv)
{
  // Global setup.  We want errors to go to stdout, so we establish our
  // own Debuggers.  libsrvexit will clean these up.

  StdDebugger *d = new StdDebugger();
  StdDebugger *w = new StdDebugger();

  if(d)
    d->enable_stdout();

  if(w)
    w->enable_stdout();
  
  if(!libsrvinit(argv[0], d, w))
    exit(1);

  // Allocate an Args object, which will be deleted by libsrvexit.
  // We don't parse the Args, we just want access to default info.

  args = new Args();

  if(args)
  {
    // We have to parse cgi.cf before we use the PageFilter since
    // the PageFilter relies on cgi.cf's definition of pageset
    // source location.
    
    cgicf = new CGIConfiguration();

    if(cgicf && cgicf->parse_cf())
    {
      CharBuffer *psdir = new CharBuffer(cgicf->pageset_directory());
      parser = new CGIParser();
      cookies = new Cookies();
      SessionState *st = new SessionState();

      if(psdir && parser && cookies && cookies->parse() && st)
      {
	int pret = parser->parse();
	bool cfsok = true;

	// Retrieve any existing session information

	Cookie *sid = cookies->cookie("sid");
	      
	if(sid)
	  session = st->find(sid->value());

	if(pret == CGIERR_OK)
	{
	  CGIValue *action = parser->value("a");
	  CGIValue *clipaction = parser->value("ca");
	  CGIValue *err = parser->value("error");
	  CGIValue *srcfile = parser->value("file");
	  CGIValue *instance = parser->value("i");
	  CGIValue *login = parser->value("l");
	  CGIValue *pageset = parser->value("pageset");
	  CGIValue *repmod = parser->value("rm");
	  CGIValue *status = parser->value("status");
	  CGIValue *tfile = parser->value("tfile");

	  if(cgivalue(status))
	  {
	    // This trumps all the other initializations so that the
	    // status mechanism can better trap the errors.

	    process_status(cgivalue(status), cgivalue(instance));
	  }
	  else
	  {	    
	    // Finish building pageset path
	    psdir->append("/");

	    if(cgivalue(pageset) && pageset->validate(CGIVALID))
	      psdir->append(cgivalue(pageset));
	    else
	      psdir->append(cgicf->pageset());
	    
	    // Parse args and instcf here, since if only one
	    // instance is defined parse_instcf will use it.
	    // It should be ok to skip the error check on
	    // parse_args, since a failure should result in
	    // no instances found (caught below).
	    
	    if(cgivalue(instance))
	      args->parse_args(instance->value(), (char *)NULL,
			       (char *)NULL);
	    else
	      args->parse_args((char *)NULL, (char *)NULL,
			       (char *)NULL);
	    
	    if(!instance && args->instances()
	       && args->instances()->entries() == 1)
	    {
	      // Insert the discovered instance into the CGI parser
	      
	      if(parser->addvalue("i", args->instname(), false))
		instance = parser->value("i");
	    }
	    
	    if(cgivalue(instance))
	    {
	      // Parse the cfs
	      
	      if(args->parse_instcf())
	      {
		cf = new Configuration();
		
		if(cf)
		{
		  if(!cf->parse_cfs())
		  {
		    SIMPLEERR("Configuration file parse failed");

#if defined(DEBUG)
		    // For debugging output to render, then reparse to
		    // trip error.  Note we have to toss cf since it
		    // might be inconsistent.
		    
		    HTMLOUT << "<PRE>" << endl;
		    dlog->set_level(DEBUG_CFERRS);
#else
		    SIMPLEERR("Debugging not compiled in");
#endif
		    
		    xdelete(cf);
		    cf = new Configuration();

		    if(cf)		    
		      cf->parse_cfs();
		    else
		      wlog->warn("Failed to reallocate Configuration");
		    
		    HTMLOUT << "</PRE>" << endl;
		    
		    cfsok = false;
		  }
		}
		else
		  wlog->warn("Failed to allocate Configuration");
	      }
	      else
	      {
		SIMPLEERR("Failed to parse instance.cf");
		
		// For debugging output to render, then reparse to
		// trip error
		
		HTMLOUT << "<PRE>" << endl;
		HTMLOUT << cgivalue(instance) << " is not a valid instance."
			<< endl;
#if defined(DEBUG)
		dlog->set_level(DEBUG_CFERRS);
		args->parse_instcf();
#else
		    SIMPLEERR("Debugging not compiled in");
#endif
		HTMLOUT << "</PRE>" << endl;
		
		cfsok = false;
	      }
	    }
	    // else args has the list of available instances
	  
	    if(cfsok)
	    {
	      // Before we do any pagefiltering, see if auth is requested
	      
	      if(cgivalue(login))
		process_auth(strcmp(cgivalue(login), "1")==0 ? true : false,
			     st);
	      else
	      {
		// Retrieve existing session information, if any
		
		Cookie *sid = cookies->cookie("sid");
		
		if(sid)
		  session = st->find(sid->value());
		
		if(cf && cgivalue(action)
		   && strcasecmp(cgivalue(action), "none")!=0)
		  process_action(cgivalue(action),
				 parser->value("h"),
				 parser->value("s"),
				 parser->value("hc"),
				 parser->value("sh"),
				 cgivalue(parser->value("cmt")));
		else if(cgivalue(clipaction))
		  process_clipboard(cgivalue(clipaction),
				    cgivalue(parser->value("cn")),
				    cgivalue(parser->value("cc")),
				    parser->value("ctl"),
				    parser->value("cti"),
				    cgivalue(parser->value("ce")),
				    cgivalue(parser->value("cp")));
		else if(cgivalue(repmod))
		  process_report_module(cgivalue(repmod),
					parser->value("h"),
					parser->value("hc"),
					parser->value("s"),
					cgivalueint(parser->value("tmf")),
					cgivalueint(parser->value("tdf")),
					cgivalueint(parser->value("tyf")),
					cgivalueint(parser->value("thf")),
					cgivalueint(parser->value("tnf")),
					cgivalueint(parser->value("tmu")),
					cgivalueint(parser->value("tdu")),
					cgivalueint(parser->value("tyu")),
					cgivalueint(parser->value("thu")),
					cgivalueint(parser->value("tnu")),
					parser->value("which"),
					cgivalue(parser->value("reverse")),
					cgivalue(parser->value("style")));
		else if(cgivalue(tfile))
		  process_tmpfile(cgivalue(tfile));
		else
		  process_page(psdir->str(), cgivalue(srcfile));
	      }
	    }
	    // Error has already been reported for parse failure
	  }
	}
	else
	  SIMPLEERR(_cgi_parse_err[pret]);
      }
      else
	SIMPLEERR("Failed to allocate CharBuffer, CGIParser, SessionState, and Cookies");

      xdelete(psdir);
      xdelete(parser);
      xdelete(cookies);
      xdelete(session);
      xdelete(st);
    }
    else
    {
      if(cgicf)
      {
	SIMPLEERR("Failed to parse cgi.cf");

	// For debugging output to render, then reparse to trip error

#if defined(DEBUG)
	HTMLOUT << "<PRE>" << endl;
	dlog->set_level(DEBUG_CFERRS);
	cgicf->parse_cf();
	HTMLOUT << "</PRE>" << endl;
#else
	SIMPLEERR("Debugging not compiled in");
#endif
      }
      else
	SIMPLEERR("Failed to allocate CGIConfig");
    }

    xdelete(cgicf);
  }
  else
    SIMPLEERR("Failed to allocate Args");
  
  // Clean up

  libsrvexit();
  
  exit(0);
}

void process_action(char *action, CGIValue *hosts, CGIValue *services,
		    CGIValue *hostclasses, CGIValue *shpairs, char *comment)
{
  // Process the requested <action> for the specified <services>, <hosts>,
  // <hostclasses>, and/or <shpairs> using <comment>.

  // Returns: Nothing.

  // Perform the requested action (if permitted).

  CharBuffer *acterr = new CharBuffer();
		
  if(acterr && action)
  {
    if(session &&
       (cgicf->authorized(session->user(), session->groups())
	>= read_write_authz))
    {
      // This is based on cli/Functionality.C, and along with gateway/main.C
      // ought to be consolidated into a libuif (UI Functionality) at some
      // point.

      if(hosts)
      {
	// For each host, retrieve each configured service

	for(int i = 0;i < hosts->values();i++)
	{
	  List *g = cf->find_groups(hosts->value(i));

	  if(g)
	  {
	    for(int j = 0;j < g->entries();j++)
	      process_action_dispatch(action, hosts->value(i),
				      g->retrieve(j), comment, acterr);
	  }
	  else
	  {
	    acterr->append(_t("No groups found for "));
	    acterr->append(hosts->value(i));
	    acterr->append(".");
	  }
	}
      }

      if(services)
      {
	// For each service, retrieve each configured host

	for(int i = 0;i < services->values();i++)
	{
	  List *g = cf->find_group(services->value(i));

	  if(g)
	  {
	    for(int j = 0;j < g->entries();j++)
	      process_action_dispatch(action, g->retrieve(j),
				      services->value(i), comment, acterr);
	  }
	  else
	  {
	    acterr->append(_t("No hosts found for "));
	    acterr->append(services->value(i));
	    acterr->append(".");
	  }
	}
      }

      if(hostclasses)
      {
	// For each hostclass, retrieve each configured host

	for(int i = 0;i < hostclasses->values();i++)
	{
	  HostClass *hc = cf->find_hostclass(hostclasses->value(i));

	  if(hc && hc->hosts() && hc->hosts()->entries() > 0)
	  {
	    // For each host, retrieve each configured service

	    for(int k = 0;k < hc->hosts()->entries();k++)
	    {
	      List *g = cf->find_groups(hc->hosts()->retrieve(k));

	      if(g)
	      {
		for(int j = 0;j < g->entries();j++)
		  process_action_dispatch(action, hc->hosts()->retrieve(k),
					  g->retrieve(j), comment, acterr);
	      }
	      else
	      {
		acterr->append(_t("No groups found for "));
		acterr->append(hc->hosts()->retrieve(k));
		acterr->append(".");
	      }
	    }
	  }
	  else
	  {
	    acterr->append(_t("No hosts found for class "));
	    acterr->append(hostclasses->value(i));
	    acterr->append(".");
	  }
	}
      }
      
      if(shpairs)
      {
	// Tokenize each pair

	for(int i = 0;i < shpairs->values();i++)
	{
	  SHPair *shp = new SHPair(shpairs->value(i));

	  if(shp && shp->host() && shp->service())
	  {
	    process_action_dispatch(action, shp->host(), shp->service(),
				    comment, acterr);

	    xdelete(shp);
	  }
	}
      }
      
    }
    else
      acterr->append(_t("Permission denied"));
  }
  
  // Generate a redirect minus the action, including any error, if
  // relevant

  HTMLOUT << CGIHEADER << endl;
  HTMLOUT << "<HTML><HEAD><META HTTP-EQUIV=\"refresh\" CONTENT=\"1;URL=";
  
  char *retv = cgivalue(parser->value("ret"));

  if(retv)
  {
    if(acterr && acterr->str() && strlen(acterr->str()) > 0)
    {
      CGIParser *xp = new CGIParser();
      
      if(xp)
      {
	char *x = strchr(retv, '?');

	if(x)
	  x++;  // Move past the ?
	else
	  x = retv;

	if(xp->parse(x)==0)
	  HTMLOUT << xp->url(false, "error", acterr->str());
	// else error
	
	xdelete(xp);
      }
    }
    else
      HTMLOUT << retv;
  }
  else
  {
    parser->urlskip("a");
    parser->urlskip("cmt");
    
    if(acterr && acterr->str() && strlen(acterr->str()) > 0)
      HTMLOUT << parser->url(false, "error", acterr->str());
    else
      HTMLOUT << parser->url(false);
  }
  
  // We use refresh rather than location: to avoid the refresh tag
  // continually reloading the action page.
	      
  HTMLOUT << "\"></HEAD><BODY BGCOLOR=gray></BODY></HTML>" << endl;

  xdelete(acterr);
}

void process_action_dispatch(char *action, char *host, char *service,
			     char *comment, CharBuffer *errs)
{
  // Dispatch the actual <action> to be processed over <service>@<host>,
  // with <comment>.  Append any errors to <errs>.

  // Returns: Nothing.

  char *acterr = NULL;  
  
  if(action && host && service && errs)
  {
    Check *c = cf->find_check(service);

    if(c)
    {
      AlertState *as = new AlertState(c, host);
      CheckState *cs = new CheckState(c, host);
      FixState *fs = new FixState(c, host);
	
      if(as && cs && fs)
      {
	switch(tolower(action[0]))
	{
	case 'a':
	  // Acknowledge
	  // ui_acknowledge will create new XState objects, but better
	  // that than code duplication.

	  ui_acknowledge(c, host, session->user(), MAYBE_EXCUSE(comment),
			 &acterr);
	  break;
	case 'e':
	  // Escalate
	  // ui_escalate_to will create new XState objects, but better
	  // that than code duplication.
	  
	  ui_escalate_to(c, host, session->user(), MAYBE_EXCUSE(comment),
			 &acterr);
	  break;
	case 'f':
	  // Fix
	  if(c && c->fix())
	    ui_execute_fix(c, host, cs, fs, session->user(),
			   MAYBE_EXCUSE(comment), &acterr);
	  else
	    acterr = xstrdup(_t("No fix defined for this service"));
	  break;
	case 'i':
	  // Inhibit
	  // ui_inhibit will create new XState objects, but better
	  // that than code duplication.

	  ui_inhibit(c, host, session->user(), MAYBE_EXCUSE(comment),
		     &acterr);
	  break;
	case 'r':
	  // Reschedule
	  if(!cs->reschedule())
	    acterr = xstrdup(_t("Reschedule failed"));
	  break;
	case 'u':
	  if(strlen(action) > 2)
	  {
	    // Unacknowledge *or* Uninhibit
	    ui_unquiet_resched(c, host, (action[2] == 'i' ? true : false),
			       session->user(), MAYBE_EXCUSE(comment),
			       &acterr);
	  }
	  else
	    acterr = xstrdup(_t("Unknown action to perform"));
	  break;
	default:
	  acterr = xstrdup(_t("Unknown action to perform"));
	  break;
	}
      }
      else
	acterr = xstrdup(_t("Failed to allocate State objects"));
      
      xdelete(as);
      xdelete(cs);
      xdelete(fs);
    }
    else
      acterr = xstrdup(_t("Unknown service"));
    
    if(acterr)
    {
      errs->append(service);
      errs->append("@");
      errs->append(host);
      errs->append(":");
      errs->append(acterr);
      errs->append(".");
      
      xdelete(acterr);
    }
  }
}

void process_auth(bool login, SessionState *st)
{
  // Process an authentication request.  If <login> is true, perform a
  // login operation, otherwise perform a logout.

  // Returns: Nothing.

  // If login operations are processed, no page filtering
  // is done.  Instead, a redirect will be issued.
	      
  authn_result_t authn = no_authn;
  char *autherr = NULL;

  if(!session && login)
  {
    // Login requested, ignore if we are already in a session
    
    Array<CGIAuthModule> *ams = cgicf->authmodules();
		
    if(ams)
    {
      // Loop through until we find one that authenticates
      // somebody
      
      for(int i = 0;i < ams->entries();i++)
      {
	CGIAuthModule *cam = ams->retrieve(i);
		    
	if(cam)
	{
	  // We only provide what REMOTE_ADDR tells us.
	  // It is up to the auth modules to do further
	  // verification.
	  
	  CGIAuthRequest *car = new CGIAuthRequest(cam, parser->urlfull(false),
						   getenv("REMOTE_ADDR"));
		      
	  Executor *e = new Executor();
		      
	  if(car && e)
	  {
	    if(e->exec_webauth(car) > -1)
	    {
	      CGIAuthResult *car = NULL;
			  
	      int r = e->result(&car);
			  
	      if(car)
	      {
		if(r == MODEXEC_OK)
		{
		  authn = car->authok();
		  
		  switch(authn)
		  {
		  case yes_authn:
		    {
		      // Create a new session
		      
		      session = new Session(prng->s20char(),
					    cgicf->session_length(),
					    car->username(),
					    car->groups(),
					    getenv("REMOTE_ADDR"));
		      
		      if(session)
		      {
			// If the session is successfully added,
			// set the session cookie
			
			if(st->add(session))
			{
			  Cookie *ck = new Cookie("sid", session->sid(), "/");
			  
			  if(ck)
			  {
			    if(cookies->add(ck))
			      cookies->dump(HTMLOUT_FILENO);
			    else
			      xdelete(ck);
			  }
			  else
			    wlog->warn("Failed to allocate Cookie");
			}
			else
			  wlog->warn("Failed to insert session %s",
				     session->sid());
		      }
		      else
			wlog->warn("Failed to allocate Session");
		    }

		    // autherr may have been set by a previous module
		    xdelete(autherr);
		    break;
		  case deferred_authn:
		    {
		      // Dump any requested data

		      HTMLOUT << CGIHEADER << endl;

		      if(car->deferral())
			HTMLOUT << car->deferral() << endl;
		      else
			wlog->warn("Received deferral from module, but no document");
		    }
		    break;
		  default:
		    autherr = xstrdup(car->error());
		    break;
		  }

		  // Add any requested skip flags to the parser

		  List *toskip = car->skipflags();

		  if(toskip)
		  {
		    for(int i = 0;i < toskip->entries();i++)
		      parser->urlskip(toskip->retrieve(i));
		  }
		}
		else
		{
		  // We may have received an error, especially
		  // if MODEXEC_MISCONFIG
			      
		  autherr = xstrdup(car->error());
		}
			    
		xdelete(car);
	      }
	      else
		wlog->warn("CGIAuthResult missing for %s", cam->module());
	    }
	    else
	      wlog->warn("Failed to exec %s", cam->module());
	  }
	  else
	    wlog->warn("Failed to allocate CGIAuthRequest or Executor");
	  
	  xdelete(car);
	  xdelete(e);
	}

	// Break on auth success or deferred or if a
	// specific error is encountered
	
	if(authn != no_authn)
	  break;
      }
    }
    else
      wlog->warn("No authmodules specified");
    
    if(authn == no_authn && !autherr)
      autherr = xstrdup(_t("Permission denied"));
  }
  else if(!login)
  {
    // Logout requested, expire any existing session
    
    Cookie *sid = cookies->cookie("sid");
		
    if(sid)
      st->remove(sid->value());
		
    // XXX should probably call auth modules to offer logout
    // opportunity
  }

  if(authn != deferred_authn)
  {
    // Generate a redirect, including an error tag (if any), but not
    // including the login tag.  If deferred, don't do anything.

    // Dump a page asking for immediate refresh.  This is a hack
    // because of browsers not sending the cookie back when a
    // location: following set-cookie: points back to the originally
    // requested location.  The cookie is only set the next time the
    // page loads.  This also prevents the refresh tag from continually
    // trying to login.

    char *xd = NULL;
	      
    parser->urlskip("l");
	      
    if(autherr)
      xd = parser->url(false, "error", autherr);
    else
      xd = parser->url(false);
    
    HTMLOUT << CGIHEADER << endl;
    HTMLOUT << "<HTML><HEAD><META HTTP-EQUIV=\"refresh\" CONTENT=\"1;URL="
	    << xd << "\"></HEAD><BODY BGCOLOR=gray></BODY></HTML>" << endl;
  }
  
  xdelete(autherr);
}

void process_clipboard(char *action, char *clipboard, char *contents,
		       CGIValue *rcptlist, CGIValue *rcptaddr, char *email,
		       char *phone)
{
  // Process the requested <action> for <clipboard>, with optional
  // <contents>.  <rcptlist>, <rcptaddr>, <email>, and <phone> are all
  // optional, for use when sending.

  // Returns: Nothing.

  // Perform the requested action (if permitted).

  char *acterr = NULL;
  char *actinfo = NULL;
		
  if(action && clipboard)
  {
    authz_level_t minauthz = clipboard_authz;

    if(session)
    {
      ClipboardManager *cbm = new ClipboardManager();
      
      if(cbm)
      {
	if((strncasecmp(action, "ad", 2)==0 || tolower(action[0]) == 'd')
	   &&
	   cgicf->authorized(session->user(), session->groups())
	   >= admin_authz)
	{
	  switch(tolower(action[0]))
	  {
	  case 'a':
	    if(cbm->create_board(clipboard))
	      actinfo = xstrdup(_t("Clipboard created"));
	    else
	      acterr = xstrdup(_t("Unable to create Clipboard"));
	    break;
	  case 'd':
	    if(cbm->delete_board(clipboard))
	      actinfo = xstrdup(_t("Clipboard deleted"));
	    else
	      acterr = xstrdup(_t("Unable to delete Clipboard"));
	    break;
	  default:
	    acterr = xstrdup(_t("Unknown command"));
	    break;
	  }

	  // Skip cn when reloading after these operations

	  parser->urlskip("cn");
	}
	else if(cgicf->authorized(session->user(), session->groups())
		>= clipboard_authz)
	{
	  Clipboard *cb = cbm->find_board(clipboard);
	  
	  if(cb)
	  {
	    switch(tolower(action[0]))
	    {
	    case 'a':
	      // Append, make sure we aren't accidentally processing add
	      if(strncasecmp(action, "ap", 2)==0 && contents)
	      {
		if(cb->append(contents))
		  actinfo = xstrdup(_t("Clipboard updated"));
		else
		  acterr = xstrdup(_t("Failed to append to clipboard"));
	      }
	      break;
	    case 'c':
	      // Clear the clipboard
	      if(!cb->write(""))
		acterr = xstrdup(_t("Failed to clear clipboard contents"));
	      break;
	    case 's':
	      // Send or Save.  We save either way.
	      if(contents)
	      {
		if(cb->write(contents))
		{
		  if(strlen(action) > 1 && tolower(action[1]) == 'e')
		  {
		    // Send the clipboard
		    
		    if(rcptlist || rcptaddr)
		    {
		      // Send the contents out.  Accumulate the addresses
		      // into a RecipientSet, which we'll then use to
		      // send out the clipboard.

		      RecipientSet *rset = new RecipientSet();

		      if(rset)
		      {
			if(rcptaddr)
			{
			  // These are individual addresses, so all we
			  // do is tokenize them

			  for(int i = 0;i < rcptaddr->values();i++)
			  {
			    List *itokens = tokenize(rcptaddr->value(i), ":");

			    if(itokens)
			    {
			      if(itokens && itokens->entries()==4)
			      {
				// Format is instance:calllist:addr:module
				
				rset->add(itokens->retrieve(2),
					  itokens->retrieve(1),
					  itokens->retrieve(3));
			      }
			      
			      xdelete(itokens);
			    }
			  }
			}

			if(rcptlist)
			{
			  // These are calllists, so we have to determine
			  // who to notify

			  for(int i = 0;i < rcptlist->values();i++)
			  {
			    List *ltokens = tokenize(rcptlist->value(i), ":");

			    if(ltokens && ltokens->entries()==3)
			    {
			      // Format is instance:calllist:module

			      char *uierr = NULL;
			      char *uivia = NULL;  // This should be module

			      // Get the list of addresses
			      
			      List *addrs =
				ui_calllist_notifies(ltokens->retrieve(0),
						     ltokens->retrieve(1),
						     &uivia,
						     &uierr);

			      if(addrs)
			      {
				// Add each address into the recipientset
				
				for(int i = 0;i < addrs->entries();i++)
				  rset->add(addrs->retrieve(i),
					    ltokens->retrieve(1),
					    uivia);
				
				xdelete(addrs);
			      }
			      else
				wlog->warn("ui_calllist_notifies failed in process_clipboard: %s", IONULL(uierr));

			      xdelete(uierr);
			      xdelete(uivia);
			    }
			  }
			}

			// Now that we've assembled all the addresses,
			// send out the clipboard once per module

			for(int i = 0;i < rset->modules();i++)
			{
			  if(cb->send(rset->addresses(i), session->user(),
				      email, phone, rset->module(i)))
			  {
			    actinfo = xstrcat(actinfo,
					      _t("Clipboard successfully sent to "));
			    actinfo = xstrcat(actinfo, rset->addresses(i));
			    actinfo = xstrcat(actinfo, ",");
			  }
			  else
			  {
			    acterr = xstrcat(acterr,
					     _t("Failed to send Clipboard to "));
			    acterr = xstrcat(acterr, rset->addresses(i));
			    acterr = xstrcat(acterr, ",");
			  }			  
			}

			// Toss the trailing commas
			xstrchop(actinfo);
			xstrchop(acterr);
			  
			xdelete(rset);
		      }
		      else
			wlog->warn("Failed to allocate RecipientSet");
		    }
		    else
		      acterr = xstrdup(_t("No destination addresses specified"));
		  }
		  else
		    actinfo = xstrdup(_t("Clipboard saved"));
		}
		else
		  acterr = xstrdup(_t("Failed to save clipboard contents"));
	      }
	      else
		acterr = xstrdup(_t("No contents provided for clipboard"));
	      break;
	    default:
	      // If we get here, it's probably because someone tried to
	      // add or delete without admin privs
	      acterr = xstrdup(_t("Permission denied"));	
	      break;
	    }
	    
	    xdelete(cb);
	  }
	  else
	    acterr = xstrdup(_t("Requested clipboard does not exist"));
	}
	else
	  acterr = xstrdup(_t("Permission denied"));	

	xdelete(cbm);
      }
      else
	wlog->warn("process_clipboard failed to allocate ClipboardManager");
    }
    else
      acterr = xstrdup(_t("Permission denied"));
  }
  
  // Generate a redirect minus the action, including any error, if
  // relevant

  // XXX We could use a "ret" tag here, the same way process_action does
  
  char *xd = NULL;
	      
  parser->urlskip("ca");
  parser->urlskip("cc");
  parser->urlskip("ctl");
  parser->urlskip("cti");
  parser->urlskip("ce");
  parser->urlskip("cp");
	      
  if(acterr)
    xd = parser->url(false, "error", acterr);
  else if(actinfo)
    xd = parser->url(false, "info", actinfo);
  else
    xd = parser->url(false);

  // We use refresh rather than location: to avoid the refresh tag
  // continually reloading the action page.
	      
  HTMLOUT << CGIHEADER << endl;
  HTMLOUT << "<HTML><HEAD><META HTTP-EQUIV=\"refresh\" CONTENT=\"1;URL="
  	  << xd << "\"></HEAD><BODY BGCOLOR=gray></BODY></HTML>" << endl;

  xdelete(acterr);
  xdelete(actinfo);
}

void process_page(char *pagedir, char *srcfile)
{
  // Process the requested page <srcfile> from the pageset directory
  // <pagedir>.  If <srcfile> is not provided, index.html.in will be
  // used.
  
  // Returns: Nothing.

  // Before we fire up the PageFilter, we need to determine which
  // PageSet to use.

  if(pagedir)
  {
    // The PageFilter does the real work
    
    Hashtable<Variable> *vh = new Hashtable<Variable>();
    PageFilter *pf = new PageFilter(vh, pagedir);
    
    if(pf && vh)
    {
      // Multiple PageFilters can be instantiated, so we send the
      // initial content-type here
      
      HTMLOUT << CGIHEADER << endl;
    
      // Always append .in here since @{INCLUDE} accepts
      // full paths
      
      if(srcfile && !strchr(srcfile, '/'))
	pf->filter(srcfile, true);
      else
	pf->filter("index.html", true);
    }
    
    xdelete(pf);
    xhdelete(vh, Variable);
  }
}

void process_report_module(char *module, CGIValue *hosts,
			   CGIValue *hostclasses, CGIValue *services,
			   int fmon, int fday, int fyear, int fhour, int fmin,
			   int umon, int uday, int uyear, int uhour, int umin,
			   CGIValue *which, char *reverse, char *style)
{
  // Process, if authorized, the requested report <module> for <hosts>
  // and all hosts in <hostclasses> that are members of <services>,
  // for the time period <fmin>:<fhour> <fmon><fday><fyear> until
  // <umin>:<uhour> <umon><uday><uyear>, for <which> data and
  // outputting in <style>.  If <reverse> is true, data is processed
  // from the most recent first.

  // Returns: Nothing.

  char *acterr = NULL;

  if(module && !strchr(module, '/') && (hosts || hostclasses) && services
     && (fmon > 0) && (fmon < 13) && (fday > 0) && (fday < 32) && (fyear > 0)
     && (fhour > -1) && (fhour < 24) && (fmin > -1) && (fmin < 60)
     && (umon > 0) && (umon < 13) && (uday > 0) && (uday < 32) && (uyear > 0)
     && (uhour > -1) && (uhour < 24) && (umin > -1) && (umin < 60)
     && which && style)
  {
    if(session &&
       cgicf->authorized(session->user(), session->groups()) >= exec_authz)
    {
      // <reverse> may be NULL

      ReportFormatting *rf = new ReportFormatting(module, style,
						  args->tmpdir(),
						  parser->urlpath("tfile", "",
								  NULL));
      List *h = new List();
      List *s = new List();
      char ftime[13];
      char utime[13];

      if(rf && h && s)
      {
	int rtype = 0;

	// Assemble report types
	
	for(int i = 0;i < which->values();i++)
	{
	  if(which->value_length(i) == 5
	     && strcmp(which->value(i), "alert")==0)
	    rtype |= UI_REPORT_ALERT;
	  else if(which->value_length(i) == 5
	     && strcmp(which->value(i), "check")==0)
	    rtype |= UI_REPORT_CHECK;
	  else if(which->value_length(i) == 7
	     && strcmp(which->value(i), "command")==0)
	    rtype |= UI_REPORT_COMMAND;
	  else if(which->value_length(i) == 3
	     && strcmp(which->value(i), "fix")==0)
	    rtype |= UI_REPORT_FIX;
	}

	// Assemble hosts
	
	if(hosts)
	{
	  for(int i = 0;i < hosts->values();i++)
	  {
	    if(h->find(hosts->value(i))==-1)
	      h->add(hosts->value(i));
	  }
	}

	if(hostclasses)
	{
	  for(int i = 0;i < hostclasses->values();i++)
	  {
	    HostClass *hc = cf->find_hostclass(hostclasses->value(i));

	    // Add all members of the class
	    
	    if(hc && hc->hosts())
	    {
	      for(int j = 0;j < hc->hosts()->entries();j++)
	      {
		if(h->find(hc->hosts()->retrieve(j))==-1)
		  h->add(hc->hosts()->retrieve(j));
	      }
	    }
	  }
	}

	// Sort the hosts

	h->sort(ascending_caseless_sort);

	// Assemble services and sort

	for(int i = 0;i < services->values();i++)
	{
	  if(s->find(services->value(i))==-1)
	    s->add(services->value(i));
	}

	s->sort(ascending_caseless_sort);

	// Assemble from and until times

	sprintf(ftime, "%2.2d%2.2d%2.2d%2.2d%4.4d",
		fhour, fmin, fmon, fday, fyear);

	sprintf(utime, "%2.2d%2.2d%2.2d%2.2d%4.4d",
		uhour, umin, umon, uday, uyear);

	// Execute the report, dump straight to htmlout

	HTMLOUT << CGIHEADER << endl;

	FileHandler *fout = new FileHandler(STDOUT_FILENO);

	if(fout)
	{
	  ui_execute_report(fout, rf, h, s, rtype,
			    convtime(ftime), convtime(utime),
			    (reverse && strcmp(reverse, "on")==0),
			    &acterr);

	  xdelete(fout);
	}
	else
	  acterr = xstrdup("Failed to allocate FileHandler");
      }
      else
	acterr = xstrdup("Failed to allocate data structures");

      xdelete(rf);
      xdelete(h);
      xdelete(s);
    }
    else
      acterr = xstrdup(_t("Permission denied"));
  }
  else
    acterr = xstrdup(_t("Incorrect parameters for report request"));

  if(acterr)
  {
    // Go back a page via refresh and append an error

    char *back = NULL;

    if(parser->value("back"))
    {
      // Find the query string within back
      
      back = cgivalue(parser->value("back"));

      if(back)
	back = strchr(back, '?') + 1;
    }

    CGIParser *backp = new CGIParser();

    if(back && backp)
    {
      int x = backp->parse(back);
      
      if(x==0)
      {
	char *xd = backp->url(false, "error", acterr);
    
	// We use refresh rather than location: to avoid the refresh tag
	// continually reloading the action page.
	
	HTMLOUT << CGIHEADER << endl;
	HTMLOUT << "<HTML><HEAD><META HTTP-EQUIV=\"refresh\" CONTENT=\"1;URL="
		<< IONULL(xd) << "\"></HEAD><BODY BGCOLOR=gray></BODY></HTML>"
		<< endl;
      }
      else
	wlog->warn("process_report_module failed to parse back %d", x);
    }
    else
      wlog->warn("process_report_module failed to find back or allocate backp");
      
    xdelete(backp);
    xdelete(acterr);
  }
  // else the module provided the output
}

void process_status(char *request, char *instance)
{
  // Process the status <request> for the provided <instance>,
  // generating a SurvivorStatus document on HTMLOUT.

  // Returns: Nothing.

  // We currently ignore <request> since it can only be "all".

  SurvivorStatus *st = new SurvivorStatus();

  if(st)
  {
    SurvivorXML *sxml = new SurvivorXML();

    if(sxml)
    {
      // Obtain the information we need and store it.  But first, we
      // must have an instance to check.
        
      args->parse_args(instance, (char *)NULL, (char *)NULL);
      
      if(!args->parse_instcf())
      {
        st->add_parse_error("Instance configuration file parse failed.\n");
        
        if(!instance)
          st->add_parse_error("No instance specified.\n");
      }
      else
      {
	// Start with CGI configuration file errors.
	// XXX This has the same debugging output problem as xcf, below.

	CGIConfiguration *cgicf = new CGIConfiguration();
	
	if(cgicf)
	{
	  if(!cgicf->parse_cf())
	    st->add_parse_error(_t("Configuration file parse failed, use sc or sw for details.\n"));
	  
	  xdelete(cgicf);
	}
	else
	  wlog->warn("process_status failed to allocate CGIConfig");
	
	// Next check for main configuration file errors.
	
	Configuration *xcf = new Configuration();
	
	if(xcf)
	{
	  // XXX Two problems here.  First, cf->parse_cfs use Debugger
	  // routines for reporting errors, so we'd have to create a new
	  // Debugger type to intercept them.  Second, if not built with
	  // -DDEBUG, debugging won't be available at all.  (The latter
	  // is probably not that big a deal.)
	  
	  if(!xcf->parse_cfs())
	    st->add_parse_error(_t("Configuration file parse failed, use sc or sw for details.\n"));
	  else
	  {
	    // While we're here, look for stalled checks.  cf shouldn't be
	    // set here, so we'll use xcf, but just in case we'll track cf

	    Configuration *ocf = cf;
	    cf = xcf;

	    List *stalled = ui_retrieve_matching_state(match_stalled);

	    if(stalled)
	    {
	      for(int i = 0;i < stalled->entries();i++)
	      {
		SHPair *shp = new SHPair(stalled->retrieve(i));

		if(shp && xcf->find_check(shp->service()))
		{
		  CheckState *cs =
		    new CheckState(xcf->find_check(shp->service()),
				   shp->host());

		  if(cs)
		  {
		    st->add_stalled_check(shp->service(), shp->host(),
					  cs->lastcheck());

		    xdelete(cs);
		  }

		  xdelete(shp);
		}
	      }
	    }
	    
	    cf = ocf;
	  }
	  
	  xdelete(xcf);
	}
	else
	  wlog->warn("process_status failed to allocate Configuration");
	
	// Next determine the last cycle times.
	
	RunningState *rstate = new RunningState();
	
	if(rstate)
	{
	  st->set_last_cycles(rstate->get("alert"), rstate->get("check"));
	  
	  xdelete(rstate);
	}
      }
      
      // We must generate content type headers
        
      HTMLOUT << CGIXMLHEADER;
        
      if(!sxml->generate(HTMLOUT_FILENO, st))
        wlog->warn("process_status failed to generate XML");
      
      xdelete(sxml);
    }
    else
      wlog->warn("process_status failed to allocate SurvivorXML");
    
    xdelete(st);
  }
  else
    wlog->warn("process_status failed to allocate SurvivorStatus");

}

void process_tmpfile(char *tfile)
{
  // Deliver the requested tmpfile <tfile>.

  // Returns: Nothing.

  if(tfile && !strchr(tfile, '/') && args->tmpdir())
  {
    CharBuffer *tf = new CharBuffer(args->tmpdir());

    if(tf)
    {
      tf->append("/");
      tf->append(tfile);
    
      char *ext = strrchr(tfile, '.');

      if(ext && *ext != '\0')
	ext++;
    
      // Look up mime type based on extension
      
      char *doctype = mime_ext_to_type(ext);
      
      // Determine size of file
      
      struct stat sb;

      if(stat(tf->str(), &sb)==0)
      {
	FILE *tin = fopen(tf->str(), "r");

	if(tin)
	{
	  // Dump header, including size and type, then file

	  if(doctype)
	    HTMLOUT << "Content-Type: " << doctype << endl;
	  else
	    HTMLOUT << "Content-Description: File Transfer" << endl
		    << "Content-Type: application/force-download" << endl
		    << "Content-Disposition: attachment; filename=" << tfile
		    << endl;
	  
	  HTMLOUT << "Accept-Ranges: bytes" << endl;
	  HTMLOUT << "Content-Length: " << sb.st_size << endl << endl;

	  for(int c = getc(tin);c != EOF;c = getc(tin))
	    putc(c, HTMLOUT_FILE); 
	  
	  fclose(tin);
	}
	else
	  wlog->warn("process_tmpfile failed to open %s", tf->str());
      }
      else
      {
	// Blank instead of server configuration error if file doesn't exist
	HTMLOUT << CGIHEADER << endl;
	
	wlog->warn("process_tmpfile failed to stat %s", tf->str());
      }
      
      xdelete(tf);
    }
    else
      wlog->warn("process_tmpfile failed to allocate CharBuffer");
  }
}
