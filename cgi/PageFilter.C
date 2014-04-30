/*
 * PageFilter.C: survivor web interface page filter object
 *
 * Version: $Revision: 0.10 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2006/11/24 04:31:04 $
 * MT-Level: Safe
 *
 * Copyright (c) 2003 - 2006
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: PageFilter.C,v $
 * Revision 0.10  2006/11/24 04:31:04  benno
 * Encode ampersands
 *
 * Revision 0.9  2006/10/17 13:59:47  benno
 * Add "first" to check,alert,fix status
 * Add "duration" to checkstatus
 *
 * Revision 0.8  2005/12/22 03:57:10  benno
 * Add "next" checkstatus
 *
 * Revision 0.7  2005/11/24 18:27:56  benno
 * Use ui_retrieve_matching_state instead of retrieve_matching_state
 *
 * Revision 0.6  2005/11/18 04:23:58  benno
 * Don't construct helpfile, just use what c->helpfile has
 *
 * Revision 0.5  2005/04/09 03:05:53  benno
 * Use CBAPPENDESCAPED
 *
 * Revision 0.4  2004/11/26 22:09:57  benno
 * Stricter input validation with cgivaluesafer, etc
 *
 * Revision 0.3  2004/08/24 23:27:05  benno
 * Add SORT, INSTALLEDMODULES to FOREACH
 * Add field, offset to TIME
 * Use tagind
 *
 * Revision 0.2  2004/04/24 14:14:24  benno
 * Add ADDRESSED, ALLACTIVE, ERRORSTATE, ESCALATED, and SERVICEHOST
 * to FOREACH tag
 * Add retrieve_matching_state
 * Add SPLIT tag
 *
 * Revision 0.1  2004/03/02 17:13:58  benno
 * Initial revision
 *
 */

#include "cgi.H"

PageFilter::PageFilter(Hashtable<Variable> *vhash, char *pageset)
{
  // Allocate and initialize a new PageFilter object, which uses
  // <vhash> as its variable source.  The PageFilter may modify the
  // contents of <vhash>, but it is the responsibility of the invoking
  // code to delete <vhash> when done.  <pageset> is the full path to
  // the PageSet that should be used.

  // Returns: A new, initialized PageFilter object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "PageFilter::PageFilter(%d,%s)",
		  vhash, IONULL(pageset));
#endif
 
  ascache = NULL;
  cscache = NULL;
  fscache = NULL;
  shpcache = NULL;
  clists = NULL;
  addresses = NULL;
  vh = vhash;
  ps = xstrdup(pageset);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "PageFilter::PageFilter()");
#endif
}

void PageFilter::filter(char *page, bool dotin)
{
  // Filter the contents of <page> to HTMLOUT.  If <dotin> is true,
  // .in is appended to the page name.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "PageFilter::filter(%s,%s)",
		  IONULL(page), IOTF(dotin));
#endif

  if(page)
  {
    Variable *v = NULL;

    // Open a FileHandler for page to read from, write to HTMLOUT.
    // Page is relative to the pageset directory.

    CharBuffer *infile = new CharBuffer(ps);

    if(infile)
    {
      infile->append('/');
      infile->append(page);
      if(dotin)
	infile->append(".in");
      
      FileHandler *src = new FileHandler();
      FileHandler *dest = new FileHandler(HTMLOUT_FILENO);
      
      if(src && dest && src->open_read(infile->str()))
	do_filter(src, dest);
      else
	wlog->warn("PageFilter::filter failed to open %s",
		   infile->str());

      xdelete(src);
      xdelete(dest);
      xdelete(infile);
    }
    else
      wlog->warn("PageFilter::filter failed to allocate CharBuffer");
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "PageFilter::filter()");
#endif
}

PageFilter::~PageFilter()
{
  // Deallocate the PageFilter.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "PageFilter::~PageFilter()");
#endif
  
  // Not ours, so don't delete.
  vh = NULL;

  uncache_state();
  xdelete(clists);
  xdelete(addresses);
  xdelete(ps);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "PageFilter::~PageFilter()");
#endif
}

