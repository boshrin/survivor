/*
 * CGI Parser object
 *
 * Version: $Revision: 0.16 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2006/11/24 04:30:51 $
 * MT-Level: Unsafe
 *
 * Copyright (c) 2002 - 2006
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: CGIParser.C,v $
 * Revision 0.16  2006/11/24 04:30:51  benno
 * Allow optional encoding of ampersands for better HTML compliance
 *
 * Revision 0.15  2004/11/26 22:11:14  benno
 * Add ok to hidden(omit)
 *
 * Revision 0.14  2004/09/09 12:46:35  benno
 * Add parse(url)
 *
 * Revision 0.13  2004/03/02 03:21:33  benno
 * Support multiple values for one tag
 * Add hidden(List)
 * Add url(List, List, List)
 *
 * Revision 0.12  2003/01/26 15:07:41  benno
 * Use xstrdup
 *
 * Revision 0.11  2003/01/24 03:16:16  benno
 * Use xdelete
 *
 * Revision 0.10  2002/09/05 21:58:20  benno
 * add url with 3 name/value pairs
 *
 * Revision 0.9  2002/08/06 20:22:03  selsky
 * Remove embedded nulls from format
 *
 * Revision 0.8  2002/06/03 02:13:20  toor
 * add urlfullskip()
 *
 * Revision 0.7  2002/06/02 23:16:15  toor
 * add addvalue()
 *
 * Revision 0.6  2002/04/04 20:02:47  benno
 * copyright
 *
 * Revision 0.5  2002/04/03 19:24:18  benno
 * add mt-level
 *
 * Revision 0.4  2002/04/02 23:04:31  benno
 * rcsify date
 *
 * Revision 0.3  2002/04/02 23:01:17  benno
 * Re-API url methods()
 * Add escape_url
 * Fix unescape_url calling bug
 *
 * Revision 0.2  2002/04/02 22:54:02  benno
 * Add url() methods
 * Add hidden() methods
 *
 * Revision 0.1  2002/04/02 22:52:00  benno
 * initial revision
 *
 */

#include "cgi-parser.H"

CGIParser::CGIParser()
{
  // Create a new CGIParser object, empty.

  // Returns: A new CGIParser object.

  cvalues = NULL;
  uskip = NULL;
  cgiencoded = false;
  cgiget = false;
  cgipost = false;
  hidr = NULL;
  urlr = NULL;
  max = CGI_MAX_QUERY;
}

bool CGIParser::addvalue(char *name, char *value, bool replace)
{
  // Add the <name>/<value> pair to this CGIParser, as if it were
  // parsed initially.  If <replace> is true, existing values of
  // <name> will be deleted, if found, otherwise the pair will simply
  // be inserted.

  bool r = false;
  
  if(cvalues && name && value)
  {
    CGIValue *x = cvalues->retrieve(name);

    if(x)
    {
      if(replace)
      {
	if(x->set_value(value, strlen(value)))
	  r = true;
      }
      else
      {
	if(x->add_value(value, strlen(value)))
	  r = true;
      }
    }

    if(!r)
    {
      CGIValue *x = new CGIValue();

      if(x)
      {
	x->set_name(name);
	x->set_value(value, strlen(value));

	if(cvalues->add(x))
	  r = true;
	else
	{
	  delete x;
	  x = NULL;
	}
      }
    }
  }

  return(r);
}

bool CGIParser::encoded()
{
  // Determine if this CGI was invoked via POST, with
  // ENCTYPE="multipart/form-data".  If so, the values in CGIValue objects
  // will not be NULL terminated.

  // Returns: true if this is a POST/ENCTYPE query, false otherwise.

  return(cgiencoded);
}

bool CGIParser::get()
{
  // Determine if this CGI was invoked via GET.

  // Returns: true if this is a GET query, false otherwise.

  return(cgiget);
}

char *CGIParser::hidden()
{
  // Generate HTML suitable for inclusion in a post form consisting of all
  // the values currently stored in the CGIParser.

  // Returns: A pointer to a string that remains valid until another hidden
  // call or for the duration of the CGIParser, or NULL on error.
  
  return(hidden(NULL));
}

char *CGIParser::hidden(List *omit)
{
  // Generate HTML suitable for inclusion in a post form consisting of all
  // the values currently stored in the CGIParser, omitting any flags
  // in <omit> (in addition to ones already skipped).

  // Returns: A pointer to a string that remains valid until another hidden
  // call or for the duration of the CGIParser, or NULL on error.

  return(hidden(omit, NULL));
}

