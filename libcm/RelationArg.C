/*
 * RelationArg: Check Module Parsed Relation Argument object
 *
 * Version: $Revision: 0.4 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2005/11/14 03:06:38 $
 * MT-Level: Safe
 *
 * Copyright (c) 2002 - 2005
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: RelationArg.C,v $
 * Revision 0.4  2005/11/14 03:06:38  benno
 * Treat all args as strings
 *
 * Revision 0.3  2003/03/04 20:54:04  benno
 * Bump copyright
 *
 * Revision 0.2  2003/01/24 02:50:23  benno
 * Use xdelete
 *
 * Revision 0.1  2002/12/16 00:57:41  benno
 * Initial revision
 *
 */

#include "survivor.H"

RelationArg::RelationArg(char *name, relation_t relation, char *x) :
  ArgParsed(name, relation_arg)
{
  // Allocate and initialize a new RelationArg object for the argument <name>,
  // of relation type <relation> with parameter <x>.

  // Returns: A newly allocated RelationArg object if successful, NULL
  // otherwise.

  rel = relation;
  xval = xstrdup(x);
  yval = NULL;
}

RelationArg::RelationArg(char *name, relation_t relation, char *x, char *y) :
  ArgParsed(name, relation_arg)
{
  // Allocate and initialize a new RelationArg object for the argument <name>,
  // of relation type <relation> with parameters <x> and <y>.

  // Returns: A newly allocated RelationArg object if successful, NULL
  // otherwise.

  rel = relation;
  xval = xstrdup(x);
  yval = xstrdup(y);
}

relation_t RelationArg::relation()
{
  // Obtain the type of this relation.

  // Returns: The type.

  return(rel);
}

char *RelationArg::x()
{
  // Obtain the x parameter of this argument.

  // Returns: The x parameter.
  
  return(xval);
}

char *RelationArg::y()
{
  // Obtain the y parameter of this argument.

  // Returns: The y parameter.
  
  return(yval);
}

RelationArg::~RelationArg()
{
  // Deallocate the RelationArg object.

  // Return: Nothing.
  
  rel = eq_rel;

  xdelete(xval);
  xdelete(yval);
}