void PageFilter::do_filter(CharHandler *src, CharHandler *dest)
{
  // Filter the input from <src>, copying the output to <dest>.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "PageFilter::do_filter(%d,%d)", src, dest);
#endif
  
  if(src && dest)
  {
    bool done = false;
    
    while(!done)
    {
      // Loop through, reading until we see a command or end of stream
      
      char *in = src->read_until('@');

      if(in)
      {
	// Copy what we read to the output since we're about to parse
	// a command
	
	dest->append(in);

	char c = src->read_char();
	
	if(c == '@')
	{
	  // We might have @{FOO}@@{BAR}, in which case we are sitting
	  // on the rightmost @ right now.  Output the first @ regardless.

	  dest->append('@');
	  c = src->read_char();
	}
	
	if(c == '{')
	{
	  // Read the tag, which includes the command and its arguments

	  char *intag = src->read_until('}');

	  if(intag && intag[0] != '#')
	  {
	    SWTag *tag = new SWTag(intag, vh);
	    
	    if(tag)
	      eval_tag(tag, src, dest);
	    else
	      wlog->warn("PageFilter::do_filter failed to allocate SWTag");

	    xdelete(tag);
	    intag = NULL;  // Not ours to delete
	  }
	  // else nothing to do
	}
	else if(c != 0)
	{
	  // Just a @ in some text, copy c and keep going

	  dest->append('@');
	  dest->append(c);
	}
      }
      else
	done = true;
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "PageFilter::do_filter()");
#endif
}

void PageFilter::eval_tag(SWTag *tag, CharHandler *src, CharHandler *dest)
{
  // Evaluate the tag <tag>, copying output to <dest>, and reading
  // additional input from <src> if required.

  // Returns: Nothing.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "PageFilter::eval_tag(%d,%d,%d)",
		  tag, src, dest);
#endif

  // IMPORTANT: cf is not necessarily valid while processing tags,
  // especially if no instance is defined.
  
  if(tag && tag->command() && src && dest)
  {
    if(tag->command()->authz == none_authz
       || (cgicf->authorized((session ? session->user() : NULL),
			     (session ? session->groups() : NULL))
	   >= tag->command()->authz))
    {
      // Figure out what command we are to run and do it
    
      if(strcmp(tag->command()->name, alertplan_tag.name)==0)
      {
	if(cf)
	{
	  char *h = tag->arg_parameter("host");
	  char *s = tag->arg_parameter("service");

	  if(h && s)
	  {
	    HostClass *hc = cf->find_class(h);
	    Check *c = cf->find_check(s);

	    if(hc && c)
	    {
	      if(c->alert_plan())
	      {
		dest->append(c->alert_plan()->name());
		dest->append(_t(" (via check)"));
	      }
	      else if(hc->alert_plan())
	      {
		dest->append(hc->alert_plan()->name());
		dest->append(_t(" (via hostclass)"));
	      }
	    }
	    else
	      wlog->warn("PageFilter::eval_tag did not find HostClass for '%s' and/or Check '%s'",
			 alertplan_tag.name, h, s);
	  }
	  else
	    wlog->warn("PageFilter::eval_tag found incorrect usage for '%s'",
		       alertplan_tag.name);
	}
      }
      else if(strcmp(tag->command()->name, alertstatus_tag.name)==0)
      {
	if(cf)
	{
	  char *h = tag->arg_parameter("host");
	  char *s = tag->arg_parameter("service");
	  char *t = tag->arg_parameter("type");

	  if(t)
	  {
	    if(h && s)
	    {
	      // Obtain the alert status of s@h

	      if(retrieve_state(s, h))
	      {
		if(strcmp(t, "acknowledged")==0)
		{
		  if(ascache->acknowledged())
		    dest->append(ascache->acknowledged());
		}
		else if(strcmp(t, "acknowledgedfor")==0)
		{
		  if(ascache->acknowledged_for())
		  {
		    CBAPPENDESCAPED(dest, ascache->acknowledged_for());
		  }
		}
		else if(strcmp(t, "alerts")==0)
		{
		  dest->append(ascache->alerts());
		}
		if(strcmp(t, "escalated")==0)
		{
		  bool esc = false;

		  if(cscache->lastcheck() > 0)
		  {
		    // Determine which AlertPlan is in effect in order
		    // to determine escalation status.

		    Check *c = cf->find_check(s);

		    if(c)
		    {
		      AlertPlan *ap = c->alert_plan();
		    
		      if(!ap)
		      {
			HostClass *hc = cf->find_class(h);
			
			if(hc)
			  ap = hc->alert_plan();
		      }
		      
		      if(ap)
			esc = ascache->escalated(ap, cscache);
		    }
		  }

		  if(esc)
		    dest->append("1");
		  else
		    dest->append("0");
		}
		else if(strcmp(t, "first")==0)
		{
		  time_t first = ascache->since();

		  if(first > 0)
		  {
		    char tbuf[26];
		  
		    dest->append(ctime_r(&first, tbuf));
		  }
		  else
		    dest->append(_t("Never"));
		}
		else if(strcmp(t, "inhibited")==0)
		{
		  if(ascache->inhibited())
		    dest->append(ascache->inhibited());
		}
		else if(strcmp(t, "inhibitedfor")==0)
		{
		  if(ascache->inhibited_for())
		  {
		    CBAPPENDESCAPED(dest, ascache->inhibited_for());
		  }
		}
		else if(strcmp(t, "last")==0)
		{
		  time_t last = ascache->lastalert();

		  if(last > 0)
		  {
		    char tbuf[26];
		  
		    dest->append(ctime_r(&last, tbuf));
		  }
		  else
		    dest->append(_t("Never"));
		}
		else if(strcmp(t, "lastnotify")==0)
		{
		  RecipientSet *rset = ascache->lastnotify();
	  
		  if(rset && rset->modules() > 0)
		  {
		    for(int i = 0;i < rset->modules();i++)
		    {
		      if(i > 0)
			dest->append(",");
		      
		      dest->append(rset->addresses(i));
		    }
		  }
		  else
		    dest->append(_t("Nobody"));
		}
		else if(strcmp(t, "manualescalated")==0)
		{
		  dest->append(ascache->escalated_manual());
		}
		else if(strcmp(t, "quiet")==0)
		{
		  if(ascache->inhibited() || ascache->acknowledged())
		    dest->append("1");
		  else
		    dest->append("0");
		}
	      }
	      else
		dest->append("-1");
	    }
	  }
	  else
	    wlog->warn("PageFilter::eval_tag did not find type for '%s'",
		       alertstatus_tag.name);
	}
      }
      else if(strcmp(tag->command()->name, authlevel_tag.name)==0)
      {
	authz_level_t authz = none_authz;
	
	if(session && session->user())
	  authz = cgicf->authorized(session->user(), session->groups());
	else
	  authz = cgicf->authorized(NULL, NULL);
	
	if(tag->arg_tokens() > 0)
	{
	  bool match = false;
	  char *a = tag->arg_token(0);

	  if(a && ((strcmp(a, "admin")==0 && authz >= admin_authz) ||
		   (strcmp(a, "clipboard")==0 && authz >= clipboard_authz) ||
		   (strcmp(a, "rw")==0 && authz >= read_write_authz) ||
		   (strcmp(a, "exec")==0 && authz >= exec_authz) ||
		   (strcmp(a, "ro")==0 && authz >= read_only_authz) ||
		   (strcmp(a, "none")==0)))
	    match = true;

	  if(match)
	    dest->append("1");
	  else
	    dest->append("0");
	}
	else
	{
	  switch(authz)
	  {
	  case admin_authz:
	    dest->append("admin");
	    break;
	  case clipboard_authz:
	    dest->append("clipboard");
	    break;
	  case read_write_authz:
	    dest->append("rw");
	    break;
	  case read_only_authz:
	    dest->append("ro");
	    break;
	  default:
	    dest->append("none");
	    break;
	  }
	}
      }
      else if(strcmp(tag->command()->name, checkschedule_tag.name)==0)
      {
	if(cf)
	{
	  char *h = tag->arg_parameter("host");
	  char *s = tag->arg_parameter("service");

	  if(h && s)
	  {
	    HostClass *hc = cf->find_class(h);
	    Check *c = cf->find_check(s);

	    if(hc && c)
	    {
	      if(c->check_schedule())
	      {
		dest->append(c->check_schedule()->name());
		dest->append(_t(" (via check)"));
	      }
	      else if(hc->check_schedule())
	      {
		dest->append(hc->check_schedule()->name());
		dest->append(_t(" (via hostclass)"));
	      }
	    }
	    else
	      wlog->warn("PageFilter::eval_tag did not find HostClass for '%s' and/or Check '%s'",
			 checkschedule_tag.name, h, s);
	  }
	  else
	    wlog->warn("PageFilter::eval_tag found incorrect usage for '%s'",
		       checkschedule_tag.name);
	}
      }
      else if(strcmp(tag->command()->name, checkstatus_tag.name)==0)
      {
	// This can be a very intensive tag to process, especially
	// when something like rc for hostclass is requested.

	if(cf)
	{
	  char *hc = tag->arg_parameter("hostclass");
	  char *h = tag->arg_parameter("host");
	  char *s = tag->arg_parameter("service");
	  char *t = tag->arg_parameter("type");

	  if(t)
	  {
	    if(h && s)
	    {
	      // Obtain the status of s@h
	      
	      if(retrieve_state(s, h))
	      {
		if(strcmp(t, "comment")==0)
		{
		  CBAPPENDESCAPED(dest, cscache->comment());
		}
		else if(strcmp(t, "duration")==0)
		  dest->append(cscache->duration());
		else if(strcmp(t, "first")==0)
		{
		  time_t first = cscache->since();

		  if(first > 0)
		  {
		    char tbuf[26];
		  
		    dest->append(ctime_r(&first, tbuf));
		  }
		  else
		    dest->append(_t("Never"));
		}
		else if(strcmp(t, "instances")==0)
		  dest->append(cscache->consecutive());
		else if(strcmp(t, "last")==0)
		{
		  time_t last = cscache->lastcheck();

		  if(last > 0)
		  {
		    char tbuf[26];
		  
		    dest->append(ctime_r(&last, tbuf));
		  }
		  else
		    dest->append(_t("Never"));
		}
		else if(strcmp(t, "next")==0)
		{
		  Check *c = cf->find_check(s);

		  if(c)
		  {
		    Array<Schedule> *csched = c->check_schedule();

		    if(!csched)
		    {
		      HostClass *hc = cf->find_class(h);
		      
		      if(hc)
			csched = hc->check_schedule();
		    }
		  
		    time_t nt = next_schedule_time(csched,
						   cscache->lastcheck());

		    if(nt > 0)
		    {
		      char tbuf[26];
		      
		      dest->append(ctime_r(&nt, tbuf));
		    }
		    else
		      dest->append(_t("Unable to calculate"));
		  }
		}
		else if(strcmp(t, "rc")==0)
		  dest->append(cscache->returncode());
	      }
	      else
		dest->append(-1);
	    }
	    else if(h)
	    {
	      if(strcmp(t, "rc")==0)
		dest->append(ui_composite_check_status(h));
	    }
	    else if(s)
	    {
	      if(strcmp(t, "rc")==0)
	      {
		Check *c = cf->find_check(s);
	      
		if(c)
		  dest->append(ui_composite_check_status(c));
	      }
	    }
	    else if(hc)
	    {
	      if(strcmp(t, "rc")==0)
	      {
		HostClass *hclass = cf->find_hostclass(hc);
	      
		if(hclass)
		  dest->append(ui_composite_check_status(hclass->hosts()));
	      }
	    }
	  }
	  else
	    wlog->warn("PageFilter::eval_tag did not find type for '%s'",
		       checkstatus_tag.name);
	}
      }
      else if(strcmp(tag->command()->name, clipboard_tag.name)==0)
      {
	// Incorporate the contents of the requested clipboard.
	
	if(tag->arg_tokens() == 1)
	{
	  ClipboardManager *cbm = new ClipboardManager();

	  if(cbm)
	  {
	    Clipboard *cb = cbm->find_board(tag->arg_token(0));

	    if(cb)
	    {
	      char *c = cb->read();

	      if(c)
	      {
		CBAPPENDESCAPED(dest, c);

		xdelete(c);
	      }
	      
	      xdelete(cb);
	    }
	    else
	      wlog->warn("Requested Clipboard '%s' does not exist in PageFilter::eval_tag",
			 tag->arg_token(0)); 
	    
	    xdelete(cbm);
	  }
	  else
	    wlog->warn("PageFilter::eval_tag failed to allocate ClipboardManager");
	}
      }
      else if(strcmp(tag->command()->name, clipemail_tag.name)==0)
      {
	// Incorporate the default clipboard email contact

	dest->append(cgicf->clip_email());
      }
      else if(strcmp(tag->command()->name, clipphone_tag.name)==0)
      {
	// Incorporate the default clipboard phone contact

	dest->append(cgicf->clip_phone());
      }
      else if(strcmp(tag->command()->name, echo_tag.name)==0)
      {
	// Echo the requested text, via localization
	
	if(tag->arg_opaque())
	{
	  CBAPPENDESCAPED(dest, _t(tag->arg_opaque()));
	}
      }
      else if(strcmp(tag->command()->name, error_tag.name)==0)
      {      
	// Dump the value of error, if set
	
	CGIValue *err = parser->value("error");

	if(cgivalue(err))
	{
	  CBAPPENDESCAPED(dest, cgivaluesafer(err));
	}
      }
      else if(strcmp(tag->command()->name, fixstatus_tag.name)==0)
      {
	if(cf)
	{
	  char *h = tag->arg_parameter("host");
	  char *s = tag->arg_parameter("service");
	  char *t = tag->arg_parameter("type");

	  if(t)
	  {
	    if(h && s)
	    {
	      // Obtain the status of s@h
	      
	      if(retrieve_state(s, h))
	      {
		if(strcmp(t, "attempts")==0)
		  dest->append(fscache->fix_attempts());
		else if(strcmp(t, "comment")==0)
		{
		  CBAPPENDESCAPED(dest, fscache->comment());
		}
		else if(strcmp(t, "first")==0)
		{
		  time_t first = fscache->since();

		  if(first > 0)
		  {
		    char tbuf[26];
		  
		    dest->append(ctime_r(&first, tbuf));
		  }
		  else
		    dest->append(_t("Never"));
		}
		else if(strcmp(t, "last")==0)
		{
		  time_t last = fscache->lastfix();

		  if(last > 0)
		  {
		    char tbuf[26];
		  
		    dest->append(ctime_r(&last, tbuf));
		  }
		  else
		    dest->append(_t("Never"));
		}
		else if(strcmp(t, "rc")==0)
		  dest->append(fscache->returncode());
		else if(strcmp(t, "who")==0)
		  dest->append(fscache->lastfix_by());
	      }
	      else
		dest->append(-1);
	    }
	  }
	  else
	    wlog->warn("PageFilter::eval_tag did not find type for '%s'",
		       fixstatus_tag.name);
	}
      }
      else if(strcmp(tag->command()->name, flag_tag.name)==0)
      {      
	// Dump the value of the requested flag, if set

	if(tag->arg_tokens() == 1)
	{
	  CGIValue *err = parser->value(tag->arg_token(0));
	
	  if(cgivalue(err))
	    dest->append(cgivaluesafer(err));
	}
      }
      else if(strcmp(tag->command()->name, foreach_tag.name)==0)
      {
	// Argument is a series of tokens: [SORT ORDER] VAR TYPE X Y Z ...
	// where ORDER is "ASC", "DESC", "ASCIN", "DESCIN", VAR is the
	// variable to use for iteration, TYPE describes what X Y Z
	// are, and X Y Z are the optional instances to be iterated.
	
	if(tag->arg_tokens() > 1)
	{
	  // We read ahead until we see a corresponding close tag,
	  // then we process the block.

	  CharBuffer *fecb = find_close_tag(tag->command(),
					    tag->command(),
					    src);

	  if(fecb)
	  {
	    // Now that we've finished reading the block, loop over it.

	    sort_t sort = no_sort;
	    
	    char *fevar = tag->arg_token(0);
	    char *fetype = tag->arg_token(1);
	    int tagind = 2;  // Index of first tag

	    if(strcmp(fevar, "SORT")==0)
	    {
	      if(strcmp(fetype, "ASC")==0)
		sort = ascending_sort;
	      else if(strcmp(fetype, "ASCIN")==0)
		sort = ascending_caseless_sort;
	      else if(strcmp(fetype, "DESC")==0)
		sort = descending_sort;
	      else if(strcmp(fetype, "DESCIN")==0)
		sort = descending_caseless_sort;
	      else
		wlog->warn("PageFilter::eval_tag found unknown sort order '%s'",
			   fetype);
	      
	      fevar = tag->arg_token(2);
	      fetype = tag->arg_token(3);
	      tagind = 4;
	    }

	    // Reuse any previous Variable
	    Variable *fev = vh->retrieve(fevar);

	    if(!fev)
	    {
	      fev = new Variable(fevar, "");
	      
	      if(!vh->insert(fev->name(), fev))
	      {
		xdelete(fev);
	      }
	    }

	    if(fev)
	    {
	      // There are two types of iterations, those where the
	      // variable x is provided as part of the foreach argument,
	      // and those where x is implied.  For the second type, we
	      // obtain a list in advance.

	      List *felist = NULL;

	      if(strcmp(fetype, "ADDRESSED")==0)
	      {
		// Obtain the list of ack'd/inhibited service@host pairs

		felist = ui_retrieve_matching_state(match_addressed);
	      }
	      else if(strcmp(fetype, "ALLACTIVE")==0)
	      {
		// Obtain the list of active service@host pairs

		felist = ui_retrieve_matching_state(match_all);
	      }
	      else if(strcmp(fetype, "CALLLISTADDRESS")==0)
	      {
		if(get_calllist_info())
		  felist = new List(clists);
	      }
	      else if(strcmp(fetype, "CLIPBOARD")==0)
	      {
		ClipboardManager *cbm = new ClipboardManager();

		if(cbm)
		{
		  felist = cbm->find_all_boards();
		  
		  xdelete(cbm);
		}
		else
		  wlog->warn("PageFilter::eval_tag failed to allocate ClipboardManager");
	      }
	      else if(strcmp(fetype, "ERRORSTATE")==0)
	      {
		// Obtain the list of service@host pairs in error state

		felist = ui_retrieve_matching_state(match_error);
	      }
	      else if(strcmp(fetype, "ESCALATED")==0)
	      {
		// Obtain the list of escalated service@host pairs

		felist = ui_retrieve_matching_state(match_escalated);
	      }
	      else if(strcmp(fetype, "FLAG")==0
		      && tag->arg_tokens() == (tagind + 1))
	      {
		// Obtain the values of the requested flag

		felist = new List();

		if(felist)
		{
		  CGIValue *flag = parser->value(tag->arg_token(tagind));

		  if(flag && flag->validate(CGIVALID))
		  {
		    for(int i = 0;i < flag->values();i++)
		      felist->add(flag->value(i));
		  }
		}
	      }
	      else if(cf && (strcmp(fetype, "GROUPS")==0 ||
			     strcmp(fetype, "SERVICES")==0))
	      {
		// Assemble the list of groups that at least one
		// host provided is a member of

		felist = new List();

		if(felist)
		{
		  for(int i = tagind;i < tag->arg_tokens();i++)
		  {
		    List *g = cf->find_groups(tag->arg_token(i));

		    if(g)
		    {
		      for(int j = 0;j < g->entries();j++)
		      {
			// Only add active checks we haven't added yet
			
			if(cf->find_check(g->retrieve(j)) &&
			   felist->find(g->retrieve(j)) == -1)
			  felist->add(g->retrieve(j));
		      }
		    }
		  }
		}
	      }
	      else if(cf && strcmp(fetype, "HOST")==0)
	      {
		// Assemble the list of hosts from all members of
		// all hostclasses, since hosts must be uniquely
		// defined there
		
		List *hcs = cf->get_all_hostclasses();

		if(hcs)
		{
		  felist = new List();

		  if(felist)
		  {
		    for(int i = 0;i < hcs->entries();i++)
		    {
		      HostClass *hc = cf->find_hostclass(hcs->retrieve(i));

		      if(hc && hc->hosts())
		      {
			for(int j = 0;j < hc->hosts()->entries();j++)
			  felist->add(hc->hosts()->retrieve(j));
		      }
		    }
		  }
		}
	      }
	      else if(strcmp(fetype, "INSTANCE")==0)
		felist = new List(args->instances());
	      else if(strcmp(fetype, "INSTALLEDMODULES")==0
		      && tag->arg_tokens() == (tagind + 1))
	      {
		CharBuffer *mdir = new CharBuffer(args->moddir());
		felist = new List();

		if(mdir && felist)
		{
		  mdir->append("/");
		  mdir->append(tag->arg_token(tagind));

		  DIR *dir = opendir(mdir->str());

		  if(dir)
		  {
		    struct dirent *entry = allocate_dirent(mdir->str());
		    struct dirent *dp;

		    if(entry)
		    {
		      while(readdir_r(dir, entry, &dp) == 0 && dp)
		      {
			if(dp && dp->d_name[0] != '.')
			  felist->add(dp->d_name);
		      }

		      free(entry);
		      entry = NULL;
		    }

		    closedir(dir);
		  }
		  else
		    wlog->warn("PageFilter::eval_tag failed to opendir %s",
			       mdir->str());
		}
		else
		{
		  wlog->warn("PageFilter::eval_tag failed to allocate mdir or felist");

		  xdelete(felist);
		}
		  
		xdelete(mdir);
	      }
	      else if(cf && strcmp(fetype, "HOSTCLASS")==0)
		felist = new List(cf->get_all_hostclasses());
	      else if(cf && strcmp(fetype, "HOSTCLASSMEMBER")==0
		   && tag->arg_tokens() == (tagind + 1))
	      {
		// Obtain the members of the requested hostclass

		HostClass *hc = cf->find_hostclass(tag->arg_token(tagind));

		if(hc)
		  felist = new List(hc->hosts());
	      }
	      else if(strcmp(fetype, "PERSONADDRESS")==0)
	      {
		if(get_calllist_info())
		  felist = new List(addresses);
	      }
	      else if(cf && strcmp(fetype, "SERVICE")==0)
	      {
		felist = new List();

		if(felist)
		{
		  Array<Check> *cs = cf->get_all_checks();

		  if(cs)
		  {
		    for(int i = 0;i < cs->entries();i++)
		    {
		      Check *c = cs->retrieve(i);

		      if(c && c->name())
			felist->add(c->name());
		    }
		  }
		}
	      }
	      else if(strcmp(fetype, "SERVICEHOST")==0)
	      {
		// Obtain the list of service@host pairs

		felist = ui_retrieve_matching_state(match_all);
	      }
	      else if(cf &&
		      (strcmp(fetype, "SERVICEMEMBER")==0 ||
		       strcmp(fetype, "GROUPMEMBER")==0)
		      && tag->arg_tokens() == (tagind + 1))
	      {
		// Obtain the members of the requested group

		 felist = new List(cf->find_group(tag->arg_token(tagind)));
	      }
	      else
	      {
		// Copy the argument tokens
		
		felist = new List();
		
		if(felist)
		{
		  for(int i = tagind;i < tag->arg_tokens();i++)
		    felist->add(tag->arg_token(i));
		}
		else
		  wlog->warn("PageFilter::eval_tag failed to allocate List");
	      }

	      if(felist)
	      {
		// Perform requested sort

		if(sort != no_sort)
		  felist->sort(sort);
		
		for(int i = 0;i < felist->entries();i++)
		{
		  // We don't have to reinsert fev each time since we
		  // retain the pointer to the object we've inserted.
		  // We just change its value.
		  
		  fev->set_value(felist->retrieve(i));
		  
		  CharBuffer *cbcopy = new CharBuffer(fecb);

		  if(cbcopy)
		  {
		    // do_filter will consume the contents of cbcopy,
		    // thus we use copies
		  
		    do_filter(cbcopy, dest);
		  
		    xdelete(cbcopy);
		  }
		  else
		    wlog->warn("PageFilter::eval_tag failed to allocate CharBuffer");
		}

		xdelete(felist);
	      }
	    }
	    else
	      wlog->warn("PageFilter::eval_tag failed to allocate Variable");

	    xdelete(fecb);
	  }
	  else
	    wlog->warn("PageFilter::eval_tag did not find '%s' for '%s'",
		       foreach_tag.name, foreach_tag.closetag);
	}
	else
	  wlog->warn("PageFilter::eval_tag found insufficent arguments to '%s'",
		     foreach_tag.name);
      }
      else if(strcmp(tag->command()->name, helpfile_tag.name)==0)
      {
	if(cf)
	{
	  if(tag->arg_tokens() == 1)
	  {
	    Check *c = cf->find_check(tag->arg_token(0));

	    if(c)
	    {
	      FileHandler *fin = new FileHandler();
	      
	      if(fin && fin->open_read(c->helpfile()))
	      {
		CBAPPENDESCAPED(dest, fin->read_until_eof());
	      }

	      xdelete(fin);
	    }
	  }
	  else
	    wlog->warn("PageFilter::eval_tag found incorrect usage for '%s'",
		       hostclass_tag.name);
	}
      }
      else if(strcmp(tag->command()->name, hostclass_tag.name)==0)
      {
	if(cf)
	{
	  if(tag->arg_tokens() == 1)
	  {
	    HostClass *hc = cf->find_class(tag->arg_token(0));

	    if(hc)
	      dest->append(hc->name());
	  }
	  else
	    wlog->warn("PageFilter::eval_tag found incorrect usage for '%s'",
		       hostclass_tag.name);
	}
      }
      else if(strcmp(tag->command()->name, if_tag.name)==0)
      {
	CharBuffer *ifcb = find_close_tag(tag->command(),
					  tag->command(),
					  src);

	if(ifcb)
	{
	  // iftrue tracks if the actual if test is true
	  bool iftrue = false;

	  if(strcmp(tag->arg_token(0), "AUTHENTICATED")==0)
	  {
	    if(session && session->user() &&
	       cgicf->authorized(session->user(), session->groups()) !=
	       none_authz)
	      iftrue = true;
	  }
	  else if(strcmp(tag->arg_token(0), "DEFINED")==0 ||
		  strcmp(tag->arg_token(0), "NOTDEFINED")==0)
	  {
	    if(tag->arg_tokens() == 3)
	    {
	      if(strcmp(tag->arg_token(1), "FLAG")==0)
	      {
		CGIValue *cv = parser->value(tag->arg_token(2));

		if(strcmp(tag->arg_token(0), "DEFINED")==0)
		{
		  if(cgivalue(cv))
		    iftrue = true;
		}
		else
		{
		  if(!cgivalue(cv))
		    iftrue = true;	      
		}
	      }
	      else if(strcmp(tag->arg_token(1), "STRING")==0)
	      {
		if(strcmp(tag->arg_token(0), "DEFINED")==0)
		{
		  if(vh->retrieve(tag->arg_token(2)))
		    iftrue = true;
		}
		else
		{
		  if(!vh->retrieve(tag->arg_token(2)))
		    iftrue = true;
		}
	      }
	      else
		wlog->warn("PageFilter::eval_tag found unknown data type '%s' for '%s' '%s'",
			   tag->arg_token(1), if_tag.name, tag->arg_token(0));
	    }
	  }
	  else if(strcmp(tag->arg_token(0), "NUMBER")==0)
	  {
	    if(tag->arg_tokens() == 4)
	    {
	      int x = atoi(tag->arg_token(1));
	      int y = atoi(tag->arg_token(3));
	      
	      if(strcmp(tag->arg_token(2), "=")==0)
	      {
		if(x == y)
		  iftrue = true;
	      }
	      else if(strcmp(tag->arg_token(2), "!=")==0)
	      {
		if(x != y)
		  iftrue = true;
	      }
	      else if(strcmp(tag->arg_token(2), "<")==0)
	      {
		if(x < y)
		  iftrue = true;
	      }
	      else if(strcmp(tag->arg_token(2), ">")==0)
	      {
		if(x > y)
		  iftrue = true;
	      }
	      else
		wlog->warn("PageFilter::eval_tag found unknown comparison operator '%s' for '%s' '%s'",
			   tag->arg_token(2), if_tag.name, tag->arg_token(0));
	    }
	    // else probably asked to compare against an empty string
	  }
	  else if(strcmp(tag->arg_token(0), "STRING")==0)
	  {
	    if(tag->arg_tokens() > 1)
	    {
	      if(strcmp(tag->arg_token(1), "EMPTY")==0)
	      {
		if(tag->arg_tokens() == 2 || strcmp(tag->arg_token(2), "")==0)
		  iftrue = true;
	      }
	      else if(strcmp(tag->arg_token(1), "NOTEMPTY")==0)
	      {
		if(tag->arg_tokens() > 2 && strcmp(tag->arg_token(2), "")!=0)
		  iftrue = true;
	      }
	      else if(tag->arg_tokens() == 4)
	      {
		if(strcmp(tag->arg_token(2), "=")==0)
		{
		  if(strcmp(tag->arg_token(1), tag->arg_token(3))==0)
		    iftrue = true;
		}
		else if(strcmp(tag->arg_token(2), "!=")==0)
		{
		  if(strcmp(tag->arg_token(1), tag->arg_token(3))!=0)
		    iftrue = true;
		}
		else
		  wlog->warn("PageFilter::eval_tag found unknown comparison operator '%s' for '%s' '%s'",
			     tag->arg_token(2), if_tag.name,
			     tag->arg_token(0));
	      }
	      // else probably asked to compare against an empty string
	    }
	    // else probably asked to compare two empty strings
	  }
	  else
	    wlog->warn("PageFilter::eval_tag found unknown '%s' type '%s'",
		       if_tag.name, tag->arg_token(0));

	  // Look for an @{ELSE} tag to determine what to incorporate.
	  // find_close_tag will return nothing if it doesn't find the
	  // @{ELSE} tag, so if iftrue is true we need to keep a copy
	  // of ifcb to work with.

	  CharBuffer *ifcbcopy = NULL;

	  if(iftrue)
	  {
	    ifcbcopy = new CharBuffer(ifcb);
	  
	    if(!ifcbcopy)
	      wlog->warn("PageFilter::eval_tag failed to copy CharBuffer");
	  }

	  CharBuffer *iecb = find_close_tag(&if_else_tag, &if_tag, ifcb);
	
	  if(iecb)
	  {
	    // We found the @{ELSE} tag.  If iftrue, we process iecb
	    // (everything up to the @{ELSE}), otherwise we process
	    // what's left in ifcb (everything after it).
	  
	    if(iftrue)
	      do_filter(iecb, dest);
	    else
	      do_filter(ifcb, dest);
	  
	    xdelete(iecb);
	  }
	  else
	  {
	    // No @{ELSE} tag was found.  If iftrue is false, then we
	    // have nothing to do, otherwise we process the copy.

	    if(iftrue)
	      do_filter(ifcbcopy, dest);
	  }
	
	  xdelete(ifcb);
	  xdelete(ifcbcopy);
	}
	else
	  wlog->warn("PageFilter::eval_tag did not find '%s' for '%s'",
		     if_tag.name, if_tag.closetag);
      }
      else if(strcmp(tag->command()->name, include_tag.name)==0)
      {
	if(tag->arg_opaque())
	{
	  // Recursively call filter() on the include argument

	  filter(tag->arg_opaque(), false);
	}
	else
	  wlog->warn("PageFilter::eval_tag found incorrect usage for '%s'",
		     include_tag.name);
      }
      else if(strcmp(tag->command()->name, instance_tag.name)==0)
      {
	CGIValue *instance = parser->value("i");

	if(cgivalue(instance))
	  dest->append(cgivaluesafer(instance));
      }
      else if(strcmp(tag->command()->name, pageset_tag.name)==0)
      {
	dest->append(ps);
      }
      else if(strcmp(tag->command()->name, rctext_tag.name)==0)
      {
	if(tag->arg_token(0))
	{
	  char *t = ui_rc_to_text(atoi(tag->arg_token(0)));

	  if(t)
	    dest->append(t);
	  else
	  {
	    dest->append(_t("Return Code "));
	    dest->append(tag->arg_token(0));
	  }
	}
      }
      else if(strcmp(tag->command()->name, referer_tag.name)==0)
      {
	if(getenv("HTTP_REFERER"))
	  dest->append(getenv("HTTP_REFERER"));
      }
      else if(strcmp(tag->command()->name, refresh_tag.name)==0)
      {
	CGIValue *refresh = parser->value("r");

	if(cgivalue(refresh))
	  dest->append(cgivaluesafer(refresh));
	else
	  dest->append(600);
      }
      else if(strcmp(tag->command()->name, runningstate_tag.name)==0)
      {
	struct timeval tv;
	time_t sched = 0;

	gettimeofday(&tv, NULL);
  
	RunningState *rstate = new RunningState();
      
	if(rstate)
	{
	  sched = rstate->get(tag->arg_parameter("lastrun"));
	  
	  xdelete(rstate);
	}

	if(sched > 0)
	{
	  int diff = tv.tv_sec - sched;
	  int min = diff/60;

	  dest->append(min);
	}
	else
	  dest->append(-1);
      }
      else if(strcmp(tag->command()->name, set_tag.name)==0)
      {
	// Argument is of the form NAME=VALUE or NAME=@[TAG].
	
	char *xa = xstrdup(tag->arg_opaque());
      
	if(xa)
	{
	  char *xb = strchr(xa, '=');
	  CharBuffer *xb2 = new CharBuffer();
	
	  if(xb && xb2)
	  {
	    *xb = '\0';
	    xb++;
	
	    if(strlen(xb) > 3 && xb[0] == '@' && xb[1] == '[')
	    {
	      // Replace xb with an evaluation of xb as a tag

	      char *xc = strchr(xb, ']');

	      if(xc)
	      {
		*xc = '\0';

		SWTag *xt = new SWTag(xb+2, vh);

		if(xt && xt->command())
		{
		  // IF and FOREACH tags cannot be nested in a SET tag
		  
		  if(strcmp(xt->command()->name, foreach_tag.name) != 0
		     && strcmp(xt->command()->name, if_tag.name) != 0)
		    eval_tag(xt, src, xb2);
		  
		  // Set xb to the contents of xb2
		  xb = xb2->str();

		  if(!xb)  // tag evaluated to nothing
		    xb = "";
		  
		  xdelete(xt);
		}
	      }
	      // else leave xb untouched and just copy it
	    }

	    // Set the variable $xa to xb

	    Variable *v = vh->retrieve(xa);

	    if(v)
	    {
	      // Overwrite previous value

	      v->set_value(xb);
	    }
	    else
	    {
	      // Insert for the first time
	    	    
	      v = new Variable(xa, xb);
	    
	      if(v)
	      {
		if(!vh->insert(v->name(), v))
		{
		  xdelete(v);
		}
	      }
	      else
		wlog->warn("PageFilter::eval_tag failed to allocate Variable");
	    }
	  }
	  else
	    wlog->warn("PageFilter::eval_tag did not find value for '%s' (%s)",
		       set_tag.name, xa);
	
	  xdelete(xa);
	  xdelete(xb2);
	}
	else
	  wlog->warn("PageFilter::eval_tag did not find name for '%s'",
		     set_tag.name);
      }
      else if(strcmp(tag->command()->name, split_tag.name)==0)
      {
	// Argument is of the form STRING CHARS VARNAME ...

	if(tag->arg_tokens() > 2)
	{
	  // First, tokenize arg 0 on arg 1

	  List *tokens = tokenize(tag->arg_token(0), tag->arg_token(1));

	  if(tokens)
	  {
	    // Store each token parsed into each corresponding variable
	    // until we run out of one or the other

	    for(int i = 2;i < tag->arg_tokens();i++)
	    {
	      char *vn = tag->arg_token(i);
	      char *vv = tokens->retrieve(i-2);

	      if(vn && vv)
	      {
		// Set $vn to vv

		Variable *v = vh->retrieve(vn);

		if(v)
		{
		  // Overwrite previous value
		  
		  v->set_value(vv);
		}
		else
		{
		  // Insert for the first time
		  
		  v = new Variable(vn, vv);
		  
		  if(v)
		  {
		    if(!vh->insert(v->name(), v))
		    {
		      xdelete(v);
		    }
		  }
		  else
		    wlog->warn("PageFilter::eval_tag failed to allocate Variable");
		}
	      }
	      else
	      {
		// We've run out of something

		break;
	      }
	    }
	    
	    xdelete(tokens);
	  }
	}
      }
      else if(strcmp(tag->command()->name, time_tag.name)==0)
      {
	struct timeval tv;
	char tbuf[26];

	char *field = tag->arg_parameter("field");
	char *off = tag->arg_parameter("offset");
	
	gettimeofday(&tv, NULL);

	if(off)
	  tv.tv_sec += atoi(off);

	if(field)
	{
	  struct tm tm;

	  if(localtime_r(&(tv.tv_sec), &tm))
	  {
	    if(strcmp(field, "mon")==0)
	      dest->append(tm.tm_mon + 1);
	    else if(strcmp(field, "mday")==0)
	      dest->append(tm.tm_mday);
	    else if(strcmp(field, "year")==0)
	      dest->append(tm.tm_year + 1900);
	    else if(strcmp(field, "hour")==0)
	      dest->append(tm.tm_hour);
	    else if(strcmp(field, "min")==0)
	    {
	      if(tm.tm_min < 10)
		dest->append("0");
	      dest->append(tm.tm_min);
	    }
	  }
	}
	else
	  dest->append(ctime_r(&(tv.tv_sec), tbuf));
      }
      else if(strcmp(tag->command()->name, uri_tag.name)==0)
      {
	List *names = new List();
	List *values = new List();
	List *skip = new List();
	
	if(names && values && skip)
	{
	  // Always skip error

	  skip->add("error");
	  
	  for(int i = 0;i < tag->arg_tokens();i++)
	  {
	    char *x = xstrdup(tag->arg_token(i));

	    if(x)
	    {
	      char *x2 = strchr(x, '=');

	      if(x2)
	      {
		*x2 = '\0';
		x2++;
	      }

	      if(x2 && strlen(x2) > 0)
	      {
		names->add(x);
		values->add(x2);
	      }
	      else
		skip->add(x);
	    
	      xdelete(x);
	    }
	  }
	  
	  dest->append(parser->url(true, names, values, skip));
	}
	else
	  wlog->warn("PageFilter::eval_tag failed to allocate lists");

	xdelete(names);
	xdelete(values);
	xdelete(skip);
      }
      else if(strcmp(tag->command()->name, uritags_tag.name)==0)
      {
	// Since the only supported style is hidden, don't bother
	// checking the style parameter.
	
	List *omit = tokenize(tag->arg_parameter("omit"), ",");

	if(!omit)
	  omit = new List();

	if(omit)
	{
	  // Always skip error and info

	  omit->add("error");
	  omit->add("info");
	}
	
	dest->append(parser->hidden(omit, CGIVALID));

	xdelete(omit);
      }
      else if(strcmp(tag->command()->name, uritop_tag.name)==0)
      {
	dest->append(parser->urlpath());
      }
      else if(strcmp(tag->command()->name, username_tag.name)==0)
      {
	if(session && session->user())
	  dest->append(session->user());
      }
      else if(strcmp(tag->command()->name, version_tag.name)==0)
      {
	char *vtype = tag->arg_parameter("type");

	if(vtype)
	{
	  if(strcmp(vtype, "build")==0)
	    dest->append(args->version(build_version));
	  else
	    dest->append(args->version(package_version));
	}
      }
      else
	wlog->warn("PageFilter::eval_tag found unknown tag '%s'",
		   tag->command()->name);
    }
    else
    {
      dest->append(_t("Not authorized for "));
      dest->append(tag->command()->name);
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "PageFilter::eval_tag()");
#endif
}

CharBuffer *PageFilter::find_close_tag(SWTagInfo *tag, SWTagInfo *nested,
				       CharHandler *src)
{
  // Read from <src> until the corresponding closing <tag> is read,
  // accounting for <nested> tags.

  // Returns: A newly allocated CharBuffer containing the block read
  // and that should be delete'd when no longer required, or NULL
  // on error, including if no closing tag is seen.

  CharBuffer *ret = NULL;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "PageFilter::find_close_tag(%d,%d,%d)",
		  tag, nested, src);
#endif

  if(tag && tag->closetag && src)
  {
    ret = new CharBuffer();

    if(ret)
    {
      bool endseen = false;
      int togo = 1;  // We assume we've already seen the opentag

      while(!endseen)
      {
	char *in = src->read_until('@');

	if(in)
	{
	  // Copy what we just read to the output

	  ret->append(in);

	  char c = src->read_char();

	  if(c == '@')
	  {
	    // We might have @{FOO}@@{BAR}, in which case we are sitting
	    // on the rightmost @ right now.  Output the first @ regardless.

	    ret->append('@');
	    c = src->read_char();
	  }

	  switch(c)
	  {
	  case 0:
	    endseen = true;
	    break;
	  case '{':
	    {
	      char *xtag = src->read_until('}');

	      if(xtag)
	      {
		// If the tag isn't the final closing tag, we need to
		// echo it to the output.
		
		bool echotag = true;
		
		if((nested &&
		    strncmp(xtag, nested->closetag,
			    strlen(nested->closetag))==0)
		   ||
		   (togo == 1 &&
		    strncmp(xtag, tag->closetag, strlen(tag->closetag))==0))
		{
		  togo--;

		  if(togo == 0)
		  {
		    endseen = true;
		    echotag = false;
		  }
		}
		else if(nested &&
			strncmp(xtag, nested->name,
				strlen(nested->name))==0)
		{
		  // This is a new open tag, so we need to see one more
		  // close tag before we're finished.

		  togo++;
		}

		if(echotag)
		{
		  // This isn't the closing tag, so copy it, including
		  // the delimiters we've consumed
		  
		  ret->append("@{");
		  ret->append(xtag);
		  ret->append('}');
		}
	      }
	      else
		endseen = true;
	    }
	    break;
	  default:
	    // Just a @ in some text, copy and keep going
	    ret->append('@');
	    ret->append(c);
	    break;
	  }
	}
      }
    }
    else
      wlog->warn("PageFilter::find_close_tag failed to allocate CharBuffer");
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "PageFilter::find_close_tag = %d", ret);
#endif

  return(ret);
}

