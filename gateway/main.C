/*
 * survivor email gateway
 *
 * Version: $Revision: 0.16 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2007/01/25 13:05:38 $
 * MT-Level: Safe
 *
 * Copyright (c) 2002 - 2007
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: main.C,v $
 * Revision 0.16  2007/01/25 13:05:38  benno
 * Build correctly with debugging disabled
 *
 * Revision 0.15  2005/11/24 02:00:46  benno
 * ui_*_notifies routines return transmit modules
 *
 * Revision 0.14  2004/11/27 00:49:05  benno
 * Overhaul, add support for relays, use RFC822Message, GatewayConfiguration
 *
 * Revision 0.13  2004/05/03 21:29:45  benno
 * Change syslogging to .info
 *
 * Revision 0.12  2004/03/02 03:51:35  benno
 * libui doesn't use throw/catch anymore
 *
 * Revision 0.11  2003/11/29 05:35:39  benno
 * ui_escalate_to takes more args
 *
 * Revision 0.10  2003/05/04 21:39:26  benno
 * Don't use string type
 *
 * Revision 0.9  2003/04/09 20:15:39  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.8  2003/03/04 20:59:23  benno
 * Change handling to add sms support
 *
 * Revision 0.7  2002/12/31 04:42:21  benno
 * use libui
 *
 * Revision 0.6  2002/12/16 01:06:45  benno
 * Use try based AlertPlans in escalation
 *
 * Revision 0.5  2002/04/26 20:09:46  toor
 * call args->parse_instcf for better error information
 *
 * Revision 0.4  2002/04/04 19:57:21  benno
 * copyright
 *
 * Revision 0.3  2002/04/02 21:14:28  benno
 * rcsify date
 *
 * Revision 0.2  2002/04/02 21:14:13  benno
 * Use libsrvinit/exit
 *
 * Revision 0.1  2002/04/02 21:13:21  benno
 * initial revision
 *
 */

#include "gateway.H"

/*
 * Global Definitions
 *
 */

GatewayConfiguration *gcf = NULL;    // gcf can be global
SyslogDebugger *mlog = NULL;         // For mail logging

/*
 * main()
 *
 */