char *CGIParser::hidden(List *omit, char *ok)
{
  // Generate HTML suitable for inclusion in a post form consisting of
  // all the values currently stored in the CGIParser, omitting any
  // flags in <omit> (in addition to ones already skipped).  If
  // provided, value must only contain the characters in <ok>.

  // Returns: A pointer to a string that remains valid until another hidden
  // call or for the duration of the CGIParser, or NULL on error.
  
  xdelete(hidr);

  hidr = xstrdup("\0");
  
  if(cvalues && hidr)
  {
    for(int i = 0;i < cvalues->size();i++)
    {
      CGIValue *v = cvalues->retrieve(i);
      char *vn = v->name();

      if(v && vn && (!uskip || uskip->find(vn)==-1)
	 && (!omit || omit->find(vn)==-1)
	 && (!ok || v->validate(ok)))
      {
	for(int j = 0;j < v->values();j++)
	{
	  char *vv = v->value(j);

	  if(vv)
	  {
	    char *newhidr = new char[strlen(hidr) + strlen(vn) + strlen(vv)
				    + 38];

	    if(newhidr)
	    {
	      sprintf(newhidr,
		      "%s<INPUT TYPE=hidden NAME=\"%s\" VALUE=\"%s\">\n",
		      hidr, vn, vv);

	      delete hidr;
	      hidr = newhidr;
	      newhidr = NULL;
	    }
	  }
	}
      }
    }
  }

  return(hidr);
}

int CGIParser::parse()
{
  // Parse the CGI query.

  // Returns: 0 if the parse is successful, or an index into the
  // _cgi_parse_err array to indicate an error message.

  // Determine if this as a GET or a POST, and hand off accordingly.

  char *rm = getenv("REQUEST_METHOD");

  if(rm)
  {
    // Make sure we have a List to store the parse into, tossing any previous
    // List if we were somehow invoked twice.
    
    xdelete(cvalues);

    cvalues = new CGIValueList();

    if(!cvalues)
      return(CGIERR_NOMEM);
    
    if(strcasecmp(rm, "GET")==0)
    {
      cgiget = true;
      return(parse_get());
    }
    else
    {
      if(strcasecmp(rm, "POST")==0)
      {
	cgipost = true;
	return(parse_post());
      }
    }
  }
  
  return(CGIERR_NORM);
}

int CGIParser::parse(char *url)
{
  // Parse <url>.

  // Returns: 0 if the parse is successful, or an index into the
  // _cgi_parse_err array to indicate an error message.

  // Make sure we have a List to store the parse into, tossing any previous
  // List if we were somehow invoked twice.
    
  xdelete(cvalues);

  cvalues = new CGIValueList();
  
  if(!cvalues)
    return(CGIERR_NOMEM);

  // We're simulating a get here

  cgiget = true;
  
  char *qs = xstrdup(url);

  if(qs)
  {
    int ret = parse_standard_query(qs);
    
    xdelete(qs);
    return(ret);
  }
  else
    return(CGIERR_NOQS);
}

int CGIParser::parse(int maxlen)
{
  // Parse the CGI query, but use <maxlen> as the maximum query length
  // instead of the previous default.  <maxlen> is remembered until reset.

  // Returns: 0 if the parse is successful, or an index into the
  // _cgi_parse_err array to indicate an error message.

  if(maxlen > 0)
  {
    max = maxlen;
    return(parse());
  }
  else
    return(false);
}

bool CGIParser::post()
{
  // Determine if this CGI was invoked via POST.

  // Returns: true if this is a POST query, false otherwise.

  return(cgipost);
}

char *CGIParser::url(bool encamp)
{
  // Obtain a relative url to reinvoke this script with the current
  // values stored in the Parser.  (Values may be updated by
  // retrieving a CGIValue object and modifying its contents or
  // substituted by using url(char *,char *,...).)  If <encamp> is
  // true, ampersands will be encoded as &amp; as appropriate for
  // embedding in HTML.  The url will be of the form
  // /path/sw?a=1&b=2..., suitable for use as a relative url in HTML
  // output.

  // Returns: A pointer to a string that remains valid until the next call
  // to a url method or until the CGIParser object is deleted, or NULL
  // on error.

  return(url(encamp, NULL, NULL, NULL, NULL, NULL, NULL));
}

