/*
 * libsrv.H
 *
 * Version: $Revision: 0.46 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2007/03/29 12:16:04 $
 *
 * Copyright (c) 2002 - 2007
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: libsrv.H,v $
 * Revision 0.46  2007/03/29 12:16:04  benno
 * Changes in support of bug 370 (queue timeouts)
 *
 * Revision 0.45  2006/10/17 14:02:03  benno
 * Add since and duration to alert,check,fix state
 *
 * Revision 0.44  2006/01/23 00:45:45  benno
 * Add support for duration measurement in CheckResult, ComponentCheckResult,
 * and CheckHistoryRecord
 *
 * Revision 0.43  2005/12/22 04:02:12  benno
 * Changes for detection of stalled checks
 *
 * Revision 0.42  2005/11/14 03:55:03  benno
 * Add CharHandler::append
 * Add ReportFormatting(module,style)
 * Fix typo
 *
 * Revision 0.41  2005/10/20 02:11:31  benno
 * Changes for clear state customization
 *
 * Revision 0.40  2005/10/05 01:45:55  benno
 * Add sortable
 * Changes to fix substitution non-termination
 *
 * Revision 0.39  2005/08/10 01:49:42  benno
 * FileHandler adds read timeout
 *
 * Revision 0.38  2005/06/09 02:26:15  benno
 * exec_pipe returns int *
 *
 * Revision 0.37  2005/04/27 01:59:26  benno
 * CheckResult->read becomes parse
 *
 * Revision 0.36  2005/04/09 02:45:07  benno
 * Changes for XML based module arguments
 *
 * Revision 0.35  2004/11/27 00:55:24  benno
 * Add seek to FileHandler
 * Add gateway-common.H
 *
 * Revision 0.34  2004/09/13 14:23:10  benno
 * Add cf->runs_on_host
 *
 * Revision 0.33  2004/08/25 01:59:00  benno
 * Add sort to List
 * Add iterate_backwards/previous to HistoryRecords
 * Add HistorySet, ReportFormatting
 * Add report module exec to Executor
 *
 * Revision 0.32  2004/06/20 01:08:10  benno
 * Changes to CallListState for clunsub
 *
 * Revision 0.31  2004/06/11 22:24:30  benno
 * Add support for "result text significant"
 * StateData and other objects for XMLification of state files
 * Move utils to utils.H
 *
 * Revision 0.30  2004/03/02 03:51:03  benno
 * CharBuffer, CharHandler, FileHandler changes
 * Add Counter and StatCounter
 * Changes to support webauth modules
 * Changes for smart scheduling
 * Add tokenize and compose_rc
 *
 * Revision 0.29  2003/11/29 05:34:11  benno
 * New CharBuffer, History routines, including prune
 * Move SHPair here from cli.H
 * Changes to Instance and Args for cli_allow/require
 * Changes to Args for fromtime, untiltime, arg_o
 * Changes to Configuration for history_consistency, remove_history
 *
 * Revision 0.28  2003/11/08 15:37:21  benno
 * Add CharBuffer
 * Add History and HistoryRecord objects
 * Move try_lock to utils
 *
 * Revision 0.27  2003/10/20 00:04:29  benno
 * Change exec_format_transmit prototype
 *
 * Revision 0.26  2003/10/06 23:23:07  benno
 * Changes for fix if defined
 * Changes for replyok in FormattedAlert
 * Changes for dependency overhaul
 * Add ModuleConfigError, SurvivorStatus
 * Add read_line
 *
 * Revision 0.25  2003/07/30 21:40:45  benno
 * Add FormattedAlert, Executor::exec_formatted_alert
 *
 * Revision 0.24  2003/06/25 21:30:40  benno
 * Add ComponentCheckResult object
 *
 * Revision 0.23  2003/06/17 15:10:45  benno
 * Changes for Format/Transmit modules
 * Changes to Executor to incrementally report results
 *
 * Revision 0.22  2003/05/04 21:38:13  benno
 * Don't use string type
 * Add xstrcat, xstrchop, xstrreplace
 *
 * Revision 0.21  2003/04/13 20:03:47  benno
 * Changes for state caching
 *
 * Revision 0.20  2003/04/07 21:02:45  benno
 * New libsrvinit
 *
 * Revision 0.19  2003/03/31 14:52:45  benno
 * Changes to support StateCache
 *
 * Revision 0.18  2003/03/04 20:56:45  benno
 * Change AlertState::acknowledge, add dump_misconfig
 *
 * Revision 0.17  2003/02/20 02:52:31  benno
 * gethostbyX_r
 * Add support for composite checks
 *
 * Revision 0.16  2003/01/01 02:06:07  benno
 * read_status in check and alert state
 *
 * Revision 0.15  2002/12/31 04:40:12  benno
 * Add fix, transport support
 *
 * Revision 0.14  2002/12/16 01:03:19  benno
 * Try based AlertPlans
 * RecipientSets
 *
 * Revision 0.13  2002/11/19 17:53:47  benno
 * add exec_pipe and pkgdir for tunnel module
 *
 * Revision 0.12  2002/10/25 23:00:22  benno
 * add Person and Contact
 *
 * Revision 0.11  2002/10/21 20:43:11  benno
 * calllist rotates using a schedule
 * add support for named args
 *
 * Revision 0.10  2002/09/05 22:00:53  benno
 * add try_fopen
 *
 * Revision 0.9  2002/05/24 20:56:15  benno
 * add versions to Args
 *
 * Revision 0.8  2002/05/13 18:06:58  benno
 * add Args::check_instance
 *
 * Revision 0.7  2002/05/10 20:07:32  benno
 * add runningasinst
 *
 * Revision 0.6  2002/05/02 22:58:27  toor
 * PidPair and process_kill prototypes
 *
 * Revision 0.5  2002/04/26 20:12:00  toor
 * make parse_instcf visible
 *
 * Revision 0.4  2002/04/19 20:09:56  benno
 * add compare_regex
 *
 * Revision 0.3  2002/04/04 19:59:49  benno
 * copyright
 *
 * Revision 0.2  2002/04/02 22:08:31  benno
 * rcsify date
 *
 * Revision 0.1  2002/04/02 22:08:04  benno
 * initial revision
 *
 */

class Sortable {
public:
  virtual int sort_key() {}
};

class iArray {
public:
  iArray();
  iArray(char *name);
  iArray(iArray *array);
  bool iadd(void *data);
  int entries();
  int ifind(void *data);
  char *name();
  bool remove(int index);
  void *iretrieve(int index);
  iArray *isort(sort_t order);
 ~iArray();

private:
  void **items;    // Array of items in the array
  char *arrayname; // The name of this array
  int size;        // Number of items stored in the array
  iArray *sorted;  // Sorted array
  void **sitems;   // Sorted items so as to leave <items> unaffected

  void init(char *name);
  void qs_sort(int p, int r, sort_t order);
  int qs_partition(int p, int r, sort_t order);
};

template <class T>
class Array : public iArray {
public:
  Array() : iArray() {}
  Array(char *name) : iArray(name) {}
  Array(Array<T> *array) : iArray((void *)array) {}
  bool add(T *data) { return(iadd((void *)data)); }
  int find(T *data) { return(ifind((void *)data)); }
  T *retrieve(int index) { return((T *)iretrieve(index)); }
  Array<T> *sort(sort_t order) { return((Array<T> *)isort(order)); }
  ~Array() {}

private:
};

