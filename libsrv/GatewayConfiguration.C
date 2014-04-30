/*
 * survivor gateway interface configuration object
 *
 * Version: $Revision: 0.2 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2005/11/26 04:22:36 $
 * MT-Level: Safe, if parse_cf is only called once
 *
 * Copyright (c) 2004 - 2005
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: GatewayConfiguration.C,v $
 * Revision 0.2  2005/11/26 04:22:36  benno
 * Add gethostbyname_r_3
 *
 * Revision 0.1  2004/11/26 22:01:29  benno
 * Initial revision
 *
 */

#include "gateway.H"

GatewayConfiguration::GatewayConfiguration()
{
  // Allocate and initialize a new GatewayConfiguration object.

  // Returns: A new GatewayConfiguration object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC,
		  "GatewayConfiguration::GatewayConfiguration()");
#endif

  rts = NULL;
  rlydmns = NULL;
  rlyall = false;
  rlylcl = false;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "GatewayConfiguration::GatewayConfiguration()");
#endif
}

bool GatewayConfiguration::add_relay_target(RelayTarget *rt)
{
  // Add <rt> to the hash of RelayTargets.  <rt> will be adopted and
  // deleted when no longer required.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "GatewayConfiguration::add_relay_target(%d)",
		  rt);
#endif

  if(rt && rt->name())
  {
    if(!rts)
      rts = new Hashtable<RelayTarget>();

    if(rts)
    {
      if(rts->insert(rt->name(), rt))
	ret = true;
      else
      {
	xdelete(rt);
      }
    }
    else
      wlog->warn("GatewayConfiguration::add_relay_target failed to allocate Hashtable");
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "GatewayConfiguration::add_relay_domain = %s",
		 IOTF(ret));
#endif

  return(ret);
}

bool GatewayConfiguration::parse_cf()
{
  // Parse the gateway configuration file.  The location of this file is
  // DEFAULT_CFDIR/gateway.cf and cannot be overridden at run time.  This
  // method should only be called once for the duration of this object.
  // Upon a failed parse, a descriptive error string will be available
  // via the parse_error method.  The absence of a configuration file is
  // permitted, and will result in a successful parse.

  // Returns: True if the configuration file was successfully parsed,
  // false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "GatewayConfiguration::parse_cf()");
#endif

  CharBuffer *cf = new CharBuffer(DEFAULT_CFDIR);

  if(cf)
  {
    cf->append("/gateway.cf");

    if(access(cf->str(), F_OK)==0)
    {
      FILE *in = fopen(cf->str(), "r");

      if(in)
      {
	lexsgstart(in, this);
	yylex();
	
	if(lexerr || lexincomplete())
	{
	  lexerr += lexincomplete();
	  
#if defined(DEBUG)
	  if(lexincomplete())
	    dlog->log_progress(DEBUG_CONFIG,
			       "Unexpected EOF (unclosed brace)");
	  
	  dlog->log_progress(DEBUG_CONFIG,
			     "%d %s encountered while parsing %s",
			     lexerr, ((lexerr == 1) ? " error" : " errors"),
			     cf->str());
#endif
	}
	else
	{
#if defined(DEBUG)
	  dlog->log_progress(DEBUG_CONFIG, "Parse of %s successful",
			     cf->str());
#endif
	  ret = true;
	}
	
	fclose(in);
      }
    }
    else
    {
#if defined(DEBUG)
      dlog->log_progress(DEBUG_CONFIG,
			 "%s does not exist, using default configuration",
			 cf->str());
#endif

      ret = true;
    }
    
    xdelete(cf);
  }
  else
    wlog->warn("GatewayConfiguration::parse_cf failed to allocate CharBuffer");

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "GatewayConfiguration::parse_cf = %s", IOTF(ret));
#endif
  
  return(ret);
}

bool GatewayConfiguration::relay_domain(char *domain)
{
  // Determine if a message with destination in <domain> should be
  // accepted for relay.

  // Returns: true if <domain> is permitted, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "GatewayConfiguration::relay_domain(%s)",
		  IONULL(domain));
