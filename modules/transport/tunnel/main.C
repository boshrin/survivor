/*
 * tunnel: A multithreaded module for tunneled communication with sr.
 *         Not to be confused with limelight.
 *
 * Version: $Revision: 0.9 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2006/01/26 22:11:51 $
 * MT-Level: Safe
 *
 * Copyright (c) 2002 - 2006
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: main.C,v $
 * Revision 0.9  2006/01/26 22:11:51  benno
 * Don't segfault if rinstdir isn't provided
 *
 * Revision 0.8  2006/01/23 02:18:44  benno
 * Better handling of CheckResult
 *
 * Revision 0.7  2005/11/11 03:57:43  benno
 * Change in default arg specification
 *
 * Revision 0.6  2005/06/09 02:27:16  benno
 * Use SRProtocol with fd pair
 *
 * Revision 0.5  2005/04/16 21:46:05  benno
 * Support sr protocol v2
 *
 * Revision 0.4  2003/10/06 22:19:37  benno
 * Simplified result reading
 *
 * Revision 0.3  2003/01/29 01:39:50  benno
 * xdelete
 *
 * Revision 0.2  2002/12/31 04:17:48  benno
 * Use formal named args
 *
 * Revision 0.1  2002/12/20 03:19:07  benno
 * Initial revision
 *
 */

/* tunnel modules use the same infrastructure as Check modules (and Fix
 * modules, for that matter) because they end up calling them.
 */

#include "survivor.H"

// Required to use libcm
CMArgs *cmargs = NULL;

// Global for easy access
char *module = NULL;
char *moduletype = NULL;
StringArg *command = NULL;
StringArg *rinstdir = NULL;
StringArg *srname = NULL;
Array<Argument> *modargs = NULL;
int timeout = 0;
char *rinstdir2 = NULL;

// Define our named argument structures.

// command, rinstdir, and srname are set via arguments rather than an
// /etc/survivor/tunnel.cf since the login user could vary on a
// per-service or per-host basis, and rinstdir could vary on a
// per-host basis.
  
ArgSpec commandarg = {
  "command",       // arg name
  string_arg,      // arg type
  NULL,            // any list
  INFINITY_ARG,    // between lower bound
  INFINITY_ARG,    // between upper bound
  false,           // expect a list
  NULL,            // one list
  false,           // optional
  NULL             // default value
};

ArgSpec rinstdirarg = {
  "rinstdir",      // arg name
  string_arg,      // arg type
  NULL,            // any list
  INFINITY_ARG,    // between lower bound
  INFINITY_ARG,    // between upper bound
  false,           // expect a list
  NULL,            // one list
  true,            // optional
  NULL             // default value
};

DefaultArg srnamedef = {
  false, "sr", NULL, 0.0, NULL, no_rel, NULL, NULL
};

ArgSpec srnamearg = {
  "srname",        // arg name
  string_arg,      // arg type
  NULL,            // any list
  INFINITY_ARG,    // between lower bound
  INFINITY_ARG,    // between upper bound
  false,           // expect a list
  NULL,            // one list
  true,            // optional
  &srnamedef       // default value
};

ArgSpec *argfmt[] = {
  &commandarg,
  &rinstdirarg,
  &srnamearg,
  NULL
};