class CharHandler {
public:
  CharHandler();
  bool append(char c);
  bool append(char *s);
  bool append(double d);
  bool append(int i);
  bool append(long l);
  char read_char();
  char *read_line();
  char *read_until(char c);
  char *read_until(char *s, bool casesen);
  char *read_until_eof();
  ~CharHandler();

protected:
  bool increase_bsize(char **buf, int *curlen, int addlen);
  
private:
  char *rbuf;
  
  virtual bool do_append(char *s) {}
  virtual char do_getchar() {}
};

class CharBuffer : public CharHandler {
public:
  CharBuffer();
  CharBuffer(char *s);
  CharBuffer(CharBuffer *cb);
  bool append_fixed_buffer();
  bool clear();
  char *create_fixed_buffer(int bsize);
  int replace(char o, char n);
  bool seek(int pos);
  char *str();
  ~CharBuffer();

private:
  char *b;      // The internal buffer
  char *tbuf;   // Temporary fixed buffer for external reads
  int gindex;   // Index for getchar()
  int blen;     // The length of the contents of b
  int bsize;    // The size of b

  bool do_append(char *s);
  char do_getchar();
  void init(char *s, bool reset);
};

class FileHandler : public CharHandler {
public:
  FileHandler();
  FileHandler(int fd);
  bool open_read(char *path);
  bool open_write(char *path, mode_t mode);
  bool set_read_timeout(int t);
  ~FileHandler();

private:
  bool do_close;
  int f;
  int to;

  bool do_append(char *s);
  char do_getchar();
};

class iWarehouse {
public:
  iWarehouse(size_t initial, size_t incremental, size_t decremental,
	     size_t size);
  void *iallocate();
  bool ideallocate(void *x);
  bool valid();
  ~iWarehouse();
  
private:
  void **block;      // Array of blocks of memory
  size_t blocks;     // Number of blocks in <block>
  size_t itemsize;   // Size of one object
  size_t init;       // Number of objects in first block
  size_t incr;       // Number of objects in additional blocks
  size_t nextfree;   // Index of next never-allocated object in last block

  void ***freelist;  // Array of arrays of deallocated objects
  size_t freelists;  // Number of arrays in <freelist>
  size_t decr;       // Number of objects stored in a freelist array
  size_t lastfree;   // Index of most recently deallocated object in last array
};

template <class T>
class Warehouse : public iWarehouse {
public:
  Warehouse(size_t initial, size_t incremental, size_t decremental) :
    iWarehouse(initial, incremental, decremental, sizeof(T)) {}
  T *allocate() { return((T *)iallocate()); }
  bool deallocate(T *x) { return(ideallocate((void *)x)); }
  ~Warehouse() {}

private:
};

class HashUnit {
public:
  HashUnit();
  bool clear();
  void *get_data();
  char *get_key();
  HashUnit *get_next();
  HashUnit *get_prev();
  bool initialize(char *key, void *data);
  bool set_data(void *data);
  bool set_next(HashUnit *h);
  bool set_prev(HashUnit *h);
  ~HashUnit();

private:
  char *k;         // The key for this HashUnit
  void *d;         // The data associated with key
  HashUnit *prev;  // A pointer to the previous HashUnit in this chain
  HashUnit *next;  // A pointer to the next HashUnit in this chain
};

class HashHandle {
public:
  int bucket;      // The current bucket being examined
  HashUnit *unit;  // The HashUnit within the bucket being examined.
  void *x;         // Data space for invoking method -- ignored by HashHandle

  HashHandle() { bucket = -1; unit = NULL; }
  ~HashHandle() { bucket = -1; unit = NULL; }

private:
};

typedef HashHandle ConfigHandle;

class iHashtable {
public:
  iHashtable();
  iHashtable(size_t buckets);
  bool iinsert(char *key, void *data);
  HashHandle *iterate_begin();
  void *iiterate_next(HashHandle *h);
  void iterate_end(HashHandle *h);
  bool remove(char *key);
  void *iretrieve(char *key);
  ~iHashtable();

private:
  Warehouse<HashUnit> *unithouse;  // Warehouse of HashUnit objects
  HashUnit **hashbuckets;          // Hashed array of HashUnits
  size_t totalbuckets;             // Number of buckets (size of hashbuckets)
  size_t entries;                  // Number of items stored in the Hashtable

  size_t compute_hash(char *key, size_t buckets);
  bool initialize_hash(size_t buckets);
  bool resize_hash();
};

template <class T>
class Hashtable : public iHashtable {
public:
  Hashtable() : iHashtable() {}
  Hashtable(int buckets) : iHashtable(buckets) {}
  bool insert(char *key, T *data) { return(iinsert(key, (void *)data)); }
  T *iterate_next(HashHandle *h) { return((T *)iiterate_next(h)); }
  T *retrieve(char *key) { return((T *)iretrieve(key)); }
  ~Hashtable() {}

private:
};

class List {
public:
  List();
  List(char *name);
  List(List *l);
  List(List *l, char *name);
  bool add(char *s);
  char *comma_list();
  int entries();
  int find(char *s);
  char *name();
  char *retrieve(int index);
  bool sort(sort_t order);
  ~List();

private:
  char **items;    // Array of items in this list
  char *commaline; // A one line, comma separated list of the items
  char *listname;  // The name of this list 
  int size;        // Number of items stored in the array

  void init(List *l, char *name);
  void qs_sort(int p, int r, sort_t order);
  int qs_partition(int p, int r, sort_t order);
};

class Counter {
public:
  Counter();
  Counter(char *name, int start);
  Counter(char *name, int start, int min, int max);
  bool decrement();
  bool increment();
  int maximum();
  int minimum();
  char *name();
  int value();
  ~Counter();

private:
  char *n;    // Counter name
  int mn;     // The minimum value
  int mx;     // The maximum value
  int v;      // The current value
  
  void init(char *name, int start, int min, int max);
};

class StatCounter {
public:
  StatCounter();
  bool analyze_configuration();
  int cf_hosts_checked_on_schedule(char *schedule, char *service);
  ~StatCounter();

private:
  Hashtable<Counter> *ctrs;

  bool increment_counter(char *name);
};

class Schedule : public Sortable {
public:
  Schedule(char *attime, char *atday);
  Schedule(int freq, char *frequnit);
  Schedule(char *fromtime, char *fromday, char *untiltime, char *untilday,
	   int freq, char *frequnit);
  bool at();
  int frequency();
  int intervals(time_t start, time_t finish);
  int next(time_t asof);
  int next_end(time_t asof);
  bool now(int offset);
  bool now(time_t last, int offset);
  int sort_key();
  ~Schedule();

private:
  int fday;           // 0-6, Sunday-Saturday, beginning period
  int fhour;          // Hour beginning period
  int fmin;           // Minute beginning period
  int sfreq;          // Frequency associated with this schedule
  int uday;           // Day ending period (may be less than fromday)
  int uhour;          // Hour ending period
  int umin;           // Minute ending period

  int computefreq(int freq, char *frequnit);
  void init(char *attime, char *atday, char *fromtime, char *fromday,
	    char *untiltime, char *untilday, int freq, char *frequnit);
};

class ScheduleAsOf : public Sortable {
public:
  ScheduleAsOf(Schedule *sched, time_t asof);
  Schedule *schedule();
  int sort_key();
  ~ScheduleAsOf();

private:
  Schedule *s;        // Pointer to Schedule
  int sk;             // Sort key, calculated from asof
};

class AlertModule {
public:
  AlertModule(char *name, char *format, char *transmit);
  char *format();
  char *name();
  char *transmit();
  ~AlertModule();

private:
  char *f;      // Format module to use
  char *n;      // Name of this module (stanza)
  char *t;      // Transmit module to use
};

