/*
 * pingx.H
 *
 * Version: $Revision: 0.2 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2006/11/19 01:20:12 $
 * MT-Level: Safe
 *
 * Copyright (c) 2005 - 2006
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: pingx.H,v $
 * Revision 0.2  2006/11/19 01:20:12  benno
 * Add Log
 *
 * Revision 0.1  2006/01/24 23:35:59  benno
 * Initial revision
 *
 */

#include "survivor.H"

#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include "icmp.h"

struct pingdata {
  char *host;          // Hostname
  char *ip;            // IP address of this ping
  char hbuf[BUFSIZE];  // Structures for gethostbyname call
  struct hostent h;    // Structures for gethostbyname call
  struct hostent *hp;  // Structures for gethostbyname call
  int seqno;           // Sequence number
  struct timeval sent; // Time ICMP_ECHO sent
  struct timeval recv; // Time reply received
  char *err;           // Any error we encountered - This points to static data
};

struct pingstat {
  char *host;          // Hostname
  int sent;            // Number of packets sent to host
  int recv;            // Number of packets received from host
  long min;            // Fastest reply, microseconds
  long max;            // Slowest reply, microseconds
  long total;          // Total reply time (ms), divide by recv for average
  char *err;           // Any error we encountered - This points to static data
};
