/*
 * pingx: A lightweight, high performance ping module based on
 * GNU inetutils 1.4.2 ping.
 *
 * Version: $Revision: 0.2 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2006/11/19 01:19:20 $
 * MT-Level: Safe
 *
 * Copyright (c) 2005 - 2006
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: main.C,v $
 * Revision 0.2  2006/11/19 01:19:20  benno
 * Add Log
 *
 * Revision 0.1  2006/01/24 23:37:11  benno
 * Initial revision
 *
 */

#include "pingx.H"

// Required to use libcm
CMArgs *cmargs = NULL;

// Define our named argument structures
DefaultArg one = {
  false, NULL, NULL, 1.0, NULL, no_rel, NULL, NULL
};

ArgSpec packets = {
  "packets",     // arg name
  number_arg,     // arg type
  NULL,           // any list
  1.0,            // between lower bound
  INFINITY_ARG,   // between upper bound
  false,          // expect a list
  NULL,           // one list
  true,           // optional
  &one            // default value
};

ArgSpec throttle = {
  "throttle",     // arg name
  number_arg,     // arg type
  NULL,           // any list
  1.0,            // between lower bound
  INFINITY_ARG,   // between upper bound
  false,          // expect a list
  NULL,           // one list
  true,           // optional
  &one            // default value
};

ArgSpec maxwait = {
  "maxwait",      // arg name
  number_arg,     // arg type
  NULL,           // any list
  1.0,            // between lower bound
  INFINITY_ARG,   // between upper bound
  false,          // expect a list
  NULL,           // one list
  true,           // optional
  &one            // default value
};

ArgSpec warnloss = {
  "warnloss",     // arg name
  number_arg,     // arg type
  NULL,           // any list
  1.0,            // between lower bound
  101.0,          // between upper bound
  false,          // expect a list
  NULL,           // one list
  true,           // optional
  NULL            // default value
};

ArgSpec probloss = {
  "probloss",     // arg name
  number_arg,     // arg type
  NULL,           // any list
  1.0,            // between lower bound
  101.00,         // between upper bound
  false,          // expect a list
  NULL,           // one list
  true,           // optional
  &one            // default value
};

ArgSpec warntime = {
  "warntime",     // arg name
  number_arg,     // arg type
  NULL,           // any list
  1.0,            // between lower bound
  INFINITY_ARG,   // between upper bound
  false,          // expect a list
  NULL,           // one list
  true,           // optional
  NULL            // default value
};

ArgSpec probtime = {
  "probtime",     // arg name
  number_arg,     // arg type
  NULL,           // any list
  1.0,            // between lower bound
  INFINITY_ARG,   // between upper bound
  false,          // expect a list
  NULL,           // one list
  true,           // optional
  NULL            // default value
};

// And put them in a NULL terminated array
ArgSpec *argfmt[] = {
  &packets,
  &throttle,
  &maxwait,
  &warnloss,
  &probloss,
  &warntime,
  &probtime,
  NULL
};

int dump_ping_result(pingstat *ps, int wloss, int ploss,
		      int wtime, int ptime)
{
  // Dump the results for <ps> according to <wloss>, <ploss>, <wtime>,
  // and <ptime>.

  // Returns: The result code.
  
  int ret = MODEXEC_OK;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "dump_ping_result(%d/%s/%d)",
		  ps, (ps ? IONULL(ps->host) : ""), (ps ? ps->total : -2));
