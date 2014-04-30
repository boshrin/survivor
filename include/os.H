/*
 * os.H
 *
 * Version: $Revision: 0.8 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2007/01/25 13:09:49 $
 *
 * Copyright (c) 2002 - 2007
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: os.H,v $
 * Revision 0.8  2007/01/25 13:09:49  benno
 * Fix HAVE_FUNC_GETHOSTBYNAME_R on darwin
 *
 * Revision 0.7  2005/11/26 04:22:58  benno
 * Toss _HAVE_GETHOST_WITH_POINTER
 *
 * Revision 0.6  2003/05/04 21:37:46  benno
 * Autoconf
 *
 * Revision 0.5  2003/04/07 21:03:06  benno
 * Broken vsnprintf
 *
 * Revision 0.4  2003/03/04 20:57:31  benno
 * Add Darwin support
 * Clean up Solaris and Linux
 *
 * Revision 0.3  2002/05/02 22:58:50  toor
 * add _HAVE_PROC
 *
 * Revision 0.2  2002/04/26 20:12:18  benno
 * add BROKEN_THREAD_SIGNALS
 *
 * Revision 0.1  2002/04/19 19:57:28  benno
 * initial revision
 *
 */

#if defined(LINUX)
// LinuxThreads signal handling is not distributed correctly across threads
// since each thread runs as a separate process.
#define _BROKEN_THREAD_SIGNALS

// Linux has <values.h>
#define _HAVE_VALUES_H

#if defined(USE_PROCESS_KILL)
// Linux has /proc
#define _HAVE_PROC
#define _PROC_DIR "/proc"
#endif
#endif /* LINUX */

#if defined(SOLARIS6) || defined(SOLARIS7) || defined(SOLARIS8)
#define SOLARIS
// Versions prior to 9 have non-threadsafe vsnprintf
#define _BROKEN_VSNPRINTF
#endif

#if defined(SOLARIS)
// Solaris has <values.h>
#define _HAVE_VALUES_H

#if defined(USE_PROCESS_KILL)
// Solaris has /proc
#define _HAVE_PROC
#define _PROC_DIR "/proc"
#endif
#endif /* SOLARIS */

#if defined(DARWIN)
// The prototype for lockf is missing, at least through apple kernel 6.3
#define _MISSING_LOCKF_PROTOTYPE
// Darwin doesn't implement reentrant version of gethostbyX
#define _MISSING_REENTRANT_GETHOST
#define HAVE_FUNC_GETHOSTBYNAME_R_5 1
#undef HAVE_FUNC_GETHOSTBYNAME_R_6
// No sigset/sigrelse
#define _BSD_SIGNALS
// pthread_t is a pointer, not a number type
#define _PTHREADT_AS_POINTER
#endif /* DARWIN */