class Contact {
public:
  Contact(char *address);
  Contact(char *address, AlertModule *module);
  char *address();
  AlertModule *module();
  ~Contact();

private:
  char *a;            // Address to send to
  AlertModule *m;     // Module to use
};

class Person {
public:
  Person(char *name);
  bool add_contact(Contact *c);
  char *find_address(char *module);
  char *name();
  ~Person();

private:
  char *n;            // Name of this person
  Array<Contact> *cs; // Contacts for this person
};

class Substitution : public Sortable {
public:
  Substitution(time_t begins, time_t ends, char *newname, char *oldname);
  time_t begins();
  time_t ends();
  char *newname();
  char *oldname();
  int sort_key();
  ~Substitution();

private:
  time_t b;         // Begin time
  time_t e;         // End time (effective through this time)
  char *n;          // New name (replacer)
  char *o;          // Old name (replacee)
};

class CallList {
public:
  CallList(char *name, Array<Person> *members, bool broadcast,
	   AlertModule *via);
  CallList(char *name, Array<Person> *members, char *rotates,
	   AlertModule *via);
  CallList(char *name, CallList *cl, AlertModule *via);
  calllist_t listtype();
  bool member(char *name);
  Array<Person> *members();
  AlertModule *module();
  char *name();
  char *notify();
  Person *notify(char *lastnotify);
  Person *notify(char *lastoncall, time_t lastrotate, bool dosubs);
  Person *notify(char *lastoncall, time_t lastrotate, time_t asof);
  Person *notify(char *lastoncall, time_t lastrotate, time_t asof,
                 bool dosubs);
  char *rotatename();
  bool set_rotatesched(Array<Schedule> *sched);
  ~CallList();

private:
  CallList *alias;            // If set, we are aliasing this CallList
  Array<Person> *memberlist;  // Array of members to notify
  calllist_t ltype;           // What type of CallList we are
  AlertModule *alertvia;      // Alert module to use
  char *listname;             // The name of this CallList
  char *rotname;              // The name of the Schedule to rotate on
  Array<Schedule> *rotsched;  // The Schedule, once looked up
  string bcache;              // Cache of broadcast addresses

  void init(char *name, CallList *cl, Array<Person> *members, bool broadcast,
	    char *rotates, AlertModule *via);
  Person *substitute(Person *p, time_t asof);
};

class AlertTry {
public:
  AlertTry(int attempts, fix_attempt_t fix, Array<CallList> *notify,
	   int degrade, Array<Schedule> *degrade_schedule, bool escalated);
  AlertTry(AlertTry *at, CallList *target, CallList *replacement);
  int attempts();
  int degrade();
  Array<Schedule> *degrade_schedule();
  bool escalated();
  fix_attempt_t fix();
  Array<CallList> *notify();
  ~AlertTry();

private:
  Array<CallList> *cl;     // CallLists to notify
  Array<Schedule> *ds;     // Schedule that degraded applies during
  bool esc;                // This stanza marks escalated
  fix_attempt_t fx;        // Try fix
  int atmp;                // Number of attempts for this try, -1 for infinite
  int deg;                 // Number of permitted failed hosts
};

class AlertSchedule {
public:
  AlertSchedule(Array<Schedule> *schedule, Array<AlertTry> *tries);
  AlertSchedule(AlertSchedule *as, CallList *target, CallList *replacement);
  Array<AlertTry> *_alltrys();
  AlertTry *alerttry(int attempt, bool fixdefined);
  int escalate_next(int from);
  bool escalated(AlertTry *t);
  bool now();
  bool now(time_t last);
  Array<Schedule> *schedule();
  ~AlertSchedule();

private:
  Array<AlertTry> *ats;    // AlertTrys
  Array<Schedule> *sched;  // Schedule this set of AlertTrys applies during
};

class AlertReturnGroup {
public:
  AlertReturnGroup(Array<int> *retvals, int failures,
		   Array<AlertSchedule> *schedules);
  AlertReturnGroup(AlertReturnGroup *arg, CallList *target,
		   CallList *replacement);
  Array<int> *_allrvs();
  int failures();
  bool match(int matchval, bool matchdef);
  Array<AlertSchedule> *schedules();
  ~AlertReturnGroup();

private:
  Array<AlertSchedule> *as;  // AlertSchedules for this ReturnGroup
  Array<int> *rvs;           // Return values for this ReturnGroup
  int fails;                 // Number of consecutive check failures required

  bool find_rv(int rv);
};

class AlertPlan {
public:
  AlertPlan(char *name, Array<AlertReturnGroup> *retgroups, bool noteclear,
	    Array<Schedule> *notesched, Array<int> *noclearon,
	    List *noclearrg);
  AlertPlan(char *name, AlertPlan *ap, CallList *target,
	    CallList *replacement);
  Array<AlertReturnGroup> *_allrgs();
  Array<int> *_noclr();
  List *_noclrrg();
  Array<Schedule> *_nocscd();
  bool _nocset();
  bool clear_state_on(int fromrc, int torc);
  AlertReturnGroup *match(int retval, bool matchdef, int failures);
  char *name();
  bool notify_on_clear();
  ~AlertPlan();

private:
  Array<AlertReturnGroup> *rgs;  // ReturnGroups for this AlertPlan
  Array<int> *noclear;           // Don't clear state on these return codes
  Array<Schedule> *nocsched;     // Notify on clear schedule
  List *noclearr;                // Don't clear state is both rcs in these rgs
  bool noc;                      // Notify on clear
  char *n;                       // The name of this AlertPlan
};

class HostClass {
public:
  HostClass(char *name, List *hosts, AlertPlan *alert, Array<Schedule> *sched);
  AlertPlan *alert_plan();
  Array<Schedule> *check_schedule();
  List *hosts();
  char *name();
  ~HostClass();
  
private:
  AlertPlan *alertplan;          // Default alert plan for this HostClass
  Array<Schedule> *checksched;   // Default check schedule for this HostClass
  char *classname;               // The name of this HostClass
  List *hostlist;                // The list of hosts in this HostClass
};

// A RecipientMethod holds addresses for notification via a specific
// module (as of v0.9.2, module refers to the alert stanza
// definition), and the calllist names from which those addresses were
// generated.

class RecipientMethod {
public:
  RecipientMethod(char *module);
  RecipientMethod(RecipientMethod *r);
  bool add(char *address, char *calllist);
  bool add(char *address, int rc);
  bool add_address(char *address);
  bool add_calllist(char *calllist);
  char *addresses();
  List *address_list();
  char *calllists();
  List *calllist_list();
  char *module();
  int rc();
  ~RecipientMethod();

protected:
  char *mod;
  List *addrs;
  List *cls;
  int maxrc;
};

// A RecipientSet object holds addresses and methods for reaching those
// addresses.

class RecipientSet {
public:
  RecipientSet();
  RecipientSet(RecipientSet *rset);
  bool add(char *address, char *calllist, char *module);
  bool add(char *address, char *module, int rc);
  bool add_address(char *address, char *module);
  bool add_calllist(char *calllist, char *module);
  char *addresses(int module);
  List *address_list(int module);
  char *calllists(int module);
  List *calllist_list(int module);
  char *module(int module);
  int rc(int module);
  int modules();
  ~RecipientSet();

protected:
  Array<RecipientMethod> *rcps;  // Where the addresses are stored

private:
  RecipientMethod *find_rm(char *module);
};

// An Alert object is used to store the actual information in an Alert to
// be sent out.  A Check object is used to store information describing a
// Check obtained from the configuration files.

