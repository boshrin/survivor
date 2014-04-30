/*
 * CGI Parser CGIValueList object
 *
 * Version: $Revision: 0.6 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2003/01/26 15:09:08 $
 * MT-Level: Unsafe
 *
 * Copyright (c) 2002 - 2003
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: CGIValueList.C,v $
 * Revision 0.6  2003/01/26 15:09:08  benno
 * Use xdelete
 *
 * Revision 0.5  2002/04/04 20:03:05  benno
 * copyright
 *
 * Revision 0.4  2002/04/03 19:25:14  benno
 * add mt-level
 *
 * Revision 0.3  2002/04/02 23:07:33  benno
 * rcsify date
 *
 * Revision 0.2  2002/04/02 23:07:24  benno
 * Fix bug in retrieve(int)
 *
 * Revision 0.1  2002/04/02 23:06:37  benno
 * initial revision
 *
 */

#include "cgi-parser.H"

CGIValueList::CGIValueList()
{
  // Allocate a new CGIValueList object, empty.

  // Returns: A new CGIValueList object.

  values = NULL;
  len = 0;
}

bool CGIValueList::add(CGIValue *v)
{
  // Add the CGIValue <v> to this CGIValueList.

  // Returns: true if fully successful, false otherwise.

  if(v)
  {
    int newlen = len + 1;
    CGIValue **newvalues = (CGIValue **)malloc(sizeof(CGIValue *) * newlen);

    if(newvalues)
    {
      // Copy old values
      
      for(int i = 0;i < len;i++)
	newvalues[i] = values[i];

      // Append new value
      
      newvalues[len] = v;

      // Toss old array and install new array
      
      free(values);
      values = newvalues;
      len = newlen;

      return(true);
    }
  }

  return(false);
}

CGIValue *CGIValueList::retrieve(char *name)
{
  // Retrieve the CGIValue object named <name>.  If more than one such object
  // exists, only the first one will be returned.

  // Returns: The CGIValue object, or NULL.

  if(values && name)
  {
    for(int i = 0;i < len;i++)
    {
      if(values[i])
      {
	char *n = values[i]->name();

	if(n && strcmp(n, name)==0)
	  return(values[i]);
      }
    }
  }

  return(NULL);
}

CGIValue *CGIValueList::retrieve(int index)
{
  // Retrieve the CGIValue object at position <index>, if such an object
  // exists.

  // Returns: The CGIValue object, or NULL.

  if(values && index >= 0 && index < len)
    return(values[index]);
  else
    return(NULL);
}

int CGIValueList::size()
{
  // Determine the number of CGIValue objects stored in this CGIValueList.

  // Returns: The number of objects.

  return(len);
}

CGIValueList::~CGIValueList()
{
  // Deallocate the CGIValueList object.  This will also deallocate the
  // CGIValue objects stored within.

  // Returns: Nothing.

  if(values)
  {
    for(int i = 0;i < len;i++)
    {
      xdelete(values[i]);
    }

    free(values);
    values = NULL;
  }

  len = 0;
}