char *CGIParser::url(bool encamp, char *name, char *value)
{
  // Obtain a relative url to reinvoke this script with the current
  // values stored in the Parser, and with the <name>/<value> pair as
  // well.  If <name> already exists in the Parser, <value> will
  // replace its original value in the output url (but not in the
  // CGIParser object).  If <encamp> is true, ampersands will be
  // encoded as &amp; as appropriate for embedding in HTML.  The url
  // will be of the form /path/sw?a=1&b=2..., suitable for use as a
  // relative url in HTML output.

  // Return: A pointer to a string that remains valid until the next call
  // to a url method or until the CGIParser object is deleted, or NULL
  // on error.

  return(url(encamp, name, value, NULL, NULL, NULL, NULL));
}

char *CGIParser::url(bool encamp, char *name, char *value,
		     char *name2, char *value2)
{
  // Obtain a relative url to reinvoke this script with the current
  // values stored in the Parser, and with the <name>/<value> and
  // <name2>/<value2> pairs appended as well.  If either name or value
  // is NULL, that pair will not be appended.  If either <name>
  // already exists in the Parser, the appropriate <value> will
  // replace the original value in the output url (but not in the
  // CGIParser object).  If <encamp> is true, ampersands will be
  // encoded as &amp; as appropriate for embedding in HTML.  The url
  // will be of the form /path/sw?a=1&b=2..., suitable for use as a
  // relative url in HTML output.

  // Returns: A pointer to a string that remains valid until the next call
  // to a url method or until the CGIParser object is deleted, or NULL
  // on error.

  return(url(encamp, name, value, name2, value2, NULL, NULL));
}

char *CGIParser::url(bool encamp, char *name, char *value,
		     char *name2, int value2)
{
  // Obtain a relative url to reinvoke this script with the current
  // values stored in the Parser, and with the <name>/<value> and
  // <name2>/<value2> pairs appended as well.  If either <name> is
  // NULL, that pair will not be appended.  If either <name> already
  // exists in the Parser, the appropriate <value> will replace the
  // original value in the output url (but not in the CGIParser
  // object).  If <encamp> is true, ampersands will be encoded as
  // &amp; as appropriate for embedding in HTML.  The url will be of
  // the form /path/sw?a=1&b=2..., suitable for use as a relative url
  // in HTML output.

  // Returns: A pointer to a string that remains valid until the next call
  // to a url method or until the CGIParser object is deleted, or NULL
  // on error.

  char v2[32];
  
  // Convert value2 to a char * and pass on.

  sprintf(v2, "%d", value2);
  return(url(encamp, name, value, name2, v2, NULL, NULL));
}

