/*
 * ArgParsed: Check Module Parsed Argument object
 *
 * Version: $Revision: 0.3 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2003/03/04 20:53:04 $
 * MT-Level: Safe
 *
 * Copyright (c) 2002 - 2003
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: ArgParsed.C,v $
 * Revision 0.3  2003/03/04 20:53:04  benno
 * Bump copyright
 *
 * Revision 0.2  2003/01/24 02:29:20  benno
 * Use xdelete
 *
 * Revision 0.1  2002/12/16 00:56:18  benno
 * Initial revision
 *
 */

#include "survivor.H"

ArgParsed::ArgParsed(char *name, argtype_t argtype)
{
  // Allocate and initialize a new ArgParsed object, for the argument <name>
  // of type <argtype>.

  // Returns: A newly allocated ArgParsed object if successful, NULL
  // otherwise.

  a = argtype;
  n = xstrdup(name);
}

argtype_t ArgParsed::argtype()
{
  // Obtain the type of this argument.

  // Returns: The argument type.
  
  return(a);
}

char *ArgParsed::name()
{
  // Obtain the name of this argument.

  // Returns: The name, or NULL.
  
  return(n);
}

ArgParsed::~ArgParsed()
{
  // Deallocate the ArgParsed object.

  // Return: Nothing.

  a = string_arg;
  xdelete(n);
}
