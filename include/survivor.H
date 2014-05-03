/*
 * survivor.H
 *
 * Version: $Revision: 0.27 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2007/01/25 13:10:33 $
 *
 * Copyright (c) 2002 - 2007
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 *
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: survivor.H,v $
 * Revision 0.27  2007/01/25 13:10:33  benno
 * Add _HAVE_LIBINTL_H
 *
 * Revision 0.26  2005/04/09 02:44:14  benno
 * Add MODEXEC_NOTYET, SET_CHECKRESULT
 *
 * Revision 0.25  2004/11/27 00:55:56  benno
 * Add resolv.h, libparse822, lexinit, lexsgstart
 *
 * Revision 0.24  2004/08/25 01:54:51  toor
 * Add WITH_MCD
 * Add DEFAULT_TDIR
 * Add utils.H
 *
 * Revision 0.23  2004/03/02 03:48:51  benno
 * Initial changes for localization
 * lexswstart
 *
 * Revision 0.22  2003/11/29 05:31:28  benno
 * Add MAYBE_EXCUSE
 *
 * Revision 0.21  2003/10/06 23:23:40  benno
 * Fix typo
 *
 * Revision 0.20  2003/06/25 21:31:13  benno
 * Toss MODEXEC_INVALID
 *
 * Revision 0.19  2003/06/17 15:11:44  benno
 * Add libparsexms, intbufsize, modexec_invalid
 *
 * Revision 0.18  2003/05/04 21:37:23  benno
 * Autoconf changes
 * Remove <cstring>
 *
 * Revision 0.17  2003/04/09 20:06:38  benno
 * Add math, stdarg.h
 *
 * Revision 0.16  2003/03/31 14:52:12  benno
 * Add xhdelete
 * Define MAXHOSTNAMELEN if missing
 *
 * Revision 0.15  2003/03/04 20:58:03  benno
 * Add IONULL, IOTF, lockf for Darwin
 *
 * Revision 0.14  2002/12/31 04:41:14  benno
 * xdelete, xadelete
 * New DEFAULTs
 * libui.H
 *
 * Revision 0.13  2002/08/06 15:35:04  selsky
 * Add header for C++ strings
 *
 * Revision 0.12  2002/08/06 14:20:09  selsky
 * Use cstring instead of string.h
 *
 * Revision 0.11  2002/08/05 20:08:36  selsky
 * be more C++ compliant
 *
 * Revision 0.10  2002/06/02 23:06:48  toor
 * change verify_directory to chgrp existing directories
 *
 * Revision 0.9  2002/05/31 21:38:53  benno
 * rollback v0.7 runningasinst mod
 *
 * Revision 0.8  2002/05/20 16:55:01  benno
 * change verify file chmod order
 *
 * Revision 0.7  2002/05/10 20:07:15  benno
 * maybe include procfs
 * use runningasinst in verify_file
 *
 * Revision 0.6  2002/04/24 17:03:22  toor
 * change default timeout to 45 seconds
 *
 * Revision 0.5  2002/04/19 20:04:55  benno
 * add os.H
 * adjust some system includes
 * cast (uid_t)-1
 *
 * Revision 0.4  2002/04/04 21:21:09  benno
 * copyright
 *
 * Revision 0.3  2002/04/02 22:13:23  benno
 * rcsify date
 *
 * Revision 0.2  2002/04/02 22:12:51  benno
 * Cleanup
 * Rename from scheduler.H
 * Add verify_file
 *
 */

using namespace std;

#define _REENTRANT
#define _POSIX_PTHREAD_SEMANTICS

#include "config.H"
#include "os.H"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <syslog.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#if defined(_HAVE_VALUES_H)
#include <values.h>
#endif // _HAVE_VALUES_H
#include <netdb.h>
#include <resolv.h>
#include <pwd.h>
#include <grp.h>
#include <regex.h>
#include <signal.h>
#include <math.h>
#include <strings.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#if defined(_HAVE_LIBINTL_H)
#include <libintl.h>
#endif // _HAVE_LIBINTL_H
#include <unistd.h>
#include <fstream>
#include <iostream>

#if defined(_HAVE_PROC) && defined(SOLARIS)
#include <procfs.h>
#endif

#if defined(WITH_SSL)
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#endif

#if !defined(MAXHOSTNAMELEN)
// Missing on Linux
#define MAXHOSTNAMELEN 256
#endif

// Localization shortcuts
#if defined(_HAVE_LIBINTL_H)
#define _t(x) gettext(x)
#else
#define _t(x) x
#endif

#include "debug.H"
#include "typedefs.H"
#include "utils.H"
#include "libsrv.H"
#include "libcm.H"
#include "libui.H"
#include "libparsexml.H"
#include "libparse822.H"