class Alert {
public:
  Alert(RecipientSet *rset, time_t checktime, char *host, char *helpfile,
	char *instance, int instances, int retval, char *service,
	char *summary, char *token, bool fix);
  time_t checktime();
  bool fix();
  int fix_retval();
  char *fix_summary();
  char *helpfile();
  char *host();
  char *instance();
  int instances();
  RecipientSet *recipients();
  int retval();
  char *service();
  bool set_fix_status(int fixrv, char *fixsummary);
  char *summary();
  char *token();
  ~Alert();

private:
  RecipientSet *rs;              // Set of recipients to notify
  int frv;                       // Return value from fix
  int inst;                      // Number of consecutive problem instances
  int rv;                        // Return value (0 for clear notification)
  bool f;                        // Try fix
  char *fs;                      // Summary of fix
  char *h;                       // Host name
  char *hf;                      // Helpfile
  char *in;                      // Instance
  char *s;                       // Summary of problem
  char *svc;                     // Service (check module)
  char *tok;                     // Token
  time_t ct;                     // Time problem was noticed
};

class FormattedAlert {
public:
  FormattedAlert(List *addresses, char *subject, char *message, bool replyok);
  FormattedAlert(List *addresses, char *replyto, char *subject,
		 char *message);
  List *addresses();
  char *message();
  bool replyok();
  char *replyto();
  char *subject();
  ~FormattedAlert();

private:
  List *as;                      // List of addresses to notify
  bool rok;                      // Two-way reply OK?
  char *m;                       // Message body
  char *r;                       // Reply-to
  char *s;                       // Subject

  void init(List *addresses, char *replyto, char *subject, char *message,
	    bool replyok);
};

// Arguments are sometimes referred to as "Options" in DTDs.  Oops.

class Argument {
public:
  Argument(char *name, char *value, bool trailingspaces);
  char *name();
  char *value();
  ~Argument();

private:
  char *n;
  char *v;
};

class Transport {
public:
  Transport(char *name, char *module, Array<Argument> *modargs);
  Array<Argument> *modargs();
  char *module();
  char *name();
  ~Transport();

private:
  Array<Argument> *modarg;       // The arguments it takes
  char *modname;                 // The name of the transport module
  char *tname;                   // The name of this transport stanza
};

class TransportRequest {
public:
  TransportRequest(List *hosts, int timeout, Array<Argument> *modargs,
		   char *module, char *modtype, Array<Argument> *rmodargs,
		   bool manage);
  List *hosts();
  Array<Argument> *modargs();
  char *modtype();
  char *module();
  Array<Argument> *rmodargs();
  int timeout();
  ~TransportRequest();

private:
  Array<Argument> *modarg;    // Transport module arguments
  Array<Argument> *rmodarg;   // Remote module arguments
  List *h;                    // List of hosts
  bool m;                     // Whether we manage the complex objects
  char *mod;                  // Remote module
  char *modt;                 // Remote module type
  int t;                      // Timeout
};

// ooo... mutually dependent definitions...

class Dependency;

class Fix {
public:
  Fix(char *name, char *module, Array<Argument> *modargs, Transport *transport,
      int timeout, int lockexpiry, fix_lock_t locktype);
  bool add_dependency(Dependency *d);
  Array<Dependency> *dependencies();
  int fix_timeout();
  int lock_timeout();
  fix_lock_t lock_type();
  Array<Argument> *modargs();
  char *module();
  char *name();
  Transport *transport();
  ~Fix();
  
private:
  Array<Dependency> *depends;   // Array of Checks this Fix depends on
  Array<Argument> *modarg;      // An argument set to pass it
  Transport *tr;                // Transport module to communicate via
  fix_lock_t ltype;             // The type of lock to use
  char *fixname;                // The name of this fix stanza
  char *modname;                // The name of the module to run
  int ft;                       // The timeout for the fix module
  int lt;                       // The timeout to remove lock files
};

class Check {
public:
  Check(char *name, char *module, Array<Argument> *modargs, int timeout,
	bool all, Array<Schedule> *sched, AlertPlan *alert, char *helpfile,
	Transport *transport, Fix *fix, bool restextsig);
  Check(char *name, Array<Check> *required, Array<Check> *optional,
	int timeout, bool all, Array<Schedule> *sched, AlertPlan *alert,
	char *helpfile, Fix *fix, bool restextsig);
  bool add_dependency(Dependency *d);
  AlertPlan *alert_plan();
  bool all_hosts();
  Array<Schedule> *check_schedule();
  Array<Check> *composite_optional();
  Array<Check> *composite_required();
  Array<Dependency> *dependencies();
  Fix *fix();
  char *helpfile();
  Array<Argument> *modargs();
  char *module();
  char *name();
  bool result_text_significant();
  int timeout();
  Transport *transport();
  ~Check();
  
private:
  AlertPlan *alertplan;          // An alert plan to override the default
  Array<Argument> *modarg;       // An argument set to pass to it
  Array<Check> *compopt;         // Optional Checks for Composite
  Array<Check> *compreq;         // Required Checks for Composite
  Array<Dependency> *depends;    // Array of Checks this Check depends on
  Array<Schedule> *checksched;   // A check schedule to override the default
  Fix *fx;                       // Fix to use for this Check
  bool checkall;                 // Check all hosts, not a corresponding group
  bool rtsig;                    // If result text changes are significant
  char *checkname;               // The name of this check stanza
  char *help;                    // A helpfile to provide with an alert
  char *modname;                 // The name of the module to run
  int t;                         // A timeout to override the global timeout
  Transport *tr;                 // Transport module to communicate via

  void init(char *name, char *module, Array<Argument> *modargs,
	    Array<Check> *required, Array<Check> *optional, int timeout,
	    bool all, Array<Schedule> *sched, AlertPlan *alert, char *helpfile,
	    Transport *transport, Fix *fix, bool restextsig);
};

class Dependency {
public:
  Dependency(Check *dependency, int deptype, char *targethost,
	     List *sourcehosts, List *srcexceptions);
  int dep_type();
  Check *dependency();
  bool dependent(char *host);
  char *target();
  ~Dependency();

private:
  Check *d;    // The Dependency Check (ie: what gets examined or run)
  int dt;      // The type of this Dependency (1=examine, 2=run)
  char *th;    // Target host
  List *shs;   // Source hosts (NULL = all)
  List *sxhs;  // Exceptions to the source host list
};

class CheckRequest {
public:
  CheckRequest(char *host, int timeout, Array<Argument> *modargs, bool manage);
  CheckRequest(List *hosts, int timeout, Array<Argument> *modargs,
	       bool manage);
  List *hosts();
  Array<Argument> *modargs();
  int timeout();
  ~CheckRequest();

private:
  Array<Argument> *modarg;  // Arguments to pass to the check
  List *h;                  // List of hosts
  List *xh;                 // For first initializer
  bool m;                   // Whether we manage complex objects
  int t;                    // Timeout
};

class CheckResult {
public:
  CheckResult();
  CheckResult(int rc, int scalar, char *comment);
  CheckResult(char *hostname, int rc, int scalar, char *comment);
  CheckResult(char *hostname, int rc, int scalar, char *comment, int duration);
  char *comment();
  int duration();
  char *hostname();
  bool parse(char *xmldoc);
  int rc();
  void reset();
  int scalar();
  bool set_comment(char *comment);
  bool set_duration(int duration);
  bool set_rc(int rc);
  bool set_scalar(int scalar);
  ~CheckResult();

private:
  char *c;   // Comment, dup'd
  char *h;   // Hostname, only provided via read()
  int d;     // Duration
  int r;     // Return code
  int s;     // Scalar value

  void init(char *hostname, int rc, int scalar, char *comment, int duration);
  bool set_hostname(char *host);
};

