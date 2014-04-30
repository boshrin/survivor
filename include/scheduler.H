/*
 * scheduler.H
 *
 * Version: $Revision: 0.12 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2004/03/02 03:48:30 $
 *
 * Copyright (c) 2002 - 2003
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: scheduler.H,v $
 * Revision 0.12  2004/03/02 03:48:30  benno
 * Changes for staggered scheduling
 *
 * Revision 0.11  2003/10/06 23:23:31  benno
 * Add verify_type2_dependencies(Fix)
 *
 * Revision 0.10  2003/07/31 17:19:56  benno
 * Add sc_unprotect_all(reset)
 *
 * Revision 0.9  2003/04/13 20:03:35  benno
 * Changes for state caching
 *
 * Revision 0.8  2003/03/31 14:52:28  benno
 * Add LockData and StateCache
 *
 * Revision 0.7  2003/03/04 20:57:41  benno
 * Bump copyright
 *
 * Revision 0.6  2002/12/31 04:40:45  benno
 * Add Fix support
 * verify degraded across hosts
 *
 * Revision 0.5  2002/04/26 20:12:41  benno
 * add BROKEN_THREAD_SIGNALS
 *
 * Revision 0.4  2002/04/04 19:59:57  benno
 * copyright
 *
 * Revision 0.3  2002/04/02 22:11:50  benno
 * rcsify date
 *
 * Revision 0.2  2002/04/02 22:11:40  benno
 * Cleanup
 * Split into survivor.H
 *
 * Revision 0.1  2002/04/02 22:10:32  benno
 * initial revision
 *
 */

#include "survivor.H"

// Scheduler specific definitions

#define STAGGERSCHED_HOST_MINIMUM 20

// Prototypes from handler.C

void  dummy_catcher(int i);
void  keepalive_catcher(int i);
void *alert_handler(void *x);
void *alert_scheduler_handler(void *x);
void *check_handler(void *x);
void *check_scheduler_handler(void *x);
void *signal_handler(void *x);
void *timer_handler(void *x);

#if defined(_BROKEN_THREAD_SIGNALS)
// An extra catcher is required to tell the signal thread to exit.
void extra_catcher(int i);
#endif // _BROKEN_THREAD_SIGNALS

// Class definitions

class ProcessControl;

class LockData {
public:
  LockData(char *id, void *data);
  void *idata();
  char *id();
  ~LockData();

private:
  char *i;
  void *d;
};

class iMutexLockData : public LockData {
public:
  iMutexLockData(char *id, void *data);
  bool lock();
  bool unlock();
  ~iMutexLockData();

private:
  bool ok;                         // If we're ready for use
  pthread_mutex_t dlock;           // Lock for the data stored here
};

template <class T>
class MutexLockData : public iMutexLockData {
public:
  MutexLockData(char *id, T *data) : iMutexLockData(id, (void *)data) {}
  T *data() { return((T *)idata()); }
  ~MutexLockData() {
    T *t = data();
    xdelete(t);
  }
  
private:
};

class iRWLockData : public LockData {
public:
  iRWLockData(char *id, void *data);
  bool read_lock();
  bool write_lock();
  bool unlock();
  ~iRWLockData();
  
private:
  bool ok;                         // If we're ready for use
  pthread_rwlock_t dlock;          // Lock for the data stored here
};

template <class T>
class RWLockData : public iRWLockData {
public:
  RWLockData(char *id, T *data) : iRWLockData(id, (void *)data) {}
  T *data() { return((T *)idata()); }
  ~RWLockData() {
    T *t = data();
    xdelete(t);
  }

private:
};

class LockManager {
public:
  LockManager();
  bool aq_lock();
  bool aq_unlock();
  bool cf_read();
  bool cf_write();
  bool cf_unlock();
  bool cl_lock();
  bool cl_unlock();
  bool cq_lock();
  bool cq_unlock();
  bool ok();
  State *sc_lock(char *id);
  bool sc_protect(char *id, State *data);
  bool sc_unlock(char *id);
  bool sc_unprotect_all();
  bool sc_unprotect_all(bool reset);
  bool ta_lock();
  bool ta_unlock();
  ~LockManager();
  
private:
  Hashtable< MutexLockData<State> > *states;  // Hash of State data
  pthread_mutex_t aqlock;          // Lock over Alert Queue
  pthread_mutex_t cllock;          // Lock over CallList state
  pthread_mutex_t cqlock;          // Lock over Check Queue
  pthread_mutex_t talock;          // Lock over Timer Array
  pthread_rwlock_t cflock;         // Lock over Configuration
  int bad;                         // True if LockManager failed to initialize
};