char *CGIParser::url(bool encamp, char *name, char *value,
		     char *name2, char *value2,
		     char *name3, char *value3)
{
  // Obtain a relative url to reinvoke this script with the current
  // values stored in the Parser, and with the three <name>/<value>
  // and pairs appended as well.  If either name or value is NULL for
  // any pair, that pair will not be appended.  If any <name> already
  // exists in the Parser, the appropriate <value> will replace the
  // original value in the output url (but not in the CGIParser
  // object).  If <encamp> is true, ampersands will be encoded as
  // &amp; as appropriate for embedding in HTML.  The url will be of
  // the form /path/sw?a=1&b=2..., suitable for use as a relative url
  // in HTML output.

  // Returns: A pointer to a string that remains valid until the next call
  // to a url method or until the CGIParser object is deleted, or NULL
  // on error.
  
  xdelete(urlr);

  // Track if we've substituted these values already.
  bool seen[3] = { false, false, false };

  urlr = xstrdup(getenv("SCRIPT_NAME"));

  if(urlr)
  {
    CharBuffer *rvals = new CharBuffer("?");
    int vcount = 0;

    if(rvals)
    {
      if(cvalues)
      {
	for(int i = 0;i < cvalues->size();i++)
	{
	  CGIValue *v = cvalues->retrieve(i);
	  
	  if(v)
	  {
	    char *va = v->name();
	    
	    if(va && (!uskip || uskip->find(va)==-1))
	    {
	      for(int j = 0;j < v->values();j++)
	      {
		char *vb = v->value(j);
		// Since this is get only, value ends in \0

		if(vb)
		{
		  // Substitute if requested
		  if(j == 0 && name && value && strcmp(va, name)==0)
		  {
		    vb = value;
		    seen[0] = true;
		  }
		  else if(j == 0 && name2 && value2 && strcmp(va, name2)==0)
		  {
		    vb = value2;
		    seen[1] = true;
		  }
		  else if(j == 0 && name3 && value3 && strcmp(va, name3)==0)
		  {
		    vb = value3;
		    seen[2] = true;
		  }

		  char *xva = escape_url(va);
		  char *xvb = escape_url(vb);

		  if(xva && xvb)
		  {
		    if(vcount > 0)
		      rvals->append((char *)(encamp ? "&amp;" : "&"));
		    rvals->append(xva);
		    rvals->append('=');
		    rvals->append(xvb);

		    vcount++;
		  }

		  xdelete(xva);
		  xdelete(xvb);
		}
	      }
	    }
	  }
	}
      }

      // If we haven't seen any of the names, append those next.
      // Don't check skip since we may want to append values
      // that aren't carried through.

      char *nvn[3] = { name, name2, name3 };
      char *nvv[3] = { value, value2, value3 };
      
      for(int i = 0;i < 3;i++)
      {
	if(nvn[i] && nvv[i] && !seen[i])
	{
	  char *xva = escape_url(nvn[i]);
	  char *xvb = escape_url(nvv[i]);

	  if(xva && xvb)
	  {
	    if(vcount > 0)
	      rvals->append((char *)(encamp ? "&amp;" : "&"));
	    rvals->append(xva);
	    rvals->append('=');
	    rvals->append(xvb);

	    vcount++;
	  }

	  xdelete(xva);
	  xdelete(xvb);
	}
      }

      if(vcount > 0 && rvals->str())
      {
	// Append rvals to urlr

	char *newurlr = new char[strlen(urlr) + strlen(rvals->str()) + 2];

	if(newurlr)
	{
	  sprintf(newurlr, "%s%s", urlr, rvals->str());
	  delete urlr;
	  urlr = newurlr;
	}
      }
      // else we didn't do anything, leave urlr alone
      
      xdelete(rvals);
    }
  }
  
  return(urlr);
}

char *CGIParser::url(bool encamp, List *names, List *values, List *skip)
{
  // Obtain a relative url to reinvoke this script with the current
  // values stored in the Parser, with the corresponding name/value
  // pairs from <names> and <values> appended.  Unlike the other url
  // methods, multiple values for the same name are permitted, and
  // existing names will not be replaced.  <names> provided in <skip>
  // will be omitted are NOT omitted, this allows existing values in
  // the parser to be replaced.  If <encamp> is true, ampersands will
  // be encoded as &amp; as appropriate for embedding in HTML.

  // Returns: A pointer to a string that remains valid until the next call
  // to a url method or until the CGIParser object is deleted, or NULL
  // on error.

  List *realuskip = NULL;
  
  xdelete(urlr);

  if(skip)
  {
    // XXX Begin hack (see also below and urlskip)
    realuskip = uskip;

    // Duplicate list
    uskip = new List(realuskip);
    
    if(!uskip)
      uskip = new List();

    if(uskip)
    {
      for(int i = 0;i < skip->entries();i++)
	uskip->add(skip->retrieve(i));
    }
  }
  
  CharBuffer *newurl = new CharBuffer();

  if(newurl)
  {
    if(names && values && names->entries() == values->entries())
    {
      // First, assemble the name/value pairs, escaping as we go
      
      for(int i = 0;i < names->entries();i++)
      {
	if(names->retrieve(i) && values->retrieve(i))
	{
	  char *xn = escape_url(names->retrieve(i));
	  char *xv = escape_url(values->retrieve(i));

	  if(xn && xv)
	  {
	    if(i != 0)
	      newurl->append((char *)(encamp ? "&amp;" : "&"));
	    newurl->append(xn);
	    newurl->append('=');
	    newurl->append(xv);
	  }

	  xdelete(xn);
	  xdelete(xv);
	}
      }
    }

    // Append to the script name

    char *evals = NULL;
    
    if(newurl->str() && strlen(newurl->str()) > 0)
      evals = xstrdup(newurl->str());
    
    newurl->clear();
    newurl->append(url(encamp));

    // url() just set it (again), so clear it out
    xdelete(urlr);
    
    if(evals)
    {
      newurl->append((char *)(strchr(newurl->str(), '?') ?
			      (encamp ? "&amp;" : "&") :
			      "?"));
      newurl->append(evals);
      xdelete(evals);
    }

    urlr = xstrdup(newurl->str());

    xdelete(newurl);
  }
  else
    wlog->warn("CGIParser::url failed to allocate new CharBuffer");

  if(realuskip)
  {
    // XXX Undo hack

    xdelete(uskip);
    uskip = realuskip;
  }
  
  return(urlr);
}