class ComponentCheckResult : public CheckResult {
public:
  ComponentCheckResult() : CheckResult() { inv = false; }
  ComponentCheckResult(int rc, int scalar, char *comment) :
    CheckResult(rc, scalar, comment) { inv = false; }
  ComponentCheckResult(char *hostname, int rc, int scalar, char *comment) :
    CheckResult(hostname, rc, scalar, comment) { inv = false; }
  ComponentCheckResult(char *hostname, int rc, int scalar, char *comment,
		       int duration) :
    CheckResult(hostname, rc, scalar, comment, duration) { inv = false; }
  bool invalid();
  void invalidate();
  ~ComponentCheckResult();

private:
  bool inv;  // Whether or not this result has been invalidated
};

class FixRequest {
public:
  FixRequest(Array<Argument> *modargs, char *host, int timeout);
  char *host();
  Array<Argument> *modargs();
  int timeout();
  ~FixRequest();

private:
  Array<Argument> *modarg;  // Arguments to pass to the fix
  char *h;                  // The host to run the fix on
  int t;                    // Request timeout
};

// We can't simply typedef because then we can't tell the difference
// (The compiler treats them the same)

class FixResult : public CheckResult {
public:
  FixResult(int rc, char *comment) : CheckResult(rc, 0, comment) {}
  ~FixResult() {}

private:
};

class ComponentResults {
public:
  ComponentResults(Array<Check> *required, Array<Check> *optional, int hosts);
  bool add(ComponentCheckResult *cr, char *service);
  bool finished(char *service);
  bool ready(char *hostname, bool invalidate);
  CheckResult *result(char *hostname);
  ~ComponentResults();

private:
  // Set of optional CheckResults, one per host
  Hashtable<ComponentCheckResult> *ocrs;
  // Set of required CheckResults, one per host
  Hashtable<ComponentCheckResult> *rcrs;
  // Set of CheckResults, by service
  Hashtable< Array<ComponentCheckResult> > *crbysvc;
  List *onames;                   // List of names of optional checks
  List *rnames;                   // List of names of required checks
  int h;                          // Number of expected hosts
  int opt;                        // Number of expected optional checks
  int req;                        // Number of expected required checks
};

class HistoryRecord {
public:
  HistoryRecord();
  int duration();
  char *generate_entry();
  bool parse_entry(char *s);
  bool parse_entry(char *s, bool splitonly);
  int timestamp();
  char *timestamp_local();
  ~HistoryRecord();

protected:
  int dur;                // Execution duration (response time)
  
private:
  struct timeval tp;      // The time the record was created
  char *e;                // The text of the entry
  char *tl;               // Local, human readable version of tp
  
  virtual char *make_string() {}
  virtual bool parse_string(char *s) {}
};

class AlertHistoryRecord : public HistoryRecord {
public:
  AlertHistoryRecord();
  AlertHistoryRecord(int alertrv, int checkrv, char *who, char *via);
  int alertrv();
  int checkrv();
  char *who();
  char *via();
  ~AlertHistoryRecord();
  
private:
  int arv;         // The alert return value
  int crv;         // The check return value for the alert
  char *m;         // String for make_string
  char *w;         // Who was notifyed
  char *v;         // How they were notified
  
  char *make_string();
  bool parse_string(char *s);

  void init(int alertrv, int checkrv, char *who, char *via);
};

class CheckHistoryRecord : public HistoryRecord {
public:
  CheckHistoryRecord();
  CheckHistoryRecord(int checkrv, int scalar, char *comment);
  CheckHistoryRecord(int checkrv, int scalar, char *comment, int duration);
  int checkrv();
  int scalar();
  char *comment();
  ~CheckHistoryRecord();
  
private:
  int crv;         // The check return value
  int sc;          // The check scalar value
  char *c;         // The check comment
  char *m;         // String for make_string
  
  char *make_string();
  bool parse_string(char *s);

  void init(int checkrv, int scalar, char *comment);
};

class CommandHistoryRecord : public HistoryRecord {
public:
  CommandHistoryRecord();
  CommandHistoryRecord(char *command, char *who, char *comment);
  char *command();
  char *comment();
  char *who();
  ~CommandHistoryRecord();
  
private:
  char *cmd;       // The command
  char *cmt;       // The command comment
  char *m;         // String for make_string
  char *w;         // Who executed the command
  
  char *make_string();
  bool parse_string(char *s);

  void init(char *command, char *who, char *comment);
};

class FixHistoryRecord : public HistoryRecord {
public:
  FixHistoryRecord();
  FixHistoryRecord(int fixrv, char *who, char *comment);
  char *comment();
  int fixrv();
  char *who();
  ~FixHistoryRecord();
  
private:
  int frv;         // The fix return value
  char *cmt;       // The fix comment
  char *m;         // String for make_string
  char *w;         // Who executed the fix
  
  char *make_string();
  bool parse_string(char *s);

  void init(int fixrv, char *who, char *comment);
};

class History {
public:
  History(Check *check, char *host);
  History(char *check, char *host);
  bool iterate_begin();
  bool iterate_begin(time_t from, time_t until);
  bool iterate_begin_backwards();
  bool iterate_begin_backwards(time_t from, time_t until);
  void iterate_end();
  bool nolock();
  bool prune(char *destination, bool desttree);
  bool prune(time_t from, time_t until, char *destination, bool desttree);
  ~History();

protected:
  bool iterate_next_entry(HistoryRecord *hr);
  bool iterate_previous_entry(HistoryRecord *hr);
  bool do_record(HistoryRecord *hr);

private:
  char *c;      // The Check (which may or may not still be defined)
  char *h;      // The host (which may or may not still be defined)
  char *hf;     // History file path (cached)
  char *lf;     // Lock file path (cached)
  bool lock;    // Whether or not to require locking when reading records
  int fd;       // File descriptor for iteration
  int lockfd;   // File descriptor for lockfile
  time_t f;     // "From" time for iteration, 0 means from the beginning
  time_t u;     // "Until" time for iteration, 0 means until the end

  char *historyfile();
  virtual char *historyfilename() {}
  void init(char *check, char *host);
  bool iterate_begin(time_t from, time_t until, bool backwards);
  char *lockfile();
  bool lock_history();
  void unlock_history();
};

class AlertHistory : public History {
public:
  AlertHistory(Check *check, char *host) : History(check, host) {}
  AlertHistory(char *check, char *host) : History(check, host) {}
  AlertHistoryRecord *iterate_next();
  AlertHistoryRecord *iterate_previous();
  bool record(AlertHistoryRecord *ahr);
  ~AlertHistory() {}

private:
  char *historyfilename() { return((char *)"alerthistory"); }
};

class CheckHistory : public History {
public:
  CheckHistory(Check *check, char *host) : History(check, host) {}
  CheckHistory(char *check, char *host) : History(check, host) {}
  CheckHistoryRecord *iterate_next();
  CheckHistoryRecord *iterate_previous();
  bool record(CheckHistoryRecord *chr);
  ~CheckHistory() {}

private:
  char *historyfilename() { return((char *)"checkhistory"); }
};

class CommandHistory : public History {
public:
  CommandHistory(Check *check, char *host) : History(check, host) {}
  CommandHistory(char *check, char *host) : History(check, host) {}
  CommandHistoryRecord *iterate_next();
  CommandHistoryRecord *iterate_previous();
  bool record(CommandHistoryRecord *chr);
  ~CommandHistory() {}

private:
  char *historyfilename() { return((char *)"commandhistory"); }
};

