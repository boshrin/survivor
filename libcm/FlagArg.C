/*
 * FlagArg: Check Module Parsed Flag Argument object
 *
 * Version: $Revision: 0.3 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2003/03/04 20:53:39 $
 * MT-Level: Safe
 *
 * Copyright (c) 2002 - 2003
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: FlagArg.C,v $
 * Revision 0.3  2003/03/04 20:53:39  benno
 * Bump copyright
 *
 * Revision 0.2  2003/01/24 02:46:31  benno
 * Use xdelete
 *
 * Revision 0.1  2002/12/16 00:57:13  benno
 * Initial revision
 *
 */

#include "survivor.H"

FlagArg::FlagArg(char *name, char *flags) : ArgParsed(name, flag_arg)
{
  // Allocate and initialize a new FlagArg object for the argument <name>,
  // with <flags> set.

  // Returns: A newly allocated FlagArg object if successful, NULL
  // otherwise.

  v = xstrdup(flags);
}

bool FlagArg::flag(char f)
{
  // Determine if <f> is a flag set in this FlagArg.

  // Returns: The value.

  if(v)
  {
    for(int i = 0;i < strlen(v);i++)
      if(v[i] == f)
	return(true);
  }

  return(false);
}

FlagArg::~FlagArg()
{
  // Deallocate the FlagArg object.

  // Return: Nothing.
  
  xdelete(v);
}
