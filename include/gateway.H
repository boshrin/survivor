/*
 * gateway.H
 *
 * Version: $Revision: 0.7 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2004/11/27 00:54:50 $
 *
 * Copyright (c) 2002 - 2004
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: gateway.H,v $
 * Revision 0.7  2004/11/27 00:54:50  benno
 * Use RFC822Message
 * Changes to correspond with gateway overhaul
 *
 * Revision 0.6  2003/05/04 21:39:01  benno
 * Add logmessage2
 *
 * Revision 0.5  2003/04/09 20:05:58  benno
 * Support Debugger
 *
 * Revision 0.4  2003/03/04 20:56:09  benno
 * Add Reply, SMSReply
 *
 * Revision 0.3  2002/04/04 19:59:29  benno
 * copyright
 *
 * Revision 0.2  2002/04/02 22:06:46  benno
 * rcsify date
 *
 * Revision 0.1  2002/04/02 22:06:10  benno
 * initial revision
 *
 */

#include "survivor.H"

// Since we may need to reparse a message, we have to store it in
// memory.  To prevent runaway messages from overallocating memory,
// define an upper limit.  If this size changes, change doc/sg.html
// as well.
#define MAXMESSAGESIZE 50000

#define LOGMESSAGE2(x,y) mlog->warn("%s%s: from=%s service=%s host=%s instance=%s token=%s reply=%d",\
			     x, y,\
			     (reply->from() ? reply->from() : "unknown"),\
			     (reply->service() ? reply->service() \
			      : "unknown"),\
			     (reply->host() ? reply->host() : "unknown"),\
			     (reply->instance() ? reply->instance() \
			      : "unknown"),\
			     (reply->token() ? reply->token() : "unknown"),\
			     reply->reply())

#define LOGMESSAGE(x) LOGMESSAGE2(x,"")

class Reply {
public:
  Reply();
  char *from();
  char *host();
  char *instance();
  virtual bool parse(RFC822Message *msg) {}
  reply_t reply();
  char *service();
  char *token();
  ~Reply();

protected:
  char *f;          // From address in message
  char *h;          // Host name
  char *inst;       // Instance name
  char *svc;        // Service
  char *t;          // Token
  reply_t ry;       // Reply
};

class SMSReply : public Reply {
public:
  SMSReply() {}
  bool parse(RFC822Message *msg);
  ~SMSReply() {}

private:
};

class TwoWayReply : public Reply {
public:
  TwoWayReply() {}
  bool parse(RFC822Message *msg);
  ~TwoWayReply() {}

private:
};

// Defined in main.C
extern GatewayConfiguration *gcf;

void find_recipients(RecipientSet *rset, RFC822Message *msg, char *header);