char *CGIParser::urlfull(bool encamp)
{
  // Obtain a full URL of the form http://hostname/path/sw?a=1&b=2....
  // This method returns https if invoked on port 443, otherwise http.
  // If <encamp> is true, ampersands will be encoded as
  // &amp; as appropriate for embedding in HTML.
  
  // Returns: A pointer to a string that remains valid until the next call
  // to a url method or until the CGIParser object is deleted, or NULL
  // on error.

  // Use url to generate the relative portion, then prepend.
  
  url(encamp);

  if(urlr)
  {
    char *servername = getenv("SERVER_NAME");
    char *protocol = "http";

    if(!servername)
      servername = "localhost";

    if(getenv("SERVER_PORT") && strcmp("SERVER_PORT", "443")==0)
      protocol = "https";

    char *newurlr = new char[strlen(protocol) + strlen(servername)
			    + strlen(urlr) + 4];

    if(newurlr)
    {
      sprintf(newurlr, "%s://%s%s", protocol, servername, urlr);

      delete urlr;
      urlr = newurlr;
      newurlr = NULL;
    }
  }

  return(urlr);
}

char *CGIParser::urlfullskip(bool encamp, char *name1, char *name2)
{
  // Obtain a full URL of the form http://hostname/path/sw?a=1&b=2....
  // This method returns https if invoked on port 443, otherwise http.
  // Skip flags named <name1> or <name2>.  If <encamp> is true,
  // ampersands will be encoded as &amp; as appropriate for embedding
  // in HTML.

  // Returns: A pointer to a string that remains valid until the next call
  // to a url method or until the CGIParser object is deleted, or NULL
  // on error.

  char *r = NULL;

  // XXX Begin hack
  List *realuskip = uskip;

  // Duplicate list
  uskip = new List(realuskip);

  if(!uskip)
    uskip = new List();

  if(uskip)
  {
    if(name1)
      uskip->add(name1);
    if(name2)
      uskip->add(name2);

    r = urlfull(encamp);

    delete uskip;
  }
  
  // Restore old skip list
  uskip = realuskip;
  realuskip = NULL;
  // XXX End hack
  
  /* XXX there is no list->remove, so urlunskip can't exist.
     As a hack, we create a temporary copy of uskip.  This should
     be replaced when list->remove is finally added.
     
  // We implement this by urlskip()'ing name1 and name2, generating the
  // full url, then unskipping name1 and name2.

  if(!name1 || urlskip(name1))
  {
    if(!name2 || urlskip(name2))
    {
      r = urlfull();

      // ignore errors
      urlunskip(name2);
    }

    // ignore errors
    urlunskip(name1);
  }
  */

  return(r);
}

char *CGIParser::urlpath()
{
  // Obtain only the path of the url: /path/sw.

  // Returns: A pointer to a string that remains valid until the next call
  // to a url method or until the CGIParser object is deleted, or NULL
  // on error.

  return(urlpath(NULL, NULL, NULL));
}

char *CGIParser::urlpath(char *login)
{
  // Obtain only the path of the url: /path/sw.  If <login> is
  // provided, "l=<login>" will also be appended.

  // Returns: A pointer to a string that remains valid until the next call
  // to a url method or until the CGIParser object is deleted, or NULL
  // on error.

  return(urlpath(NULL, NULL, login));
}

