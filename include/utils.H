/*
 * utils.H
 *
 * Version: $Revision: 0.8 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2005/12/22 04:02:39 $
 *
 * Copyright (c) 2004 - 2005
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: utils.H,v $
 * Revision 0.8  2005/12/22 04:02:39  benno
 * Add next_schedule_time
 *
 * Revision 0.7  2005/11/30 01:39:01  benno
 * Add module serialization
 *
 * Revision 0.6  2005/08/10 01:51:22  benno
 * read_xml_document takes timeout
 *
 * Revision 0.5  2005/04/27 01:59:17  benno
 * Add read_xml_document
 *
 * Revision 0.4  2005/04/09 02:43:48  benno
 * escape_html, escape_xml, xstrnchr
 *
 * Revision 0.3  2004/11/27 00:56:29  benno
 * Add get_localhost_fqdn, tokenize
 *
 * Revision 0.2  2004/08/25 01:58:05  benno
 * Add read_previous_line
 *
 * Revision 0.1  2004/06/11 21:51:47  benno
 * Initial revision
 *
 */

template <class T> class Array;
class List;
class Schedule;

// Prototypes from utils.C

struct dirent *allocate_dirent(char *path);
bool compare_regex(char *s, char *exp);
int compose_rc(int current, int additional, bool required);
time_t convtime(char *time);
char *escape_html(char *s);
char *escape_xml(char *s, int len);
bool expire_file(char *filedir, char *filename, int age);
char *generate_csid(char *stype, char *host, char *service);
int getday(char *day);
#if defined(_MISSING_REENTRANT_GETHOST)
struct hostent *gethostbyaddr_r(const char *addr, int len, int type,
				struct hostent *hostent, char *buf,
				int bufsize, int *h_errno);
struct hostent *gethostbyname_r(const char *name, struct hostent *hostent,
				char *buf, int bufsize, int *h_errno);
#endif
char *get_localhost_fqdn();
int gettime(char *time, bool hour);
void libsrvexit();
bool libsrvinit(char *argv0);
bool libsrvinit(char *argv0, Debugger *d, Debugger *w);
time_t next_schedule_time(Array<Schedule> *sched, time_t last);
#if defined(USE_PROCESS_KILL)
void process_kill(pid_t pid, int signal);
void process_kill_target(Array<PidPair> *pidarray, int target, int signal);
#endif
char *read_line(int fd);
char *read_previous_line(int fd);
char *read_xml_document(int fd, char *endtag, int timeout);
List *tokenize(char *s, const char *c);
List *tokenize(char *s, const char *c, int max);
void toss_eol(char *s);
FILE *try_fopen(const char *filename, const char *mode);
int  try_lock(char *lockfile);
int  try_open(const char *filename, int flags, mode_t mode);
char *xnumdup(int i);
char *xstrcat(char *s1, char *s2);
char *xstrncat(char *s1, char *s2, int len);
char *xstrnchr(char *s, char c, int len);
void xstrchop(char *s);
char *xstrdup(char *s);
char *xstrreplace(char *s, char *sfind, char *sreplace);

#if defined(ENABLE_MODULE_SERIALIZATION)
extern pthread_mutex_t msmutex;
#endif