class FixHistory : public History {
public:
  FixHistory(Check *check, char *host) : History(check, host) {}
  FixHistory(char *check, char *host) : History(check, host) {}
  FixHistoryRecord *iterate_next();
  FixHistoryRecord *iterate_previous();
  bool record(FixHistoryRecord *fhr);
  ~FixHistory() {}

private:
  char *historyfilename() { return((char *)"fixhistory"); }
};

class CommonStateData {
public:
  CommonStateData(int instances, int returncode, char *summary, time_t time,
		  time_t since, int duration);
  int duration();  // Currently only used by checkstate
  int instances();
  int returncode();
  time_t since();
  char *summary();
  time_t time();
  ~CommonStateData();
  
private:
  char *s;      // Summary
  int dur;      // Duration
  int insts;    // Instances
  int rc;       // Return code
  time_t st;    // Time of first instance
  time_t t;     // Time
};

typedef CommonStateData CheckStateData;

class AlertStateData : public CommonStateData {
public:
  AlertStateData(RecipientSet *recipients, int instances, int checkreturncode,
		 char *checksummary, time_t time, time_t since);
  int checkreturncode() { return(returncode()); }
  char *checksummary() { return(summary()); }
  RecipientSet *recipients();
  ~AlertStateData();
  
private:
  RecipientSet *rs;   // Recipients
};

class Acknowledgement {
public:
  Acknowledgement(char *who, char *why);
  char *who();
  char *why();
  ~Acknowledgement();

private:
  char *w;      // Who set the acknowledgement
  char *y;      // The reason provided
};

// Inhibition isn't typedef'd because that breaks SurvivorXML
class Inhibition {
public:
  Inhibition(char *who, char *why);
  char *who();
  char *why();
  ~Inhibition();

private:
  char *w;      // Who set the inhibition
  char *y;      // The reason provided
};

class CallListStateData {
public:
  CallListStateData(char *lnperson, char *lnaddress, char *lnmodule);
  CallListStateData(char *lnperson, char *lnaddress, char *lnmodule,
		    time_t lrotate, char *ocperson);
  char *last_notified_address();
  char *last_notified_person();
  char *last_notified_via();
  time_t last_rotated();
  char *oncall_person();
  ~CallListStateData();
  
private:
  char *p;      // Last Person notified
  char *a;      // Last address notified
  char *m;      // Module used for notification
  time_t r;     // Time of last rotation
  char *ocp;    // OnCall Person

  void init(char *lnperson, char *lnaddress, char *lnmodule, time_t lrotate,
	    char *ocperson);
};

class Escalation {
public:
  Escalation(int escto);
  int escalated_to();
  ~Escalation();

private:
  int e;    // Escalation level
};

class FixStateData : public CommonStateData {
public:
  FixStateData(char *initiatedby, int instances, int returncode,
	       char *summary, time_t time, time_t since) :
    CommonStateData(instances, returncode, summary, time, since, -1) {
    // Allocate and initialize a new FixStateData object, holding
    // <initiatedby>, <instance>, <returncode>, <summary>, and <time>.

    // Returns: A new FixStateData object.

    by = xstrdup(initiatedby);
  }
  char *initiatedby();
  ~FixStateData();
  
private:
  char *by;     // Initiated by
};

class State {
public:
  State();
  char *id();
  bool nolock();
  ~State();

protected:
  char *csid;

  time_t last_host_state_update(char *host, char *service);
  int lock_calllist_state(char *list);
  int lock_host_state(char *host, char *service, bool write);
  int lock_misc_state(char *statefile);
  int lock_service_state(char *service);
  bool unlock_state(int x);

private:
  bool lock;

  char *hlfile(char *host, char *service);
  time_t last_update(char *lockfile);
  bool mark_update(char *lockfile);
};

class CheckState : public State {
public:
  CheckState(Check *check);
  CheckState(Check *check, char *host);
  char *comment();
  int consecutive();
  int duration();
  time_t lastcheck();
  bool parse_results(Array<CheckResult> *crs);
  bool parse_tmp_state_results(List *hosts);
  bool reschedule();
  int returncode();
  time_t since();
  char *tmp_state_filename();
  bool verify_freshness();
  bool write_misconfig(List *hosts);
  bool write_results();
  bool write_timeout(List *hosts);
  List *written_hosts();
  ~CheckState();

private:
  Array<CheckResult> *rbuf;      // Result buffer
  Check *c;                      // A pointer to the Check object in question
  List *written;                 // Hosts successfully written by _results()
  char *h;                       // A hostname for state info
  char *hf;                      // Buffer for historyfile();
  char *sf;                      // Buffer for statusfile()
  char *tsf;                     // Buffer for tmp_state_filename()
  time_t last;                   // Time of last refresh
  CheckStateData *dcache;        // Cached data

  void init(Check *check, char *host);
  char *lastfile();
  bool read_status(bool dolock);
  char *statusfile();
  void uncache(bool init);
  bool write_results(List *hosts);
  bool write_unfinished(List *hosts, int rc, char *comment);
};

class AlertState : public State {
public:
  AlertState(Check *check, char *host);
  bool acknowledge(CheckState *cs, char *who, char *why);
  char *acknowledged();
  char *acknowledged_for();
  time_t acktime();
  int alertfor();
  int alerts();
  bool clearstate();
  char *commentfor();
  bool escalate_manual(int to);
  bool escalated(AlertPlan *ap, CheckState *cs);
  int escalated_manual();
  bool inhibit(char *who, char *why);
  char *inhibited();
  char *inhibited_for();
  time_t inhibittime();
  time_t lastalert();
  RecipientSet *lastnotify();
  bool quiet();
  bool results_accumulate(int rc, char *module, char *notify);
  void results_finish();
  bool results_init(int crc, char *cmt);
  bool results_write();
  time_t since();
  char *token();
  bool unacknowledge();
  bool uninhibit();
  bool verify_freshness();
  ~AlertState();
  
private:
  Check *c;                      // A pointer to the Check object in question
  char *h;                       // A hostname for state info
  char *af;                      // Buffer for ackfile
  char *ef;                      // Buffer for escalatefile
  char *nf;                      // Buffer for noalertfile
  char *sf;                      // Buffer for statusfile
  char *tf;                      // Buffer for tokenfile
  time_t last;                   // Time of last refresh
  Escalation *ecache;            // Cache for escalate();
  int qcache;                    // Cache for quiet();
  Acknowledgement *acache;       // Cache for acknowledged[_for]();
  Inhibition *icache;            // Cache for inhibited[_for]();
  char *tcache;                  // Cache for token();
  AlertStateData *dcache;        // Cached status data
  char *r_cmt;                   // Buffer for results_*();
  int r_crc;                     // Buffer for results_*();
  RecipientSet *r_rset;          // Buffer for results_*();

  char *ackfile();
  char *escalatefile();
  char *noalertfile();
  bool read_status(bool dolock);
  char *statusfile();
  char *tokenfile();
  void  uncache(bool init);
};

class CallListState : public State {
public:
  CallListState(CallList *calllist);
  bool addsub(Substitution *sub);
  char *checksubs(char *name, time_t asof);
  char *lastnotify();
  char *lastnotifyaddress();
  char *lastnotifyvia();
  time_t lastrotate();
  bool notenotify(char *person, char *via, char *address);
  bool notenotify(char *person, char *via, char *address, char *oncall);
  char *oncall();
  bool prunesubs(time_t fromtime, time_t untiltime);
  Array<Substitution> *readsubs();
  ~CallListState();

private:
  CallList *cl;                  // A pointer to the CallList in question
  char *sf;                      // Buffer for substitutionfile
  char *stf;                     // Buffer for statusfile
  CallListStateData *dcache;     // Cache for read_status();

