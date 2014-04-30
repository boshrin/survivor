/*
 * cli.H
 *
 * Version: $Revision: 0.12 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2006/10/14 03:08:10 $
 *  
 * Copyright (c) 2002 - 2006
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: cli.H,v $
 * Revision 0.12  2006/10/14 03:08:10  benno
 * Add checkcf
 *
 * Revision 0.11  2006/10/10 13:17:23  benno
 * Add signal_catcher
 *
 * Revision 0.10  2005/11/24 18:25:42  benno
 * Add do_status()
 *
 * Revision 0.9  2005/09/26 13:58:04  benno
 * Add clset, report_substitution()
 *
 * Revision 0.8  2004/08/25 01:51:10  benno
 * Add opts for CLCAL, HISTORY, REPORT
 * Add sh_list_args, dispatch_list_arg
 * Add do_report
 *
 * Revision 0.7  2004/06/20 01:06:47  benno
 * Add clunsub
 *
 * Revision 0.6  2003/11/29 05:35:12  benno
 * Replace functionality_t with CommandInfo
 * Add command_args_t
 * Move SHPair to libsrv.H
 * Revisions to Functionality class
 *
 * Revision 0.5  2003/03/04 20:55:31  benno
 * Bump copyright
 *
 * Revision 0.4  2002/12/31 04:39:37  benno
 * Add fix support, username()
 *
 * Revision 0.3  2002/04/04 19:59:08  benno
 * copyright
 *
 * Revision 0.2  2002/04/02 21:46:42  benno
 * rcsify date
 *
 * Revision 0.1  2002/04/02 21:46:21  benno
 * initial revision
 *
 */

#include "survivor.H"

// Information about each command we accept

enum command_args_t {
  no_args,               // No args accepted (maxargs must be set to 0)
  opaque_args,           // Argument string is opaque, do not preprocess
  sh_explicit_args,      // Argument must be of form service@host
  sh_implied_args,       // Argument may be of form service@, @host, or s@h
  sh_list_args           // Argument may only be service or host
};

struct CommandInfo {
  char *name;             // The name of the command
  int minlen;             // The minimum prefix to uniquely match name
  int minargs;            // The minimum number of post-command arguments
  int maxargs;            // The maximum number of post-command args (or -1)
  command_args_t argtype; // What type of arguments are expected
  bool comments;          // Whether the command accepts comments
};

// The defined commands

extern CommandInfo acknowledge_cmd;
extern CommandInfo alerthistory_cmd;
extern CommandInfo archivehistory_cmd;
#define OPT_ARCHIVEHISTORY_ALL "all"
#define OPT_ARCHIVEHISTORY_DIRECTORY "directory"
#define OPT_ARCHIVEHISTORY_FILE "file"
#define OPT_ARCHIVEHISTORY_STALE "stale"
extern CommandInfo check_cmd;
extern CommandInfo checkcf_cmd;
extern CommandInfo checkhistory_cmd;
#define OPT_CLCAL_PERSON "person"
extern CommandInfo clcal_cmd;
extern CommandInfo clprune_cmd;
#define OPT_CLSET_PERSON "person"
extern CommandInfo clset_cmd;
extern CommandInfo clstat_cmd;
extern CommandInfo clsub_cmd;
extern CommandInfo clunsub_cmd;
#define OPT_CLSUB_REPLACE "replace"
#define OPT_CLSUB_WITH "with"
extern CommandInfo commandhistory_cmd;
#if defined(DEBUG)
extern CommandInfo dtest_cmd;
#define OPT_DTEST_ADDRESS "address"
#define OPT_DTEST_MODULE "module"
#define OPT_DTEST_TEST "test"
#endif
extern CommandInfo escalate_cmd;
extern CommandInfo fix_cmd;
extern CommandInfo fixhistory_cmd;
#define OPT_HISTORY_REVERSE "reverse"
extern CommandInfo inhibit_cmd;
extern CommandInfo report_cmd;
#define OPT_REPORT_MODULE "module"
#define OPT_REPORT_REVERSE "reverse"
#define OPT_REPORT_TYPE "type"
extern CommandInfo reschedule_cmd;
extern CommandInfo status_cmd;
#define OPT_STATUS_MATCH "match"
extern CommandInfo trip_cmd;
#define OPT_TRIP_RC "rc"
extern CommandInfo unacknowledge_cmd;
extern CommandInfo uninhibit_cmd;
  
// The array of command information, defined in commands.C

extern CommandInfo *commands[];
extern int commandslen;

// Prototypes from main.C

void signal_catcher(int i);

// Class definitions

class Functionality {
public:
  Functionality();
  void dispatch_all(CommandInfo *ci);
  void dispatch_arg(CommandInfo *ci, char *arg);
  void dispatch_listarg(CommandInfo *ci, int optind, int argc, char **argv);
  void dispatch_noarg(CommandInfo *ci);

  void trip(int optind, int argc, char **argv);
  ~Functionality();
  
private:
  struct passwd pw;
  struct passwd *pwr;
  char pwbuf[BUFSIZE];

  void do_acknowledge(Check *check, char *host);
  void do_alerthistory(Check *check, char *host);
  void do_archive();
  void do_archive(Check *check, char *host);
  void do_check(Check *check, char *host);
  void do_checkhistory(Check *check, char *host);
  void do_clcal(char *calllist);
  void do_clprune(char *calllist);
  void do_clset(char *calllist);
  void do_clstat(char *calllist);
  void do_clsub(char *calllist);
  void do_commandhistory(Check *check, char *host);
#if defined(DEBUG)
  void do_dtest();
#endif
  void do_escalate(Check *check, char *host);
  void do_f(CommandInfo *ci, Check *check, char *host);
  void do_fix(Check *check, char *host);
  void do_fixhistory(Check *check, char *host);
  void do_inhibit(Check *check, char *host);
  void do_report(List *checks, List *hosts);
  void do_reschedule(Check *check, char *host);
  void do_status();
  void do_status(Check *check, char *host);
  void do_trip(Check *check, char *host);
  void do_unacknowledge(Check *check, char *host);
  void do_uninhibit(Check *check, char *host);
  void report_substitution(Substitution *sub);
  char *username();
};
