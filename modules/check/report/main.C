/*
 * report: A module for long term trend monitoring via report modules
 *
 * Version: $Revision: 0.1 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2005/11/14 03:08:03 $
 * MT-Level: Safe
 *
 * Copyright (c) 2005
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 */

#include "survivor.H"

// Required to use libcm
CMArgs *cmargs = NULL;

// Define our named argument structures
ArgSpec modulearg = {
  "module",       // arg name
  string_arg,     // arg type
  NULL,           // any list
  INFINITY_ARG,   // between lower bound
  INFINITY_ARG,   // between upper bound
  false,          // expect a list
  NULL,           // one list
  false,          // optional
  NULL            // default value
};

DefaultArg idefault = { false, "default", NULL, 0, NULL, no_rel, NULL, NULL };

ArgSpec instancearg = {
  "instance",     // arg name
  string_arg,     // arg type
  NULL,           // any list
  INFINITY_ARG,   // between lower bound
  INFINITY_ARG,   // between upper bound
  false,          // expect a list
  NULL,           // one list
  true,           // optional
  &idefault       // default value
};

ArgSpec servicearg = {
  "service",      // arg name
  string_arg,     // arg type
  NULL,           // any list
  INFINITY_ARG,   // between lower bound
  INFINITY_ARG,   // between upper bound
  false,          // expect a list
  NULL,           // one list
  false,          // optional
  NULL            // default value
};

char *sources[] = { "alert", "check", "command", "fix", NULL };

ArgSpec sourcearg = {
  "source",       // arg name
  string_arg,     // arg type
  sources,        // any list
  INFINITY_ARG,   // between lower bound
  INFINITY_ARG,   // between upper bound
  false,          // expect a list
  NULL,           // one list
  false,          // optional
  NULL            // default value
};

ArgSpec recordspanarg = {
  "recordspan",   // arg name
  relation_arg,   // arg type
  NULL,           // any list
  INFINITY_ARG,   // between lower bound
  INFINITY_ARG,   // between upper bound
  false,          // expect a list
  NULL,           // one list
  true,           // optional
  NULL            // default value
};

ArgSpec warnarg = {
  "warn",         // arg name
  relation_arg,   // arg type
  NULL,           // any list
  INFINITY_ARG,   // between lower bound
  INFINITY_ARG,   // between upper bound
  false,          // expect a list
  NULL,           // one list
  true,           // optional
  NULL            // default value
};

ArgSpec problemarg = {
  "problem",      // arg name
  relation_arg,   // arg type
  NULL,           // any list
  INFINITY_ARG,   // between lower bound
  INFINITY_ARG,   // between upper bound
  false,          // expect a list
  NULL,           // one list
  true,           // optional
  NULL            // default value
};

ArgSpec extractionarg = {
  "extraction",   // arg name
  extraction_arg, // arg type
  NULL,           // any list
  INFINITY_ARG,   // between lower bound
  INFINITY_ARG,   // between upper bound
  false,          // expect a list
  NULL,           // one list
  true,           // optional
  NULL            // default value
};

ArgSpec tmpdirarg = {
  "tmpdir",       // arg name
  directory_arg,  // arg type
  NULL,           // any list
  INFINITY_ARG,   // between lower bound
  INFINITY_ARG,   // between upper bound
  false,          // expect a list
  NULL,           // one list
  true,           // optional
  NULL            // default value
};

// And put them in a NULL terminated array
ArgSpec *argfmt[] = {
  &modulearg,
  &instancearg,
  &servicearg,
  &sourcearg,
  &recordspanarg,
  &warnarg,
  &problemarg,
  &extractionarg,
  &tmpdirarg,
  NULL
};

CheckResult *validate()
{
  CheckResult *rv = new CheckResult(MODEXEC_OK, 0, "Report OK");

  return(rv);
}

