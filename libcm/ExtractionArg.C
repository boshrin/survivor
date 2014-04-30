/*
 * ExtractionArg: Check Module Parsed Extraction Argument object
 *
 * Version: $Revision: 0.1 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2004/06/11 22:16:26 $
 * MT-Level: Safe
 *
 * Copyright (c) 2004
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: ExtractionArg.C,v $
 * Revision 0.1  2004/06/11 22:16:26  benno
 * Initial revision
 *
 */

#include "survivor.H"

ExtractionArg::ExtractionArg(char *name, extraction_t extraction, int x) :
  ArgParsed(name, extraction_arg)
{
  // Allocate and initialize a new ExtractionArg object for the
  // argument <name>, of extraction type <extraction> with parameter
  // <x>.

  // Returns: A newly allocated ExtractionArg object if successful, NULL
  // otherwise.

  ext = extraction;
  xval = x;
  yval = 0;
}

ExtractionArg::ExtractionArg(char *name, extraction_t extraction,
			     int x, int y) :
  ArgParsed(name, extraction_arg)
{
  // Allocate and initialize a new ExtractionArg object for the
  // argument <name>, of extraction type <extraction> with parameters
  // <x> and <y>.

  // Returns: A newly allocated ExtractionArg object if successful, NULL
  // otherwise.

  ext = extraction;
  xval = x;
  yval = y;
}

extraction_t ExtractionArg::extraction()
{
  // Obtain the type of this extraction.

  // Returns: The type.

  return(ext);
}

int ExtractionArg::x()
{
  // Obtain the x parameter of this argument.

  // Returns: The x parameter.
  
  return(xval);
}

int ExtractionArg::y()
{
  // Obtain the y parameter of this argument.

  // Returns: The y parameter.
  
  return(yval);
}

ExtractionArg::~ExtractionArg()
{
  // Deallocate the ExtractionArg object.

  // Return: Nothing.
  
  ext = no_ext;
  xval = 0;
  yval = 0;
}
