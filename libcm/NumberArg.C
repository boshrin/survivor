/*
 * NumberArg: Check Module Parsed Number Argument object
 *
 * Version: $Revision: 0.3 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2003/03/04 20:53:57 $
 * MT-Level: Safe
 *
 * Copyright (c) 2002 - 2003
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: NumberArg.C,v $
 * Revision 0.3  2003/03/04 20:53:57  benno
 * Bump copyright
 *
 * Revision 0.2  2003/01/24 02:47:56  benno
 * Use xadelete
 *
 * Revision 0.1  2002/12/16 00:57:27  benno
 * Initial revision
 *
 */

#include "survivor.H"

NumberArg::NumberArg(char *name, float value) : ArgParsed(name, number_arg)
{
  // Allocate and initialize a new NumberArg object for the argument <name>,
  // set to <value>.

  // Returns: A newly allocated NumberArg object if successful, NULL
  // otherwise.

  v = value;
  vs = NULL;
}

NumberArg::NumberArg(char *name, Array<float> *values) :
  ArgParsed(name, number_arg)
{
  // Allocate and initialize a new NumberArg object for the argument <name>,
  // set to the array of pointers to floats <values>.  <values> will be
  // deleted by the object when no longer required.

  // Returns: A newly allocated NumberArg object if successful, NULL
  // otherwise.

  v = 0.0;
  vs = values;
}

float NumberArg::value()
{
  // Obtain the value of this argument.

  // Returns: The value.
  
  return(v);
}

Array<float> *NumberArg::values()
{
  // Obtain the array of values for this argument.

  // Returns: An array of pointers to floats, or NULL.

  return(vs);
}

NumberArg::~NumberArg()
{
  // Deallocate the NumberArg object.

  // Return: Nothing.
  
  v = 0.0;

  xadelete(vs, float);
}
