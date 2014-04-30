/*
 * CMArgs: Check Module arg parser
 *
 * Version: $Revision: 0.19 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2007/01/25 13:05:44 $
 * MT-Level: Safe, if parse_args is only called once.
 *
 * Copyright (c) 2002 - 2007
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: CMArgs.C,v $
 * Revision 0.19  2007/01/25 13:05:44  benno
 * Build correctly with debugging disabled
 *
 * Revision 0.18  2005/11/14 03:06:11  benno
 * Add debugfile/debugsyslog support
 * Cleanup relation handling
 *
 * Revision 0.17  2005/04/30 21:39:23  benno
 * Use sxml->set_read_timeout
 *
 * Revision 0.16  2005/04/09 02:42:49  benno
 * Changes for XML based args
 *
 * Revision 0.15  2004/09/09 12:44:33  benno
 * Remove debugging
 *
 * Revision 0.14  2004/06/12 01:01:31  benno
 * Add support for ExtractionArg
 *
 * Revision 0.13  2003/09/26 03:32:25  selsky
 * Allow 'file' argument type to include symlinks, pipes, and sockets.
 *
 * Revision 0.12  2003/05/04 21:36:02  benno
 * Don't use string type
 *
 * Revision 0.11  2003/04/09 20:04:14  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.10  2003/04/07 16:28:02  benno
 * Use Debugger
 *
 * Revision 0.9  2003/03/04 20:53:26  benno
 * Bump copyright
 *
 * Revision 0.8  2003/01/24 02:41:42  benno
 * Use IONULL, IOTF, xdelete, xadelete
 *
 * Revision 0.7  2002/12/31 04:37:55  benno
 * Add unparsed support for transport modules (== syntax)
 *
 * Revision 0.6  2002/12/16 00:53:11  benno
 * Change lists from comma separated to multiple name=value pairs
 *
 * Revision 0.5  2002/12/05 19:02:04  benno
 * add support for default args
 *
 * Revision 0.4  2002/10/21 20:44:22  benno
 * add support for named args
 *
 * Revision 0.3  2002/04/04 20:01:06  benno
 * copyright
 *
 * Revision 0.2  2002/04/02 22:46:33  benno
 * rcsify date
 *
 * Revision 0.1  2002/04/02 22:46:09  benno
 * initial revision
 *
 */

#include "survivor.H"

CMArgs::CMArgs()
{
  // Allocate and initialize a new CMArgs object.

  // Returns: A new CMArgs object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CMArgs::CMArgs()");
#endif

  h = NULL;
  t = DEFAULT_TIMEOUT;
  v = false;
  ht = NULL;
  // Really for TMArgs
  tm = false;
  rmargs = NULL;
  mod = NULL;
  mtype = NULL;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CMArgs::CMArgs()");
#endif
}

ArgParsed *CMArgs::arg(char *name)
{
  // Obtain the parsed argument named <name>.

  // Returns: A pointer to the parsed argument object, or NULL.

  ArgParsed *ret = NULL;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CMArgs::arg(%s)", IONULL(name));
#endif

  if(ht && name)
    ret = ht->retrieve(name);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CMArgs::arg = %d", ret);
#endif

  return(ret);
}

List *CMArgs::hosts()
{
  // Obtain the list of hosts specified, if provided.

  // Returns: The list of hosts, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CMArgs::hosts()");
  dlog->log_exit(DEBUG_MINTRC, "CMArgs::hosts = %d", h);
#endif
  
  return(h);
}

CheckResult *CMArgs::parse_args(int argc, char **argv, ArgSpec **argfmt,
				CheckResult *(*validate)())
{
  // Parse the arguments provided in <argc> and <argv> and via a
  // SurvivorCheckData document provided on stdin, and verify
  // them against the specification <argfmt>.  <validate> is a
  // function to call to make sure the Check is properly installed.
  // The expected optarg string is "d:v" (d = debuglevel).

  // Returns: A newly allocated CheckResult that should be deleted when
  // no longer required and that contains the results of the parse, or
  // NULL on non-parsing error.

  CheckResult *ret = NULL;
  bool ok = true;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "CMArgs::parse_args(%d,%d,%d,%s)",
		  argc, argv, argfmt, (validate ? "<validate>" : "(null)"));