CheckResult *check(int i)
{
  CheckResult *rv = NULL;
  
  // List->retrieve is MT-Safe.  cmargs is set before this is called.

  char *h = cmargs->hosts()->retrieve(i);
  char *rinstdirp = (rinstdir && rinstdir->value()) ? rinstdir->value() :
    rinstdir2;

  if(h && command->value() && module && moduletype && rinstdirp &&
     srname->value())
  {
    rv = new CheckResult(MODEXEC_MISCONFIG, 0, "");
    
    if(rv)
    {
      char *cmd = new char[strlen(command->value()) + strlen(h)
			   + (2 * strlen(rinstdirp))
			   + strlen(srname->value()) + 20];

      if(cmd)
      {
	sprintf(cmd, "%s %s %s/sbin/%s -m %s/mod", command->value(), h,
		rinstdirp, srname->value(), rinstdirp);

	// We need one Executor per thread.
	
	Executor *e = new Executor();
	
	if(e)
	{
	  int *fds = e->exec_pipe(cmd);
	  
	  if(fds)
	  {
	    // This code is similar to the plaintext module
	    
	    // SRProtocol handles the communication

	    SRProtocol *srp = new SRProtocol(fds);

	    if(srp)
	    {
	      CheckResult *srpres = NULL;

	      // Make the appropriate request

	      if(strcmp(moduletype, "check")==0)
	      {
		CheckRequest *cr = new CheckRequest("localhost",
						    timeout,
						    modargs,
						    false);

		if(cr)
		{
		  srpres = srp->check(module, cr);
		  
		  xdelete(cr);
		}
		else
		  SET_CHECKRESULT(rv, MODEXEC_PROBLEM, 0,
				  "Failed to allocate CheckRequest");
	      }
	      else
	      {
		FixRequest *fr = new FixRequest(modargs, "localhost", timeout);

		if(fr)
		{
		  srpres = srp->fix(module, fr);
		  
		  xdelete(fr);
		}
		else
		  SET_CHECKRESULT(rv, MODEXEC_PROBLEM, 0,
				  "Failed to allocate FixRequest");
	      }

	      // Store the answer
	      
	      if(srpres)
	      {
		xdelete(rv);
		
		rv = srpres;
	      }

	      // Tell the other end we're done

	      srp->exit();
	      
	      xdelete(srp);
	    }
	    else
	      SET_CHECKRESULT(rv, MODEXEC_PROBLEM, 0,
			      "Failed to allocate SRProtocol");

	    e->close_pipe();

	    xdelete(fds);
	  }
	  else
	    SET_CHECKRESULT(rv, MODEXEC_PROBLEM, 0, "exec_pipe failed")
	}
	else
	  SET_CHECKRESULT(rv, MODEXEC_PROBLEM, 0, "Executor allocation failed")

	xdelete(cmd);
      }
      else
	SET_CHECKRESULT(rv, MODEXEC_PROBLEM, 0, "cmd allocation failed");
    }
  }
    
  return(rv);
}

CheckResult *validate()
{
  CheckResult *rv = new CheckResult(MODEXEC_OK, 0, "Tunnel OK");

  return(rv);
}

int main(int argc, char **argv)
{
  int r = MODEXEC_OK;  

  if(!libsrvinit(argv[0]))
    exit(MODEXEC_MISCONFIG);
    
  // TMArgs is derived from CMArgs, so this is OK.
  cmargs = new TMArgs();

  if(cmargs)
  {
    CheckResult *rv = cmargs->parse_args(argc, argv, argfmt, validate);

    if(rv)
    {
      timeout = cmargs->timeout();
      
      if(rv->rc() == MODEXEC_OK && !cmargs->validating())
      {
	// We won't get here if the required arguments were not provided.
	
	module = ((TMArgs *)cmargs)->module();
	moduletype = ((TMArgs *)cmargs)->modtype();
	command = (StringArg *)((TMArgs *)cmargs)->arg("command");
	rinstdir = (StringArg *)((TMArgs *)cmargs)->arg("rinstdir");
	srname = (StringArg *)((TMArgs *)cmargs)->arg("srname");
	modargs = ((TMArgs *)cmargs)->rmodargs();

	if(!rinstdir)
	{
	  // This is a special case.  We can't provide a DefaultArg
	  // since it's sort of determinable at runtime.  Worse, we
	  // have to create Args here since we aren't properly using
	  // it.  (We're not calling a parse_X function, so we don't
	  // want to accidentally rely on it outside of this stanza.)

	  args = new Args();

	  if(args && args->pkgdir() && strlen(args->pkgdir()) < PATH_MAX)
	  {
	    rinstdir2 = xstrdup(args->pkgdir());
	    
	    // libsrvexit() will clean up args
	  }
	}

	if(!rinstdir && !rinstdir2)
	{
	  r = MODEXEC_MISCONFIG;
	  cm_error(r, 0, "tunnel given no installation directory");
	}

	if(r == MODEXEC_OK)
	{
	  // Off we go
	  
	  r = cm_thread_exec(check);
	}

	xdelete(rinstdir2);
      }
      else
      {
	r = rv->rc();
	cm_error(r, 0, rv->comment());
      }

      xdelete(rv);
    }

    xdelete(cmargs);
  }
  else  // No point calling cm_error with no hosts
    r = MODEXEC_PROBLEM;

  libsrvexit();
  
  exit(r);
}