#endif
  
  if(ps && ps->host && ps->total != -1)
  {
    long avg = (ps->recv > 0) ? ps->total/ps->recv : 0;

    CharBuffer *s = new CharBuffer();
    SurvivorXML *sxml = new SurvivorXML();

    if(s && sxml)
    {
      if(ps->err)
      {
	s->append(ps->err);
	ret = MODEXEC_MISCONFIG;
      }
      else
      {
	if(ploss > 0 && ploss < 101)
	{
	  if(ps->recv == 0
	     ||
	     ((ps->sent - ps->recv) * 100)/ps->sent > ploss)
	  {
	    ret = MODEXEC_PROBLEM;
	    s->append("Packet Loss: ");
	  }
	}
	
	if(ret == MODEXEC_OK && ptime > 0)
	{
	  if(ps->recv == 0
	     ||
	     ((ps->total / ps->recv) > ptime))
	  {
	    ret = MODEXEC_PROBLEM;
	    s->append("Packet Delay: ");
	  }
	}
	
	if(ret == MODEXEC_OK && wloss > 0 && wloss < 101)
	{
	  if(ps->recv == 0
	     ||
	     ((ps->sent - ps->recv) * 100)/ps->sent > wloss)
	  {
	    ret = MODEXEC_WARNING;
	    s->append("Packet Loss: ");
	  }
	}
	
	if(ret == MODEXEC_OK && wtime > 0)
	{
	  if(ps->recv == 0
	     ||
	     ((ps->total / ps->recv) > wtime))
	  {
	    ret = MODEXEC_WARNING;
	    s->append("Packet Delay: ");
	  }
	}
	
	s->append(ps->recv);
	s->append("/");
	s->append(ps->sent);
	s->append(" packets, min=");
	s->append(ps->min);
	s->append(", max=");
	s->append(ps->max);
	s->append(", avg=");
	s->append(avg);
      }
      
      CheckResult *cr = new CheckResult(ps->host, ret, avg, s->str(), ps->max);

      if(cr)
      {
	sxml->generate(STDOUT_FILENO, cr);

	xdelete(cr);
      }
    }
      
    xdelete(s);
    xdelete(sxml);
  }
  else
    ret = MODEXEC_MISCONFIG;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "dump_ping_result = %d", ret);
#endif
  
  return(ret);
}

bool sendping(int fd, char *host, Array<pingdata> *pings, int pingid)
{
  // Send a ping over <fd> to <host>, identified by <pingid>, and
  // recorded in <pings>.  The sequence number will correlate to the
  // position in the <pings> array.

  // Returns: true if fully successful, false otherwise.  On error,
  // pings->pingid->err will contain the error description.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "sendping(%d,%s,%d,%d)", fd, IONULL(host),
		  pings, pingid);
#endif
  
  if(fd > -1 && host && pings);
  {
    // Note that we assume the entry in the array correlates to the seqno
    
    pingdata *pd = new pingdata;

    if(pd)
    {
      if(pings->add(pd))
      {
	pd->host = xstrdup(host);  // We keep what we were given for host
	pd->ip = NULL;             // since that's used to correlate results
	pd->hp = NULL;
	pd->seqno = pings->entries() - 1;
	pd->sent.tv_sec = 0;
	pd->sent.tv_usec = 0;
	pd->recv.tv_sec = 0;
	pd->recv.tv_usec = 0;
	pd->err = NULL;

	struct sockaddr_in sdest, *sin;
	sin = &sdest;
	sin->sin_family = AF_INET;

	// We could optimize here by seeing if we've already looked up
	// this host, but we expect the site to have fast DNS or nscd
	
	if(!inet_aton(host, &sin->sin_addr))
	{
	  // Name, not number, so look it up

	  int herr;

#if defined(HAVE_FUNC_GETHOSTBYNAME_R_3)
	  struct hostent_data data;
	
	  int r = gethostbyname_r(pd->host, &(pd->h), &data);
	  pd->hp = &(pd->h);
	  
	  if(r == 0)
#elif defined(HAVE_FUNC_GETHOSTBYNAME_R_5)
	    pd->hp = gethostbyname_r(host, &(pd->h), pd->hbuf, BUFSIZE, &herr);
    
	  if(pd->hp)
#elif defined(HAVE_FUNC_GETHOSTBYNAME_R_6)
	    int r = gethostbyname_r(host, &(pd->h), pd->hbuf, BUFSIZE,
				    &(pd->hp), &herr);
    
	  if(r == 0 && pd->hp)
#else
#error Supported gethostbyname_r not found
#endif
	  {
	    // For now we just look at the first address.  We could support
	    // multiple addresses by allocating additional pingdatas for
	    // each address and doing some other magic to correlate the
	    // results when we're done.

	    pd->ip = xstrdup(pd->hp->h_addr);

	    sin->sin_family = pd->hp->h_addrtype;
	    if (pd->hp->h_length > (int)sizeof(sin->sin_addr))
	      pd->hp->h_length = sizeof(sin->sin_addr);
	    
	    memcpy(&sin->sin_addr, pd->hp->h_addr, pd->hp->h_length);
	  }
	}
	else
	{
	  // We were given a number
	  pd->ip = xstrdup(host);
	}
      
	if(pd->ip)  // Did we get an address?
	{
	  int buflen;
	  u_char *ibuf;
	  
	  buflen = 8 + sizeof(icmphdr_t);
	  
	  ibuf = (u_char *)malloc(buflen);

	  if(ibuf)
	  {
	    icmp_echo_encode(ibuf, buflen, pingid, pd->seqno);
	    //icmp_timestamp_encode(ibuf, buflen, pingid, pd->seqno);
	    // timestamp not supported by all targets (eg: darwin),
	    // and doesn't necessarily have useful info anyway

	    // Before we send it, record the current time

	    if(gettimeofday(&(pd->sent), NULL)==0)
	    {
	      if(sendto(fd, (char *)ibuf, buflen, 0,
			(struct sockaddr *)&sdest, sizeof(struct sockaddr_in))
		 < 0)
		pd->err = "sendto failed";
	      else
		ret = true;
	    }
	    else
	      pd->err = "gettimeofday failed";

	    free(ibuf);
	    ibuf = NULL;
	  }
	}
	else
	  pd->err = "Unable to find address";
      }
      else
      {
	xdelete(pd);
      }
    }
    // else not much to do
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "sendping = %s", IOTF(ret));
#endif
  
  return(ret);
}

