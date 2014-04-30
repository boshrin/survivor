/*
 * protocol: A multithreaded module for parallel checking of protocols
 *
 * Version: $Revision: 0.26 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2007/01/16 15:14:26 $
 * MT-Level: Safe
 *
 * Copyright (c) 2002 - 2007
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: main.C,v $
 * Revision 0.26  2007/01/16 15:14:26  benno
 * Fix read loop (bug #1240)
 *
 * Revision 0.25  2006/11/19 01:14:45  benno
 * Add Log
 *
 * Revision 0.24  2006/10/13 02:58:23  benno
 * Fix bug in hostname substitution
 * 
 * Revision 0.23  2005/12/23 16:17:27  benno
 * Make sure port via parg is in network byte order
 * 
 * Revision 0.22  2005/11/26 04:22:14  benno
 * Changes for gethostbyname_r
 * 
 * Revision 0.21  2005/11/24 02:05:00  benno
 * Minor change in relation arg handling
 * 
 * Revision 0.20  2005/05/14 03:20:36  benno
 * Use memcpy instead of strncpy to work around localhost connect bug
 * 
 * Revision 0.19  2004/09/12 16:42:08  benno
 * Allow port to override default port via service
 * 
 * Revision 0.18  2004/04/26 00:47:17  selsky
 * remove log
 * add smtps support
 * 
 * Revision 0.17  2003/10/21 18:43:35  benno
 * Better handling of ssl read errors
 * 
 * Revision 0.16  2003/05/04 21:21:03  benno
 * Don't use string type
 * 
 * Revision 0.15  2003/03/31 12:52:20  benno
 * Add @HOSTNAME@ substitution
 * 
 * Revision 0.14  2003/02/14 13:43:04  benno
 * Allow port numbers without corresponding services definitions
 * 
 * Revision 0.13  2003/01/29 01:25:03  benno
 * Copyright, xdelete
 * 
 * Revision 0.12  2002/12/16 00:09:09  benno
 * Convert to named args
 * 
 * Revision 0.11  2002/10/23 05:24:56  selsky
 * Add nntps/snews support.
 * 
 * Revision 0.10  2002/10/14 01:14:01  selsky
 * add pop3/pop3s support
 * 
 * Revision 0.9  2002/09/05 21:55:49  benno
 * add none and don't require port number to be found in /etc/services
 * 
 * Revision 0.8  2002/04/28 00:08:49  toor
 * imap aka imap2
 * 
 * Revision 0.7  2002/04/19 20:06:47  benno
 * use compare_regex
 * multiple flavors of gethostbyX_r
 * http aka www
 * 
 * Revision 0.6  2002/04/04 21:02:03  benno
 * copyright
 * 
 * Revision 0.5  2002/04/03 21:39:12  benno
 * rcsify date
 * 
 * Revision 0.4  2002/04/03 21:39:03  benno
 * Use libsrvinit/exit
 * 
 * Revision 0.3  2002/04/03 21:38:30  benno
 * Fix read bug
 * 
 * (v0.2)
 * Use libcm
 * Add SSL
 * 
 * Revision 0.2  2002/04/03 21:34:54  benno
 * v0.2 unavailable
 * 
 * Revision 0.1  2002/04/03 21:34:14  benno
 * initial revision
 *
 */

#include "protocol.H"

// Required to use libcm
CMArgs *cmargs = NULL;

// Define our named argument structures
ArgSpec portarg = {
  "port",         // arg name
  number_arg,     // arg type
  NULL,           // any list
  1.0,            // between lower bound
  INFINITY_ARG,   // between upper bound
  false,          // expect a list
  NULL,           // one list
  true,           // optional
  NULL            // default value
};

ArgSpec servicearg = {
  "service",      // arg name
  string_arg,     // arg type
  NULL,           // any list
  INFINITY_ARG,   // between lower bound
  INFINITY_ARG,   // between upper bound
  false,          // expect a list
  NULL,           // one list
  true,           // optional
  NULL            // default value
};

ArgSpec responsearg = {
  "response",     // arg name
  relation_arg,   // arg type
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
  &portarg,
  &servicearg,
  &responsearg,
  NULL
};

// Information about this request, global for easy access
pdef *protocol = NULL;
char *service = NULL;
int   port = 0;