#endif
  
  if(argc > 0 && argv)
  {
    int c;
    extern char *optarg;
    extern int optind;

    // First look at command line
    
    while((c = getopt(argc, argv,
#if defined(DEBUG)
		      "d:v"
#else
		      "v"
#endif
		      )) != EOF)
    {
      switch(c)
      {
#if defined(DEBUG)
      case 'd':
	dlog->set_level(atoi(optarg));
	break;
#endif
      case 'v':
	v = true;
	break;
      default:
	ok = false;
	break;
      }
    }
  }
  else
    ok = false;

  if(ok)
  {
    if(v)
    {
      // Perform validation rather than parse the named args, since validation
      // is not dependent on knowing what arguments to pass.  We also exit
      // here since an OK result would look like an OK parse to the caller.

      CheckResult *rv = validate();
      int rc = MODEXEC_MISCONFIG;

      if(rv)
      {
	cm_error(rv->rc(), rv->scalar(), rv->comment());
	rc = rv->rc();

	delete rv;
	rv = NULL;
      }
      else
	cm_error(MODEXEC_MISCONFIG, 0,
		 "Module did not return expected validation");

      exit(rc);
    }
    else
    {
      // If we weren't given an argfmt, don't bother trying to parse
      // any named args.

      if(argfmt)
	ret = parse_named_args(argfmt);
      else
	ret = new CheckResult(MODEXEC_OK, 0, "getopt successful");
    }
  }
 
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "CMArgs::parse_args = %d", ret);
#endif
  
  return(ret);
}

int CMArgs::timeout()
{
  // Obtain the timeout, if provided.

  // Returns: The timeout, or DEFAULT_TIMEOUT.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CMArgs::timeout()");
  dlog->log_exit(DEBUG_MINTRC, "CMArgs::timeout = %d", t);
#endif
  
  return(t);
}

bool CMArgs::validating()
{
  // Determine if the module should be validating instead of checking.

  // Returns: true if the validating flag was present, false otherwise.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CMArgs::validating()");
  dlog->log_exit(DEBUG_MINTRC, "CMArgs::validating = %s", IOTF(v));
#endif
  
  return(v);
}