int main(int argc, char **argv)
{
  // Although we currently don't have a mechanism for enabling debugging,
  // we set up Syslog debuggers anyway.  Then we set up a third to handle
  // our additional logging.

  SyslogDebugger *d = new SyslogDebugger(LOG_MAIL);
  SyslogDebugger *w = new SyslogDebugger(LOG_MAIL);

  mlog = new SyslogDebugger(LOG_MAIL);

  if(!mlog)
    exit(1);

  if(d)
    d->set_priority(LOG_DEBUG);

  if(w)
    w->set_priority(LOG_WARNING);
  
  mlog->set_priority(LOG_INFO);
  
  // Global setup

  if(!libsrvinit(argv[0], d, w))
    exit(1);

  // Allocate Args, but don't parse it yet since we may get an instance
  // later.

  args = new Args();

  if(args)
  {
    // Grab the config, if it exists.

    gcf = new GatewayConfiguration();

    if(gcf && gcf->parse_cf())
    {
      // Read in the message from stdin, up to MAXMESSAGESIZE.
      // The maximum size isn't to make the coding easier, it's to
      // prevent runaway messages from allocating all our memory.

      char inbuf[MAXMESSAGESIZE+1];

      memset(inbuf, 0, MAXMESSAGESIZE+1);
      read(STDIN_FILENO, inbuf, MAXMESSAGESIZE);

      // Parse the message and first see if it's a two way reply

      RFC822Message *msg = new RFC822Message();
      Reply *reply = NULL;
      
      if(msg)
      {
	if(msg->parse(inbuf))
	{
	  // We now need to figure out what we just read.
	  
	  for(int i = 0;i < 2;i++)
	  {
	    switch(i)
	    {
	    case 0:
	      reply = new SMSReply();
	      break;
	    case 1:
	      reply = new TwoWayReply();
	      break;
	    }
	    
	    if(reply && !reply->parse(msg))
	    {
	      xdelete(reply);
	      
	      // Make sure body CharBuffer is reset for subsequent loops
	      
	      CharBuffer *body = msg->body();
	      
	      if(body)
		body->seek(0);
	    }
	    
	    if(reply)
	      break;
	  }
	  
	  if(reply)
	  {
	    // Take an action based on what we parsed
    
	    args->parse_args(reply->instance(), (char *)NULL, (char *)NULL);

	    if(args->parse_instcf())
	    {
	      cf = new Configuration();
	    
	      if(cf && cf->parse_cfs())
	      {
		Check *c = cf->find_check(reply->service());
		
		if(c)
		{
		  AlertState *as = new AlertState(c, reply->host());
		  
		  if(as)
		  {
		    if(as->token() && strcmp(as->token(), reply->token())==0)
		    {
		      char *err = NULL;
		    
		      switch(reply->reply())
		      {
		      case acknowledge_reply:
			if(ui_acknowledge(c, reply->host(), reply->from(),
					  DEFAULT_EXCUSE, &err))
			  LOGMESSAGE("Acknowledged alert");
			else
			  LOGMESSAGE2("WARNING: ", err);
			break;
		      case escalate_reply:
			if(ui_escalate_to(c, reply->host(), reply->from(),
					  DEFAULT_EXCUSE, &err) > -1)
			  LOGMESSAGE("Alert escalated");
			else
			  LOGMESSAGE2("WARNING: ", err);
			break;
		      case inhibit_reply:
			if(ui_inhibit(c, reply->host(), reply->from(),
				      DEFAULT_EXCUSE, &err))
			  LOGMESSAGE("Alert inhibited");
			else
			  LOGMESSAGE2("WARNING: ", err);
			break;
		      default:
			LOGMESSAGE("WARNING: Unknown reply type");
			break;
		      }
		
		      xdelete(err);
		    }
		    else
		      LOGMESSAGE("WARNING: Token mismatch");
		  }
		  else
		    wlog->warn("WARNING: Failed to allocate AlertState");
		}
		else
		  LOGMESSAGE("WARNING: Unknown service");
	      }
	      else
		wlog->warn("WARNING: Configuration parse failed");
	    }
	    else
	      wlog->warn("WARNING: Instance configuration parse failed");

	    xdelete(reply);
	  }
	  else
	  {
	    // Message doesn't appear to be a reply command, so try
	    // relaying it.

	    // There can be multiple recipients per message, each in a
	    // different instance.  Iterate through them and build a
	    // RecipientSet.

 	    RecipientSet *rset = new RecipientSet();
	    
	    if(rset)
	    {
	      find_recipients(rset, msg, "To");
	      find_recipients(rset, msg, "Cc");
	      
	      // Now send out the message once per module
	      
	      for(int i = 0;i < rset->modules();i++)
	      {
		FormattedAlert *fa =
		  new FormattedAlert(rset->address_list(i),
				     msg->header_entry("From", 0),
				     msg->header_entry("Subject", 0),
				     (char *)(msg->body() ?
					      msg->body()->str() : ""));
		
		if(fa)
		{
		  Executor *e = new Executor();
		  
		  if(e)
		  {
		    pid_t pid = e->exec_formatted_alert(fa, rset->module(i));
		    
		    if(pid > -1)
		    {
		      int rc = e->result();
		      
		      if(rc == MODEXEC_OK)
			mlog->warn("Relayed '%s' from '%s' to '%s' via '%s'",
				   msg->header_entry("Subject", 0),
				   msg->header_entry("From", 0),
				   rset->addresses(i),
				   rset->module(i));
		      else
			mlog->warn("WARNING: Module %s exited with %d relaying '%s' from '%s'",
				   rset->module(i), rc,
				   msg->header_entry("Subject", 0),
				   msg->header_entry("From", 0));
		    }
		    else
		      wlog->warn("WARNING: Failed to exec transmit module");
		  }
		  else
		    wlog->warn("WARNING: Failed to allocate Executor");
		  
		  xdelete(fa);
		}
		else
		  wlog->warn("WARNING: Failed to allocate FormattedAlert");
	      }
	      
	      xdelete(rset);
	    }
	    else
	      wlog->warn("WARNING: Failed to allocate RecipientSet");
	  }
	}
	else
	  mlog->warn("WARNING: Incoming message failed to parse");
      }
      else
	wlog->warn("WARNING: Failed to allocate RFC822Message");
    }
    else
    {
      if(gcf)
      {
	wlog->warn("WARNING: Failed to parse gateway.cf");
	
#if defined(DEBUG)
	// Get debugging output via new gcf to avoid reparse errors
	
	GatewayConfiguration *dgcf = new GatewayConfiguration();
	
	if(dgcf)
	{	    
	  dlog->set_level(DEBUG_CONFIG);
	  dgcf->parse_cf();
	  
	  xdelete(dgcf);
	}
#endif
      }
      else
	wlog->warn("WARNING: Failed to allocate GatewayConfiguration");
    }
			
    xdelete(gcf);
  }
  else
    mlog->warn("WARNING: Failed to allocate Args");

  // Clean up

  libsrvexit();
  closelog();
  
  exit(0);
}