bool PageFilter::get_calllist_info()
{
  // Obtain all calllist information from all instances, and store it
  // in clists and addresses.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "PageFilter::get_calllist_info()");
#endif

  if(!clists && !addresses)
  {
    clists = new List();
    addresses = new List();

    // We need to copy this List since we're about to delete it
    List *instances = new List(args->instances());

    if(clists && addresses && instances)
    {
      // Assume success unless failure noted

      ret = true;
      
      for(int i = 0;i < instances->entries();i++)
      {
	// The Configuration object uses args to determine which
	// configuration directory to use.  When this method is
	// accessed, args has been allocated, but nothing has been
	// done with it.  So we cycle through each instance name,
	// parsing its configuration and storing its addresses.

	if(args && instances->retrieve(i))
	{
	  args->parse_args(instances->retrieve(i), (char *)NULL,
			   (char *)NULL);
		
	  if(args->parse_instcf())
	  {
	    cf = new Configuration();
		  
	    if(cf && cf->parse_cfs())
	    {
	      Array<CallList> *cla = cf->get_all_calllists();
		    
	      if(cla)
	      {
		CharBuffer *cb = new CharBuffer();

		if(cb)
		{		
		  for(int j = 0;j < cla->entries();j++)
		  {
		    CallList *cl = cla->retrieve(j);
		    
		    if(cl && cl->name() && cl->module()
		       && cl->module()->transmit())
		    {
		      cb->append(instances->retrieve(i));
		      cb->append(':');
		      cb->append(cl->name());
		      cb->append(':');
		      cb->append(cl->module()->transmit());
		      
		      clists->add(cb->str());
		      cb->clear();
		    }

		    Array<Person> *persons = cl->members();
			  
		    if(persons)
		    {
		      for(int k = 0;k < persons->entries();k++)
		      {
			Person *p = persons->retrieve(k);
			
			if(p && cl->module()->name())
			{
			  char *pa = p->find_address(cl->module()->name());

			  if(pa)
			  {
			    cb->append(instances->retrieve(i));
			    cb->append(':');
			    cb->append(cl->name());
			    cb->append(':');
			    cb->append(pa);
			    cb->append(':');
			    cb->append(cl->module()->transmit());
			    
			    addresses->add(cb->str());
			    cb->clear();
			  }
			}
		      }
		    }
		  }

		  xdelete(cb);
		}
		else
		{
		  wlog->warn("PageFilter::get_calllist_info() failed to allocate CharBuffer");
		  ret = false;
		}
	      }
	    }
	    else
	      ret = false;

	    xdelete(cf);
	  }
	  else
	    ret = false;
	}
	else
	  ret = false;
      }

      // Restore an empty args
      delete args;
      args = new Args();
    }
    else
      ret = false;

    xdelete(instances);
  }
  else
    ret = true;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "PageFilter::get_calllist_info = %s",
		 IOTF(ret));