CMArgs::~CMArgs()
{
  // Deallocate the CMArgs object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CMArgs::~CMArgs()");
#endif

  tm = false;
  xadelete(rmargs, Argument);
  xdelete(mod);
  xdelete(mtype);
  
  xdelete(h);
  t = 0;
  v = false;

  if(ht)
  {
    // Iterate through the table and delete each item
    HashHandle *hh = ht->iterate_begin();

    if(hh)
    {
      for(ArgParsed *arg = (ArgParsed *)ht->iterate_next(hh);
	  arg != NULL;
	  arg = (ArgParsed *)ht->iterate_next(hh))
      {
	switch(arg->argtype())
	{
	case boolean_arg:
	  delete (BooleanArg *)arg;
	  break;
	case extraction_arg:
	  delete (ExtractionArg *)arg;
	  break;
	case flag_arg:
	  delete (FlagArg *)arg;
	  break;
	case number_arg:
	  delete (NumberArg *)arg;
	  break;
	case relation_arg:
	  delete (RelationArg *)arg;
	  break;
	case directory_arg:
	case file_arg:
	case password_arg:
	case string_arg:
	  delete (StringArg *)arg;
	  break;
	default:
	  wlog->warn("CMArgs::~CMArgs reached unknown argument type");
	  break;
	}
      }
      
      ht->iterate_end(hh);

      // Then delete the table

      xdelete(ht);
    }
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CMArgs::~CMArgs()");
#endif
}

CheckResult *CMArgs::parse_named_args(ArgSpec **argfmt)
{
  // Parse the SurvivorCheckData document provided on stdin into a set
  // of named arguments as specified by <argfmt>.  If this method is
  // called from within a TMArgs object, a SurvivorTransportData
  // document will be expected instead.

  // Returns: A newly allocated CheckResult that should be deleted when no
  // longer required and that contains the results of the parse, or NULL
  // on non-parsing error.

  CheckResult *ret = new CheckResult(MODEXEC_OK, 0, "parse successful");
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "CMArgs::parse_named_args(%d)", argfmt);
#endif

  if(ret)
  {
    char *err = NULL;
    
    if(argfmt)
    {
      // Make sure we have a hashtable

      if(ht)
	err = xstrcat(err, "parser already run,");
      else
	ht = new Hashtable<ArgParsed>();

      if(!ht)
	err = xstrcat(err, "failed to allocate hashtable,");

      if(ht)
      {
	SurvivorXML *sxml = new SurvivorXML();

	if(sxml)
	{
	  // We need modargs to remain valid long enough to validate it
	  CheckRequest *creq = NULL;
	  TransportRequest *treq = NULL;
	  Array<Argument> *modargs = NULL;

	  // On some platforms (eg: Solaris 9), close() doesn't
	  // reliably send EOF, so we hang on reading the XML doc.  We
	  // set a one second timeout as a workaround (note this uses
	  // alarm()).

	  sxml->set_read_timeout(1);
	  
	  if(tm)
	  {
	    // We expect a TransportRequest

	    treq = sxml->parse_transportrequest(STDIN_FILENO);

	    if(treq)
	    {
	      // Copy what we need out of treq

	      mod = xstrdup(treq->module());
	      mtype = xstrdup(treq->modtype());
	      h = new List(treq->hosts());
	      t = treq->timeout();

	      if(treq->rmodargs())
	      {
		rmargs = new Array<Argument>();

		if(rmargs)
		{
		  for(int i = 0;i < treq->rmodargs()->entries();i++)
		  {
		    Argument *a = treq->rmodargs()->retrieve(i);

		    Argument *a2 = new Argument(a->name(), a->value(), true);

		    if(a2)
		    {
		      if(!rmargs->add(a2))
		      {
			xdelete(a2);
		      }
		    }
		    else
		      err = xstrcat(err, "Memory allocation error,");
		  }
		}
	      }
	      else
		rmargs = NULL;

	      modargs = treq->modargs();
	    }
	    else
	      err = xstrcat(err, "failed to parse SurvivorTransportRequest,");
	  }
	  else
	  {
	    // We expect a CheckRequest
	    
	    creq = sxml->parse_checkrequest(STDIN_FILENO);

	    if(creq)
	    {
	      // Copy what we need out of creq

	      h = new List(creq->hosts());
	      t = creq->timeout();
	      modargs = creq->modargs();
	    }
	    else
	      err = xstrcat(err, "failed to parse SurvivorCheckRequest,");
	  }

	  if(modargs)
	  {
	    for(int j = 0;j < modargs->entries();j++)
	    {
	      Argument *a = modargs->retrieve(j);

	      if(a && a->name() && a->value())
	      {
		// Validate the argument

		int i = 0;
		bool found = false;

		char *name = a->name();   // We're reusing old code here,
		char *value = a->value(); // so define the old names

		for(ArgSpec *arg = argfmt[i];
		    arg != NULL && !found;
		    i++,arg = argfmt[i])
		{
		  if(arg->argname && strcmp(name, arg->argname)==0)
		  {
		    found = true;
		      
		    // See if we've already seen an arg by this name
		    ArgParsed *ap = ht->retrieve(name);
		      
		    if(ap && !arg->list)
		    {
		      err = xstrcat(err, "Argument '");
		      err = xstrcat(err, name);
		      err = xstrcat(err, "' does not permit multiple values,");
		    }
		    else
		    {
		      switch(arg->argtype)
		      {
		      case boolean_arg:
			{
			  BooleanArg *b = NULL;
			    
			  // Value must be appropriate
			  if(strcasecmp(value, "true")==0 ||
			     strcasecmp(value, "t")==0 ||
			     strcasecmp(value, "yes")==0 ||
			     strcasecmp(value, "y")==0 ||
			     strcmp(value, "1")==0)
			    b = new BooleanArg(name, true);
			  else if(strcasecmp(value, "false")==0 ||
				  strcasecmp(value, "f")==0 ||
				  strcasecmp(value, "no")==0 ||
				  strcasecmp(value, "n")==0 ||
				  strcmp(value, "0")==0)
			    b = new BooleanArg(name, false);
			  else
			  {
			    err = xstrcat(err, "Value '");
			    err = xstrcat(err, value);
			    err = xstrcat(err, "' for argument ");
			    err = xstrcat(err, name);
			    err = xstrcat(err, " is not boolean,");
			  }
			    
			  if(b)
			    ht->insert(b->name(), b);
			}
			break;
		      case extraction_arg:
			{
			  ExtractionArg *e = NULL;
			  
			  if(strlen(value) > 8)
			  {
			    // Parse out the operation
			      
			    extraction_t xt = no_ext;
			      
			    if(strncmp(value, "column[", 7)==0)
			      xt = column_ext;
			    else if(strncmp(value, "substr[", 7)==0)
			      xt = substr_ext;
			    else
			    {
			      err = xstrcat(err, "Invalid extraction '");
			      err = xstrcat(err, value);
			      err = xstrcat(err, ",' for argument ");
			      err = xstrcat(err, name);
			      err = xstrcat(err, ",");
			    }
			      
			    // Validate number of parameters
			      
			    switch(xt)
			    {
			    case substr_ext:
			      if(strchr(value+7, ','))
			      {
				// Parse out the two parameters
				
				char *last3;
				char *n = strtok_r(value+7, ",", &last3);
				
				if(n)
				{
				  int x = atoi(n);
				  n = strtok_r(NULL, "]", &last3);
				  
				  if(n)
				  {
				    int y = atoi(n);
				    e = new ExtractionArg(name, xt, x, y);
				  }
				}
			      }
			      else
			      {
				err = xstrcat(err, "Extraction '");
				err = xstrcat(err, value);
				err = xstrcat(err, "' for argument ");
				err = xstrcat(err, name);
				err = xstrcat(err, " requires two values,");
			      }
			      break;
			    default:
			      // These take only one parameter.
			      {
				char *p = strchr(value, '[');
				
				if(p)
				{
				  p++;
				  
				  char *q = strchr(p, ']');
				    
				  if(q)
				    *q = '\0';
				  // else ignore missing close brace
				    
				  e = new ExtractionArg(name, xt, atoi(p));
				    
				  if(q)  // restore it to be nice
				    *q = ']';
				}
				// We already know [ is there, don't bother
				// with error checking
			      }
			      break;
			    }
			  }
			  else
			  {
			    err = xstrcat(err, "Value '");
			    err = xstrcat(err, value);
			    err = xstrcat(err, "' for argument ");
			    err = xstrcat(err, name);
			    err = xstrcat(err, " is not permitted,");
			  }
			
			  if(e)
			    ht->insert(e->name(), e);
			}
			break;
		      case flag_arg:
			{
			  FlagArg *f = NULL;
			    
			  // The characters in are just accepted, unless
			  // any is defined
			    
			  if(arg->any)
			  {
			    for(int j = 0;j < strlen(value);j++)
			    {
			      // Check each character
			      
			      bool found = false;
			      int i = 0;
				
			      for(char *a = arg->any[i];
				  a != NULL && !found;
				  i++,a = arg->any[i])
			      {
				if(a[0] == value[j])
				  found = true;
			      }
				
			      if(!found)
			      {
				char buf1[2];
				buf1[0] = value[j];
				buf1[1] = '\0';
				
				err = xstrcat(err, "Value '");
				err = xstrcat(err, buf1);
				err = xstrcat(err, "' for argument ");
				err = xstrcat(err, name);
				err = xstrcat(err, " is not permitted,");
			      }
			    }
			  }
			    
			  f = new FlagArg(name, value);
			    
			  if(f)
			    ht->insert(f->name(), f);
			}
			break;
		      case number_arg:
			{
			  NumberArg *n = NULL;
			    
			  if(arg->list)
			  {
			    char *vf = validate_float(arg, value, true);

			    if(vf)
			    {
			      err = xstrcat(err, vf);
			      xdelete(vf);
			    }
			      
			    float *f = new float;
			      
			    if(f)
			    {
			      *f = atof(value);
				
			      if(ap)
			      {
				// Add this value to the existing Array
				Array<float> *values =
				  ((NumberArg *)ap)->values();
				  
				if(values)
				{
				  if(!values->add(f))
				  {
				    err = xstrcat(err,
						  "Failed to add f to Array,");
				      
				    xdelete(f);
				  }
				}
				else
				  err = xstrcat(err,
						"Failed to retrieve Array,");
			      }
			      else
			      {
				// Allocate a new Array containing
				// this value
				  
				Array<float> *values = new Array<float>();
			      
				if(values)
				{
				  if(values->add(f))
				  {
				    n = new NumberArg(name, values);
				    
				    if(!n)
				    {
				      err = xstrcat(err,
						    "Failed to allocate NumberArg,");

				      xdelete(f);
				      xdelete(values);
				    }
				  }
				  else
				  {
				    err = xstrcat(err,
						  "Failed to add f to Array,");
				      
				    xdelete(f);
				    xdelete(values);
				  }
				}
				else
				  err = xstrcat(err,
						"Failed to allocate Array,");
			      }
			    }
			    else
			      err = xstrcat(err, "Failed to allocate float,");
			  }
			  else
			  {
			    // Single item
			    
			    char *vf = validate_float(arg, value, false);

			    if(vf)
			    {
			      err = xstrcat(err, vf);
			      xdelete(vf);
			    }
			      
			    n = new NumberArg(name, atof(value));
			  }
			    
			  // n will be NULL if we are adding another value
			  // to an existing array
			    
			  if(n)
			    ht->insert(n->name(), n);
			}
			break;
		      case relation_arg:
			{
			  RelationArg *r = NULL;
			    
			  if(strlen(value) > 4)
			  {
			    // Parse out the operation
			    
			    relation_t rt = bt_rel;
			      
			    if(strncmp(value, "eq[", 3)==0)
			      rt = eq_rel;
			    else if(strncmp(value, "gt[", 3)==0)
			      rt = gt_rel;
			    else if(strncmp(value, "lt[", 3)==0)
			      rt = lt_rel;
			    else if(strncmp(value, "ne[", 3)==0)
			      rt = ne_rel;
			    else if(strncmp(value, "bt[", 3)==0)
			      rt = bt_rel;
			    else if(strncmp(value, "nb[", 3)==0)
			      rt = nb_rel;
			    else if(strncmp(value, "reg[", 4)==0)
			      rt = re_rel;
			    else if(strncmp(value, "regv[", 5)==0)
			      rt = rv_rel;
			    else if(strncmp(value, "tnt[", 4)==0)
			      rt = tn_rel;
			    else if(strncmp(value, "tot[", 4)==0)
			      rt = to_rel;
			    else
			    {
			      err = xstrcat(err, "Invalid relation '");
			      err = xstrcat(err, value);
			      err = xstrcat(err, ",' for argument ");
			      err = xstrcat(err, name);
			      err = xstrcat(err, ",");
			    }
			      
			    // Validate number of parameters
			      
			    switch(rt)
			    {
			    case bt_rel:
			    case nb_rel:
			      if(strchr(value+3, ','))
			      {
				// Parse out the two parameters
				  
				char *last3;
				char *x = strtok_r(value+3, ",", &last3);
				  
				if(x)
				{
				  char *y = strtok_r(NULL, "]", &last3);
				    
				  if(y)
				    r = new RelationArg(name, rt, x, y);
				}
			      }
			      else
			      {
				err = xstrcat(err, "Relation '");
				err = xstrcat(err, value);
				err = xstrcat(err, "' for argument ");
				err = xstrcat(err, name);
				err = xstrcat(err, " requires two values,");
			      }
			      break;
			    default:
			      // The rest take only one parameter.
			      {
				char *p = strchr(value, '[');
				
				if(p)
				{
				  p++;

				  if(rt == re_rel || rt == rv_rel
				     || !strchr(value+3, ','))
				  {
				    char *q = strchr(p, ']');
				    
				    if(q)
				      *q = '\0';
				    // else ignore missing close brace
				    
				    r = new RelationArg(name, rt, p);
				  
				    if(q)  // restore it to be nice
				      *q = ']';
				  }
				  else
				  {
				    err = xstrcat(err, "Relation '");
				    err = xstrcat(err, value);
				    err = xstrcat(err, "' for argument ");
				    err = xstrcat(err, name);
				    err = xstrcat(err, " requires one value,");
				  }
				}
				// We already know [ is there, don't bother
				// with error checking
			      }
			      break;
			    }
			  }
			  else
			  {
			    err = xstrcat(err, "Value '");
			    err = xstrcat(err, value);
			    err = xstrcat(err, "' for argument ");
			    err = xstrcat(err, name);
			    err = xstrcat(err, " is not permitted,");
			  }
			
			  if(r)
			    ht->insert(r->name(), r);
			}
			break;
		      case directory_arg:
		      case file_arg:
		      case password_arg:
		      case string_arg:
			{
			  StringArg *s = NULL;
			    
			  if(arg->list)
			  {
			    char *vs = validate_string(arg, value, true);

			    if(vs)
			    {
			      err = xstrcat(err, vs);
			      xdelete(err);
			    }
			      
			    if(ap)
			    {
			      // Add this value to the existing List
			      List *values = ((StringArg *)ap)->values();
				
			      if(values)
			      {
				if(!values->add(value))
				  err = xstrcat(err,
						"Failed to add item to List,");
			      }
			      else
				err = xstrcat(err, "Failed to retrieve List,");
			    }
			    else
			    {
			      // Allocate a new List containing this value
				
			      List *values = new List();
				
			      if(values)
			      {
				if(values->add(value))
				{
				  s = new StringArg(name, values);
				    
				  if(!s)
				  {
				    err = xstrcat(err,
						  "Failed to allocate StringArg,");
				      
				    xdelete(values);
				  }
				}
				else
				{
				  err = xstrcat(err,
						"Failed to add item to List,");
				
				  xdelete(values);
				}
			      }
			      else
				err = xstrcat(err, "Failed to allocate List,");
			    }
			  }
			  else
			  {
			    // Single item
			    
			    char *vs = validate_string(arg, value, false);
			    
			    if(vs)
			    {
			      err = xstrcat(err, vs);
			      xdelete(vs);
			    }
			      
			    s = new StringArg(name, value);
			  }
			    
			  // If we have a new StringArg, add it
			  
			  if(s)
			    ht->insert(s->name(), s);
			}
			break;
		      default:
			err = xstrcat(err, "Unknown argument type,");
			break;
		      }
		    }
		  }
		}

#if defined(DEBUG)
		if(!found)
		{
		  if(strcmp(name, "debugfile")==0
		     || strcmp(name, "debugsyslog")==0)
		  {
		    // Special debugging arguments, valid in all
		    // modules.  Since dlog is allocated by default by
		    // libsrvinit() as a StdDebugger, we need to toss
		    // and recreate it.  This also makes it harder to
		    // check if debugfile or debugsyslog was already
		    // specified, so for now we don't.

		    Debugger *newdlog = NULL;

		    if(strcmp(name, "debugfile")==0)
		      newdlog = new FileDebugger(a->value());
		    else
		      newdlog = new SyslogDebugger();

		    if(newdlog)
		    {
		      newdlog->enable_tid();
		      newdlog->enable_timestamp();
		      
		      xdelete(dlog);
		      dlog = newdlog;
		      newdlog = NULL;
		    }
		    
		    // Enable debugging at DEBUG_MOST.  This is what
		    // will actually start the debugging, since modules
		    // generally don't otherwise set this.

		    dlog->set_level(DEBUG_MOST);
		    dlog->log_progress(DEBUG_CONFIG,
				       "Debugging enabled (level %d)",
				       DEBUG_MOST);

		    found = true;
		  }
		}
#endif
		
		if(!found)
		{
		  err = xstrcat(err, "Provided argument '");
		  err = xstrcat(err, name);
		  err = xstrcat(err, "' is neither required nor optional,");
		}
	      }
	      else
		err = xstrcat(err, "Unidentified argument,");
	    }

	    modargs = NULL;  // We're pointing into creq or treq
	  }
	  else
	    err = xstrcat(err, "Memory allocation error,");
	  
	  xdelete(creq);
	  xdelete(treq);
	
	  xdelete(sxml);
	}
	else
	  err = xstrcat(err, "failed to allocate SurvivorXML,");
	
	// Check through the argument format and make sure all
	// required arguments have been specified.
	
	int i = 0;
	
	for(ArgSpec *arg = argfmt[i];
	    arg != NULL;
	    i++,arg = argfmt[i])
	{
	  if(arg->argname && !ht->retrieve(arg->argname))
	  {
	    if(!arg->optional)
	    {
	      err = xstrcat(err, "Required argument '");
	      err = xstrcat(err, arg->argname);
	      err = xstrcat(err, "' not provided,");
	    }
	    else
	    {
	      if(arg->defval)
	      {
		// We assume the default value is correctly defined.
		
		switch(arg->argtype)
		{
		case boolean_arg:
		  {
		    BooleanArg *b = new BooleanArg(arg->argname,
						   arg->defval->bool_val);

		    if(b)
		      ht->insert(b->name(), b);
		  }
		  break;
		case directory_arg:
		case file_arg:
		case password_arg:
		case string_arg:
		  {
		    StringArg *s = NULL;

		    if(arg->defval->char_array)
		      s = new StringArg(arg->argname, arg->defval->char_array);
		    else
		      s = new StringArg(arg->argname, arg->defval->char_val);

		    if(s)
		      ht->insert(s->name(), s);
		  }
		  break;
		case flag_arg:
		  {
		    FlagArg *f = new FlagArg(arg->argname,
					     arg->defval->char_val);

		    if(f)
		      ht->insert(f->name(), f);
		  }
		  break;
		case number_arg:
		  {
		    NumberArg *n = NULL;

		    if(arg->defval->float_array)
		      n = new NumberArg(arg->argname,
					arg->defval->float_array);
		    else
		      n = new NumberArg(arg->argname, arg->defval->float_val);

		    if(n)
		      ht->insert(n->name(), n);
		  }
		  break;
		case relation_arg:
		  {
		    RelationArg *r = NULL;

		    switch(arg->defval->relation_type)
		    {
		    case no_rel:
		      break;
		    case bt_rel:
		    case nb_rel:
		      r = new RelationArg(arg->argname,
					  arg->defval->relation_type,
					  arg->defval->relation_x,
					  arg->defval->relation_y);
		      break;
		    default:
		      r = new RelationArg(arg->argname,
					  arg->defval->relation_type,
					  arg->defval->relation_x);
		      break;
		    }
		    
		    if(r)
		      ht->insert(r->name(), r);
		  }
		  break;
		default:
		  err = xstrcat(err,
				"Unknown argument type for default value,");
		  break;
		}
	      }
	    }
	  }
	}
      }
      else
	err = xstrcat(err, "Memory allocation error,");
    }
    else
      err = xstrdup("argument string or syntax not provided,");

    if(err)
    {
      // Chop the last character off

      xstrchop(err);
      ret->set_rc(MODEXEC_MISCONFIG);
      ret->set_comment(err);
    }

    xdelete(err);
  }
 
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "CMArgs::parse_named_args = %d", ret);
#endif
  
  return(ret);
}