void find_recipients(RecipientSet *rset, RFC822Message *msg, char *header)
{
  // Look up the addresses in the <header> of <msg> and determine
  // recipients as permitted by the configuration.  Permitted
  // recipients are added to <rset>.

  // Returns: Nothing.

  if(rset && msg && header)
  {
    for(int i = 0;i < msg->header_entries(header);i++)
    {
      // First, split the address up into its components

      List *addr = tokenize(msg->header_entry(header, i), "@");
    
      if(addr && addr->entries()==2)
      {
	if(gcf->relay_domain(addr->retrieve(1)))
	{
	  RelayTarget *rt = gcf->relay_target(addr->retrieve(0));

	  if(rt && rt->instance())
	  {
	    // Map the calllist or person to one or more addresses.

	    char *uierr = NULL;
	    char *tmod = NULL;

	    if(rt->calllist())
	    {
	      // CallList
	      
	      List *addrs = ui_calllist_notifies(rt->instance(),
						 rt->calllist(),
						 &tmod, &uierr);

	      if(addrs)
	      {
		// Add each address to the RecipientSet.
		
		for(int j = 0;j < addrs->entries();j++)
		  rset->add(addrs->retrieve(j), rt->calllist(), tmod);
		
		xdelete(addrs);
	      }
	      else
		mlog->warn("WARNING: ui_calllist_notifies: %s", IONULL(uierr));
	    }
	    else
	    {
	      // Person
	    
	      char *addr = ui_person_notifies(rt->instance(), rt->person(),
					      rt->via(), &tmod, &uierr);
	    
	      if(addr)
	      {
		// Add the address to the RecipientSet without CallList info.

		rset->add(addr, "", tmod);
		xdelete(addr);
	      }
	      else
		mlog->warn("WARNING: ui_person_notifies: %s", IONULL(uierr));
	    }
	    
	    xdelete(uierr);
	    xdelete(tmod);
	  }
	  else
	    mlog->warn("WARNING: No relay target found for recipient %s",
		       msg->header_entry(header, i));
	  
	  xdelete(rt);
	}
	else
	  mlog->warn("WARNING: Relay denied by domain for recipient %s",
		     msg->header_entry(header, i));
      }
      else
	mlog->warn("WARNING: Failed to parse recipient address %s",
		   msg->header_entry(header, i));
      
      xdelete(addr);
    }
  }
}