int main(int argc, char **argv)
{
  // We don't actually use timeout

  int r = MODEXEC_OK;  

  if(!libsrvinit(argv[0]))
    exit(MODEXEC_MISCONFIG);
    
  cmargs = new CMArgs();

  if(cmargs)
  {
    CheckResult *rv = cmargs->parse_args(argc, argv, argfmt, validate);

    if(rv)
    {
      if(rv->rc() == MODEXEC_OK && !cmargs->validating())
      {
	if(cmargs->hosts())
	{
	  StringArg *marg = (StringArg *)cmargs->arg("module");
	  StringArg *iarg = (StringArg *)cmargs->arg("instance");
	  StringArg *svarg = (StringArg *)cmargs->arg("service");
	  StringArg *sarg = (StringArg *)cmargs->arg("source");
	  DirectoryArg *darg = (DirectoryArg *)cmargs->arg("tmpdir");
	  RelationArg *rsarg = (RelationArg *)cmargs->arg("recordspan");
	  RelationArg *warg = (RelationArg *)cmargs->arg("warn");
	  RelationArg *parg = (RelationArg *)cmargs->arg("problem");
	  ExtractionArg *earg = (ExtractionArg *)cmargs->arg("extraction");

	  if(iarg && iarg->value())
	  {
	    // Parse the instance, ui_execute_report will need it.
	    // libsrvexit will clean this up.

	    args = new Args();

	    if(args
	       && args->parse_args(iarg->value(), (char *)NULL, (char *)NULL)
	       && args->parse_instcf())
	    {	  
	      if(svarg && svarg->value() && sarg && sarg->value()
		 && marg && marg->value())
	      {
		CharBuffer *cb = new CharBuffer();
		List *svc = new List();
		ReportFormatting *rf = NULL;
		char *rerr = NULL;
		
		if(darg && darg->value())
		  rf = new ReportFormatting(marg->value(), "check",
					    darg->value());
		else
		  rf = new ReportFormatting(marg->value(), "check");
		
		if(cb && svc && rf)
		{
		  // Add the service to the list
		  svc->add(svarg->value());
	      
		  // Determine which history to look at
		  int which = 0;

		  if(strcmp(sarg->value(), "alert")==0)
		    which = UI_REPORT_ALERT;
		  else if(strcmp(sarg->value(), "check")==0)
		    which = UI_REPORT_CHECK;
		  else if(strcmp(sarg->value(), "command")==0)
		    which = UI_REPORT_COMMAND;
		  else if(strcmp(sarg->value(), "fix")==0)
		    which = UI_REPORT_FIX;

		  // Determine time boundaries, if specified
		  time_t f = 0;
		  time_t u = 0;
		  bool b = false;

		  if(rsarg)
		  {
		    struct timeval tp;

		    gettimeofday(&tp, NULL);

		    if(rsarg->relation() == tn_rel)
		    {
		      // Look at records from now-x until now.  Go backwards
		      // as an efficiency hack.

		      f = tp.tv_sec - (time_t)rsarg->x();
		      b = true;
		    }
		    else if(rsarg->relation() == to_rel)
		    {
		      // Look at records from the beginning until now-x.

		      u = tp.tv_sec - (time_t)rsarg->x();
		    }
		  }
		  
		  if(ui_execute_report(cb, rf, cmargs->hosts(), svc, which,
				       f, u, b, &rerr))
		  {
		    SurvivorXML *sxml = new SurvivorXML();

		    if(sxml)
		    {
		      // Each line in cb is of the form
		      //  service host <some output>

		      // We should but don't track that we got exactly one
		      // line of output per service@host pair.  We do check
		      // that service is the expected service.

		      char *l;
		      
		      while((l = cb->read_line()) != NULL)
		      {
			// Tokenize l into the above form
			
			List *ls = tokenize(l, " ", 3);
			CheckResult *cr = NULL;
			
			if(ls && ls->entries() == 3 && ls->retrieve(0)
			   && strcmp(ls->retrieve(0), svarg->value())==0)
			{
			  char *d = NULL;   // The string to compare
			  char *xs = NULL;  // Temporary string (to delete)
			  
			  if(earg)
			  {
			    // Perform extraction

			    xs = cm_perform_extraction(earg, ls->retrieve(2));
			    d = xs;
			  }
			  else
			    d = ls->retrieve(2);
			  
			  if(d)
			  {
			    // cm_relation_compare will handle numerical
			    // conversions for us

			    bool resmatch = false;
			    char *rctxt = NULL;

			    if(parg)
			    {
			      // If problem relation matches, use it

			      if(cm_relation_compare(parg, d, NULL, &rctxt))
			      {
				resmatch = true;

				cr = new CheckResult(ls->retrieve(1),
						     MODEXEC_PROBLEM,
						     0,
						     rctxt);
			      
				r = compose_rc(r, MODEXEC_PROBLEM, true);
			      }
			    }

			    if(!resmatch && warg)
			    {
			      // Else if warning relation matches, use it

			      if(cm_relation_compare(warg, d, NULL, &rctxt))
			      {
				resmatch = true;

				cr = new CheckResult(ls->retrieve(1),
						     MODEXEC_WARNING,
						     0,
						     rctxt);
			      
				r = compose_rc(r, MODEXEC_WARNING, true);
			      }
			    }

			    if(!resmatch && !parg && !warg)
			    {
			      // Return NOTICE if no warning or problem
			      // criteria were provided

			      cr = new CheckResult(ls->retrieve(1),
						   MODEXEC_NOTICE,
						   0,
						   d);
			      
			      resmatch = true;
			      
			      r = compose_rc(r, MODEXEC_NOTICE, true);
			    }

			    if(!resmatch)
			    {
			      // Return OK otherwise

			      cr = new CheckResult(ls->retrieve(1),
						   MODEXEC_OK,
						   0,
						   d);
			      
			      r = compose_rc(r, MODEXEC_OK, true);
			    }

			    xdelete(rctxt);
			  }
			  else
			  {
			    // Didn't find matching output text
			    
			    cr = new CheckResult(ls->retrieve(1),
						 MODEXEC_MISCONFIG,
						 0,
						 "No output received");
			  }

			  if(cr)
			  {
			    // Dump result

			    sxml->generate(STDOUT_FILENO, cr);

			    xdelete(cr);
			  }

			  xdelete(xs);
			  d = NULL;
			}
			else
			{
			  // The module should only emit lines for the
			  // service and hosts we expect, so choke on
			  // anything else.
			  
			  r = MODEXEC_MISCONFIG;
			  cm_error(r, 0, l);
			}
			
			xdelete(ls);
		      }

		      xdelete(sxml);
		    }
		    // else no point in reporting error
		  }
		  else
		  {
		    r = MODEXEC_MISCONFIG;
		    cm_error(r, 0, rerr);
		  }
		}
		else
		{
		  r = MODEXEC_MISCONFIG;
		  cm_error(r, 0, "Failed to allocate structures");
		}
	    
		xdelete(cb);
		xdelete(svc);
		xdelete(rf);
		xdelete(rerr);
	      }
	    }
	    else
	    {
	      r = MODEXEC_MISCONFIG;
	      cm_error(r, 0, "Failed to parse args/instance");
	    }
	  }
	}
	else  // No point calling cm_error with no hosts
	  r = MODEXEC_MISCONFIG;
      }
      else
      {
	r = rv->rc();
	cm_error(r, 0, rv->comment());
      }

      xdelete(rv);
    }
    else
    {
      r = MODEXEC_MISCONFIG;
      cm_error(r, 0, "Failed to parse named args");
    }
    
    xdelete(cmargs);
  }
  else  // No point calling cm_error with no hosts
    r = MODEXEC_PROBLEM;

  libsrvexit();

  exit(r);
}