char *CMArgs::validate_float(ArgSpec *arg, char *f, bool list)
{
  // Verify that <f> meets the requirements of <arg>, according to
  // whether or not it is part of a <list>.

  // Returns: A newly allocated string containing any problems in the
  // parse and that should be deleted when no longer required, or NULL
  // on success.

  char *err = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CMArgs::validate_float(%d,%s,%s)",
		   arg, IONULL(f), IOTF(list));
#endif

  if(arg && f)
  {
    if(arg->any)
    {
      // value must be found within any
      
      bool found = false;
      int i = 0;
      
      for(char *a = arg->any[i];a != NULL && !found;i++,a = arg->any[i])
      {
	if(strcmp(a, f)==0)
	  found = true;
      }
      
      if(!found)
      {
	err = xstrcat(err, "Value '");
	err = xstrcat(err, f);
	err = xstrcat(err, "' for argument ");
	err = xstrcat(err, arg->argname);
	err = xstrcat(err, " is not permitted,");
      }
    }

    if(arg->one)
    {
      if(list)
      {
	err = xstrcat(err,
		      "Multiple values are not permitted to be assigned to argument ");
	err = xstrcat(err, arg->argname);
	err = xstrcat(err, ",");
      }
      else
      {
	// item must be found within one
	  
	bool found = false;
	int i = 0;
	
	for(char *a = arg->one[i];a != NULL && !found;i++,a = arg->one[i])
	{
	  if(strcmp(a, f)==0)
	    found = true;
	}
	  
	if(!found)
	{
	  err = xstrcat(err, "Value '");
	  err = xstrcat(err, f);
	  err = xstrcat(err, "' for argument ");
	  err = xstrcat(err, arg->argname);
	  err = xstrcat(err, " is not permitted,");
	}
      }
    }

    // Now convert to a float and compare between values

    float fx = atof(f);

    if(arg->betweenx != INFINITY_ARG && fx < arg->betweenx)
    {
      char buf[64];
      sprintf(buf, "%f", arg->betweenx);
      
      err = xstrcat(err, "Value '");
      err = xstrcat(err, f);
      err = xstrcat(err, "' for argument ");
      err = xstrcat(err, arg->argname);
      err = xstrcat(err, " must be at least ");
      err = xstrcat(err, buf);
      err = xstrcat(err, ",");
    }

    if(arg->betweeny != INFINITY_ARG && fx > arg->betweeny)
    {
      char buf[64];
      sprintf(buf, "%f", arg->betweeny);
      
      err = xstrcat(err, "Value '");
      err = xstrcat(err, f);
      err = xstrcat(err, "' for argument ");
      err = xstrcat(err, arg->argname);
      err = xstrcat(err, " must not be more than ");
      err = xstrcat(err, buf);
      err = xstrcat(err, ",");
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CMArgs::validate_float = %s", IONULL(err));
#endif

  return(err);
}

char *CMArgs::validate_string(ArgSpec *arg, char *value, bool list)
{
  // Verify that <value> meets the requirements of <arg>, according to
  // whether or not it is part of a <list>.

  // Returns: An newly allocated string containing any problems in the parse
  // and that should be deleted when no longer required, or NULL on success.

  char *err = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CMArgs::validate_string(%d,%s,%s)",
		   arg, IONULL(value), IOTF(list));
#endif

  if(arg && value)
  {
    if(arg->any)
    {
      // value must be found within any
      
      bool found = false;
      int i = 0;
      
      for(char *a = arg->any[i];a != NULL && !found;i++,a = arg->any[i])
      {
	if(strcmp(a, value)==0)
	  found = true;
      }
      
      if(!found)
      {
	err = xstrcat(err, "Value '");
	err = xstrcat(err, value);
	err = xstrcat(err, "' for argument ");
	err = xstrcat(err, arg->argname);
	err = xstrcat(err, " is not permitted,");
      }
    }

    if(arg->one)
    {
      if(list)
      {
	err = xstrcat(err,
		      "Multiple values are not permitted to be assigned to argument ");
	err = xstrcat(err, arg->argname);
	err = xstrcat(err, ",");
      }
      else
      {
	// item must be found within one
	  
	bool found = false;
	int i = 0;
	
	for(char *a = arg->one[i];a != NULL && !found;i++,a = arg->one[i])
	{
	  if(strcmp(a, value)==0)
	    found = true;
	}	  
	if(!found)
	{
	  err = xstrcat(err, "Value '");
	  err = xstrcat(err, value);
	  err = xstrcat(err, "' for argument ");
	  err = xstrcat(err, arg->argname);
	  err = xstrcat(err, " is not permitted,");
	}
      }
    }

    if(arg->argtype == directory_arg)
    {
      // Item must exist as a directory
      
      struct stat sb;
      
      if(stat(value, &sb)!=0 || !(sb.st_mode & S_IFDIR))
      {
	err = xstrcat(err, "Directory '");
	err = xstrcat(err, value);
	err = xstrcat(err, "' does not exist for argument ");
	err = xstrcat(err, arg->argname);
	err = xstrcat(err, ",");
      }
    }
    else if(arg->argtype == file_arg)
    {
      // Item must exist as a file
      
      struct stat sb;
      
      if(stat(value, &sb)!=0 || 
	 (!(sb.st_mode & S_IFREG) && !(sb.st_mode & S_IFLNK) &&
	  !(sb.st_mode & S_IFIFO) && !(sb.st_mode & S_IFSOCK))
	 )
      {
	err = xstrcat(err, "File '");
	err = xstrcat(err, value);
	err = xstrcat(err, "' does not exist for argument ");
	err = xstrcat(err, arg->argname);
	err = xstrcat(err, ",");
      }
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CMArgs::validate_string = %s", IONULL(err));
#endif

  return(err);
}
