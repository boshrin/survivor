/*
 * SWTag.C: Tag parsing object
 *
 * Version: $Revision: 0.1 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2004/03/02 17:12:11 $
 * MT-Level: Safe
 *
 * Copyright (c) 2003
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: SWTag.C,v $
 * Revision 0.1  2004/03/02 17:12:11  benno
 * Initial revision
 *
 */

#include "cgi.H"

/*
 * We don't subclass SWTag into OpaqueTag/ParameterTag/TokenTag because
 * it makes the constructors overly complicated.
 *
 */

SWTag::SWTag(char *rawtag, Hashtable<Variable> *vhash)
{
  // Allocate and initialize a new SWTag object, parsing the contents
  // of <rawtag> in accordance with the externally defined SWTagInfo
  // array <tags>.  <vhash> is used to translate any encountered
  // variables.

  // Returns: A new, initialized SWTag object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "SWTag::SWTag(%s,%d)", IONULL(rawtag), vhash);
#endif

  // Start by initializing the parsed data storage.  We will only use
  // one of these, depending on rawtag's type.

  ats = NULL;
  aps = NULL;
  tip = NULL;
  ao = NULL;
  
  if(rawtag && vhash)
  {
    // Perform variable substitution on rawtag before we parse it.
    // To do this, we copy rawtag into a charbuffer.

    CharBuffer *cb = new CharBuffer();
    char *xtag = NULL;
    
    if(cb)
    {
      for(int i = 0;i < strlen(rawtag);i++)
      {
	if(rawtag[i] == '$' && (i == 0 || rawtag[i-1] != '\\'))
	{
	  // Read up to the space or the end of the tag to get
	  // the variable name.

	  char *vname = new char[strlen(rawtag) + 1];

	  if(vname)
	  {
	    memset(vname, 0, strlen(rawtag) + 1);

	    for(int j = i + 1;j < strlen(rawtag);j++)
	    {
	      // ] is a nested close tag which we may get parsed, as in
	      //  @{SET foo=@[ECHO $var]}
	      
	      if(rawtag[j] != ' ' && rawtag[j] != ']')
		vname[j - (i + 1)] = rawtag[j];
	      else
		break;

	      // We shouldn't have to account for the space since we
	      // don't transfer it, the outer for loop will continue
	      // from there
	    }

	    // Advance i by a suitable length, then look up the
	    // value of the variable

	    i += strlen(vname);
	    
	    Variable *vvar = vhash->retrieve(vname);

	    if(vvar && vvar->value())
	      cb->append(vvar->value());
	    // else silently ignore undefined variables, they just
	    // get filtered
	    
	    xdelete(vname);
	  }
	  else
	    wlog->warn("SWTag::SWTag failed to allocate vname");
	}
	else
	  cb->append(rawtag[i]);
      }

      // Make a copy of the contents to work with, replacing the literal
      // string "\n" with a newline.  We should probably do a more
      // general escape processing here, but this will do for now.

      xtag = xstrreplace(cb->str(), "\\n", "\n");
      
      xdelete(cb);
    }
    else
      wlog->warn("SWTag::SWTag failed to allocate CharBuffer");
    
    if(xtag)
    {
      // The command contains the first sequence of letters, up to
      // the first space, or the end of the string.  Commands can't
      // have spaces, so we don't need to worry about escaping.

      char *p = strchr(xtag, ' ');

      if(p)
	*p = '\0';
      // else arg is entire string

      // Find the information about the command
      
      for(int i = 0;i < swtagslen;i++)
      {
	if(strcmp(swtags[i]->name, xtag)==0)
	{
	  tip = swtags[i];
	  break;
	}
      }

      if(tip)
      {
	if(p)
	{
	  // Only parse args if we have any.
	  // Advance p past the NULL we just inserted.
	  p++;

	  switch(tip->argfmt)
	  {
	  case opaque_tag:
	    // Simple, just copy the rest of the string
	    ao = xstrdup(p);
	    break;
	  case token_tag:
	    // Loop through, breaking on spaces, adding each token to
	    // the list.
	    ats = tokenize(p, " ");
	    break;
	  case parameter_tag:
	    // Loop through as for token, but split each parsed word
	    // into a Variable.
	    ats = tokenize(p, " ");

	    if(ats)
	    {
	      aps = new Hashtable<Variable>();

	      if(aps)
	      {
		for(int i = 0;i < ats->entries();i++)
		{
		  char *x = xstrdup(ats->retrieve(i));

		  if(x)
		  {
		    char *x2 = strchr(x, '=');

		    if(x2)
		    {
		      *x2 = '\0';
		      x2++;

		      Variable *v = new Variable(x, x2);

		      if(v)
		      {
			if(!aps->insert(v->name(), v))
			{
			  xdelete(v);
			}
		      }
		      else
			wlog->warn("SWTag::SWTag() failed to allocate Variable");
		    }
		    else
		      wlog->warn("SWTag::SWTag() found command '%s' argument '%s' without value",
				 tip->name, x);
		    
		    xdelete(x);
		  }
		}
	      }
	      else
		wlog->warn("SWTag::SWTag() failed to allocate Hashtable");
	    }
	    break;
	  case noarg_tag:
	    // Don't parse anything
	    break;
	  }
	}
      }
      else
	wlog->warn("SWTag::SWTag() found unknown command '%s'", xtag);
      
      xdelete(xtag);
    }
    else
      wlog->warn("SWTag::SWTag() failed to copy xtag");
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "SWTag::SWTag()");
#endif
}

