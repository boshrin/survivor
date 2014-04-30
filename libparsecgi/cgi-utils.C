/*
 * CGI Parser Utilities
 *
 * Version: $Revision: 0.5 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2003/01/26 15:09:42 $
 * MT-Level: Safe
 *
 * Copyright (c) 2002 - 2003
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: cgi-utils.C,v $
 * Revision 0.5  2003/01/26 15:09:42  benno
 * Toss _cgi_strdup
 *
 * Revision 0.4  2002/04/04 20:02:17  benno
 * copyright
 *
 * Revision 0.3  2002/04/03 19:23:55  benno
 * add mt-level
 *
 * Revision 0.2  2002/04/02 22:51:34  benno
 * rcsify date
 *
 * Revision 0.1  2002/04/02 22:51:05  benno
 * initial revision
 *
 */

#include "cgi-parser.H"

char *_cgi_parse_err[] = {
  "Parse successful",
  "Not implemented",
  "Memory allocation error",
  "I/O error",
  "REQUEST_METHOD not provided",
  "CONTENT_LENGTH not provided",
  "Query exceeds maximum query length",
  "Stupid Programming Error",
  "No multi-part separator found",
  "Failed to set CGIValue or add to CGIValueList",
  "QUERY_STRING not provided",
  "Invalid syntax",
};
