/*
 * SRProtocol: Object implementing SR protocol
 *
 * Version: $Revision: 0.3 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2006/01/23 00:46:36 $
 * MT-Level: Unsafe.
 *
 * Copyright (c) 2005 - 2006
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: SRProtocol.C,v $
 * Revision 0.3  2006/01/23 00:46:36  benno
 * Better CheckResult handling
 *
 * Revision 0.2  2005/06/09 02:26:41  benno
 * Operate over fd pair as well as socket
 *
 * Revision 0.1  2005/04/19 22:49:59  benno
 * Survivor Protocol Implementation
 *
 */

#include "survivor.H"

SRProtocol::SRProtocol(int fd)
{
  // Allocate and initialize a new SRProtocol object, communicating
  // over <fd>.  <fd> will not be closed when the object is deleted.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "SRProtocol::SRProtocol(%d)", fd);
#endif

  fin = new FileHandler(fd);
  fout = fin;  // We just point to fin here
  invalid = false;
  fs = NULL;
  f = fd;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "SRProtocol::SRProtocol()");
#endif
}

SRProtocol::SRProtocol(int *fds)
{
  // Allocate and initialize a new SRProtocol object, communicating
  // over <fds> (input, output).  <fds> will not be closed when the
  // object is deleted.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "SRProtocol::SRProtocol(%d)", fds);
#endif

  fin = NULL;
  fout = NULL;
  
  if(fds)
  {
    fin = new FileHandler(fds[0]);
    fout = new FileHandler(fds[1]);
  }
  
  invalid = false;
  fs = fds;
  f = -1;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "SRProtocol::SRProtocol()");
#endif
}

CheckResult *SRProtocol::check(char *module, CheckRequest *cr)
{
  // Send a 'check' command for the specified <module> and <cr>.

  // Returns: A newly allocated CheckResult object, or NULL on
  // allocation error.

  CheckResult *ret = NULL;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "SRProtocol::check(%d)", cr);
#endif
  
  if(!invalid && fin && fout && cr)
  {
    ret = new CheckResult(MODEXEC_MISCONFIG, 0, "");
    SurvivorXML *sxml = new SurvivorXML();
    CharBuffer *resp = new CharBuffer();   // For RESPONSE

    if(ret && sxml && resp)
    {
      // First, send the command and module name

      if(fout->append("check ") && fout->append(module) && fout->append("\n"))
      {
	// Check for OK
	
	char *l = fin->read_line();
	  
	if(l && strncmp(l, "OK send SurvivorCheckData", 25)==0)
	{
	  // Send the CheckData document
	  
	  if(fout->append("BEGIN DATA\n")
	     && sxml->generate((fs ? fs[1] : f), cr)
	     && fout->append("END DATA\n"))
	  {
	    // Read a CheckResult

	    l = fin->read_line();

	    if(l && strncmp(l, "BEGIN RESPONSE", 14)==0)
	    {
	      // We read the response and store it in a charbuffer so
	      // the XML parser isn't confused by the END RESPONSE

	      for(l = fin->read_line();
		  l && strncmp(l, "END RESPONSE", 12)!=0;
		  l = fin->read_line())
		resp->append(l);

	      // Get a CheckResult from what we read

	      CheckResult *cres = sxml->parse_checkresult(resp->str(),
							  strlen(resp->str()));

	      if(cres)
	      {
		xdelete(ret);
		
		ret = cres;
	      }
	      else
	      {
		SET_CHECKRESULT(ret, MODEXEC_PROBLEM, 0,
				"Did not receive valid CheckResult document");
		invalid = true;
	      }
	    }
	    else
	    {
	      SET_CHECKRESULT(ret, MODEXEC_PROBLEM, 0, l);
	      invalid = true;
	    }
	  }
	  else
	  {
	    SET_CHECKRESULT(ret, MODEXEC_PROBLEM, 0, "Error sending DATA");
	    invalid = true;
	  }
	}
	else
	{
	  SET_CHECKRESULT(ret, MODEXEC_PROBLEM, 0,
			  (char *)(l ? l :
			   "NULL response received waiting for OK send"));
	  invalid = true;
	}
      }
      else
      {
	SET_CHECKRESULT(ret, MODEXEC_PROBLEM, 0, "Write failed");
	invalid = true;
      }
    }
    // on error, if ret is allocated, we'll still want to return it

    xdelete(sxml);
    xdelete(resp);
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "SRProtocol::check = %d", ret);
#endif

  return(ret);
}

