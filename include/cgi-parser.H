/*
 * CGI Parser
 *
 * Version: $Revision: 0.12 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2006/11/24 04:31:40 $
 *
 * Copyright (c) 2002 - 2006
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: cgi-parser.H,v $
 * Revision 0.12  2006/11/24 04:31:40  benno
 * Changes for ampersand encoding
 *
 * Revision 0.11  2004/11/27 00:53:49  benno
 * Add hidden(omit,ok) to parser
 *
 * Revision 0.10  2004/09/09 12:48:01  benno
 * Add parse(url)
 *
 * Revision 0.9  2004/08/25 01:50:08  benno
 * Add cgivalueint
 *
 * Revision 0.8  2004/03/02 03:24:24  benno
 * CGIValue accepts multiple values
 * New CGIParser methods
 * Add Cookie/Cookies
 *
 * Revision 0.7  2003/03/04 20:55:12  benno
 * Toss cgi_strdup
 *
 * Revision 0.6  2002/09/05 22:01:08  benno
 * add url with 3 name/value pairs
 *
 * Revision 0.5  2002/06/03 02:13:50  toor
 * urlfullskip to CGIParser
 *
 * Revision 0.4  2002/06/02 23:17:11  toor
 * addvalue to CGIParser
 *
 * Revision 0.3  2002/04/04 19:58:44  benno
 * copyright
 *
 * Revision 0.2  2002/04/02 21:43:01  benno
 * rcsify date
 *
 * Revision 0.1  2002/04/02 21:42:39  benno
 * initial revision
 *
 */

#if !defined(_CGI_PARSER_H)
#define _CGI_PARSER_H

#include "survivor.H"

// Default maximum query length, about 8MB

#define CGI_MAX_QUERY 8388608

// A useful macro

#define cgivalue(x) (x ? x->value() : NULL)
#define cgivalueint(x) (x && x->value() ? atoi(x->value()) : 0)

// CGI Parse Error Messages

#define CGIERR_OK     0
#define CGIERR_NOTIMP 1
#define CGIERR_NOMEM  2
#define CGIERR_IO     3
#define CGIERR_NORM   4
#define CGIERR_NOCL   5
#define CGIERR_TOOBIG 6
#define CGIERR_STUPID 7
#define CGIERR_NOSEP  8
#define CGIERR_CGIV   9
#define CGIERR_NOQS   10
#define CGIERR_SYNTAX 11

extern char *_cgi_parse_err[];

// A CGIValue object represents the name, value, and supporting attributes
// of one segment of form data.

class CGIValue {
public:
  CGIValue();
  bool add_value(char *value, int valuelen);
  char *content_type();
  char *filename();
  char *name();
  bool  set_filename(char *filename);
  bool  set_name(char *name);
  bool  set_content_type(char *content_type);
  bool  set_value(char *value, int valuelen);
  bool  validate(char *ok);
  char *value();
  char *value(int i);
  int   value_length();
  int   value_length(int i);
  int   values();
  ~CGIValue();

private:
  char *vfilename;
  char *vname;
  char *vtype;
  char **vvalues;
  int  *vvlens;
  int  vvsize;

  void clear_values();
};

// A CGIValueList object holds CGIValue objects.  This could probably be done
// with existing standard classes, etc.

class CGIValueList {
public:
  CGIValueList();
  bool add(CGIValue *v);
  CGIValue *retrieve(char *name);
  CGIValue *retrieve(int index);
  int size();
  ~CGIValueList();

private:
  CGIValue **values;
  int len;
};

// The CGIParser object handles the main parsing operations.

class CGIParser {
public:
  CGIParser();
  bool addvalue(char *name, char *value, bool replace);
  bool encoded();
  bool get();
  char *hidden();
  char *hidden(List *omit);
  char *hidden(List *omit, char *ok);
  int  parse();
  int  parse(char *url);
  int  parse(int maxlen);
  bool post();
  char *url(bool encamp);
  char *url(bool encamp, char *name, char *value);
  char *url(bool encamp, char *name, char *value, char *name2, char *value2);
  char *url(bool encamp, char *name, char *value, char *name2, int value2);
  char *url(bool encamp, char *name, char *value, char *name2, char *value2,
	    char *name3, char *value3);
  char *url(bool encamp, List *names, List *values, List *skip);
  char *urlfull(bool encamp);
  char *urlfullskip(bool encamp, char *name1, char *name2);
  char *urlpath();
  char *urlpath(char *login);
  char *urlpath(char *name, char *val, char *login);
  bool urlskip(char *name);
  CGIValue *value(char *name);
  CGIValue *value(int index);
  int values();
  ~CGIParser();

private:
  CGIValueList *cvalues;
  List *uskip;
  bool cgiencoded;
  bool cgiget;
  bool cgipost;
  char *hidr;
  char *urlr;
  int max;

  int parse_get();
  int parse_post();
  int parse_post_encoded(char *query, int querylen);
  int parse_standard_query(char *query);
  char *escape_url(char *url);
  char x2c(char *what);
  void unescape_url(char *url);
};

class Cookie {
public:
  Cookie(char *name, char *value);
  Cookie(char *name, char *value, char *path);
  char *name();
  char *path();
  char *value();
  ~Cookie();

private:
  char *n;   // The name of the cookie
  char *p;   // The path of the cookie
  char *v;   // The value of the cookie
};

class Cookies {
public:
  Cookies();
  bool add(Cookie *cookie);
  Cookie *cookie(char *name);
  bool dump(int fd);
  bool parse();
  ~Cookies();

private:
  Array<Cookie> *ckin;     // Array of parsed cookies
  Array<Cookie> *ckout;    // Array of outgoing cookies
};

#endif // _CGI_PARSER_H
