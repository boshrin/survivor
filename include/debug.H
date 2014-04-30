/*
 * debug.H: survivor debugging definitions
 *
 * Version: $Revision: 0.11 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2005/11/14 03:57:03 $
 *
 * Copyright (c) 2002 - 2005
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: debug.H,v $
 * Revision 0.11  2005/11/14 03:57:03  benno
 * Fix DEBUG_MOST
 *
 * Revision 0.10  2005/04/09 03:01:40  benno
 * Add DEBUG_PARSER
 *
 * Revision 0.9  2003/10/06 23:21:31  benno
 * Add FileDebugger
 *
 * Revision 0.8  2003/04/09 20:05:05  benno
 * Rename debug and warn to dlog and wlog to avoid namespace collisions
 * Revisions to derived Debuggers
 *
 * Revision 0.7  2003/04/07 21:02:18  benno
 * Use object based debugging
 *
 * Revision 0.6  2003/03/31 14:53:02  benno
 * Add DEBUG_CACHES
 *
 * Revision 0.5  2003/03/04 20:55:44  benno
 * Add DEBUG_CFERRS
 *
 * Revision 0.4  2002/04/04 19:59:16  benno
 * copyright
 *
 * Revision 0.3  2002/04/02 22:05:41  benno
 * rcsify date
 *
 * Revision 0.2  2002/04/02 22:05:30  benno
 * Add DEBUG_CFLESS
 * Redefine WARN to use timestamp
 * Define debuglevel in utils.C
 *
 * Revision 0.1  2002/04/02 22:04:41  benno
 * initial revision
 *
 */

#define DEBUG_ANY     65535     // Enable all debugging
#define DEBUG_MOST    65528     // Enable most debugging
#define DEBUG_PARSER  8192      // Trace non-configuration file parsing
#define DEBUG_CACHES  4096      // Trace cache accesses
#define DEBUG_SCHEDS  2048      // Trace schedulers
#define DEBUG_FILEIO  1024      // Trace file access and I/O
#define DEBUG_ACCESS  512       // Trace access control
#define DEBUG_THREAD  256       // Trace thread creation and execution
#define DEBUG_CFERRS  128       // Only trace configuration errors
#define DEBUG_CFLESS  64        // Ignore less important warnings w/cf parse
#define DEBUG_CONFIG  32        // Trace configuration file parsing
#define DEBUG_NETCOM  16        // Trace network communications
#define DEBUG_ANYTRC  12        // Trace all method invocations
#define DEBUG_MAJTRC  8         // Trace major (infrequent) method invocations
#define DEBUG_MINTRC  4         // Trace minor (frequent) method invocations
#define DEBUG_ANYMEM  3         // Trace all memory allocation
#define DEBUG_MAJMEM  2         // Trace major (large) memory allocation
#define DEBUG_MINMEM  1         // Trace minor (small) memory allocation

// Debugging objects are always built, even without ENABLE_DEBUG, since
// they are used for run time warnings as well.

// do_log calls must be threadsafe since Debugger can't know about
// scheduler's LockManager

class Debugger {
public:
  Debugger();
  bool enable_tid();
  bool enable_timestamp();
#if defined(_BROKEN_VSNPRINTF)
  void enable_vsprintf() { use_vsp = true; };
#endif
  void log_entry(int level, char *fmt, ...);
  void log_exit(int level, char *fmt, ...);
  void log_lex(int level, bool err, char *cf, int line, char *fmt, ...);
  void log_progress(int level, char *fmt, ...);
  bool set_level(int level);
  void warn(char *fmt, ...);
  ~Debugger();
  
private:
  bool tid;          // Include thread ID in entry
  bool ts;           // Include timestamp in entry
#if defined(_BROKEN_VSNPRINTF)
  bool use_vsp;      // Use less safe vsprintf()
#endif
  int dl;            // Debug level
  
  virtual bool do_log(bool warn, char *s) {}
  int prepare_entry(char *buf, int bufsize, char *prefix);
};

class NullDebugger : public Debugger {
public:
  NullDebugger() {}
  ~NullDebugger() {}

private:
  bool do_log(bool warn, char *s) { return(true); }
};

class FileDebugger : public Debugger {
public:
  FileDebugger();
  FileDebugger(char *file);
  ~FileDebugger();

private:
  char *f;           // File to log to

  bool do_log(bool warn, char *s);
};

class StdDebugger : public Debugger {
public:
  StdDebugger() { so = false; }
  bool enable_stdout();
  ~StdDebugger() {}

private:
  bool so;           // Use stdout instead of stderr
  
  bool do_log(bool warn, char *s);
};

class SyslogDebugger : public Debugger {
public:
  SyslogDebugger();
  SyslogDebugger(int facility);
  bool set_priority(int priority);
  ~SyslogDebugger() {}

private:
  int f;             // Facility to log at
  int p;             // Priority to log at, if not default
  
  bool do_log(bool warn, char *s);
};

#if defined(ENABLE_DEBUG)
// This must be provided by the main application. (debugging logger)
extern Debugger *dlog;
// Shortcut
#define DEBUG
#endif

// This must be provided by the main application. (warning logger)
extern Debugger *wlog;