#endif

  return(ret);
}

bool PageFilter::retrieve_state(char *service, char *host)
{
  // Obtain the state for <service>@<host>.  This method will cache
  // State objects for better performance over repeated accesses.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "PageFilter::retrieve_state(%s,%s)",
		  IONULL(service), IONULL(host));
#endif

  if(service && host)
  {
    // First, check the cache.

    if(!shpcache || strcmp(shpcache->host(), host)!=0
       || strcmp(shpcache->service(), service)!=0)
    {
      // Clear the cache, we missed
      uncache_state();
      
      Check *c = cf->find_check(service);
      List *g = cf->find_group(service);

      if(c && g && g->find(host)!=-1)
      {
	ascache = new AlertState(c, host);
	cscache = new CheckState(c, host);
	fscache = new FixState(c, host);
	shpcache = new SHPair(service, host);

	if(ascache && cscache && fscache && shpcache)
	  ret = true;
	else
	{
	  wlog->warn("PageFilter::retrieve_state failed to allocate state");

	  uncache_state();
	}
      }
      // Else not a warning since we'll be asked for a lot of stuff that
      // doesn't exist.
    }
    else
    {
      // Return the cached entry
      ret = true;
    }
  }
  else
    uncache_state();
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "PageFilter::retrieve_state = %s", IOTF(ret));
#endif
  
  return(ret);
}

void PageFilter::uncache_state()
{
  // Uncache any state information.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "PageFilter::uncache_state()");
#endif

  xdelete(ascache);
  xdelete(cscache);
  xdelete(fscache);
  xdelete(shpcache);
		      
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "PageFilter::uncache_state()");
#endif
}
