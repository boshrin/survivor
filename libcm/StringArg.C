/*
 * StringArg: Check Module Parsed String Argument object
 *
 * Version: $Revision: 0.3 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2003/03/04 20:54:14 $
 * MT-Level: Safe
 *
 * Copyright (c) 2002 - 2003
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: StringArg.C,v $
 * Revision 0.3  2003/03/04 20:54:14  benno
 * Bump copyright
 *
 * Revision 0.2  2003/01/24 02:50:57  benno
 * Use xdelete
 *
 * Revision 0.1  2002/12/16 00:57:55  benno
 * Initial revision
 *
 */

#include "survivor.H"

StringArg::StringArg(char *name, char *value) : ArgParsed(name, string_arg)
{
  // Allocate and initialize a new StringArg object for the argument <name>,
  // set to <value>.

  // Returns: A newly allocated StringArg object if successful, NULL
  // otherwise.

  v = xstrdup(value);
  vs = NULL;
}

StringArg::StringArg(char *name, List *values) : ArgParsed(name, string_arg)
{
  // Allocate and initialize a new StringArg object for the argument <name>,
  // set to <values>.  <values> will be maintained by the newly created
  // object.

  // Returns: A newly allocated StringArg object if successful, NULL
  // otherwise.

  v = NULL;
  vs = values;
}

char *StringArg::value()
{
  // Obtain the value of this argument.

  // Returns: The value.
  
  return(v);
}

List *StringArg::values()
{
  // Obtain the values of this argument.

  // Returns: The values.
  
  return(vs);
}

StringArg::~StringArg()
{
  // Deallocate the StringArg object.

  // Return: Nothing.

  xdelete(v);
  xdelete(vs);
}