char *CGIParser::urlpath(char *name, char *val, char *login)
{
  // Obtain only the path of the url: /path/sw.  The <name>/<val>
  // pair, if provided, will be appended to the path.  If <login> is
  // provided, "l=<login>" will also be appended.  If <login> is 1,
  // any session information will also be provided.

  // Returns: A pointer to a string that remains valid until the next call
  // to a url method or until the CGIParser object is deleted, or NULL
  // on error.

  xdelete(urlr);

  urlr = xstrdup(getenv("SCRIPT_NAME"));

  if(urlr)
  {
    char *rvals = NULL;

    if(name && val)
    {
      rvals = new char[strlen(name) + strlen(val) + 2];

      if(rvals)
	sprintf(rvals, "%s=%s", name, val);
    }

    if(login)
    {
      char *newrvals = new char[strlen(login) + 8
			       + (rvals ? strlen(rvals) : 0)];

      if(newrvals)
      {
	sprintf(newrvals, "%s%sl=%s", (char *)(rvals ? rvals : ""),
		(char *)(rvals ? "&amp;" : ""), login);
	
	if(rvals)
	  delete rvals;

	rvals = newrvals;
	newrvals = NULL;

	if(strcmp(login, "1")==0)
	{
	  // Maintain the current session information, if present
	  
	  CGIValue *cv = value("sid");
	  
	  if(cv && cv->value())
	  {
	    newrvals = new char[strlen(rvals) + strlen(cv->value()) + 6];

	    if(newrvals)
	    {
	      sprintf(newrvals, "%s&sid=%s", rvals, cv->value());

	      delete rvals;
	      rvals = newrvals;
	      newrvals = NULL;
	    }
	  }
	}
      }
    }

    if(rvals)
    {
      char *ervals = escape_url(rvals);

      if(ervals)
      {
	char *newr = new char[strlen(ervals) + strlen(urlr) + 2];

	if(newr)
	{
	  sprintf(newr, "%s?%s", urlr, ervals);
	  
	  delete urlr;
	  urlr = newr;
	  newr = NULL;
	}
	
	xdelete(ervals);
      }
      // else error, just use urlr rather than unescaped rvals

      xdelete(rvals);
    }
  }

  return(urlr);
}

bool CGIParser::urlskip(char *name)
{
  // Add <name> to the list of flags not to output when rewriting urls.

  // Returns: true if fully successful, false otherwise.

  bool r = false;

  if(name)
  {
    if(!uskip)
      uskip = new List();
    
    if(uskip && uskip->add(name))
      r = true;
  }

  return(r);
}

CGIValue *CGIParser::value(char *name)
{
  // Find a CGIValue object named <name>, if one exists.  If multiple
  // values are stored for the same <name> parsed from an encoded
  // POST request, value(int) must be used.

  // Returns: A CGIValue object, if found, or NULL.

  if(name && cvalues)
    return(cvalues->retrieve(name));
  else
    return(NULL);
}

CGIValue *CGIParser::value(int index)
{
  // Obtain the CGIValue object at position <index>.

  // Returns: A CGIValue object, if found, or NULL.

  if(cvalues)
    return(cvalues->retrieve(index));
  else
    return(NULL);
}

int CGIParser::values()
{
  // Determine the number of CGIValues obtained from the parse.
  // Note that one CGIValue could have more than one value stored
  // within.

  // Returns: The number obtained, or -1.

  if(cvalues)
    return(cvalues->size());
  else
    return(-1);
}

CGIParser::~CGIParser()
{
  // Delete the CGIParser object.

  // Returns: Nothing.

  xdelete(cvalues);
  xdelete(uskip);

  cgiencoded = false;
  cgiget = false;
  cgipost = false;

  xdelete(hidr);
  xdelete(urlr);
  
  max = 0;
}

int CGIParser::parse_get()
{
  // Parse a GET query.

  // Returns: 0 if the parse is successful, or an index into the
  // _cgi_parse_err array to indicate an error message.

  char *qs = xstrdup(getenv("QUERY_STRING"));

  if(qs)
  {
    int ret = parse_standard_query(qs);
    
    xdelete(qs);
    return(ret);
  }
  else
    return(CGIERR_NOQS);
}

