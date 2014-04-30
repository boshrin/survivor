/*
 * ReportFormatting.C: Object to manage a report request formatting
 *
 * Version: $Revision: 0.2 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2005/11/14 03:52:31 $
 * MT-Level: Unsafe
 *
 * Copyright (c) 2004 - 2005
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: ReportFormatting.C,v $
 * Revision 0.2  2005/11/14 03:52:31  benno
 * Add ReportFormatting(module,style)
 *
 * Revision 0.1  2004/09/09 12:58:09  benno
 * Initial Revision
 *
 */

#include "survivor.H"

ReportFormatting::ReportFormatting(char *module, char *style)
{
  // Allocate and initialize a new ReportFormatting object, with
  // <module> and <style>.

  // Returns: A new ReportFormatting object.

  init(module, style, NULL, NULL);
}

ReportFormatting::ReportFormatting(char *module, char *style, char *tmpdir)
{
  // Allocate and initialize a new ReportFormatting object, with
  // <module>, <style>, and <tmpdir>.

  // Returns: A new ReportFormatting object.

  init(module, style, tmpdir, NULL);
}

ReportFormatting::ReportFormatting(char *module, char *style, char *tmpdir,
				   char *uriprefix)
{
  // Allocate and initialize a new ReportFormatting object, with
  // <module>, <style>, <tmpdir>, and <uriprefix>.

  // Returns: A new ReportFormatting object.

  init(module, style, tmpdir, uriprefix);
}

char *ReportFormatting::module()
{
  // Obtain the module for this ReportFormatting.

  // Returns: The module, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "ReportFormatting::module()");
  dlog->log_exit(DEBUG_MINTRC, "ReportFormatting::module = %s", IONULL(md));
#endif

  return(md);
}

char *ReportFormatting::style()
{
  // Obtain the style for this ReportFormatting.

  // Returns: The style, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "ReportFormatting::style()");
  dlog->log_exit(DEBUG_MINTRC, "ReportFormatting::style = %s", IONULL(st));
#endif

  return(st);
}

char *ReportFormatting::tmpdir()
{
  // Obtain the tmpdir for this ReportFormatting.

  // Returns: The tmpdir, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "ReportFormatting::tmpdir()");
  dlog->log_exit(DEBUG_MINTRC, "ReportFormatting::tmpdir = %s", IONULL(td));
#endif

  return(td);
}

char *ReportFormatting::uriprefix()
{
  // Obtain the uriprefix for this ReportFormatting.

  // Returns: The uriprefix, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "ReportFormatting::uriprefix()");
  dlog->log_exit(DEBUG_MINTRC, "ReportFormatting::uriprefix = %s", IONULL(up));
#endif

  return(up);
}

ReportFormatting::~ReportFormatting()
{
  // Deallocate the ReportFormatting object.

  // Returns: Nothing.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "ReportFormatting::~ReportFormatting()");
#endif

  xdelete(md);
  xdelete(st);
  xdelete(td);
  xdelete(up);
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "ReportFormatting::~ReportFormatting()");
#endif
}

void ReportFormatting::init(char *module, char *style, char *tmpdir,
			    char *uriprefix)
{
  // Initialize the ReportFormatting object, with <module>, <style>,
  // <tmpdir>, and <uriprefix>.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "ReportFormatting::init(%s,%s,%s,%s)",
		  IONULL(module), IONULL(style), IONULL(tmpdir),
		  IONULL(uriprefix));
#endif

  md = xstrdup(module);
  st = xstrdup(style);
  td = xstrdup(tmpdir);
  up = xstrdup(uriprefix);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "ReportFormatting::init()");
#endif
}