class Worker {
public:
  Worker(ProcessControl *p, bool alert, bool check);
  bool assign(QueueUnit *q);
  void begin();
  bool end();
  ~Worker();

private:
  ProcessControl *pc;    // Process manager that will queue checks and alerts
  QueueUnit *task;       // QueueUnit to process
  bool aw;               // Flag to indicate that we are processing Alerts
  bool cw;               // Flag to indicate that we are processing Checks
  bool exit;             // Flag to indicate scheduler should exit

  void process_alert();
  void process_check();
  bool verify_type2_dependencies(Check *c, int rcount, List *hosts);
  bool verify_type2_dependencies(Fix *f);
};

class AlertWorker : public Worker {
public:
  AlertWorker(ProcessControl *p) : Worker(p, true, false) {}
  ~AlertWorker() {}

private:
};

class CheckWorker : public Worker {
public:
  CheckWorker(ProcessControl *p) : Worker(p, false, true) {}
  ~CheckWorker() {}

private:
};

class Timer {
public:
  Timer();
  void begin();
  bool end();
  bool request(int signal, int seconds);
  bool request(pid_t pid, int signal, int seconds);
  bool unrequest();
  bool unrequest(pid_t pid);
  ~Timer();
  
private:
  bool exit;             // Flag to indicate timer should exit
  Array<TimerUnit> *ts;  // Array of TimerUnits
};

class Scheduler {
public:
  Scheduler(ProcessControl *p);
  void begin_alert();
  void begin_check();
  bool end();
  ~Scheduler();

private:
  ProcessControl *pc;    // Process manager that will queue checks and alerts
  bool exit;             // Flag to indicate scheduler should exit

  int run_alert(Check *c, List *g, int throttle);
  void run_check(Check *c, List *g);
  void run_loop(bool alert, bool check);
  bool schedule_alert(AlertPlan *ap, Check *c, List *g, char *host,
		      AlertState *as, CheckState *cs, FixState *fs);
  bool schedule_check(Check *c, char *host, Array<Schedule> *sched,
		      CheckState *cs, Hashtable<Counter> *ctrs);
  bool verify_degraded(int hostfail, int checkfail, Check *c, List *g,
		       char *host, Array<Schedule> *sched);
  bool verify_type1_dependencies(Check *c, char *host);
};

class StateCache {
public:
  StateCache();
  bool disable();
  bool enable();
  AlertState *get_alert_state(Check *c, char *h);
  CheckState *get_check_state(Check *c, char *h);
  FixState *get_fix_state(Check *c, char *h);
  AlertState *release(AlertState *as);
  CheckState *release(CheckState *cs);
  FixState *release(FixState *fs);
  ~StateCache();

private:
  bool caching;                  // Whether or not caching is enabled
};

class ProcessControl {
public:
  ProcessControl();
  QueueUnit *alert_worker_available();
  QueueUnit *check_worker_available();
  bool discontinue();
  bool establish_schedulers();
  bool establish_signals();
  bool establish_timer();
  bool establish_workers();
  bool queue_alert(Alert *alert);
  bool queue_check(char *check, List *hosts);
  bool request_timeout(int signal, int seconds);
  bool request_timeout(pid_t pid, int signal, int seconds);
  bool unrequest_timeout();
  bool unrequest_timeout(pid_t pid);
  ~ProcessControl();

private:
  AlertWorker **aws;               // AlertWorkers, one per thread
  CheckWorker **cws;               // CheckWorkers, one per thread
  Queue *alertq;                   // Alert queue, via LockManager
  Queue *checkq;                   // Check queue, via LockManager
  Scheduler *as;                   // Alert Scheduler object
  Scheduler *cs;                   // Check Scheduler object
  Timer *t;                        // Timer object
  pthread_t *aworkers;             // Thread IDs of alert worker threads
  pthread_t *cworkers;             // Thread IDs of check worker threads
  pthread_t aschedtid;             // Thread ID of alert scheduler thread
  pthread_t cschedtid;             // Thread ID of check scheduler thread
  pthread_t sigtid;                // Thread ID of signal handler
  pthread_t timertid;              // Thread ID of timer thread
  int awavail;                     // Available alert workers, via alertq Lock
  int cwavail;                     // Available check workers, via checkq Lock
};

// Externs

extern LockManager *locks;
extern StateCache *scache;
extern StatCounter *statc;

#if defined(_BROKEN_THREAD_SIGNALS)
// Make this global so that the extra catcher can tell the signal thread
// to exit.
extern pthread_t glsigtid;         // Global equivalent of PC->sigtid
#endif // _BROKEN_THREAD_SIGNALS
