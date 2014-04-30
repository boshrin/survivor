/*
 * BooleanArg: Check Module Parsed Boolean Argument object
 *
 * Version: $Revision: 0.2 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2003/03/04 20:53:18 $
 * MT-Level: Safe
 *
 * Copyright (c) 2002 - 2003
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: BooleanArg.C,v $
 * Revision 0.2  2003/03/04 20:53:18  benno
 * Bump copyright
 *
 * Revision 0.1  2002/12/16 00:56:40  benno
 * Initial revision
 *
 */

#include "survivor.H"

BooleanArg::BooleanArg(char *name, bool value) : ArgParsed(name, boolean_arg)
{
  // Allocate and initialize a new BooleanArg object for the argument <name>,
  // set to <value>.

  // Returns: A newly allocated BooleanArg object if successful, NULL
  // otherwise.

  v = value;
}

bool BooleanArg::value()
{
  // Obtain the value of this argument.

  // Returns: The value.
  
  return(v);
}

BooleanArg::~BooleanArg()
{
  // Deallocate the BooleanArg object.

  // Return: Nothing.
  
  v = false;
}