// Some default values
#define DEFAULT_ATHREADS 5
#define DEFAULT_CTHREADS 15
#define DEFAULT_CFDIR "/etc/survivor"
#define DEFAULT_HDIR  "/var/survivor/history"
#define DEFAULT_HPDIR "/etc/survivor/help"
#define DEFAULT_INSTANCE "default"
#define DEFAULT_INSTCF "/etc/survivor/instance.cf"
#define DEFAULT_SDIR  "/var/survivor/state"
#define DEFAULT_TDIR "/tmp"
#define DEFAULT_TIMEOUT 45
#define DEFAULT_ALERT_TIMEOUT 300
#define DEFAULT_ALERT_THROTTLE 10
#define DEFAULT_FIX_TIMEOUT 120
#define DEFAULT_MAXDEPEND 10
#define DEFAULT_EXCUSE "(no reason given)"
#define MAYBE_EXCUSE(x) (char *)(x ? x : DEFAULT_EXCUSE)
#define BUFSIZE 1024
#define INTBUFSIZE 64

// Module exit codes
#define MODEXEC_OK        0
#define MODEXEC_PROBLEM   1
#define MODEXEC_WARNING   2
#define MODEXEC_NOTICE    3
#define MODEXEC_MISCONFIG 4
#define MODEXEC_TIMEDOUT  5
#define MODEXEC_DEPEND    6    // Failed dependency: behave appropriately
#define MODEXEC_NOTYET    7    // Module has not yet executed
#define MODEXEC_MAXRETURN 1000

// These allow us to access and manipulate the state of lex

class Configuration;

extern int lexerr;
extern void lexfinish(void);
extern int lexincomplete(void);
extern void lexinit(void);
extern void lexstart(config_file_t cft, FILE *in, Configuration *cfg);
extern void lexiastart(FILE *in, List *l);
extern void lexistart(FILE *in, Instance *i);
extern void lexsgstart(FILE *in, GatewayConfiguration *gcf);
extern void lexswstart(FILE *in, CGIConfiguration *ccf);
extern "C" {
  int yylex(void);
}

// Each program must set progname to argv[0] (or basename(argv[0]))
// and must allocate and initialize the other objects.

extern Args *args;            // Parsed Args (i:I:m:)
extern Configuration *cf;     // Use LockManager before accessing <cf>
extern char *progname;        // argv[0] or basename(argv[0])
extern PRNG *prng;            // Init'd PRNG object

// verify_X require args in scope.  Only verify_file checks the current
// uid, since it may be called by applications that aren't running as
// INSTUSER.  verify_directory, however, should only be called by the
// scheduler, which must run as INSTUSER.

#define DIR_GRP_RD S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH
#define DIR_GRP_WR S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH
#define verify_directory(x,y,z)  ((stat(x,y)==0 || mkdir(x,z)==0) && \
				  (!args || \
				   (chown(x, (uid_t)-1, args->instgid())==0 \
				    && chmod(x,z)==0)))

#define FILE_GRP_WR S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH
#define FILE_OTH_NO S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP
#define verify_file(x,y)  (args && chmod(x, y)==0 && \
			   chown(x, (uid_t)-1, args->instgid())==0)

// Shorthand to delete stuff.  Delete an item only if it is defined.
#define xdelete(x) if(x) { delete x; x = NULL; }
// Delete an array <x>, if defined, holding items of *type* <y>.
#define xadelete(x,y) if(x) {\
  for(int i = 0;i < x->entries();i++) { \
    y *__a = x->retrieve(i); \
    if(__a) delete __a; \
  } \
  delete x; \
  x = NULL; \
}
// Delete a Hashtable <x>, if defined, holding items of *type* <y>.
#define xhdelete(x,y) if(x) {\
  HashHandle *__h = x->iterate_begin();\
  if(__h) {\
    y *__a;\
    while((__a = x->iterate_next(__h)) != NULL)\
      delete __a;\
    x->iterate_end(__h);\
  }\
  delete x;\
  x = NULL;\
}

// Shorthand for iostream messages
#define IONULL(x) (x ? x : "(null)")
#define IOTF(x) (x ? "true" : "false")

// Shorthand for setting CheckResult values
#define SET_CHECKRESULT(cr,rc,sc,cmt) if(cr) { \
  cr->set_rc(rc); cr->set_scalar(sc); cr->set_comment(cmt); \
}

#if defined(_MISSING_LOCKF_PROTOTYPE)
extern "C" {
  int lockf(int filedes, int function, off_t size);
}
#endif

//#define _WITH_MCD
#if defined(_WITH_MCD)
#include "mcd.h"
#endif
