/*
 * cgi.H
 *
 * Version: $Revision: 0.22 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2005/12/22 04:01:33 $
 *  
 * Copyright (c) 2002 - 2005
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: cgi.H,v $
 * Revision 0.22  2005/12/22 04:01:33  benno
 * Change Session constructor argument order for AIX
 *
 * Revision 0.21  2005/11/24 18:25:09  benno
 * Remove pf_matchstate_t, retrieve_matching_state
 *
 * Revision 0.20  2005/06/09 02:50:22  benno
 * Somewhat more generous CGIVALID
 * Add more args to process_action
 * Add process_action_dispatch
 *
 * Revision 0.19  2005/04/09 03:01:50  benno
 * Add CBAPPENDESCAPED
 *
 * Revision 0.18  2004/11/27 00:54:12  benno
 * Add cgivaluesafer
 *
 * Revision 0.17  2004/08/25 01:50:28  benno
 * Add process_report_module, process_tmpfile, mime_ext_to_type
 *
 * Revision 0.16  2004/04/26 17:27:42  benno
 * Add process_status, CGIXMLHEADER
 *
 * Revision 0.15  2004/04/24 14:49:11  benno
 * Add pf_matchstate_t
 * Add split_tag
 * Add retrieve_matching_state
 *
 * Revision 0.14  2004/04/03 19:55:35  benno
 * process_clipboard takes CGIValues
 *
 * Revision 0.13  2004/03/02 03:42:51  benno
 * Overhaul of web interface
 * Add tags, move config to libsrv
 *
 * Revision 0.12  2003/10/06 23:21:23  benno
 * Add HTMLOUT_FILENO, do_status_check
 *
 * Revision 0.11  2003/07/30 21:40:00  benno
 * Clipboard::send uses transmit module
 *
 * Revision 0.10  2003/03/04 20:55:23  benno
 * Add config_error
 *
 * Revision 0.9  2002/12/31 04:39:16  benno
 * perform_action takes comment
 *
 * Revision 0.8  2002/09/09 02:26:32  benno
 * add Auth::cookie_sid
 *
 * Revision 0.7  2002/08/21 21:33:50  benno
 * Add name to Clipboard
 *
 * Revision 0.6  2002/08/16 22:51:57  benno
 * Add CGIConfig::servicename()
 *
 * Revision 0.5  2002/06/02 23:14:32  toor
 * support for "Back to" link
 * Session expiry and init
 *
 * Revision 0.4  2002/04/12 15:03:26  benno
 * move SENDMAIL define to Makefile.inc
 *
 * Revision 0.3  2002/04/04 19:58:58  benno
 * copyright
 *
 * Revision 0.2  2002/04/02 21:45:48  benno
 * rcsify date
 *
 * Revision 0.1  2002/04/02 21:45:26  benno
 * initial revision
 *
 */

#include "cgi-parser.H"

#define HTMLOUT cout
#define HTMLOUT_FILE stdout
#define HTMLOUT_FILENO STDOUT_FILENO

#define CGIHEADER "Content-Type: text/html" << endl << \
       	"Pragma: no-cache" << endl << \
       	"Cache-Control: no-cache" << endl

#define CGIXMLHEADER "Content-Type: text/xml" << endl << endl

// For reporting errors before PageFilters are available
#define SIMPLEERR(x) HTMLOUT << CGIHEADER << endl << x << endl

// Default length of time of web session for redirect based auth (in minutes)
#define SESSIONTIME 30

// Append escaped text to CharBuffer
#define CBAPPENDESCAPED(d,t) \
  char *x = escape_html(t); \
  if(x) { d->append(x); delete(x); } else { d->append(t); }

enum authsuccess_t {
  auth_ok,              // Auth succeeded
  auth_fail,            // Permission denied
  auth_err              // Configuration (or other) error
};

// Valid characters in "restricted" values
// before checking in, make sure it's ok to put the stuff after the -
#define CGIVALID "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.+%@_-/?&= "
#define cgivaluesafer(x) (char *)((x && x->validate(CGIVALID)) ? x->value() : "")

// Information about the tags we filter

enum swtag_format_t {
  noarg_tag,         // Tag takes no arguments
  opaque_tag,        // Parse everything as one string
  token_tag,         // Tokenize on unescaped spaces
  parameter_tag      // Tokenize on unescaped spaces, formatted as ARG=VALUE
};

struct SWTagInfo {
  char *name;            // The command name (the first token in @{})
  swtag_format_t argfmt; // The argument format
  char *closetag;        // If relevant, the corresponding closing tag
  authz_level_t authz;   // The minimum required authorization
};

// The defined tags

