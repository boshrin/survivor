/*
 * protocols: Pre-defined protocols
 *
 * Version: $Revision: 0.11 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2006/11/19 01:16:36 $
 * MT-Level: Safe
 *
 * Copyright (c) 2002 - 2006
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: protocols.C,v $
 * Revision 0.11  2006/11/19 01:16:36  benno
 * Add Log
 *
 * Revision 0.10  2005/06/06 01:22:59  benno
 * POP RFC says anything after OK is optional (no space required)
 * 
 * Revision 0.9  2004/04/26 00:47:17  selsky
 * remove log
 * add smtps support
 * 
 * Revision 0.8  2003/03/31 12:51:47  benno
 * Longer SMTP conversation
 * 
 * Revision 0.7  2003/01/29 01:25:25  benno
 * Copyright
 * 
 * Revision 0.6  2002/10/23 05:24:56  selsky
 * Add nntps/snews support.
 * 
 * Revision 0.5  2002/10/14 01:14:01  selsky
 * add pop3/pop3s support
 * 
 * Revision 0.4  2002/09/05 21:55:09  benno
 * add none
 * 
 * Revision 0.3  2002/04/04 21:02:23  benno
 * copyright
 * 
 * Revision 0.2  2002/04/03 21:41:04  benno
 * rcsify date
 * 
 * Revision 0.1  2002/04/03 21:40:44  benno
 * initial revision
 *
 */

#include "protocol.H"

#define HTTPLEN 1
#define HTTPKEEP 0
char *_http_s[] = {
  "GET "HTTP_GET" HTTP/1.0\r\n\r\n"
};
char *_http_r[] = {
  "HTTP/1\\.. 200 OK.*"
};

#define IMAPLEN 2
#define IMAPKEEP 0
char *_imap_s[] = {
  NULL,
  "1 LOGOUT\r\n"
};
char *_imap_r[] = {
  "\\* OK .*",
  "\\* BYE .*"
};

#define NNTPLEN 2
#define NNTPKEEP 0
char *_nntp_s[] = {
  NULL,
  "QUIT\r\n"
};
char *_nntp_r[] = {
  "200 .*",
  "205 .*"
};

#define NONELEN 0
#define NONEKEEP 0
char *_none_s[] = {
};
char *_none_r[] = {
};

#define SMTPLEN 4
#define SMTPKEEP 0
char *_smtp_s[] = {
  NULL,
  "HELO @HOSTNAME@\r\n",
  SMTP_MAIL_FROM,
  "QUIT\r\n"
};
char *_smtp_r[] = {
  "220 .*",
  "250 .*",
  "250 .*",
  "221 .*"
};

#define POPLEN 2
#define POPKEEP 0
char *_pop_s[] = {
  NULL,
  "QUIT\r\n"
};
char *_pop_r[] = {
  "\\+OK.*",
  "\\+OK.*"
};

// Only pdefs should be visible outside this file

pdef p_http = {
  HTTPLEN,
  HTTPKEEP,
  false,
  _http_s,
  _http_r
};

pdef p_https = {
  HTTPLEN,
  HTTPKEEP,
  true,
  _http_s,
  _http_r
};

pdef p_imap = {
  IMAPLEN,
  IMAPKEEP,
  false,
  _imap_s,
  _imap_r
};

pdef p_imaps = {
  IMAPLEN,
  IMAPKEEP,
  true,
  _imap_s,
  _imap_r
};

pdef p_nntp = {
  NNTPLEN,
  NNTPKEEP,
  false,
  _nntp_s,
  _nntp_r
};

pdef p_nntps = {
  NNTPLEN,
  NNTPKEEP,
  true,
  _nntp_s,
  _nntp_r
};

pdef p_none = {
  NONELEN,
  NONEKEEP,
  false,
  _none_s,
  _none_r
};

pdef p_smtp = {
  SMTPLEN,
  SMTPKEEP,
  false,
  _smtp_s,
  _smtp_r
};

pdef p_smtps = {
  SMTPLEN,
  SMTPKEEP,
  true,
  _smtp_s,
  _smtp_r
};

pdef p_pop = {
  POPLEN,
  POPKEEP,
  false,
  _pop_s,
  _pop_r
};

pdef p_pops = {
  POPLEN,
  POPKEEP,
  true,
  _pop_s,
  _pop_r
};
