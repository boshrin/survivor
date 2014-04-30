/*
 * test: Sample compiled test module
 *
 * Version: $Revision: 0.3 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2005/11/11 03:58:21 $
 * MT-Level: Safe
 *
 * Copyright (c) 2002 - 2005
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: test.C,v $
 * Revision 0.3  2005/11/11 03:58:21  benno
 * Change in default arg specification
 *
 * Revision 0.2  2003/01/29 01:31:37  benno
 * xdelete
 *
 * Revision 0.1  2002/12/31 04:14:59  benno
 * Initial revision
 *
 */

#include "survivor.H"

// Required to use libcm
CMArgs *cmargs = NULL;

// Define our named argument structures here.  Basically, each argument
// gets defined in an ArgSpec structure, with libcm.H defining what
// arguments are supported.  These arguments (and their modifiers, which
// are specified in the ArgSpec structure) correspond to those defined in
// doc/cm-args.html.

// When in doubt, leave the default values as given in this first example
// (for things like INFINITY_ARG, DEFAULTSEP_ARG, etc).

// This first example allows an optional simple string named "exittext".

DefaultArg arg1default = {
  false, "Test requested", NULL, 0.0, NULL, no_rel, NULL, NULL
};

ArgSpec arg1 = {
  "exittext",     // arg name
  string_arg,     // arg type
  NULL,           // any values list
  INFINITY_ARG,   // between lower bound
  INFINITY_ARG,   // between upper bound
  false,          // expect a list
  NULL,           // one values list
  true,           // optional
  &arg1default    // default value
};

// This next example requires a number between 0 and 1000 named "exitcode".

ArgSpec arg2 = {
  "exitcode",     // arg name
  number_arg,     // arg type
  NULL,           // any values list
  0.0,            // between lower bound
  1000.0,         // between upper bound
  false,          // expect a list
  NULL,           // one values list
  false,          // optional
  NULL
};

// For more examples, see main.C.

// Now, put our arguments in a NULL terminated array.
ArgSpec *argfmt[] = {
  &arg1,
  &arg2,
  NULL
};

// This is a thread safe routine that actually performs the checking.
// By writing the check this way, it is easy to make it run in parallel.
// This routine will be called once per host, with an index indicating
// which host is to be checked.

CheckResult *check(int i)
{
  CheckResult *rv = NULL;
  
  // List->retrieve is MT-Safe.  cmargs is set for us before this is called.
  // Retrieve the host we are responsible for.

  char *h = cmargs->hosts()->retrieve(i);

  if(h)
  {
    // Retrieve our configuration arguments.  Since the arguments aren't
    // changing, this is thread safe.  Note that exittext may not be
    // defined, since it was specified as optional.

    NumberArg *exitcode = (NumberArg *)cmargs->arg("exitcode");
    StringArg *exittext = (StringArg *)cmargs->arg("exittext");
    
    // Simple test, just exit as configured.

    rv = new CheckResult(exitcode->value(), 0,
			 (char *)(exittext ? exittext->value() : "Check OK"));
    
    if(rv)
    {
      // We could also change the values in the rv structure if we wanted:
      // rv->set_rc(0);
      // rv->set_scalar(100);
      // rv->set_comment("comment");
    }
  }

  // Return the pointer to the structure.  libcm will handle the rest.
  
  return(rv);
}

// This is a routine that gets called when the module is invoked -v.
// It is intended to verify that anything the module depends on (such
// as shared libraries, configuration files, etc) is in place.
// This routine will only be called once, if at all.

CheckResult *validate()
{
  // Since this is a simple module with no dependencies, we simply
  // indicate success.  A more sophisticated module may require a
  // more sophisticated test.
  
  CheckResult *rv = new CheckResult(MODEXEC_OK, 0, "Test OK");

  return(rv);
}

// Finally, our main.  libcm will handle most of the hard work for us,
// we just need the right incantations.

int main(int argc, char **argv)
{
  // Usage: test -h host[,host...] [-t timeout] [-v] [name=value [...] ]

  int r = MODEXEC_OK;  

  // We must initialize libsrv and allocate CMArgs.
  
  if(!libsrvinit(argv[0]))
    exit(MODEXEC_MISCONFIG);
    
  cmargs = new CMArgs();

  if(cmargs)
  {
    // Let libcm parse the arguments on the command line using the
    // argument format structure we defined above and the validation
    // routine we wrote.
    
    CheckResult *rv = cmargs->parse_args(argc, argv, argfmt, validate);

    if(rv)
    {
      if(rv->rc() == MODEXEC_OK && !cmargs->validating())
      {
	// Set up any global items relevant to your check here, for
	// example looking up port numbers or parsing configuration
	// files.
	
	// Now that we've set everything up, let cm_thread_exec do its work.
	// Pass it the name of the check routine we defined above.
	
	r = cm_thread_exec(check);	
      }
      else
      {
	// Set our return status to indicate what happened and use
	// cm_error to generate the appropriate output.
	
	r = rv->rc();
	cm_error(r, 0, rv->comment());
      }
    }
    else  // No point calling cm_error with no hosts
      r = MODEXEC_PROBLEM;
    
    // Clean up
    xdelete(cmargs);
  }
  else  // No point calling cm_error with no hosts
    r = MODEXEC_PROBLEM;

  // Tell libsrv to clean up
  libsrvexit();
  
  exit(r);
}
