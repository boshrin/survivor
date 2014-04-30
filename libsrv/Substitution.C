/*
 * Substitution.C: A simple object for describing a replacement.
 *
 * Version: $Revision: 0.7 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2005/09/26 13:41:15 $
 * MT-Level: Unsafe
 *
 * Copyright (c) 2002 - 2005
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: Substitution.C,v $
 * Revision 0.7  2005/09/26 13:41:15  benno
 * Add sort_key
 *
 * Revision 0.6  2003/04/09 20:23:50  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.5  2003/04/06 13:40:17  benno
 * Use Debugger
 *
 * Revision 0.4  2003/01/24 18:17:36  benno
 * Use IONULL, IOTF, xdelete, and xadelete
 *
 * Revision 0.3  2002/04/04 20:11:47  benno
 * copyright
 *
 * Revision 0.2  2002/04/03 18:49:25  benno
 * rcsify date
 *
 * Revision 0.1  2002/04/03 18:48:56  benno
 * initial revision
 *
 */

#include "survivor.H"

Substitution::Substitution(time_t begins, time_t ends, char *newname,
			   char *oldname)
{
  // Allocate and initialize a new Substitution object where <begins>
  // and <ends> represent the validity period of the Substitution (up to
  // and including <ends>), and <newname> replaces <oldname>.

  // Returns: A new Substitution object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Substitution::Substitution(%d,%d,%s,%s)",
		   begins, ends, IONULL(newname), IONULL(oldname));
#endif
  
  b = begins;
  e = ends;
  n = xstrdup(newname);
  o = xstrdup(oldname);

  if((newname && !n) || (oldname && !o))
    wlog->warn("Substitution::Substitution failed to duplicate name string");

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Substitution::Substitution()");
#endif
}

time_t Substitution::begins()
{
  // Obtain the time the Substitution takes effect.

  // Returns: The time, or 0.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Substitution::begins()");
  dlog->log_exit(DEBUG_MINTRC, "Substitution::begins = %d", b);
#endif
  
  return(b);
}

time_t Substitution::ends()
{
  // Obtain the time the Substitution ends.

  // Returns: The time, or 0.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Substitution::ends()");
  dlog->log_exit(DEBUG_MINTRC, "Substitution::ends = %d", e);
#endif
  
  return(e);
}

char *Substitution::newname()
{
  // Obtain the new name to be used.

  // Returns: The name, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Substitution::newname()");
  dlog->log_exit(DEBUG_MINTRC, "Substitution::newname = %s", IONULL(n));
#endif
  
  return(n);
}

char *Substitution::oldname()
{
  // Obtain the name that is replaced.

  // Returns: The name, or NULL.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Substitution::oldname()");
  dlog->log_exit(DEBUG_MINTRC, "Substitution::oldname = %s", IONULL(o));
#endif
  
  return(o);
}

int Substitution::sort_key()
{
  // Return a sort key for this Substitution, based on the substitution
  // begin time.

  // Returns: An integer useful for sorting.

  return((int)b);
}

Substitution::~Substitution()
{
  // Deallocate the Substitution object.

  // Returns: Nothing.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Substitution::~Substitution()");
#endif

  b = 0;
  e = 0;

  xdelete(n);
  xdelete(o);
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Substitution::~Substitution()");
#endif
}