bool SRProtocol::exit()
{
  // Send an 'exit' command to terminate the connection.  This method
  // will not close <fd>.

  // Returns: true if fully successful, false otherwise.
  
  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "SRProtocol::exit()");
#endif

  if(!invalid && fout && fout->append("exit\n"))
    ret = true;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "SRProtocol::exit = %s", IOTF(ret));
#endif

  return(ret);
}

CheckResult *SRProtocol::fix(char *module, FixRequest *fr)
{
  // Send a 'fix' command for the specified <module> and <cr>.

  // Returns: A newly allocated CheckResult object, or NULL on
  // allocation error.

  CheckResult *ret = NULL;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "SRProtocol::fix(%d)", fr);
#endif

  if(!invalid && fin && fout && fr)
  {
    ret = new CheckResult(MODEXEC_MISCONFIG, 0, "");
    SurvivorXML *sxml = new SurvivorXML();
    CharBuffer *resp = new CharBuffer();   // For RESPONSE

    if(ret && sxml && resp)
    {
      // First, send the command and module name

      if(fout->append("fix ") && fout->append(module) && fout->append("\n"))
      {
	// Check for OK
	
	char *l = fin->read_line();
      
	if(l && strncmp(l, "OK send SurvivorFixData", 25)==0)
	{
	  // Send the FixData document
	  
	  if(fout->append("BEGIN DATA\n")
	     && sxml->generate((fs ? fs[1] : f), fr)
	     && fout->append("END DATA\n"))
	  {
	    // Read a FixResult

	    l = fin->read_line();

	    if(l && strncmp(l, "BEGIN RESPONSE", 14)==0)
	    {
	      // We read the response and store it in a charbuffer so
	      // the XML parser isn't confused by the END RESPONSE

	      for(l = fin->read_line();
		  l && strncmp(l, "END RESPONSE", 12)!=0;
		  l = fin->read_line())
		resp->append(l);

	      // Get a FixResult from what we read
	      
	      FixResult *fres = sxml->parse_fixresult(resp->str(),
						      strlen(resp->str()));

	      if(fres)
	      {
		ret->set_comment(fres->comment());
		ret->set_rc(fres->rc());
		ret->set_scalar(fres->scalar());
		
		xdelete(fres);
	      }
	      else
	      {
		SET_CHECKRESULT(ret, MODEXEC_PROBLEM, 0,
				"Did not receive valid FixResult document");
		invalid = true;
	      }
	    }
	    else
	    {
	      SET_CHECKRESULT(ret, MODEXEC_PROBLEM, 0, l);
	      invalid = true;
	    }
	  }
	  else
	  {
	    SET_CHECKRESULT(ret, MODEXEC_PROBLEM, 0, "Error sending DATA");
	    invalid = true;
	  }
	}
	else
	{
	  SET_CHECKRESULT(ret, MODEXEC_PROBLEM, 0, l);
	  invalid = true;
	}
      }
      else
      {
	SET_CHECKRESULT(ret, MODEXEC_PROBLEM, 0, "Write failed");
	invalid = true;
      }
    }
    // on error, if ret is allocated, we'll still want to return it

    xdelete(sxml);
    xdelete(resp);
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "SRProtocol::fix = %d", ret);
#endif

  return(ret);
}

char *SRProtocol::version()
{
  // Send an 'exit' command to obtain the server version string.

  // Returns: A pointer to a string that should not be modified and
  // remains valid until the next command or until the SRProtocol
  // object is deleted, or NULL on error.
  
  char *ret = NULL;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "SRProtocol::version()");
#endif

  if(!invalid && fin && fout && fout->append("version\n"))
    ret = fin->read_line();

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "SRProtocol::version = %s", ret);
#endif
}

SRProtocol::~SRProtocol()
{
  // Deallocate the SRProtocol object.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "SRProtocol::~SRProtocol()");
#endif

  xdelete(fin);
  if(fs)
  {
    xdelete(fout);
  }
  else
    fout = NULL;  // fout just points to fin
  invalid = true;
  fs = NULL;
  f = -1;
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "SRProtocol::~SRProtocol()");
#endif
}
