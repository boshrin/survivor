/*
 * protocol.H
 *
 * Version: $Revision: 0.9 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2006/11/19 01:18:07 $
 * MT-Level: Safe
 *
 * Copyright (c) 2002 - 2006
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: protocol.H,v $
 * Revision 0.9  2006/11/19 01:18:07  benno
 * Add Log
 *
 * Revision 0.8  2004/04/26 00:47:17  selsky
 * remove log
 * add smtps support
 * 
 * Revision 0.7  2003/01/29 01:25:16  benno
 * Copyright
 * 
 * Revision 0.6  2002/10/23 05:24:56  selsky
 * Add nntps/snews support.
 * 
 * Revision 0.5  2002/10/14 01:14:01  selsky
 * add pop3/pop3s support
 * 
 * Revision 0.4  2002/09/05 21:54:53  benno
 * add p_none
 * 
 * Revision 0.3  2002/04/04 21:02:12  benno
 * copyright
 * 
 * Revision 0.2  2002/04/03 21:40:30  benno
 * rcsify date
 * 
 * Revision 0.1  2002/04/03 21:40:08  benno
 * initial revision
 *
 */

#include "survivor.H"

// A pdef structure specifies a set of send text/receive regexps that
// define the protocol check.  Note that keep may be from 0 to len-1.

struct pdef {
  int len;         // How many send/receive pairs are present
  int keep;        // Which receive text to use for the output comment
  bool ssl;        // Whether or not to use SSL
  char **send;     // Text to send to the server
  char **receive;  // Regexps to expect as corresponding responses
};

// Pre-defined protocols

extern pdef p_http;
extern pdef p_https;
extern pdef p_imap;
extern pdef p_imaps;
extern pdef p_nntp;
extern pdef p_nntps;
extern pdef p_none;
extern pdef p_smtp;
extern pdef p_smtps;
extern pdef p_pop;
extern pdef p_pops;

// Utilities from ssl-utils.C

#if defined(WITH_SSL)
void  posix_locking_callback(int mode, int type, char *file, int line);
void  ssl_discontinue();
bool  ssl_initialize();
SSL  *ssl_session_connect(int sd);
void  ssl_session_disconnect(SSL *con);
#endif // WITH_SSL
