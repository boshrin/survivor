/*
 * plaintext: A multithreaded module for communicating with sr.
 *
 * Version: $Revision: 0.14 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2006/01/23 02:18:31 $
 * MT-Level: Safe
 *
 * Copyright (c) 2002 - 2006
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: main.C,v $
 * Revision 0.14  2006/01/23 02:18:31  benno
 * Better handling of CheckResult
 *
 * Revision 0.13  2005/11/26 04:21:56  benno
 * Changes for gethostbyname_r
 *
 * Revision 0.12  2005/05/14 03:20:52  benno
 * Use memcpy instead of strncpy to work around localhost connect bug
 *
 * Revision 0.11  2005/04/16 21:46:27  benno
 * Support sr protocol v2
 *
 * Revision 0.10  2003/10/06 22:21:57  benno
 * Simplify result reading
 *
 * Revision 0.9  2003/01/29 01:38:30  benno
 * xdelete
 *
 * Revision 0.8  2002/12/31 04:18:28  benno
 * Use formal named arguments
 *
 * Revision 0.7  2002/12/16 00:08:13  benno
 * Convert to named arguments
 *
 * Revision 0.6  2002/08/06 14:27:47  selsky
 * Remove embedded nulls in format
 *
 * Revision 0.5  2002/04/19 20:07:20  benno
 * multiple flavors of gethostbyX_r
 *
 * Revision 0.4  2002/04/04 21:05:10  benno
 * copyright
 *
 * Revision 0.3  2002/04/03 21:48:55  benno
 * rcsify date
 *
 * Revision 0.2  2002/04/03 21:48:47  benno
 * Use libsrvinit/exit
 * Include "unexpected" response
 *
 * Revision 0.1  2002/04/03 21:48:00  benno
 * initial revision
 *
 */

#include "survivor.H"

#define SRSERVICE "sr"

// Required to use libcm
CMArgs *cmargs = NULL;

// Global for easy access
char *module = NULL;
char *moduletype = NULL;
Array<Argument> *modargs = NULL;
int timeout = 0;
int port = -1;

// Define our named argument structures (none).

ArgSpec *argfmt[] = {
  NULL
};

CheckResult *check(int i)
{
  CheckResult *rv = NULL;
  
  // List->retrieve is MT-Safe.  cmargs is set before this is called.

  char *domain = cmargs->hosts()->retrieve(i);
  
  if(domain && module && moduletype)
  {
    rv = new CheckResult(MODEXEC_MISCONFIG, 0, "");
    
    if(rv)
    {
      struct hostent h, *hp;
      struct sockaddr_in s;
      char hbuf[BUFSIZE];
      int herr;

#if defined(HAVE_FUNC_GETHOSTBYNAME_R_3)
      struct hostent_data data;

      int r = gethostbyname_r(domain, &h, &data);
      hp = &h;

      if(r == 0)
#elif defined(HAVE_FUNC_GETHOSTBYNAME_R_5)
        hp = gethostbyname_r(domain, &h, hbuf, BUFSIZE, &herr);
              
      if(hp)
#elif defined(HAVE_FUNC_GETHOSTBYNAME_R_6)
      int r = gethostbyname_r(domain, &h, hbuf, BUFSIZE, &hp, &herr);
      
      if(r == 0)
#else
#error Supported gethostbyname_r not found
#endif
      {
	memset(&s, 0, sizeof(s));
        s.sin_family = hp->h_addrtype;
        strncpy((char *)&s.sin_addr, hp->h_addr, hp->h_length);
	s.sin_port = port;
	
	int sd = socket(AF_INET, SOCK_STREAM, 0);
	
	if(sd >= 0)
	{
	  if(connect(sd, (struct sockaddr *)&s, sizeof(s)) == 0)
	  {
	    // SRProtocol handles the communication
	    
	    SRProtocol *srp = new SRProtocol(sd);

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
	  }
	  else
	    SET_CHECKRESULT(rv, MODEXEC_PROBLEM, 0, "Connection refused");
	  
	  close(sd);
	}
	else
	  SET_CHECKRESULT(rv, MODEXEC_PROBLEM, 0, "Failed to obtain socket");
      }
      else
	SET_CHECKRESULT(rv, MODEXEC_PROBLEM, 0, "Unknown host");
    }
  }
    
  return(rv);
}

CheckResult *validate()
{
  CheckResult *rv = new CheckResult(MODEXEC_OK, 0, "Plaintext OK");

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
	// Since these arguments are all required, we won't get here
	// if any were not provided.
	
	module = ((TMArgs *)cmargs)->module();
	moduletype = ((TMArgs *)cmargs)->modtype();
	modargs = ((TMArgs *)cmargs)->rmodargs();

	if(strcmp(moduletype, "fix")==0)
	  _cm_fix = true; // Tell libcm to operate on fixes
	
	// Look up the service once here

	struct servent *sent = getservbyname(SRSERVICE, "tcp");
	
	if(sent)
	{
	  port = sent->s_port;
	  
	  // Now that we've set everything up, let fm_thread_exec do
	  // the work
	  
	  r = cm_thread_exec(check);
	}
	else
	{
	  r = MODEXEC_MISCONFIG;
	  cm_error(r, 0, "plaintext failed to find service for sr");
	}
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
