/*
 * CGI Parser CGIValue object
 *
 * Version: $Revision: 0.7 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2004/03/02 03:20:40 $
 * MT-Level: Unsafe
 *
 * Copyright (c) 2002 - 2004
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: CGIValue.C,v $
 * Revision 0.7  2004/03/02 03:20:40  benno
 * Allow multiple values
 *
 * Revision 0.6  2003/01/26 15:09:28  benno
 * Use xstrdup, xdelete
 *
 * Revision 0.5  2002/04/04 20:02:57  benno
 * copyright
 *
 * Revision 0.4  2002/04/03 19:24:42  benno
 * add mt-level
 *
 * Revision 0.3  2002/04/02 23:06:00  benno
 * rcsify date
 *
 * Revision 0.2  2002/04/02 23:05:50  benno
 * Off by one bug fix
 * Add validate
 *
 * Revision 0.1  2002/04/02 23:04:59  benno
 * initial revision
 *
 */

#include "cgi-parser.H"

CGIValue::CGIValue()
{
  // Create a new CGIValue, empty, object to store parsed information in.

  // Returns: A new CGIValue object.

  vfilename = NULL;
  vname = NULL;
  vtype = NULL;
  vvalues = NULL;
  vvlens = NULL;
  vvsize = 0;
}

bool CGIValue::add_value(char *value, int valuelen)
{
  // Add <value> of length <len> to this CGIValue.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;

  if(value && valuelen > 0)
  {
    char **newvvalues = (char **)malloc((vvsize + 1) * sizeof(char *));

    if(newvvalues)
    {
      int *newvvlens = (int *)malloc((vvsize + 1) * sizeof(int));

      if(newvvlens)
      {
	// Move old data

	for(int i = 0;i < vvsize;i++)
	{
	  newvvalues[i] = vvalues[i];
	  newvvlens[i] = vvlens[i];
	}

	// Add new data

	newvvalues[vvsize] = new char[valuelen + 1];

	if(newvvalues[vvsize])
	{
	  memset(newvvalues[vvsize], 0, valuelen + 1);
	  memcpy(newvvalues[vvsize], value, valuelen);
	  newvvlens[vvsize] = valuelen;
	}
	else
	  newvvlens[vvsize] = 0;

	// Toss old arrays and store new ones

	if(vvsize > 0)
	{
	  free(vvalues);
	  free(vvlens);
	}

	vvalues = newvvalues;
	vvlens = newvvlens;
	vvsize++;

	ret = true;
      }
      else
	free(newvvalues);
    }
  }
  
  return(ret);
}

char *CGIValue::content_type()
{
  // Obtain the content type attribute of this CGIValue, if one was set.

  // Returns: The content type, or NULL.

  return(vtype);
}

char *CGIValue::filename()
{
  // Obtain the filename attribute of this CGIValue, if one was set.

  // Returns: The filename, or NULL.

  return(vfilename);
}

char *CGIValue::name()
{
  // Obtain the name attribute of this CGIValue, if one was set.

  // Returns: The name, or NULL.

  return(vname);
}

bool CGIValue::set_filename(char *filename)
{
  // Set the filename for this CGIValue to <filename>.  The data is copied
  // internally.

  // Returns: true if fully successful, false otherwise.

  if(filename)
  {
    xdelete(vfilename);
    
    vfilename = xstrdup(filename);

    if(vfilename)
      return(true);
  }

  return(false);
}

bool CGIValue::set_name(char *name)
{
  // Set the name for this CGIValue to <name>.  The data is copied internally.

  // Returns: true if fully successful, false otherwise.

  if(name)
  {
    xdelete(vname);
    
    vname = xstrdup(name);

    if(vname)
      return(true);
  }

  return(false);
}