#endif

  if(domain)
  {
    if(rlyall)
    {
#if defined(DEBUG)
      dlog->log_progress(DEBUG_CONFIG,
			 "Domain %s accepted: All domains permitted", domain);
#endif
      
      ret = true;
    }
    else
    {
      if(rlydmns)
      {
	for(int i = 0;i < rlydmns->entries();i++)
	{
	  if(compare_regex(domain, rlydmns->retrieve(i)))
	  {
#if defined(DEBUG)
	    dlog->log_progress(DEBUG_CONFIG,
			       "Domain %s accepted: Matched %s",
			       domain, rlydmns->retrieve(i));
#endif
      
	    ret = true;
	    break;
	  }
	}
      }

      if(!ret && rlylcl)
      {
 	// It's unlikely that we'll actually get something addressed
	// to "localhost" here, but we'll check anyway

	if(strcmp(domain, "localhost")==0)
	  ret = true;
	else
	{
	  // Check the FQDN

	  char *hostname = get_localhost_fqdn();
	  
	  if(hostname)
	  {
	    if(strcmp(domain, hostname)==0)
	      ret = true;
	    else
	    {
	      // Look up domain and see if it is an alias for hostname

	      char hbuf[BUFSIZE];	      
	      struct hostent h, *hp;
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
		// Iterate through the alias list

		for(char **q = hp->h_aliases;*q != NULL;q++)
		{
		  if(strcmp(hostname, *q)==0)
		  {
		    ret = true;
		    break;
		  }
		}
	      }
	    }
	    
	    xdelete(hostname);
	  }
	}
     }
    }
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "GatewayConfiguration::relay = %s", IOTF(ret));
#endif
  
  return(ret);
}

RelayTarget *GatewayConfiguration::relay_target(char *user)
{
  // Find a RelayTarget for <user>, if one exists.  If none does, but
  // <user> is of the form person.inst.name.via or calllist.inst.name, a
  // RelayTarget will be created.

  // Returns: A newly allocated RelayTarget that should be deleted
  // when no longer required, or NULL if not found.

  RelayTarget *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "GatewayConfiguration::relay_target(%s)",
		  IONULL(user));
#endif
  
  if(rts)
  {
    RelayTarget *rt = rts->retrieve(user);

    if(rt)
    {
      // Make a copy since we expect the returned RT to be deleted below

      ret = new RelayTarget(user, rt);
    }
  }

  if(!ret)
  {
    List *tokens = tokenize(user, ".");

    if(tokens)
    {
      if(tokens->entries() == 4 && strcmp(tokens->retrieve(0), "person")==0)
	ret = new RelayTarget(user, tokens->retrieve(1), tokens->retrieve(2),
			      tokens->retrieve(3));
      else if(tokens->entries() == 3
	      && strcmp(tokens->retrieve(0), "calllist")==0)
	ret = new RelayTarget(user, tokens->retrieve(1), tokens->retrieve(2));
    }

    xdelete(tokens);
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "GatewayConfiguration::relay_target = %d", ret);
#endif
  
  return(ret);
}

bool GatewayConfiguration::set_relay_all_domains()
{
  // Accept all messages for relaying regardless of destination domain.

  // Returns: true if fully successful, false otherwise.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC,
		  "GatewayConfiguration::set_relay_all_domains()");
  dlog->log_exit(DEBUG_MAJTRC,
		 "GatewayConfiguration::set_relay_all_domains = %s",
		 IOTF(true));
#endif

  rlyall = true;
  return(true);
}

bool GatewayConfiguration::set_relay_domains(List *regexs)
{
  // Set the list of expressions describing domains to accept relays
  // for to <regexs>.  <regexs> will be adopted and deleted when no
  // longer required.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "GatewayConfiguration::set_relay_domains(%d)",
		  regexs);
#endif

  if(!rlydmns && regexs)
  {
    rlydmns = regexs;
    ret = true;
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "GatewayConfiguration::set_relay_domains = %s",
		 IOTF(ret));
#endif

  return(ret);
}

bool GatewayConfiguration::set_relay_localhost()
{
  // Accept messages for relaying only if they have a destination domain
  // equivalent to the host the gateway is installed on.

  // Returns: true if fully successful, false otherwise.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "GatewayConfiguration::set_relay_localhost()");
  dlog->log_exit(DEBUG_MAJTRC,
		 "GatewayConfiguration::set_relay_localhost = %s",
		 IOTF(true));
#endif

  rlylcl = true;
  return(true);
}

GatewayConfiguration::~GatewayConfiguration()
{
  // Deallocate the GatewayConfiguration object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC,
		  "GatewayConfiguration::~GatewayConfiguration()");
#endif

  xhdelete(rts, RelayTarget);
  xdelete(rlydmns);
  rlyall = false;
  rlylcl = false;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC,
		 "GatewayConfiguration::~GatewayConfiguration()");
#endif
}