int CGIParser::parse_post()
{
  // Parse a POST query.

  // Returns: 0 if the parse is successful, or an index into the
  // _cgi_parse_err array to indicate an error message.

  char *cl = getenv("CONTENT_LENGTH");
  
  if(cl)
  {
    int len = atoi(cl);

    if(len > 0)
    {
      if(len > max)
	return(CGIERR_TOOBIG);

      // Allocate a block to store the query

      char *inbuf = new char[len + 1];

      if(inbuf)
      {
	int ret = CGIERR_OK;
	
	// Try to read the query

	if(fread(inbuf, len, 1, stdin))
	{
	  // NULL terminate the buffer
	  inbuf[len] = '\0';
	  
	  // If this is encoded form data, hand it off.  We can use this
	  // check because we haven't yet replaced + chars with spaces,
	  // so there shouldn't be spaces present in an unencoded POST.
  
	  if(strstr(inbuf, "Content-Disposition: form-data;"))
	  {	    
	    cgiencoded = true;
	    ret = parse_post_encoded(inbuf, len);
	  }
	  else
	  {
	    // The standard POST looks a lot like a GET.

	    ret = parse_standard_query(inbuf);
	  }
	}
	else
	  ret = CGIERR_IO;

	// Done with inbuf

	xdelete(inbuf);

	return(ret);
      }
      else
	return(CGIERR_NOMEM);
    }
    else
      return(CGIERR_NOCL);
  }
  else
    return(CGIERR_NOCL);
}

int CGIParser::parse_post_encoded(char *query, int querylen)
{
  // Parse an encoded POST query <query>, of length <querylen>, possibly
  // including interleaved NULLs, and not necessarily NULL terminated.

  // Returns: 0 if the parse is successful, or an index into the
  // _cgi_parse_err array to indicate an error message.

  if(query && querylen > 0)
  {
    // First we need look for the line part separator, which is the first
    // line up to \r\n (13,10).

    char *p = strstr(query, "\r\n");

    if(p)
    {
      *p = '\0';
      p += 2;

      char *linesep = query;
      int lslen = strlen(linesep);

      // Begin looping

      bool done = false;
      int ret = CGIERR_OK;

      while(!done)
      {
	if(!p || (p + 31 >= query + querylen))
	{
	  // Nothing else to look at, so break

	  done = true;
	}
	else
	{
	  // Move to the beginning of the next part

	  char *name = NULL;
	  char *filename = NULL;
	  char *conttype = NULL;
	  char *value = NULL;
	  int valuelen = -1;
	  
	  p = strstr(p, "Content-Disposition: form-data;");
	  
	  if(p)
	  {
	    // name="inputname" is next

	    p = strstr(p, "name=\"");

	    if(p)
	    {
	      name = p + 6;

	      p = strchr(name, '"');

	      if(p)
	      {
		// We now have the name.
		
		*p = '\0';
		p++;

		if(strncmp(p, "; filename=\"", 12)==0)
		{
		  // Parse a filename

		  filename = p + 12;

		  p = strchr(filename, '"');

		  if(p)
		  {
		    *p = '\0';
		    p++;
		  }
		  else
		    done = true;  // Unexpected lack of close quote
		}

		if(!done)
		{
		  // Move past the CR/NL.

		  p = strstr(p, "\r\n");

		  if(p)
		  {
		    p += 2;

		    if(strncmp(p, "Content-Type: ", 14)==0)
		    {
		      // Parse a content type

		      conttype = p + 14;

		      p = strchr(conttype, '\r');

		      if(p)
		      {
			*p = '\0';
			p++;
		      }
		      else
			done = true;  // Unexpected lack of close quote
		    }

		    if(!done)
		    {
		      // Move past the CR/NL.

		      p = strstr(p, "\r\n");

		      if(p)
		      {
			p += 2;
			
			// Parse the data.  To do this, since there may be
			// nulls within, we need to look for the line
			// separator.  When we find it, we backtrack past
			// the CR/NL, and mark that as the end of
			// the data.

			char *q = p;
			bool ldone = false;

			while((q - query) < (querylen - 31) && !done && !ldone)
			{
			  if(strncmp(q, linesep, lslen)==0)
			  {
			    // Match found, backtrack to the last position
			    // of the value.

			    q -= 2;

			    if(q > p)
			    {
			      valuelen = q - p;

			      value = (char *)malloc(valuelen);

			      if(value)
			      {
				for(int i = 0;i < valuelen;i++)
				{
				  value[i] = *p;
				  p++;
				}
				
				// Advance p past the line separator for the
				// next go.
				
				p += 2 + lslen;
				
				// Line sep followed by two dashes means parse
				// no more.
				
				if(strncmp(p, "--", 2)==0)
				  done = true;
			      }
			      else
			      {
				ret = CGIERR_NOMEM;
				done = true;
			      }
			    }
			    // else we leave value to be NULL

			    // Break the loop
			    ldone = true;
			  }
			  else
			    q++;
			}
		      }
		    }
		  }
		}

		// We have at least a name, so store whatever we have

		CGIValue *cv = new CGIValue();

		if(cv)
		{
		  if(cv->set_name(name) &&
		     (!filename || cv->set_filename(filename)) &&
		     (!conttype || cv->set_content_type(conttype)) &&
		     (!value || cv->set_value(value, valuelen)) &&
		     cvalues->add(cv))
		    ;
		  else
		  {
		    xdelete(cv);

		    ret = CGIERR_CGIV;
		    done = true;
		  }  
		}
		else
		{
		  ret = CGIERR_NOMEM;
		  done = true;
		}
	      }
	    }
	  }
	}
      }
      
      return(ret);
    }
    else
      return(CGIERR_NOSEP);
  }
  else
    return(CGIERR_STUPID);
}