char *SWTag::arg_opaque()
{
  // When the command's SWTagInfo has argfmt of opaque_tag, obtain the
  // argument string provided.

  // Returns: A pointer to a string that should not be modified, or
  // NULL if no argument was provided or if argfmt is not opaque_tag.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "SWTag::arg_opaque()");
  dlog->log_exit(DEBUG_MINTRC, "SWTag::arg_opaque = %s", IONULL(ao));
#endif
  
  return(ao);
}

char *SWTag::arg_parameter(char *name)
{
  // When the command's SWTagInfo has argfmt of parameter_tag, obtain
  // the value for the parameter <name>.

  // Returns: A pointer to a string that should not be modified, or
  // NULL if no such parameter was found or if argfmt is not
  // parameter_tag.

  char *ret = NULL;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "SWTag::arg_parameter(%s)", IONULL(name));
#endif

  if(aps)
  {
    Variable *v = aps->retrieve(name);

    if(v)
      ret = v->value();
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "SWTag::arg_parameter = %s", IONULL(ret));
#endif

  return(ret);
}

char *SWTag::arg_token(int i)
{
  // When the comnand's SWTagInfo has argfmt of token_tag or
  // parameter_tag, obtain the argument token at position <i> (from 0
  // to arg_tokens() - 1).

  // Returns: A pointer to a string that should not be modified, or
  // NULL if no such token was found or if argfmt is not token_tag
  // or parameter_tag.

  char *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "SWTag::arg_token(%d)", i);
#endif

  if(ats)
    ret = ats->retrieve(i);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "SWTag::arg_token = %s", IONULL(ret));
#endif

  return(ret);
}

int SWTag::arg_tokens()
{
  // When the command's SWTagInfo has argfmt of token_tag or
  // parameter_tag, obtain the number of argument tokens that were
  // parsed.

  // Returns: The number of tokens parsed, or 0 if no arguments were
  // parsed or if argfmt is not token_tag or parameter_tag.

  int ret = 0;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "SWTag::arg_tokens()");
#endif

  if(ats)
    ret = ats->entries();
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "SWTag::arg_tokens = %d", ret);
#endif

  return(ret);
}

SWTagInfo *SWTag::command()
{
  // Obtain the information regarding the parsed tag.

  // Returns: A pointer to a SWTagInfo object which should not be modified,
  // or NULL on error.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "SWTag::command()");
  dlog->log_exit(DEBUG_MINTRC, "SWTag::command = %d", tip);
#endif
  
  return(tip);
}

SWTag::~SWTag()
{
  // Deallocate the SWTag object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "SWTag::SWTag()");
#endif

  xdelete(ats);
  xhdelete(aps, Variable);
  xdelete(ao);
  tip = NULL;  // This is just a pointer to a static structure
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "SWTag::~SWTag()");
#endif
}