CheckResult *check(int i)
{
  CheckResult *rv = NULL;
  
  // List->retrieve is MT-Safe.  cmargs is set before this is called.

  char *domain = cmargs->hosts()->retrieve(i);

  if(domain && protocol)
  {
    rv = new CheckResult(MODEXEC_MISCONFIG, 0, "");
    
    if(rv)
    {
      struct hostent h, *hp;
      sockaddr_in s;
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
	    char rbuf[BUFSIZE];

#if defined(WITH_SSL)
	    SSL *con = NULL;

	    if(protocol->ssl)
	      con = ssl_session_connect(sd);

	    if(!protocol->ssl || con)
	    {
#endif	      
	      // At this point, we're successful unless we see a problem
	      
	      rv->set_rc(MODEXEC_OK);
	      rv->set_scalar(1);
	      rv->set_comment("Connection established");
	      
	      for(int i = 0;i < protocol->len;i++)
	      {
		if(protocol->send && protocol->send[i])
		{
		  char *sendme = protocol->send[i];
		  char *altsendme = NULL;

		  // Perform any requested substitutions.  Right now,
		  // we only support @HOSTNAME@.

		  if(strstr(sendme, "@HOSTNAME@"))
		  {
		    char hnbuf[MAXHOSTNAMELEN];

		    if(gethostname(hnbuf, MAXHOSTNAMELEN-1)==0)
		      altsendme = xstrreplace(sendme, "@HOSTNAME@", hnbuf);
		    // else error, no substitution
		  }

		  if(altsendme)
		    sendme = altsendme;
		  
#if defined(WITH_SSL)
		  if(protocol->ssl)
		    SSL_write(con, sendme, strlen(sendme));
		  else
		    write(sd, sendme, strlen(sendme));
#else
		  write(sd, sendme, strlen(sendme));
#endif
		  xdelete(altsendme);
		}

		// Read in until we see a \r or \n

		bool endseen = false;
		int rin = 0;

		while(!endseen)
		{
		  int rsize = 0;

		  // SSL_read and read have slightly different semantics.
		  // SSL_read returning 0 indicates failure.  read
		  // returning 0 does not necessarily.
		  
#if defined(WITH_SSL)
		  if(protocol->ssl)
		    rsize = SSL_read(con, rbuf+rin, BUFSIZE-1-rin);
		  else
		    rsize = read(sd, rbuf+rin, BUFSIZE-1-rin);
#else
		  rsize = read(sd, rbuf+rin, BUFSIZE-1-rin);
#endif
		  if(rsize < 0
#if defined(WITH_SSL)
		     || (protocol->ssl && rsize == 0)
#endif
		     )
		  {
		    // Error encountered, we're done

		    rv->set_rc(MODEXEC_PROBLEM);
		    rv->set_scalar(0);
		    rv->set_comment("read error encountered");
		    
		    endseen = true;
		  }
		  else if(rsize == 0)
		  {
		    if(rin == 0)
		    {
		      // Received 0 bytes, we're done

		      rv->set_rc(MODEXEC_PROBLEM);
		      rv->set_scalar(0);
		      rv->set_comment("no data received");
		    }
		    
		    endseen = true;
		  }
		  else
		  {
		    rin += rsize;
		  
		    rbuf[rin] = '\0';
		
		    char *p = strchr(rbuf, '\n');
		    if(p)
		    {
		      *p = '\0';
		      endseen = true;
		    }
		    
		    p = strchr(rbuf, '\r');
		    if(p)
		    {
		      *p = '\0';
		      endseen = true;
		    }
		  }
		}

		if(rv->rc() == MODEXEC_OK
		   && protocol->receive && protocol->receive[i])
		{
		  if(compare_regex(rbuf, protocol->receive[i]))
		  {
		    // This line matches
		      
		    if(i == protocol->keep && rv->rc()==MODEXEC_OK)
		      rv->set_comment(rbuf);
		  }
		  else
		  {
		    // No match, set comment to failed buf contents
		    // and break the loop
		    
		    rv->set_comment(rbuf);
		    rv->set_rc(MODEXEC_PROBLEM);
		    rv->set_scalar(0);
		    i = protocol->len + 1;
		  }
		}
	      }
#if defined(WITH_SSL)
	    }

	    if(protocol->ssl)
	    {
	      if(con)
	      {
		ssl_session_disconnect(con);
		con = NULL;
	      }
	      else
	      {
		rv->set_comment("Secure connection failed");
		rv->set_rc(MODEXEC_PROBLEM);
	      }
	    }
#endif
	  }
	  else
	  {
	    rv->set_comment("Connection refused");
	    rv->set_rc(MODEXEC_PROBLEM);
	  }
	}
	else
	{
	  rv->set_comment("Failed to obtain socket");
	  rv->set_rc(MODEXEC_PROBLEM);
	}
      }
      else
      {
	rv->set_comment("Unknown host");
	rv->set_rc(MODEXEC_PROBLEM);
      }
    }
  }
    
  return(rv);
}