bool CGIValue::set_content_type(char *content_type)
{
  // Set the content type for this CGIValue to <content_type>.  The data is
  // copied internally.

  // Returns: true if fully successful, false otherwise.

  if(content_type)
  {
    xdelete(vtype);

    vtype = xstrdup(content_type);

    if(vtype)
      return(true);
  }

  return(false);
}

bool CGIValue::set_value(char *value, int valuelen)
{
  // Set the value for this CGIValue to <value>, which is of length
  // <valuelen>.  <value> may include intermediate NULLs and need not
  // be NULL terminated, so long as it is not greater than <valuelen>
  // bytes.  An internal copy is made, with a terminating NULL
  // appended (but without affecting <valuelen>.)  Existing values
  // are deleted.

  // Returns: true if fully successful, false otherwise.

  clear_values();
  return(add_value(value, valuelen));
}

bool CGIValue::validate(char *ok)
{
  // Verify that the values of this CGIValue only contains characters in
  // the string <ok>.  (The filename, name, and content_type contents
  // are not checked.)

  // Returns: true if the characters in value() are all within <ok>,
  // false otherwise.

  if(ok)
  {
    // This is an O(n*m) operation for each value.  Yum!

    for(int k = 0;k < vvsize;k++)
    {
      if(vvalues[k])
      {
	for(int i = 0;i < vvlens[k];i++)
	{
	  bool match = false;
      
	  for(int j = 0;j < strlen(ok);j++)
	    if(vvalues[k][i] == ok[j])
	    {
	      match = true;
	      break;
	    }
	  
	  if(!match)
	    return(false);
	}
      }
    }
  }
  else
    return(false);
  
  return(true);
}

char *CGIValue::value()
{
  // Obtain the contents of this CGIValue, if provided.  Although the
  // value will be NULL terminated, if CGIParser->post() and
  // CGIParser->encoded() both return true, the value may contain
  // intermediate NULLs.  In such a case, value_length() should be used
  // to determine the actual length of the reply.

  // Returns: The value, or NULL.

  return(value(0));
}

char *CGIValue::value(int i)
{
  // Obtain the contents of this CGIValue, in position <i>, where <i>
  // may be between 0 and values() - 1.  Although the value will be
  // NULL terminated, if CGIParser->post() and CGIParser->encoded()
  // both return true, the value may contain intermediate NULLs.  In
  // such a case, value_length(i) should be used to determine the
  // actual length of the reply.

  // Returns: The value at position <i>, or NULL.

  if(i >= 0 && i < vvsize && vvalues)
    return(vvalues[i]);
  else
    return(NULL);
}

int CGIValue::value_length()
{
  // Determine the length of the contents returned by the value() method.

  // Returns: The length, or -1 on error, including if no value was stored.

  return(value_length(0));
}

int CGIValue::value_length(int i)
{
  // Determine the length of the contents returned by the value(int) method.

  // Returns: The length, or -1 on error, including if no value was stored.

  if(i >= 0 && i < vvsize && vvlens)
    return(vvlens[i]);
  else
    return(0);
}

int CGIValue::values()
{
  // Determine the number of values stored within.

  // Returns: The number of values.

  return(vvsize);
}

CGIValue::~CGIValue()
{
  // Delete the CGIValue object, also deleting any information stored within.

  // Returns: Nothing.

  xdelete(vfilename);
  xdelete(vname);
  xdelete(vtype);

  clear_values();
}

void CGIValue::clear_values()
{
  // Delete the values stored within the CGIValue object.

  // Returns: Nothing.

  if(vvsize > 0 && vvalues && vvlens)
  {
    for(int i = 0;i < vvsize;i++)
    {
      if(vvalues[i])
      {
	if(vvlens[i] > 0)
	{
	  // Zero out value, in case it held sensitive information

	  memset(vvalues[i], 0, vvlens[i]);
	}

	delete(vvalues[i]);
      }
    }

    free(vvalues);
    vvalues = NULL;
    free(vvlens);
    vvlens = NULL;
    vvsize = 0;
  }
}
