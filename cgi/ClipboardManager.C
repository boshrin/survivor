/*
 * survivor web interface clipboard manager
 *
 * Version: $Revision: 0.8 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2003/04/09 20:14:28 $
 * MT-Level: Safe
 *
 * Copyright (c) 2002 - 2003
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: ClipboardManager.C,v $
 * Revision 0.8  2003/04/09 20:14:28  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.7  2003/01/27 01:50:43  benno
 * Use IONULL, IOTF, xdelete, xadelete
 *
 * Revision 0.6  2002/08/21 21:37:59  benno
 * call Clipboard with name
 *
 * Revision 0.5  2002/08/06 16:17:12  selsky
 * Remove embedded nulls in format
 *
 * Revision 0.4  2002/05/31 21:40:23  benno
 * unlink before verify_file
 *
 * Revision 0.3  2002/04/04 19:53:53  benno
 * copyright
 *
 * Revision 0.2  2002/04/02 19:50:38  benno
 * rcsify date
 *
 * Revision 0.1  2002/04/02 19:50:02  benno
 * initial revision
 *
 */

#include "cgi.H"

bool ClipboardManager::create_board(char *name)
{
  // Create a new Clipboard named <name>.

  // Returns: true if the Clipboard was successfully created, false
  // otherwise, including if a Clipboard already exists with <name>.

  bool r = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "ClipboardManager::create_board(%s)",
		  IONULL(name));
#endif

  if(name)
  {
    char *p = boardpath(name);

    if(p)
    {
      if(access(p, F_OK)!=0)
      {
	// Truncate (via unlink, in case the owner is changing) the
	// file and verify it.

	unlink(p);
	FILE *f = fopen(p, "w");
	
	if(f)
	{
	  fclose(f);

	  if(verify_file(p, FILE_OTH_NO))
	    r = true;
	  else
	    wlog->warn("ClipboardManager::create_board failed to verify %s",
		       p);
	}
	else
	  wlog->warn("ClipboardManager::create_board failed to open %s (%s)",
		     p, strerror(errno));
      }
      
      xdelete(p);
    }
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "ClipboardManager::create_board = %s", IOTF(r));
#endif

  return(r);
}

bool ClipboardManager::delete_board(char *name)
{
  // Delete the Clipboard named <name>.

  // Returns: true if the Clipboard was successfully deleted, false
  // otherwise, including if a no such clipboard exists.

  bool r = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "ClipboardManager::delete_board(%s)",
		  IONULL(name));
#endif

  if(name)
  {
    char *p = boardpath(name);

    if(p)
    {
      // Toss the file

      if(unlink(p)==0)
	r = true;
      
      xdelete(p);
    }
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "ClipboardManager::delete_board = %s", IOTF(r));
#endif

  return(r);
}

List *ClipboardManager::find_all_boards()
{
  // Obtain a list of all available Clipboards.

  // Returns: A newly allocated List object that should be deleted when
  // no longer required, or NULL on error.

  List *r = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "ClipboardManager::find_all_boards()");
#endif

  if(cgicf && cgicf->statedir())
  {
    r = new List();
    
    if(r)
    {
      // Open the state directory and look for files ending in .swb.
      // Chop off the extension and return that list.

      struct dirent *entry = allocate_dirent(cgicf->statedir());
      struct dirent *dp;

      if(entry)
      {
	DIR *dirp = opendir(cgicf->statedir());

	if(dirp)
	{
	  while(readdir_r(dirp, entry, &dp) == 0 && dp)
	  {
	    int len = strlen(dp->d_name);
	    
	    if(len > 4 && strcmp(dp->d_name+(len-4), ".swb")==0)
	    {
	      char *x = xstrdup(dp->d_name);

	      if(x)
	      {
		x[(len-4)] = '\0';  // Truncate the extension
		r->add(x);          // And add it to the list

		xdelete(x);
	      }
	    }
	  }

	  closedir(dirp);
	}
#if defined(DEBUG)
	dlog->log_progress(DEBUG_MAJTRC,
			 "ClipboardManager::find_all_boards failed to open %s",
			   cgicf->statedir());
#endif

	free(entry);
	entry = NULL;
      }
      else
	wlog->warn("ClipboardManager::find_all_boards failed to allocate dirent");
    }
    else
      wlog->warn("ClipboardManager::find_all_boards failed to allocate List");
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "ClipboardManager::find_all_boards = %d", r);
#endif
  
  return(r);
}

Clipboard *ClipboardManager::find_board(char *name)
{
  // Obtain the Clipboard named <name>, if one exists.

  // Returns: A newly allocated Clipboard object that should be deleted
  // when no longer required, or NULL on error.

  Clipboard *r = NULL;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "ClipboardManager::find_board(%s)",
		  IONULL(name));
#endif

  if(name)
  {
    char *p = boardpath(name);

    if(p)
    {
      // We check that p exists here.  This could be done in the Clipboard
      // object itself, but that isn't really any safer since this is a CGI
      // that isn't well protected against race conditions here to begin with.

      if(access(p, R_OK|W_OK)==0)
      {
	r = new Clipboard(p, name);

	if(!r)
	  wlog->warn("ClipboardManager::find_board failed to allocate Clipboard");
      }
      
      xdelete(p);
    }
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "ClipboardManager::find_board = %d", r);
#endif

  return(r);
}

char *ClipboardManager::boardpath(char *name)
{
  // Determine the path for a Clipboard named <name>.

  // Returns: A newly allocated string that should be deleted when no
  // longer required, or NULL on error.

  char *r = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "ClipboardManager::boardpath(%s)",
		  IONULL(name));
#endif
  
  if(name && cgicf && cgicf->statedir()) 
  {
    r = new char[strlen(cgicf->statedir()) + strlen(name) + 6];

    if(r)
      sprintf(r, "%s/%s.swb", cgicf->statedir(), name);
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "ClipboardManager::boardpath = %s", IONULL(r));
#endif
  
  return(r);
}