int CGIParser::parse_standard_query(char *query)
{
  // Parse an unencoded query <query>, which is NULL terminated.

  // Returns: 0 if the parse is successful, or an index into the
  // _cgi_parse_err array to indicate an error message.

  if(query)
  {
    int ret = CGIERR_OK;

    // Unescape any encoded characters
    unescape_url(query);
    
    // Loop through the string looking for ampersands

    char *p = strtok(query, "&");

    while(p)
    {
      // We now have a string of the form name=value

      // First, convert any + chars to spaces

      for(int i = 0;i < strlen(p);i++)
	if(p[i] == '+')
	  p[i] = ' ';

      // Next, break up the name and value portions

      char *name = p;
      char *value = strchr(p, '=');

      if(value)
      {
	*value = '\0';
	value++;
	
	// Finally, store them
	
	if(!addvalue(name, value, false))
	  ret = CGIERR_CGIV;
      }
      else
      {
	ret = CGIERR_SYNTAX;
	break;
      }
      
      // Advance to the next pair
      p = strtok(NULL, "&");
    }
    
    return(ret);
  }
  else
    return(CGIERR_STUPID);
}

char *CGIParser::escape_url(char *url)
{
  // Escape the contents of url into a format suitable for a URL.

  // Returns: A pointer to a newly allocated string that should be
  // deleted when no longer required, if successful, or NULL otherwise.

  char *r = NULL;
  
  if(url)
  {
    // We convert ASCII decimal 32 (space) to '+', 0 - 31, 33 - 44,
    // 58 - 63, 91 - 96, or 123 - 255 to %##.  (42 and 95 aren't
    // actually required to be escaped.  47 is, but confuses browsers
    // when we do so.)

    // To begin, we allocate a new buffer 3 times the length of the
    // string, because the worst case is we escape every character
    // (and append a terminating NULL).

    r = new char[(strlen(url) * 3) + 1];

    if(r)
    {
      memset(r, 0, (strlen(url) * 3) + 1);
      int j = 0;

      for(int i = 0;i < strlen(url);i++)
      {
	if(url[i] == 32)
	{
	  // Replace with +
	  r[j] = '+';
	  j++;
	}
	else if((url[i] >= 45 && url[i] <= 57)
		|| (url[i] >= 64 && url[i] <= 90)
		|| (url[i] >= 97 && url[i] <= 122))
	{
	  // Just copy the character

	  r[j] = url[i];
	  j++;
	}
	else if(url[i] >= 0 && url[i] <= 255)   // Not need if char = 2^8
	{
	  // Escape the character (255 = FF)

	  r[j] = '%';
	  r[j+1] = ((url[i]/16) > 9) ? (url[i]/16)+55 : (url[i]/16)+48;
	  r[j+2] = ((url[i]%16) > 9) ? (url[i]%16)+55 : (url[i]%16)+48;
	  j+=3;
	}
	// else ignore out of range
      }
    }
  }

  return(r);
}

// These are from NCSA, and don't appear to have any associated legalese.

char CGIParser::x2c(char *what) {
    register char digit;

    digit = (what[0] >= 'A' ? ((what[0] & 0xdf) - 'A')+10 : (what[0] - '0'));
    digit *= 16;
    digit += (what[1] >= 'A' ? ((what[1] & 0xdf) - 'A')+10 : (what[1] - '0'));
    return(digit);
}

void CGIParser::unescape_url(char *url) {
    register int x,y;

    for(x=0,y=0;url[y];++x,++y) {
        if((url[x] = url[y]) == '%') {
            url[x] = x2c(&url[y+1]);
            y+=2;
        }
    }
    url[x] = '\0';
}