CheckResult *validate()
{
  CheckResult *rv = new CheckResult(MODEXEC_OK, 0, "Protocol OK");

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
	  // For when the protocol is specified on the command line
	  char *transmit[] = { NULL };
	  char *receive[] = { NULL };
	  pdef p_cli = {
	    1,
	    0,
	    false,
	    transmit,
	    receive
	  };
	  	    
	  struct servent *sent = NULL;
	    
	  StringArg *sarg = (StringArg *)cmargs->arg("service");
	  NumberArg *parg = (NumberArg *)cmargs->arg("port");
	  RelationArg *rarg = (RelationArg *)cmargs->arg("response");

	  // First, sanity check what we were given
	  
	  if(sarg || parg)
	  {
	    // A response argument trumps a protocol lookup

	    if(parg)
	      port = htons((int)parg->value());

	    if(sarg)
	      service = sarg->value();

	    // Look up the service once here

	    if(service)
	    {
	      sent = getservbyname(service, "tcp");
	      
	      if(port == 0 && sent)
		port = sent->s_port;
	    }
	    else
	      sent = getservbyport(port, "tcp");
  
	    if(rarg)
	    {
	      // A protocol was specified, it should be a response regex.
	      // Support for a transmit regex was removed since it was
	      // implemented incorrectly and didn't seem to be useful.
	      
	      if(rarg->relation()==re_rel)
	      {
		receive[0] = rarg->x();
		protocol = &p_cli;
	      }
	      else
	      {
		r = MODEXEC_MISCONFIG;
		cm_error(r, 0,
			 "Response argument must be of reg relation type");
	      }
	    }
	    else
	    {
	      // Determine protocol based on service/port

	      if((port == 0 && sent && sent->s_name) || port)
	      {
		if(port && !sent)
		  protocol = &p_none;  // Don't know what to do, so just ping
		else if(strcmp(sent->s_name, "http")==0 ||
			strcmp(sent->s_name, "www")==0)
		  protocol = &p_http;
		else if(strcmp(sent->s_name, "https")==0)
		  protocol = &p_https;
		else if(strcmp(sent->s_name, "imap")==0 ||
			strcmp(sent->s_name, "imap2")==0)
		  protocol = &p_imap;
		else if(strcmp(sent->s_name, "imaps")==0)
		  protocol = &p_imaps;
		else if(strcmp(sent->s_name, "nntp")==0)
		  protocol = &p_nntp;
		else if(strcmp(sent->s_name, "nntps")==0 ||
			strcmp(sent->s_name, "snews")==0)
		  protocol = &p_nntps;
		else if(strcmp(sent->s_name, "smtp")==0)
		  protocol = &p_smtp;
		else if(strcmp(sent->s_name, "smtps")==0)
		  protocol = &p_smtps;
		else if(strcmp(sent->s_name, "pop")==0 ||
			strcmp(sent->s_name, "pop3")==0)
		  protocol = &p_pop;
		else if(strcmp(sent->s_name, "pop3s")==0)
		  protocol = &p_pops;
		else
		  protocol = &p_none;  // If no protocol, just do a ping
	      }
	    }

	    if(sarg && !sent && !rarg)
	    {
	      r = MODEXEC_MISCONFIG;
	      cm_error(r, 0, "Undefined service");
	    }

	    if(r == MODEXEC_OK)
	    {
	      if(protocol)
	      {
#if defined(WITH_SSL)
		if(!protocol->ssl || ssl_initialize())
#else
		if(!protocol->ssl)
#endif
		{	    
		  // Now that we've set everything up, let cm_thread_exec
		  // do the work
		  
		  r = cm_thread_exec(check);
		  
#if defined(WITH_SSL)
		  if(protocol->ssl)
		    ssl_discontinue();
#endif
		}
		else
		{
		  r = MODEXEC_MISCONFIG;
		  
#if defined(WITH_SSL)
		  cm_error(r, 0, "Module failed to initialize SSL");
#else
		  cm_error(r, 0, "SSL support not compiled");
#endif
		}
	      }
	      else
	      {
		r = MODEXEC_MISCONFIG;
		cm_error(r, 0, "Module failed due to programmer error");
	      }
	    }
	  }
	  else
	  {
	    r = MODEXEC_MISCONFIG;
	    cm_error(r, 0, "Neither service nor port argument specified");
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
