/*
 * libparse822.H
 *
 * Version: $Revision: 0.1 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2004/11/26 22:03:28 $
 *
 * Copyright (c) 2004
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: libparse822.H,v $
 * Revision 0.1  2004/11/26 22:03:28  benno
 * Initial revision
 *
 */

#include "rfc822.hh"

// Committer is used to store addresses parsed by the parser
class RFC822Committer : public rfc822parser::address_committer {
public:
  RFC822Committer(List *dest) { l = dest; }
  void operator() (const rfc822address & addr) {
    if(l)
    {
      string s = addr.localpart;
      s += "@";
      s += addr.hostpart;

      l->add((char *)s.c_str());
    }
  };
  ~RFC822Committer() { l = NULL; }

private:
  List *l;
};

class RFC822Message {
public:
  RFC822Message();
  CharBuffer *body();
  int header_entries(char *header);
  char *header_entry(char *header, int entry);
  bool parse(char *msg);
  ~RFC822Message();

private:
  CharBuffer *cmsg;         // The message body
  Hashtable<List> *headers; // The message headers 
};