  bool  read_status();
  Array<Substitution> *read_substitution_status(bool dolock);
  char *statusfile();
  char *substitutionfile();
  void  uncache(bool init);
  bool  write_substitutions(Array<Substitution> *sarray);
};

class FixState : public State {
public:
  FixState(Check *check, char *host);
  bool clearstate();
  char *comment();
  int fix_attempts();
  time_t lastfix();
  char *lastfix_by();
  bool lock_fix(fix_lock_t locktype, int expire);
  bool parse_results(int fd);
  int returncode();
  time_t since();
  bool unlock_fix();
  bool verify_freshness();
  bool write_results(char *who);
  ~FixState();

private:
  Check *c;                      // The Check for this FixState
  fix_lock_t lt;                 // Lock type
  char *h;                       // The host for this FixState
  char *fld;                     // Fix lock directory
  char *lfile;                   // Lock file created
  char *sf;                      // Buffer for statusfile()
  FixResult *parsed;             // Read by parse_results()
  time_t last;                   // Time of last refresh
  FixStateData *dcache;          // Cached data

  bool read_status(bool dolock);
  char *statusfile();
  void uncache(bool init);
};

class RunningState : public State {
public:
  RunningState();
  time_t get(char *scheduler);
  bool note(char *scheduler);
  ~RunningState();

private:
};

class QueueUnit : public HashUnit {
public:
  QueueUnit(Alert *alert);
  QueueUnit(char *check, char *group, int timeout);
  QueueUnit(char *check, List *hosts, int timeout);
  Alert *alert();
  char *check();
  char *group();
  List *hosts();
  bool mark(queueunit_t s);
  queueunit_t status();
  bool timedout();
  int timeout();
  ~QueueUnit();

private:
  Alert *a;           // Alert object that is queued
  List *h;            // List of hosts that will be checked
  char *c;            // Name of the Check object that is queued
  char *g;            // Name of host Group consisting of hosts to be checked
  queueunit_t s;      // Status of this QueueUnit
  int t;              // Timeout for this Check
  struct timeval ta;  // Time this QueueUnit was marked assigned
  
  void init(Alert *alert, char *check, char *group, List *hosts, int timeout);
};

class Queue {
public:
  Queue();
  bool append(QueueUnit *q);
  bool find(char *check);
  QueueUnit *next();
  ~Queue();

private:
  QueueUnit *head;    // The head of the linked list of QueueUnits
  QueueUnit *tail;    // The tail of the linked list of QueueUnits

  void gc();
};

class TimerUnit {
public:
  TimerUnit(pid_t pid, pthread_t tid, int signal, time_t when);
  bool now();
  pid_t pid();
  int signal();
  pthread_t tid();
  ~TimerUnit();
  
private:
  pid_t p;            // Process to signal
  int s;              // Signal to send
  pthread_t t;        // Thread to signal
  time_t w;           // When to send it
};

class PidPair {
public:
  PidPair(pid_t pid, pid_t ppid);
  pid_t pid();
  pid_t ppid();
  ~PidPair();

private:
  pid_t p;            // PID
  pid_t pp;           // Parent PID
};

class PRNG {
public:
  PRNG();
  bool init();
  long n32bit();
  char *s20char();
  ~PRNG();
  
private:
  char s[24];
};

class SHPair {
public:
  SHPair(char *arg);
  SHPair(char *service, char *host);
  char *host();
  char *service();
  ~SHPair();

private:
  char *h;               // The hostname
  char *s;               // The service name
};

class HistorySet {
public:
  HistorySet(char *host, char *service);
  bool attach_history(AlertHistory *ahist, bool backwards);
  bool attach_history(CheckHistory *chist, bool backwards);
  bool attach_history(CommandHistory *cmhist, bool backwards);
  bool attach_history(FixHistory *fhist, bool backwards);
  char *host();
  AlertHistoryRecord *iterate_alerthistory();
  CheckHistoryRecord *iterate_checkhistory();
  CommandHistoryRecord *iterate_commandhistory();
  FixHistoryRecord *iterate_fixhistory();
  char *service();
  ~HistorySet();

private:
  AlertHistory *ah;    // If not NULL, generate alert data from this 
  CheckHistory *ch;    // If not NULL, generate check data from this 
  CommandHistory *cmh; // If not NULL, generate command data from this 
  FixHistory *fh;      // If not NULL, generate fix data from this
  bool ar;             // Iterate alert history backwards
  bool cr;             // Iterate check history backwards
  bool cmr;            // Iterate command history backwards
  bool fr;             // Iterate fix history backwards
  char *h;             // Host
  char *s;             // Service
};

class ReportFormatting {
public:
  ReportFormatting(char *module, char *style);
  ReportFormatting(char *module, char *style, char *tmpdir);
  ReportFormatting(char *module, char *style, char *tmpdir, char *uriprefix);
  char *module();
  char *style();
  char *tmpdir();
  char *uriprefix();
  ~ReportFormatting();

private:
  char *md;            // Module
  char *st;            // Formatting style
  char *td;            // Temp file directory to use
  char *up;            // URI prefix
  
  void init(char *module, char *style, char *tmpdir, char *uriprefix);
};

// Some objects in cgi-common and gateway-common need items defined above
#include "cgi-common.H"
#include "gateway-common.H"

class Executor {
public:
  Executor();
  bool close_pipe();
  pid_t exec_alert(Alert *alert, AlertModule *module);
  pid_t exec_check(Check *check, CheckRequest *cr);
  pid_t exec_check(Check *check, CheckState *cstate, CheckRequest *cr);
  pid_t exec_fix(Check *check, FixRequest *fr);
  pid_t exec_formatted_alert(FormattedAlert *alert, char *transmit);
  int *exec_pipe(char *command);
  pid_t exec_report_begin(ReportFormatting *rf);
  bool exec_report_continue(HistorySet *hs);
  int exec_report_end();
  pid_t exec_webauth(CGIAuthRequest *wreq);
  int result();
  int result(bool reset);
  int result(AlertState *as, char *module);
  int result(CGIAuthResult **arptr);
  int result(CheckResult **crptr);
  int result(FixState *fs);
  ~Executor();

private:
  Alert *a;               // Alert object that was run, if any
  CGIAuthRequest *ar;     // CGIAuthRequest, if any
  Check *c;               // Check object that was run, if any
  CheckState *cs;         // CheckState to record to, if any
  FormattedAlert *f;      // FormattedAlert object that was run, if any
  List *h;                // List of hosts we were provided, if any
  ReportFormatting *rep;  // ReportFormatting, if any
  pid_t pid;              // pid of child process
  int fds[2];             // I/O file descriptors for child process
  int fds2[2];            // Additional I/O file descriptors for child process

  char **allocate_argv(char *cmd);
  void closepipe();
  void do_exec_check_child(Check *check);
  void do_exec_check_parent(int xmlfd, Check *check, CheckRequest *cr);
  void dump_misconfig(List *hosts, char *msg1, char *msg2);
  void exec_check_components(Check *check, CheckRequest *creq);
  void exec_format_transmit(Alert *alert, AlertModule *module);
  void reset_executor();
};

class ModuleConfigError {
public:
  ModuleConfigError(char *modname, char *modtype, char *moderr);
  char *moderr();
  char *modname();
  char *modtype();
  ~ModuleConfigError();

private:
  char *e;    // Module configuration error
  char *n;    // Name of module that is misconfigured
  char *t;    // Type of module ("check", "fix", etc)
};