extern SWTagInfo alertplan_tag;
extern SWTagInfo alertstatus_tag;
extern SWTagInfo authlevel_tag;
extern SWTagInfo checkschedule_tag;
extern SWTagInfo checkstatus_tag;
extern SWTagInfo clipboard_tag;
extern SWTagInfo clipemail_tag;
extern SWTagInfo clipphone_tag;
extern SWTagInfo echo_tag;
extern SWTagInfo error_tag;
extern SWTagInfo fixstatus_tag;
extern SWTagInfo flag_tag;
extern SWTagInfo foreach_tag;
extern SWTagInfo helpfile_tag;
extern SWTagInfo hostclass_tag;
extern SWTagInfo if_tag;
extern SWTagInfo if_else_tag;
extern SWTagInfo include_tag;
extern SWTagInfo instance_tag;
extern SWTagInfo pageset_tag;
extern SWTagInfo rctext_tag;
extern SWTagInfo referer_tag;
extern SWTagInfo refresh_tag;
extern SWTagInfo runningstate_tag;
extern SWTagInfo set_tag;
extern SWTagInfo split_tag;
extern SWTagInfo time_tag;
extern SWTagInfo uri_tag;
extern SWTagInfo uritags_tag;
extern SWTagInfo uritop_tag;
extern SWTagInfo username_tag;
extern SWTagInfo version_tag;

// The array of tag information, defined in tags.C

extern SWTagInfo *swtags[];
extern int swtagslen;

// Clipboards are a CGI feature, not a general package feature, and so
// are maintained here rather than in libsrv.

class Clipboard {
public:
  Clipboard(char *path, char *name);
  bool append(char *contents);
  char *read();
  bool send(char *address, char *from, char *mail, char *phone, char *via);
  bool write(char *contents);
  ~Clipboard();

private:
  char *clipname;     // Name of this clipboard
  char *clippath;     // Path to this clipboard

  bool output(char *contents, char *mode);
};

class ClipboardManager {
public:
  ClipboardManager() {}
  bool create_board(char *name);
  bool delete_board(char *name);
  List *find_all_boards();
  Clipboard *find_board(char *name);
  ~ClipboardManager() {}

private:
  char *boardpath(char *name);
};

class Variable {
public:
  Variable(char *name, char *value);
  char *name();
  bool set_value(char *value);
  char *value();
  ~Variable();

private:
  char *n;   // Name
  char *v;   // Value
};

class SWTag {
public:
  SWTag(char *rawtag, Hashtable<Variable> *vhash);
  char *arg_opaque();
  char *arg_parameter(char *name);
  char *arg_token(int i);
  int arg_tokens();
  SWTagInfo *command();
  ~SWTag();

private:
  List *ats;                   // For token_tag
  Hashtable<Variable> *aps;    // For parameter_tag
  SWTagInfo *tip;              // Pointer to SWTagInfo describing this command
  char *ao;                    // For opaque_tag
};

class Session {
public:
  Session(char *sid, int validity, char *user, List *groups, char *hostid);
  Session(char *sid, char *user, List *groups, char *hostid, time_t expiry);
  time_t expiry();
  List *groups();
  char *hostid();
  char *sid();
  char *user();
  bool valid(char *hostid);
  ~Session();

private:
  List *g;                // Groups
  char *h;                // Host ID (IP address, etc)
  char *s;                // Session ID
  char *u;                // Username for this session
  time_t exp;             // Expiry time of this session

  void init(char *sid, char *user, List *groups, char *hostid, time_t expiry);
};

class PageFilter {
public:
  PageFilter(Hashtable<Variable> *vhash, char *pageset);
  void filter(char *page, bool dotin);
  ~PageFilter();

private:
  AlertState *ascache;       // Cached for repeated access
  CheckState *cscache;       // Cached for repeated access
  FixState *fscache;         // Cached for repeated access
  SHPair *shpcache;          // What is in the cache
  List *clists;              // List of all CallLists
  List *addresses;           // List of all individuals
  Hashtable<Variable> *vh;   // Variable hash
  char *ps;                  // PageSet to use

  void do_filter(CharHandler *src, CharHandler *dest);
  void eval_tag(SWTag *tag, CharHandler *src, CharHandler *dest);
  CharBuffer *find_close_tag(SWTagInfo *tag, SWTagInfo *nested,
			     CharHandler *src);
  bool get_calllist_info();
  bool retrieve_state(char *service, char *host);
  void uncache_state();
};

class SessionState : public State {
public:
  SessionState();
  bool add(Session *s);
  Session *find(char *sid);
  bool remove(char *sid);
  ~SessionState();

private:
  char *statefile;        // Location of session state information

  bool rewrite(Session *add, char *remove);
};

// Defined in main.C
extern CGIConfiguration *cgicf;
extern CGIParser *parser;
extern Cookies *cookies;
extern Session *session;

void process_action(char *action, CGIValue *hosts, CGIValue *services,
		    CGIValue *hostclasses, CGIValue *shpairs, char *comment);
void process_action_dispatch(char *action, char *host, char *service,
			     char *comment, CharBuffer *errs);
void process_auth(bool login, SessionState *st);
void process_clipboard(char *action, char *clipboard, char *contents,
		       CGIValue *rcptlist, CGIValue *rcptaddr, char *email,
		       char *phone);
void process_page(char *pagedir, char *srcfile);
void process_report_module(char *module, CGIValue *hosts,
			   CGIValue *hostclasses, CGIValue *services,
			   int fmon, int fday, int fyear, int fhour, int fmin,
			   int umon, int uday, int uyear, int uhour, int umin,
			   CGIValue *which, char *reverse, char *style);
void process_status(char *request, char *instance);
void process_tmpfile(char *tfile);

// Defined in mimetypes.C
char *mime_ext_to_type(char *ext);