CheckResult *validate()
{
  CheckResult *rv = new CheckResult(MODEXEC_OK, 0, "Pingx OK");

  return(rv);
}

int main(int argc, char **argv)
{
  // For the current implementation, these could be global, but if this
  // ever becomes a resident .so it wouldn't be threadsafe that way.
  
  Array<pingdata> *pings = NULL;     // pings we've sent
  Hashtable<pingstat> *stats = NULL; // stats per host
  int fd = -1;                       // socket to ping out on

  // Make sure we use only 16 bits in this field, id for icmp is
  // a u_short.
  int pingident = getpid() & 0xFFFF;

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
	  pings = new Array<pingdata>();
	  stats = new Hashtable<pingstat>();

	  if(pings && stats)
	  {
	    // Create pingstats for each host

	    for(int i = 0;i < cmargs->hosts()->entries();i++)
	    {
	      pingstat *ps = new pingstat;

	      if(ps)
	      {
		ps->host = xstrdup(cmargs->hosts()->retrieve(i));
		ps->sent = 0;
		ps->recv = 0;
		ps->min = 0;
		ps->max = 0;
		ps->total = 0;
		ps->err = NULL;
		
		if(!stats->insert(ps->host, ps))
		{
		  xdelete(ps);
		}
	      }
	    }
	  
	    struct protoent *proto;

	    proto = getprotobyname("icmp");
	    
	    if(proto)
	    {
	      fd = socket(AF_INET, SOCK_RAW, proto->p_proto);
	      
	      if(fd > -1)
	      {
		int one = 1;

		setsockopt(fd, SOL_SOCKET, SO_BROADCAST, (char *)&one,
			   sizeof(one));

		// Drop root privileges
		setuid(getuid());

		// We fire off a volley of pings, one for each host,
		// then wait for results.  Then, repeat if desired.
	      
		int packets = 1;
	      
		NumberArg *parg = (NumberArg *)cmargs->arg("packets");
		
		if(parg)
		  packets = (int)parg->value();

		// While we're here, pull out the other args we'll need

		int wloss = -1;
		int ploss = -1;
		int wtime = -1;
		int ptime = -1;
		int thr = 1;
		int mwait = 1;

		parg = (NumberArg *)cmargs->arg("warnloss");
		if(parg)
		  wloss = (int)parg->value();

		parg = (NumberArg *)cmargs->arg("probloss");
		if(parg)
		  ploss = (int)parg->value();

		parg = (NumberArg *)cmargs->arg("warntime");
		if(parg)
		  wtime = (int)parg->value();

		parg = (NumberArg *)cmargs->arg("probtime");
		if(parg)
		  ptime = (int)parg->value();
		
		parg = (NumberArg *)cmargs->arg("throttle");
		if(parg)
		  thr = (int)parg->value();
		
		parg = (NumberArg *)cmargs->arg("maxwait");
		if(parg)
		  mwait = (int)parg->value();
		
		if(packets > 0)
		{
		  // Set up some structures for later

		  int buflen = 8 + sizeof(icmphdr_t);
		  u_char *ibuf = (u_char *)malloc(buflen);
		  fd_set fdset;
		  int fdmax = fd+1;
		  int n;
		  struct timeval timeout;
		  struct sockaddr_in pingfrom;
		  socklen_t fromlen = sizeof(pingfrom);
		  
		  if(ibuf)
		  {
		    for(int i = 0;i < packets;i++)
		    {
		      // Send the pings in sets of <throttle>

		      int cur = 0;
		      int togo = cmargs->hosts()->entries();

		      while(togo > 0)
		      {
			int pending = 0;

			for(int j = 0;j < thr && togo > 0;j++)
			{
			  pingstat *ps =
			    stats->retrieve(cmargs->hosts()->retrieve(cur));

			  if(ps && !ps->err)
			  {
			    if(sendping(fd, cmargs->hosts()->retrieve(cur),
					pings, pingident))
			    {
			      ps->sent++;
			      pending++;
			    }
			    else
			    {
			      pingdata *pd = pings->retrieve(pings->entries()-1);
			      if(pd)
				ps->err = pd->err;
			    }
			  }

			  cur++;
			  togo--;
			}
		      
			// Collect the results
		      
			while(pending > 0)
			{
			  FD_ZERO(&fdset);
			  FD_SET(fd, &fdset);
			  
			  // set a max wait time
			  timeout.tv_sec = mwait;
			  timeout.tv_usec = 0;
			  
			  // try to read a reply
			  n = select(fdmax, &fdset, NULL, NULL, &timeout);
			  
			  if(n > 0)
			  {
			    memset(ibuf, 0, buflen);
			    
			    n = recvfrom(fd, ibuf, buflen, 0,
					 (struct sockaddr *)&pingfrom,
					 &fromlen);

			    if(n > -1)
			    {
			      struct ip *ip;
			      icmphdr_t *icmp;
			      int rc;
			      
			      rc = icmp_generic_decode(ibuf, n, &ip, &icmp);
			      
			      if(rc > -1 && icmp->icmp_id == pingident)
			      {
				// Key on icmp->icmp_seq.  Ignore duplicate
				// receives of the same seqno.
				
				pingdata *pd = pings->retrieve(icmp->icmp_seq);
				
				if(pd && pd->recv.tv_sec == 0)
				{
				  // Mark receive time
				  gettimeofday(&(pd->recv), NULL);
				  
				  // Add to the stats
				  pingstat *ps = stats->retrieve(pd->host);
				  
				  if(ps)
				  {
				    long t = (((pd->recv.tv_sec -
						pd->sent.tv_sec)
					       * 1000000) + 
					      pd->recv.tv_usec -
					      pd->sent.tv_usec) / 1000;
				    
				    ps->recv++;
				    
				    if(ps->min == 0 || t < ps->min)
				      ps->min = t;
				  
				    if(t > ps->max)
				      ps->max = t;

				    ps->total += t;

				    if(i+1 == packets)
				    {
				      // All packets sent, dump the result
				      
				      int ret = dump_ping_result(ps, wloss,
								 ploss,
								 wtime, ptime);

				      if(ret > r)
					r = ret;
				      
				      ps->total = -1;
				    }
				  }
			      
				  pending--;
				}
			      }
			    }
			    // else recvfrom failed
			  }
			  else
			    pending = 0;  // Timed out on read
			}
		      }
		    }

		    free(ibuf);

		    // Dump results for anything we haven't yet

		    HashHandle *hh = stats->iterate_begin();

		    if(hh)
		    {
		      pingstat *ps;

		      while((ps = stats->iterate_next(hh)) != NULL)
		      {
			if(ps->total != -1)
			{
			  int ret = dump_ping_result(ps, wloss, ploss,
						     wtime, ptime);

			  if(ret > r)
			    r = ret;
			}
		      }

		      stats->iterate_end(hh);
		    }
		  }
		  else
		  {
		    r = MODEXEC_MISCONFIG;
		    cm_error(r, 0, "malloc failed");
		  }
		}
	      }
	      else
	      {
		r = MODEXEC_MISCONFIG;
		cm_error(r, 0, "Failed to obtain socket");
	      }
	    }
	    else
	    {
	      r = MODEXEC_MISCONFIG;
	      cm_error(r, 0, "Unknown protocol: icmp");
	    }
	  }
	  else
	  {
	    r = MODEXEC_MISCONFIG;
	    cm_error(r, 0, "Failed to allocate Array and Hashtable");
	  }

	  xadelete(pings, pingdata);
	  xhdelete(stats, pingstat);
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