class StalledCheck : public SHPair {
public:
  StalledCheck(char *arg, time_t lastcheck) : SHPair(arg) {
    lc = lastcheck;
  }
  StalledCheck(char *service, char *host, time_t lastcheck) :
    SHPair(service, host) {
    lc = lastcheck;
  }
  time_t lastcheck();
  ~StalledCheck();

private:
  time_t lc;         // Time of last successful check
};

class SurvivorStatus {
public:
  SurvivorStatus();
  bool add_parse_error(char *err);
  bool add_module_error(char *modname, char *modtype, char *err);
  bool add_stalled_check(char *service, char *host, time_t lastcheck);
  time_t last_alert_cycle();
  time_t last_check_cycle();
  Array<ModuleConfigError> *module_errors();
  char *parse_error();
  bool set_last_cycles(time_t alert, time_t check);
  Array<StalledCheck> *stalled_checks();
  ~SurvivorStatus();

private:
  Array<ModuleConfigError> *merrs;  // Array of ModuleConfigErrors
  Array<StalledCheck> *scks;   // Array of StalledChecks
  char *perr;                // Config file parse error(s), if any
  time_t atime;              // Time of last alert scheduler cycle
  time_t ctime;              // Time of last check scheduler cycle
};

class Configuration {
public:
  Configuration();
  bool add_alertmodule(AlertModule *module);
  bool add_alertplan(AlertPlan *alertplan);
  bool add_alertplan_alias(AlertPlan *original, char *alias,
			   CallList *target, CallList *replacement);
  bool add_calllist(CallList *calllist);
  bool add_check(Check *check);
  bool add_class(HostClass *hostclass);
  bool add_dependency(Dependency *dependency);
  bool add_fix(Fix *fix);
  bool add_group(List *group);
  bool add_named_returngroup(Array<int> *returngroup);
  bool add_named_schedule(Array<Schedule> *schedule);
  bool add_person(Person *person);
  bool add_transport(Transport *transport);
  int alert_shift();
  int alert_throttle();
  AlertModule *find_alertmodule(char *name);
  AlertPlan *find_alertplan(char *name);
  CallList *find_calllist(char *name);
  Check *find_check(char *name);
  HostClass *find_class(char *hostname);
  Fix *find_fix(char *name);
  List *find_group(char *name);
  List *find_groups(char *hostname);
  HostClass *find_hostclass(char *classname);
  Person *find_person(char *name);
  Array<int> *find_returngroup(char *name);
  Array<Schedule> *find_schedule(char *name);
  Transport *find_transport(char *name);
  Array<CallList> *get_all_calllists();
  Array<Check> *get_all_checks();
  Array<Fix> *get_all_fixes();
  List *get_all_hostclasses();
  List *get_all_returngroups();
  Array<SHPair> *history_consistency();
  int max_dependencies();
  bool parse_cfs();
  bool remove_history(char *service, char *host);
  bool runs_on_host(char *service, char *host);
  bool set_alert_shift(int seconds);
  bool set_alert_throttle(int value);
  bool set_max_dependencies(int max);
  bool state_consistency();
  ~Configuration();
  
private:
  Array<CallList> *calllistarray;           // Array of CallLists for iteration
  Array<Check> *checkarray;                 // Array of Checks for iteration
  Array<Dependency> *deparray;              // Array of Dependencies
  Array<Fix> *fixarray;                     // Array of Fixes for iteration
  Hashtable<AlertModule> *alertmodhash;     // Hashtable of AlertModules
  Hashtable<AlertPlan> *alerthash;          // Hashtable of AlertPlans
  Hashtable<CallList> *calllisthash;        // Hashtable of CallLists
  Hashtable<Check> *checkhash;              // Hashtable of Checks
  Hashtable<HostClass> *classhash;          // Hashtable of HostClasses
  Hashtable<HostClass> *classbymember; // Hashtable of HostClasses by hostname
  Hashtable<Fix> *fixhash;                  // Hashtable of Fixes
  Hashtable<List> *grouphash;               // Hashtable of Group names
  Hashtable<List> *groupbymember;           // Hashtable of Groups by hostname
  Hashtable<Person> *personhash;            // Hashtable of Persons (People)
  Hashtable< Array<int> > *returngrouphash; // Hashtable of Return Groups
  Hashtable< Array<Schedule> > *schedhash;  // Hashtable of Schedules
  Hashtable<Transport> *transporthash;      // Hashtable of Transports
  List *classlist;                          // List of HostClasses
  List *hostlist;                           // List of hosts
  List *returngrouplist;                    // List of return groups
  int as;                                   // Alert shift value
  int at;                                   // Alert throttle value
  int m;                                    // Maximum dependencies

  bool add_all_hosts(List *l);
  List *find_all_hosts(char *name);
  bool remove_directory(char *parent, char *dir);
};

class Instance {
public:
  Instance(char *name);
  bool cli_allow_archive();
  bool cli_allow_root();
  bool cli_require_comment();
  char *configdir();
  char *helpdir();
  char *historydir();
  char *name();
  char *statedir();
  bool set_cli_allow_archive(); 
  bool set_cli_allow_root(); 
  bool set_cli_require_comment(); 
  bool set_config(char *dir);
  bool set_help(char *dir);
  bool set_history(char *dir);
  bool set_state(char *dir);
  bool set_tmp(char *dir);
  char *tmpdir();
  ~Instance();

private:
  bool cliarc;           // Whether history archiving is enabled
  bool cliroot;          // Whether root can run sc
  bool clicmt;           // Whether comments are required
  char *cfdir;           // Configuration directory of this Instance
  char *hdir;            // History directory of this Instance
  char *hpdir;           // Help directory of this Instance
  char *instname;        // Name of this Instance
  char *sdir;            // State directory of this Instance
  char *tdir;            // Temporary directory of this Instance
};

class Args {
public:
  Args();
  int alert_threads();
  bool arg(char c);
  char *arg_o(char *option);
  bool background();
  int check_threads();
  bool cli_allow_archive();
  bool cli_allow_root();
  bool cli_require_comment();
  char *comment();
  char *configdir();
  time_t fromtime();
  char *helpdir();
  char *histdir();
  List *instances();
  gid_t instgid();
  char *instname();
  char *instuser();
  bool keepalive();
  char *moddir();
  bool parse_args(char *instance, char *instconfig, char *moddir);
  bool parse_args(char *opts, int argc, char **argv);
  bool parse_args(char *opts, char **oopts, int argc, char **argv);
  bool parse_instcf();
  char *pidfile();
  char *pkgdir();
  bool runningasinst();
  bool smartscheduling();
  char *statedir();
  bool timestamp();
  char *tmpdir();
  time_t untiltime();
  bool version();
  char *version(version_t info);
  ~Args();

private:
  Instance *inst;                           // Instance object
  Hashtable<char> *oopthash;                // Parsed -o options
  bool bg;                                  // Whether or not to background
  bool ka;                                  // Whether or not to keepalive
  bool smartsched;                          // Enable smart scheduling?
  bool tstamp;                              // Timestamp debug output?
  bool vers;                                // Display version?
  char *cmt;                                // Comment
  char *instance;                           // Instance we are
  char *instancebuf;                        // Buffer holding instance
  char *instcf;                             // Instance CF to use, non default
  char *mdir;                               // Where to find modules
  char *pfile;                              // Where to write pid file
  char *trueargs;                           // Other args passed to parse
  gid_t gid;                                // INSTGROUP gid
  int athreads;                             // Number of alert threads
  int cthreads;                             // Number of check threads
  int runuid;                               // Whether or not running as uid
  time_t ft;                                // From time
  time_t ut;                                // Until time

  bool check_instance();
};
