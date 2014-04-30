/*
 * PRNG.C: survivor pseudrandom number object
 *
 * Version: $Revision: 0.7 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2004/03/01 23:31:46 $
 * MT-Level: Safe
 *  init() should only be called once
 *
 * Copyright (c) 2002 - 2003
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: PRNG.C,v $
 * Revision 0.7  2004/03/01 23:31:46  benno
 * Add s20char
 *
 * Revision 0.6  2003/04/09 20:23:49  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.5  2003/04/05 23:26:45  benno
 * Use Debugger
 *
 * Revision 0.4  2003/03/04 20:29:25  benno
 * Bump copyright
 *
 * Revision 0.3  2002/04/04 20:10:57  benno
 * copyright
 *
 * Revision 0.2  2002/04/03 18:25:30  benno
 * rcsify date
 *
 * Revision 0.1  2002/04/03 18:25:07  benno
 * initial revision
 *
 */

/* This object does not claim to provide strong random numbers, and so is
 * not useful for anything better than protecting against nuisance attacks.
 *
 */

#include "survivor.H"

PRNG::PRNG()
{
  // Allocate and initialize a new PRNG object.

  // Returns: A new PRNG object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "PRNG::PRNG()");
#endif

  memset(s, 0, 24);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "PRNG::PRNG()");
#endif
}

bool PRNG::init()
{
  // Initialize the PRNG object.  This method should only be called once.

  // Returns: true if the object is successfully initialized, false
  // otherwise.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "PRNG::init()");
#endif
  
  // Seed the 48bit PRNG.  This is a pretty weak seed, but is suitable
  // for current requirements.

  unsigned short x[3];
  struct timeval tv;

  // We ignore the unlikely failure of gettimeofday.

  gettimeofday(&tv, NULL);
  x[0] = tv.tv_sec % 65535;
  x[1] = (getpid() * x[0]) % 65535;
  x[2] = (tv.tv_usec * x[1]) % 65535;

  seed48(x);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "PRNG::init = true");
#endif
  
  return(true);
}

long PRNG::n32bit()
{
  // Obtain a 32 bit pseudo random number.

  // Returns: a 32 bit pseudo random number.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "PRNG::n32bit()");
#endif
  
  // lrand48() claims to be MT Safe

  long l = lrand48();

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "PRNG::n32bit() = %d", l);
#endif

  return(l);
}

char *PRNG::s20char()
{
  // Obtain a 20 character random string.

  // Returns: A pointer to a string that will remain valid until the
  // next call to s20char() or until the PRNG object is deleted, or
  // NULL on error.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "PRNG::s20char()");
#endif

  sprintf(s, "%10.10ld%10.10ld", n32bit(), n32bit());
  s[0] += 33;   // 33 is 'Q' - '0'

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "PRNG::s20char() = %s", s);
#endif

  return(s);
}

PRNG::~PRNG()
{
  // Delete the PRNG object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "PRNG::~PRNG()");
#endif

  memset(s, 0, 24);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "PRNG::~PRNG()");
#endif
}
