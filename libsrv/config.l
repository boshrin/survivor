%start APALIAS
%start APSTANZA
%start CALLLISTALERT
%start CALLLISTALIAS
%start CALLLISTCF
%start CALLLISTSTANZA
%start CGIAUTHZLEVEL
%start CGIAUTHZTYPE
%start CGIAUTHZWHO
%start CGICF
%start CGIVALUE
%start CHECKCF
%start CHECKLIST
%start CHECKSTANZA
%start CLASSSTANZA
%start DEFAULT
%start DEPENDENCYCF
%start DEPENDSTANZA
%start EVERY
%start FILENAME
%start FIXSTANZA
%start GATEWAYCF
%start GROUP
%start GROUPALIAS
%start HOSTCF
%start INSTANCECF
%start INSTANCESTANZA
%start LIST
%start LISTX
%start MODULE
%start MODULEARG
%start MODULEVALUE
%start NAME
%start NAMEDEF
%start NOTIFIES
%start NOTIFY
%start NUMBER
%start PERSONSTANZA
%start RELAYTARGETSTANZA
%start RETURNSTANZA
%start SCHEDCF
%start SCHEDSTANZA
%start TIME
%start TRANSPORTSTANZA
%start TRYSET
%start TRYSTANZA

%a 5000
%e 2000
%n 1500
%p 7000
%o 4500

%{
/*
 * survivor configuration file parser
 *
 * Version: $Revision: 0.38 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2005/11/24 02:44:41 $
 * MT-Level: Unsafe
 *  Multiple threads calling lex is probably a bad idea.
 *
 * For some bizarre reason, you can't have apostrophes in c++ style comments,
 * unless you have two of them in the same line.
 *
 * %p says to allocate that many positions, since we exceed the default amount.
 * %a says to allocate that many packed transitions for the same reason
 * %n says to allocate that many states
 * %e is the size of the parse tree
 * %o is the size of the output table
 *
 * Copyright (c) 2002 - 2005
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: config.l,v $
 * Revision 0.38  2005/11/24 02:44:41  benno
 * Do not allow the same person twice in the same calllist
 *
 * Revision 0.37  2005/10/20 02:12:02  benno
 * Add "do not clear state" and "clear state honors returngroups"
 *
 * Revision 0.36  2005/04/07 01:27:30  benno
 * Arguments are stored in Arrays instead of as Lists
 *
 * Revision 0.35  2004/12/05 22:12:04  benno
 * Store full path to helpdir
 *
 * Revision 0.34  2004/11/26 22:15:36  benno
 * Add support for gateway.cf
 * Add lexinit()
 * Add LISTX
 *
 * Revision 0.33  2004/08/25 00:48:05  benno
 * Add statedir and tmpdir for instance.cf
 *
 * Revision 0.32  2004/06/12 00:59:44  benno
 * Add result text significant
 *
 * Revision 0.31  2004/03/01 23:32:24  benno
 * Parse cgi.cf
 *
 * Revision 0.30  2003/11/29 05:29:29  benno
 * Add allow archive, all root, require comment in instance.cf
 *
 * Revision 0.29  2003/10/13 16:16:19  benno
 * Fix bug in parsing foo@bar dependency specification
 *
 * Revision 0.28  2003/10/06 23:17:22  benno
 * AlertPlan alias "adding"
 *
 * Revision 0.27  2003/09/16 01:52:27  benno
 * Overhaul dependencies
 *
 * Revision 0.26  2003/09/02 01:24:15  paul
 * Use fix_attempt_t
 * Use trailing context matching
 *
 * Revision 0.25  2003/06/17 15:14:08  benno
 * Bump limits for sun lex
 *
 * Revision 0.24  2003/05/29 00:32:26  benno
 * Add support for alert stanza in calllist.cf
 *
 * Revision 0.23  2003/05/14 02:30:30  selsky
 * Remove embedded nulls
 *
 * Revision 0.22  2003/05/04 21:31:33  benno
 * Fix reference to cf
 * Fix LEXLOGERR definition
 * Don't use string type
 *
 * Revision 0.21  2003/04/09 20:23:51  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.20  2003/04/07 04:37:48  benno
 * Use Debugger
 *
 * Revision 0.19  2003/03/31 13:27:44  benno
 * Add protection against redefining objects with the same names
 *
 * Revision 0.18  2003/03/06 17:29:05  benno
 * Fix type assignment
 *
 * Revision 0.17  2003/03/06 03:11:21  benno
 * Fix comment breaking solaris lex
 *
 * Revision 0.16  2003/03/02 05:16:13  benno
 * Add DEBUG_CFERRS
 * Move lexline to lexlineno to display config filename with line number
 * Check for mutual Type I dependencies
 * Fix bug when parsing dependency with exceptions
 *
 * Revision 0.15  2003/02/23 18:05:44  benno
 * Trim whitespace from named arg values
 *
 * Revision 0.14  2003/02/23 16:36:04  benno
 * Add composite checks
 *
 * Revision 0.13  2003/02/04 02:44:04  benno
 * Use IONULL
 *
 * Revision 0.12  2003/01/02 17:16:04  benno
 * Bump up % allocations
 *
 * Revision 0.11  2002/12/31 04:35:01  benno
 * Add fix support
 *
 * Revision 0.10  2002/12/22 17:14:22  benno
 * Add default keywords for notify on clear, timeouts, and n check failures
 * Add transports
 * Use xdelete
 *
 * Revision 0.9  2002/12/16 00:49:07  benno
 * Switch to try based AlertPlans
 *
 * Revision 0.8  2002/10/25 22:48:59  benno
 * add persons
 *
 * Revision 0.7  2002/10/21 20:45:56  benno
 * call list rotates using a schedule
 * named args
 *
 * Revision 0.6  2002/04/19 20:00:46  benno
 * use lexline instead of yylineno
 *
 * Revision 0.5  2002/04/04 20:09:38  benno
 * copyright
 *
 * Revision 0.4  2002/04/03 18:03:59  benno
 * rcsify date
 *
 * Revision 0.3  2002/04/03 18:03:46  benno
 * Add help directory to instance cf
 * notify using schedule on clear
 *
 * Revision 0.2  2002/04/03 18:02:43  benno
 * calllist.cf uses stanzas
 * "never" frequency
 *
 * Revision 0.1  2002/04/03 18:01:02  benno
 * initial revision
 *
 */

#include "survivor.H"

/* Redefine input to use get_yy instead to fix telnet read problem.
 * This also works around some sun lex compiler errors.
 */
int get_yy();

#undef input
#define input() (((yytchar=yysptr>yysbuf?U(*--yysptr):get_yy())==10?(lexline++,yytchar):yytchar)==EOF?0:yytchar)

/* Maintain some extra state information for error status and reuse of code.
 */

// cf object to use for lookups, since we may be reparsing
Configuration *newcf = NULL;
int lexerr = 0;
int lexline = 1;   // equivalent of yylineno
int yylaststate = 0;
char yynote = '\0';
config_file_t cftype = none_cf;

#define LEXLOGERR DEBUG_CFERRS | DEBUG_CONFIG | DEBUG_CFLESS, true, \
                  _config_files[cftype], lexline
#define LEXLOG    DEBUG_CONFIG | DEBUG_CFLESS, false, _config_files[cftype], \
                  lexline
#define LEXLOGMORE DEBUG_CONFIG, false, _config_files[cftype], lexline
 
// If we're parsing instance.cf, it'll go here instead
Instance *newi = NULL;
List *newil = NULL;

// Or if we are parsing cgi.cf
CGIConfiguration *newccf = NULL;

// Or if we are parsing gateway.cf
GatewayConfiguration *newgcf = NULL; 

#define BEGINX(x) yylaststate = YYSTATE; yynote = '\0'; BEGIN x;
#define BEGINXN(x,n) yylaststate = YYSTATE; yynote = n; BEGIN x;

/* In-progress buffers
 */

Array<AlertReturnGroup> *_aarray = NULL;
Array<AlertSchedule> *_asarray = NULL;
Array<AlertTry> *_atarray = NULL;
Array<Argument> *_ararray = NULL;
Array<CallList> *_calls = NULL;
Array<Check> *_ochecks = NULL; 
Array<Check> *_rchecks = NULL;
Array<Fix> *_fixes = NULL; 
Array<Person> *_parray = NULL; 
Array<Schedule> *_nsched = NULL;
Array<Schedule> *_sarray = NULL;
Array<Schedule> *_sarray2 = NULL; 
Array<int> *_rvs = NULL;
Array<int> *_ncrvs = NULL;
AlertPlan *_ap = NULL;
CallList *_call = NULL;
Check *_check = NULL;
Fix *_fix = NULL;
List *_list1 = NULL;
List *_list2 = NULL;
Person *_person = NULL;
Transport *_transport = NULL;
fix_attempt_t _tryfix = no_fix;
fix_lock_t _lockt = standard_lock;
bool  _all = false;
bool  _esc = false;
bool  _noncrvs = false;
bool  _nocsrg = false;
bool  _nonoc = false; 
bool  _notify = false;
bool  _notransport = false; 
char *_alert = NULL;
char *_arg = NULL;
char *_day = NULL;
char *_fromday = NULL;
char *_fromtime = NULL;
char *_group = NULL;
char *_help = NULL;
char *_name = NULL;
char *_inst = NULL;
char *_sched = NULL;
char *_untilday = NULL;
char *_untiltime = NULL;
char *_via = NULL;
int   _degrade = 0; 
int   _fail = 0;
int   _freq = -1;
int   _lock = -1;
int   _tout = -1; 

/* Default buffers
 */
 
// after n check failures
int _deffail = 1;
// do not clear state on return value
Array<int> *_defnoclearon = NULL;
// do not clear state on return group
List *_defnoclearrg = NULL; 
// expire fix lock
int _deflock = DEFAULT_FIX_TIMEOUT;
// notify on clear
bool _defnoc = false;
Array<Schedule> *_defnocsched = NULL;
// timeout
int _deftimeout = DEFAULT_TIMEOUT;
// transport (NULL = scheduler executes directly)
Transport *_defchecktm = NULL;
Transport *_deffixtm = NULL; 
%}

%%

<APALIAS>"adding" {
  // Find the list to be added

  BEGINXN(NAME, 'a');
}

<APALIAS>"replacing" {
  // Find the list to be replaced

  if(_call)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "AlertPlan alias target already specified");
#endif
    
    lexerr++;
  }
  else
  {
    BEGINXN(NAME, 'r');
  }
}

<APALIAS>"to" {
  // Read the name of the alias

  if(_name)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "AlertPlan alias already specified");
#endif
    
    lexerr++;
  }
  else
  {
    BEGINXN(NAME, 't');
  }
}

<APALIAS>"with" {
  // Find the list to be the replacement

  BEGINXN(NAME, 'w');
}

<APSTANZA>"always clear state" {
  // Do not use default clear state on

  if(_ncrvs || _noncrvs)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Clear state directive already specified");
#endif
    
    lexerr++;
  }
  else
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOG,
		  " - Will always clear state on any return code change");
#endif
    
    _noncrvs = true;
  }
}

<APSTANZA>"clear state honors returngroups" {
  // Use specified returngroups for clearing state

  if(_nocsrg || _list1)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Clear state returngroup directive already specified");
#endif
    
    lexerr++;
  }
  else
  {
    BEGINX(LIST);
  }
}

<APSTANZA>"clear state honors all returngroups" {
  // Use all currently defined returngroups for clearing state

  if(_nocsrg || _list1)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Clear state returngroup directive already specified");
#endif
    
    lexerr++;
  }
  else
  {
    _list1 = new List(newcf->get_all_returngroups());

    if(!_list1)
    {
      wlog->warn("lex unable to allocate new List");
      lexerr++;
    }
#if defined(DEBUG)
    else
      dlog->log_lex(LEXLOG,
		    " - Will not clear state when old and new return codes are both in same return group");
#endif
  }
}

<APSTANZA>"clear state ignores returngroups" {
  // Do not use returngroups for clearing state

  if(_nocsrg || _list1)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Clear state returngroup directive already specified");
#endif
    
    lexerr++;
  }
  else
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOG,
		  " - Will ignore returngroups when clearing state");
#endif
    
    _nocsrg = true;
  }
}

<APSTANZA>"do not clear state on return values"[ \t]*"{" {
  // Do not use default clear state on

  if(_ncrvs || _noncrvs)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Clear state directive already specified");
#endif
    
    lexerr++;
  }
  else
  {
    _ncrvs = new Array<int>();

    if(_ncrvs)
    {
      BEGINXN(NUMBER, 'c');
    }
    else
    {
      wlog->warn("lex unable to allocate new int Array");
      lexerr++;
    }
  }
}

<APSTANZA>"do not notify on clear" {
  // Do not use default notify on clear

  if(_notify || _nsched)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Notify on clear already specified");
#endif
    
    lexerr++;
  }
  else
    _nonoc = true;
}

<APSTANZA>"notify" {
  // Begin notify on clear definition
  //  _nsched = schedule to use

  if(_notify || _nsched)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR,
		   "Schedule already specified for notify on clear");
#endif
    
    lexerr++;
  }
  else
  {
    if(_nonoc)
    {
#if defined(DEBUG)
      dlog->log_lex(LEXLOGERR, "No notify on clear already specified");
#endif
    
      lexerr++;
    }
    else
    {
      BEGINXN(NOTIFY, 'a');
    }
  }
}

<APSTANZA>"on return value"|"on return values" {
  // Read one or more numbers

  if(_rvs)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Return value(s) already specified");
#endif

    lexerr++;
  }
  else
  {
    // Important distinction here.  The alertplan only deletes Array<int>
    // objects that are not named, since they were allocated here.
    // Named arrays are pointers to objects stored in the Configuration.
    
    _rvs = new Array<int>();

    if(_rvs)
    {
      BEGINXN(NUMBER, 'r');
    }
    else
    {
      wlog->warn("lex unable to allocate new int Array");
      lexerr++;
    }
  }
}

<APSTANZA>"on returngroup" {
  // Read a group name and look for the pointer in the configuration

  if(_rvs)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "returngroup already specified");
#endif

    lexerr++;
  }
  else
  {
    BEGINX(NAME);
  }
}

<APSTANZA>"default"/[{]? {
  if(_rvs)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Return value(s) already specified");
#endif
    
    lexerr++;
  }
  // else _rvs is already NULL
}

<APSTANZA>"{" {
  // We can reach this state with _rvs = NULL (for "default") or with
  // _rvs set (for "on returngroup"), so do not verify _rvs.
  // This stanza is essentially identical to one for NUMBER.
  //  _asarray = array of alert schedules
  //  _fail = number of required check failures
  //  _sarray = "using" this schedule
  
  if(_asarray)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "AlertSchedule Array already exists");
#endif
    
    lexerr++;
  }
  else
  {
    _asarray = new Array<AlertSchedule>();
    
    if(_asarray)
    {
      BEGINX(RETURNSTANZA);
    }
    else
    {
      wlog->warn("lex unable to allocate AlertSchedule Array");
      lexerr++;
    }
  }
}

<APSTANZA>"}" {
  // Contents of the AlertPlan are now defined, create an AlertPlan
  // and add it to the Configuration.

  if(_aarray && _name)
  {
    // Use default notify on clear setting unless a local setting was
    // provided
    
    bool donoc = _defnoc;
    Array<Schedule>* nocsched = _defnocsched;

    if(_nonoc)
    {
      donoc = false;
      nocsched = NULL;
    }
    else if(_notify)
    {
      donoc = _notify;
      nocsched = _nsched;
    }

    // Use default clear state values unless a local setting was provided

    if(!_ncrvs && !_noncrvs && _defnoclearon)
      _ncrvs = (Array<int> *)new iArray(_defnoclearon);

    if(!_list1 && !_nocsrg && _defnoclearrg)
      _list1 = new List(_defnoclearrg);

    if(!newcf->find_alertplan(_name))
    {
      AlertPlan *ap = new AlertPlan(_name, _aarray, donoc, nocsched, _ncrvs,
				    _list1);
    
      if(ap)
      {
	// ap takes over _ncrvs and _list1
	_ncrvs = NULL;
	_list1 = NULL;
	
	if(newcf->add_alertplan(ap))
	  _aarray = NULL;
	else
	{
	  delete ap;
	  wlog->warn("lex unable to add AlertPlan");
	  lexerr++;
	}
      }
      else
      {
	wlog->warn("lex unable to allocate AlertPlan");
	lexerr++;
      }
    }
    else
    {
#if defined(DEBUG)
      dlog->log_lex(LEXLOGERR, "AlertPlan '%s' redefined", _name);
#endif
      
      lexerr++;
    }
  }
  else
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "AlertPlan not fully defined");
#endif

    lexerr++;
  }

  xdelete(_name);
  xdelete(_ncrvs); // ints, not pointers, so do not use xadelete
  xdelete(_list1);
  xadelete(_aarray, AlertReturnGroup);

  _notify = false;
  _nsched = NULL;
  _nonoc = false;
  _noncrvs = false;
  _nocsrg = false;
  
  BEGINX(SCHEDCF);
}

<CALLLISTALERT>"format as" {
  // Read the name of the format module

  BEGINXN(NAME, 'f');
}

<CALLLISTALERT>"transmit with" {
  // Read the name of the transmit module

  BEGINXN(NAME, 't');
}

<CALLLISTALERT>"}" {
  // Store the alert stanza information in the configuration file

  if(_name && _arg && _via)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOG, "   Format with: %s", _arg);
    dlog->log_lex(LEXLOG, "   Transmit via: %s", _via);
#endif
    
    if(!newcf->find_alertmodule(_name))
    {
      AlertModule *am = new AlertModule(_name, _arg, _via);

      if(am)
      {
	if(!newcf->add_alertmodule(am))
	{
	  wlog->warn("lex unable to add AlertModule");
	  lexerr++;

	  xdelete(am);
	}
      }
      else
      {
	wlog->warn("lex unable to allocate new AlertModule object");
	lexerr++;
      }
    }
    else
    {
#if defined(DEBUG)
      dlog->log_lex(LEXLOGERR, "Alert stanza '%s' redefined", _name);
#endif
      lexerr++;
    }
  }
  else
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Incomplete alert stanza");
#endif
    lexerr++;
  }
  
  xdelete(_name);
  xdelete(_arg);
  xdelete(_via);

  BEGINX(CALLLISTCF);
}

<CALLLISTALIAS>"to" {
  // Read the name of the alias and then allocate the calllist
  
  BEGINXN(NAME, 't');
}

<CALLLISTALIAS>"via" {
  // Read the alert module
  
  BEGINXN(NAME, 'v');
}

<CALLLISTCF>"alert via" {
  // Begin alert module definition
  //  _name = name of stanza
  //  _arg = format module
  //  _via = transmit module

  if(_name || _arg || _via)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Alert stanza definition already in progress");
#endif
    
    lexerr++;
  }
  else
  {
    BEGINXN(NAMEDEF, 'A');
  }
}

<CALLLISTCF>"alias" {
  // Begin call list alias definition
  //  _name = alias (this does not actually get set)
  //  _call = original list
  //  _via = alert module

  if(_call)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR,
		   "Call list alias definition already in progress");
#endif
    
    lexerr++;
  }
  else
  {
    BEGINX(NAME);
  }
}

<CALLLISTCF>"call list" {
  // Begin call list definition
  //  _name = list name
  //  _notify = broadcast list
  //  _sched = rotation schedule
  //  _parray = call list
  //  _via = alert module

  if(_name)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Call list definition already in progress");
#endif
    
    lexerr++;
  }
  else
  {
    _notify = false;
    BEGINXN(NAMEDEF, 'l');
  }
}

<CALLLISTCF>"person" {
  // Begin person definition
  // _name = name of person
  // _person = person
  // _all = true if default contact already seen
  
  if(_person)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Person definition already in progress");
#endif
    
    lexerr++;
  }
  else
  {
    BEGINXN(NAMEDEF, 'p');
  }
}

<CALLLISTSTANZA>"broadcasts to all" {
  // This is a broadcast list
  
  _notify = true;
}

<CALLLISTSTANZA>"notifies"/[{]? {
  // Read an array of persons
  
  BEGINX(NOTIFIES);
}

<CALLLISTSTANZA>"rotates using" {
  // Determine the name of the scheduler on which this call list rotates.

  _notify = false;
  BEGINXN(NAME, 'r');
}

<CALLLISTSTANZA>"schedule" {
  // This is effectively optional
}

<CALLLISTSTANZA>"via" {
  // Read the name of the module
  
  BEGINXN(NAME, 'v');
}

<CALLLISTSTANZA>"}" {
  // Store this call list information

  if(_parray && _name && _via)
  {
    // Verify each address can be reached by _via.

    for(int i = 0;i < _parray->entries();i++)
    {
      Person *p = _parray->retrieve(i);

      if(!p || !p->find_address(_via))
      {
#if defined(DEBUG)
	dlog->log_lex(LEXLOGERR,
		       "Person '%s' cannot be reached via %s for call list %s",
		       IONULL(p->name()), _via, _name);
#endif

	lexerr++;
      }
    }
    
    CallList *c = NULL;

#if defined(DEBUG)
    if(_notify)
      dlog->log_lex(LEXLOG, " - Broadcasts to all");

    // Since calllist.cf is parsed before schedule.cf (which uses call lists
    // for alertplan definitions), it is not yet possible to verify the
    // name of this schedule.  That will be done after schedule.cf is
    // parsed.
      
    if(_sched)
      dlog->log_lex(LEXLOG, " - Rotates using %s schedule (unverified)",
		     _sched);

    dlog->log_lex(LEXLOG, " - Notifies via %s alert module", _via);
#endif

    if(!newcf->find_calllist(_name))
    {
      AlertModule *am = newcf->find_alertmodule(_via);

      if(am)
      {
	if(_sched)
	  c = new CallList(_name, _parray, _sched, am);
	else
	  c = new CallList(_name, _parray, _notify, am);
      
	if(c)
	{
	  if(!newcf->add_calllist(c))
	  {
	    wlog->warn("lex unable to add CallList");
	    lexerr++;
	    
	    delete c;  // This also gets _parray
	  }
	}
	else
	{
	  wlog->warn("lex unable to allocate new CallList");
	  lexerr++;
	  
	  delete _parray;
	}
      }
      else
      {
#if defined(DEBUG)
	dlog->log_lex(LEXLOGERR, "Requested alert module '%s' is not defined",
		      _via);
#endif
	delete _parray;
	lexerr++;
      }
    }
    else
    {
#if defined(DEBUG)
      dlog->log_lex(LEXLOGERR, "CallList '%s' redefined", _name);
#endif
      
      delete _parray;
      lexerr++;
    }

    // Clean up

    xdelete(_name);
    xdelete(_sched);

    _notify = false;
    _parray = NULL;

    xdelete(_via);
  }
  else
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR,
		   "CallList name, notification list, or via missing");
#endif

    lexerr++;
  }

  BEGINX(CALLLISTCF);
}

<CGIAUTHZLEVEL>"admin"|"clipboard"|"none"|"ro"|"rw" {
  // Take everything we have and add it to the cgi configuration

  if(_name && _via)
  {
    authz_type_t at = (strcmp(_via, "group")==0 ? group_authz_id :
		       user_authz_id);
    authz_level_t al = none_authz;

    switch(yytext[0])
    {
    case 'a':
      al = admin_authz;
      break;
    case 'c':
      al = clipboard_authz;
      break;
    case 'r':
      if(yytext[1] == 'w')
	al = read_write_authz;
      else
	al = read_only_authz;
      break;
    default:
      break;
    }
    
#if defined(DEBUG)
    dlog->log_lex(LEXLOG, "New authorization: %s %s = %s",
		  _via, _name, yytext);
#endif
    
    CGIAuthorization *cam = new CGIAuthorization(at, al, _name);

    if(cam)
    {
      if(!newccf->add_authorization(cam))
      {
	xdelete(cam);
	wlog->warn("lex unable to add Authorization");
	lexerr++;
      }
    }
    else
    {
      wlog->warn("lex failed to allocate CGIAuthModule");
      lexerr++;
    }
  }
  else
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Authorization definition incomplete");
#endif
    
    lexerr++;
  }

  xdelete(_name);
  xdelete(_via);
  
  BEGINX(CGIAUTHZTYPE);
}

<CGIAUTHZTYPE>"group"|"user" {
  // Make a note and proceed
  
  _via = xstrdup(yytext);
  BEGINX(CGIAUTHZWHO);
}

<CGIAUTHZTYPE>"}" {
  // Done parsing authorizations
  
  BEGINX(CGICF);
}

<CGIAUTHZWHO>[^ \n\t]+ {
  // Accept anything that is not a space

  _name = xstrdup(yytext);
  BEGINX(CGIAUTHZLEVEL);
}

<CGICF>"authmodule" {
  // Which auth module to use
  //  _name = module name
  //  _list2 = module options

  if(_name)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "AuthModule name already specified");
#endif

    lexerr++;
  }
  else
  {
    BEGINXN(NAME, 'a');
  }
}

<CGICF>"authorization"[ \t]*"{" {
  // Begin parsing authorizations
  //  _via holds identity type
  //  _name is regular expression of authorized identity

  if(_name || _via)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Authorization already in progress");
#endif

    lexerr++;
  }
  else
  {
    BEGINX(CGIAUTHZTYPE);
  }
}

<CGICF>"authsessiontime" {
  // Begin parsing session timeout
  
  BEGINX(NUMBER);
}

<CGICF>"clipemail" {
  // Begin parsing default clip email address
  
  BEGINXN(CGIVALUE, 'c');
}

<CGICF>"clipphone" {
  // Begin parsing default clip phone number
  
  BEGINXN(CGIVALUE, 'C');
}

<CGICF>"pageset" {
  // Begin parsing pageset to use
  
  BEGINXN(CGIVALUE, 'p');
}

<CGICF>"srcdir" {
  // Begin parsing pageset source directory
  
  BEGINXN(CGIVALUE, 's');
}

<CGICF>"statedir" {
  // Begin parsing state directory
  
  BEGINXN(CGIVALUE, 't');
}

<CGIVALUE>[^ \n\t][^\n]* {
  // Accept anything up to the newline as as the value, skipping past
  // the initial whitespace.

  // Just store what we read in the appropriate location

  switch(yynote)
  {
  case 'c':
#if defined(DEBUG)
    dlog->log_lex(LEXLOG, "Setting default clip email to %s", yytext);
#endif
    newccf->set_clip_email(yytext);
    break;
  case 'C':
#if defined(DEBUG)
    dlog->log_lex(LEXLOG, "Setting default clip phone to %s", yytext);
#endif
    newccf->set_clip_phone(yytext);
    break;
  case 'p':
#if defined(DEBUG)
    dlog->log_lex(LEXLOG, "Setting default pageset to %s", yytext);
#endif
    newccf->set_pageset(yytext);
    break;
  case 's':
#if defined(DEBUG)
    dlog->log_lex(LEXLOG, "Setting pageset directory to %s", yytext);
#endif
    newccf->set_pageset_directory(yytext);
    break;
  case 't':
#if defined(DEBUG)
    dlog->log_lex(LEXLOG, "Setting state directory to %s", yytext);
#endif
    newccf->set_state_directory(yytext);
    break;
  default:
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Programmer stupidity reached parsing cgivalue");
#endif
    lexerr++;
    break;
  }
  
  // Return to CGICF state

  BEGINX(CGICF);
}

<CHECKCF>"check" {
  // Begin parsing check stanza
  //  _name = name
  //  _via = module
  //  _ararray = mod args
  //  _arg = arg currently being parsed
  //  _freq = override timeout
  //  _all = whether to check all hosts
  //  _sarray = override schedule
  //  _ap = override alertplan
  //  _help = help file name
  //  _transport = override transport
  //  _notransport = override transport, use scheduler
  //  _ochecks = optional checks for composite
  //  _rchecks = required checks for composite
  //  _esc = whether result text significant
  
  if(_name)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Check definition already in progress");
#endif
    
    lexerr++;
  }
  else
  {
    BEGINXN(NAMEDEF, 'h');
  }
}

<CHECKCF>"default" {
  BEGINX(DEFAULT);
}

<CHECKCF>"fix" {
  // Begin parsing fix stanza
  //  _name = name
  //  _via = module
  //  _ararray = mod args
  //  _tout = override timeout (cannot use _freq since that will be used
  //                            to parse _lock)
  //  _transport = override transport
  //  _notransport = override transport, use scheduler
  //  _lock = lock expiry
  //  _lockt = lock type
  
  if(_name)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Fix definition already in progress");
#endif
    
    lexerr++;
  }
  else
  {
    BEGINXN(NAMEDEF, 'f');
  }
}

<CHECKCF>"transport" {
  // Begin parsing transport stanza
  //  _name = name
  //  _via = module
  //  _ararray = mod args

  if(_name)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Transport definition already in progress");
#endif

    lexerr++;
  }
  else
  {
    BEGINXN(NAMEDEF, 't');
  }
}

<CHECKLIST>"{" {
  // Just ignore this
}

<CHECKLIST>[a-zA-Z0-9\.+%@_-]+ {
  // An entry in the list.  The name must match an existing
  // non-composite check.

  Check *c = newcf->find_check(yytext);

  if(c)
  {
    if(c->module())
    {
      switch(yynote)
      {
      case 'o':
	if(_ochecks)
	{
	  if(!_rchecks || _rchecks->find(c)==-1)
	    _ochecks->add(c);
	  else
	  {
#if defined(DEBUG)
	    dlog->log_lex(LEXLOGERR,
			   "Component check '%s' is already required and so cannot be optional",
			   c->name());
#endif
	    lexerr++;
	  }
	}
	break;
      case 'r':
	if(_rchecks)
	{
	  if(!_ochecks || _ochecks->find(c)==-1)
	    _rchecks->add(c);
	  else
	  {
#if defined(DEBUG)
	    dlog->log_lex(LEXLOGERR,
			   "Component check '%s' is already optional and so cannot be required",
			   c->name());
#endif
	    lexerr++;
	  }
	}
	break;
      default:
#if defined(DEBUG)
	dlog->log_lex(LEXLOGERR, "Programmer error reached");
#endif
	lexerr++;
	break;
      }
    }
    else
    {
#if defined(DEBUG)
      dlog->log_lex(LEXLOGERR,
		     "Composite check '%s' cannot be included in composite check",
		     yytext);
#endif
    
      lexerr++;
    }
  }
  else
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Requested check '%s' does not exist", yytext);
#endif
    
    lexerr++;
  }
}

<CHECKLIST>"}" {
  // Done parsing
  
  BEGINX(CHECKSTANZA);
}

<CHECKSTANZA>"alert on" {
  // Override alert plan
  
  if(_ap)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "AlertPlan already specified");
#endif

    lexerr++;
  }
  else
  {
    BEGINXN(NAME, 'a');
  }
}

<CHECKSTANZA>"alertplan" {
  // alertplan is an optional keyword
}

<CHECKSTANZA>"check all hosts" {
  // Simply set the flag
  
  _all = true;
}

<CHECKSTANZA>"check on" {
  // Override check schedule

  if(_sarray)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Check schedule already specified");
#endif
    
    lexerr++;
  }
  else
  {
    BEGINXN(NAME, 'c');
  }
}

<CHECKSTANZA>"fix with" {
  // Fix module to use

  if(_fix)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Fix module already specified");
#endif
    
    lexerr++;
  }
  else
  {
    BEGINXN(NAME, 'f');
  }
}

<CHECKSTANZA>"helpfile" {
  // Optional help file name

  if(_help)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Help file already specified");
#endif
    
    lexerr++;
  }
  else
  {
    BEGINXN(NAME, 'h');
  }
}

<CHECKSTANZA>"module" {
  // Parse module name and args

  if(_via)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Module already specified");
#endif
    
    lexerr++;
  }
  else if(_ochecks || _rchecks)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR,
		   "'module' keyword not permitted in composite check definition");
#endif
    
    lexerr++;
  }
  else
  {
    BEGINXN(NAME, 'm');
  }
}

<CHECKSTANZA>"optional checks" {
  // Optional checks in the composite

  if(_via)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR,
		   "'optional checks' keyword not permitted in check definition");
#endif
    
    lexerr++;
  }
  else if(_ochecks)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Optional composite checks already specified");
#endif
    
    lexerr++;
  }
  else
  {
    _ochecks = new Array<Check>();

    if(_ochecks)
    {
      BEGINXN(CHECKLIST, 'o');
    }
    else
    {
      wlog->warn("lex unable to allocate new Check Array");
      lexerr++;
    }
  }
}

<CHECKSTANZA>"required checks" {
  // Required checks in the composite
  
  if(_via)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR,
		   "'required checks' keyword not permitted in check definition");
#endif
    
    lexerr++;
  }
  else if(_rchecks)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Required composite checks already specified");
#endif
    
    lexerr++;
  }
  else
  {
    _rchecks = new Array<Check>();

    if(_rchecks)
    {
      BEGINXN(CHECKLIST, 'r');
    }
    else
    {
      wlog->warn("lex unable to allocate new Check Array");
      lexerr++;
    }
  }
}

<CHECKSTANZA>"result text significant" {
  // simply set the flag

  _esc = true;
}

<CHECKSTANZA>"schedule" {
  // schedule is an optional keyword
}

<CHECKSTANZA>"timeout" {
  // Override timeout.  EVERY handles our error checking

  BEGINX(EVERY);
}

<CHECKSTANZA>"via" {
  // Find a transport module

  if(_notransport || _transport)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Transport already specified");
#endif
    
    lexerr++;
  }
  else if(_ochecks || _rchecks)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR,
		   "'via' keyword invalid in composite check definition");
#endif
    
    lexerr++;
  }
  else
  {
    BEGINXN(NAME, 't');
  }    
}

<CHECKSTANZA>"via scheduler" {
  // Use scheduler (no remote)

  if(_notransport || _transport)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Transport already specified");
#endif
    
    lexerr++;
  }
  else if(_ochecks || _rchecks)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR,
		   "'via' keyword invalid in composite check definition");
#endif
    
    lexerr++;
  }
  else
    _notransport = true;
}

<CHECKSTANZA>"}" {
  // check stanza definition complete, allocate an object and stuff it in cf

  if(_name && (_via || _rchecks || _ochecks))
  {
    // If no timeout was specified, use the default
    if(_freq == -1)
      _freq = _deftimeout;

    // If no transport was specified, use the default
    if(!_transport && !_notransport)
      _transport = _defchecktm;
    
#if defined(DEBUG)
    if(_via)
      dlog->log_lex(LEXLOG, "   Module: %s", _via);
    else
    {
      dlog->log_lex(LEXLOG, "   Composite check consisting of");

      if(_rchecks)
      {
	dlog->log_lex(LEXLOG, "    Required Checks:");

	for(int i = 0;i < _rchecks->entries();i++)
	{
	  Check *c = _rchecks->retrieve(i);

	  if(c && c->name())
	    dlog->log_lex(LEXLOG, "    - %s", IONULL(c->name())); 
	}
      }

      if(_ochecks)
      {
	dlog->log_lex(LEXLOG, "    Optional Checks:");

	for(int i = 0;i < _ochecks->entries();i++)
	{
	  Check *c = _ochecks->retrieve(i);

	  if(c && c->name())
	    dlog->log_lex(LEXLOG, "    - %s", IONULL(c->name())); 
	}
      }
    }

    if(_list2)
      for(int i = 0;i < _list2->entries();i++)
	dlog->log_lex(LEXLOG, "     %s", IONULL(_list2->retrieve(i)));

    dlog->log_lex(LEXLOG, "   Timeout: %d seconds", _freq);

    if(_sarray)
      dlog->log_lex(LEXLOG, "   Check schedule: %s", IONULL(_sarray->name()));
    
    if(_ap)
      dlog->log_lex(LEXLOG, "   Alertplan: %s", IONULL(_ap->name()));
    
    if(_via)
    {
      if(_transport)
	dlog->log_lex(LEXLOG, "   Via transport: %s",
		       IONULL(_transport->name()));
      else
	dlog->log_lex(LEXLOG, "   Via scheduler");
    }

    if(_help)
      dlog->log_lex(LEXLOG, "   Help file: %s", _help);
#endif

    if(!newcf->find_check(_name))
    {
      Check *c = NULL;

      // Create full path to helpfile if it isn't provided.
      // We can do this because we must know args->helpdir().

      char *help = NULL;

      if(_help)
      {
	if(_help[0] == '/')
	  help = xstrdup(_help);
	else
	{
	  help = xstrdup(args->helpdir());
	  help = xstrcat(help, "/");
	  help = xstrcat(help, _help);
	}
      }
      
      if(_via)
	c = new Check(_name, _via, _ararray, _freq, _all, _sarray, _ap,
		      help, _transport, _fix, _esc);
      else
	c = new Check(_name, _rchecks, _ochecks, _freq, _all, _sarray, _ap,
		      help, _fix, _esc);

      xdelete(help);
      
      if(c)
      {
	if(!newcf->add_check(c))
	{
	  wlog->warn("lex unable to add Check");
	  lexerr++;
	  
	  xdelete(c);
	  xadelete(_ararray, Argument);
	}
      }
      else
      {
	wlog->warn("lex unable to allocate new Check object");
	xadelete(_ararray, Argument);
	lexerr++;
      }
    }
    else
    {
#if defined(DEBUG)
      dlog->log_lex(LEXLOGERR, "Check module '%s' redefined", _name);
#endif

      xadelete(_ararray, Argument);
      lexerr++;
    }
  }
  else
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Check module not fully defined");
#endif
    
    lexerr++;
  }

  xdelete(_name);
  xdelete(_via);

  // Just reset _ararray since the Check will manage it
  _ararray = NULL;
  
  _freq = -1;
  _all = false;
  _sarray = NULL;
  _ap = NULL;
  xdelete(_help);
  _transport = NULL;
  _notransport = false;
  _fix = NULL;
  _ochecks = NULL;
  _rchecks = NULL;
  _esc = false;
  
  BEGINX(CHECKCF);
}

<CLASSSTANZA>"alert on" {
  // Class default alert plan
  
  if(_ap)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "AlertPlan already specified");
#endif

    lexerr++;
  }
  else
  {
    BEGINXN(NAME, 'a');
  }
}

<CLASSSTANZA>"alertplan" {
  // alertplan is an optional keyword
}

<CLASSSTANZA>"check on" {
  // Class default check schedule
  
  if(_sarray)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Check schedule already specified");
#endif

    lexerr++;
  }
  else
  {
    BEGINXN(NAME, 'c');
  }
}

<CLASSSTANZA>"hosts"/[{]? {
  // Parse class host members
  
  if(_list1)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Hosts already specified");
#endif

    lexerr++;
  }
  else
  {
    BEGINX(LIST);
  }
}

<CLASSSTANZA>"schedule" {
  // schedule is an optional keyword
}

<CLASSSTANZA>"}" {
  // class definition is complete
  
  if(_name)
  {
    // All that is really required is the class name

#if defined(DEBUG)
    if(_sarray)
      dlog->log_lex(LEXLOG, "   Default check schedule: %s",
		     IONULL(_sarray->name()));

    if(_ap)
      dlog->log_lex(LEXLOG, "   Default alertplan: %s", IONULL(_ap->name()));
#endif

    if(!newcf->find_hostclass(_name))
    {
      HostClass *hc = new HostClass(_name, _list1, _ap, _sarray);

      if(hc)
      {
	if(!newcf->add_class(hc))
	{
	  wlog->warn("lex unable to add HostClass");
	  lexerr++;
	  
	  xdelete(hc);
	}
      }
      else
      {
	wlog->warn("lex unable to allocate HostClass");
	lexerr++;
      }
    }
    else
    {
#if defined(DEBUG)
      dlog->log_lex(LEXLOGERR, "HostClass '%s' redefined", _name);
#endif
    
      lexerr++;
    }

    // Clean up the temp stuff regardless of whether or not the add
    // succeeded

    _list1 = NULL;  // Do not delete these
    _sarray = NULL;
    _ap = NULL;

    xdelete(_name);
  }
  
  BEGINX(HOSTCF);
}

<DEFAULT>"after" {
  // Begin adjustment of default number of check failures, to be incorporated
  // into subsequent alertplans.  This can be adjusted multiple times.
  //  _deffail = default number of check failures

  if(yylaststate == SCHEDCF)
  {
    BEGINXN(NUMBER, 'a');
  }
  else
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Unexpected keyword 'after'");
#endif
    
    lexerr++;
  }
}

<DEFAULT>"always clear state" {
  // Subsequent alert plans always clear state on return value status change

  if(yylaststate == SCHEDCF)
  {
    // Just reset the array (which holds ints, not pointers, so no xadelete).
    
#if defined(DEBUG)
    dlog->log_lex(LEXLOG,
		  "Setting default clear state on return value change to always");
#endif
    
    xdelete(_defnoclearon);
    BEGINX(SCHEDCF);
  }
  else
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Unexpected keyword 'always'");
#endif
    
    lexerr++;
  }
}

<DEFAULT>"check via" {
  if(yylaststate == CHECKCF)
  {
    BEGINXN(NAME, 'c');
  }
  else
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Unexpected keyword 'check'");
#endif
    
    lexerr++;
  }
}

<DEFAULT>"check via scheduler" {
  // Clear default transport

  if(yylaststate == CHECKCF)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOG, "Setting default check transport to scheduler");
#endif
  
    _defchecktm = NULL;
    BEGINX(CHECKCF);
  }
  else
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Unexpected keyword 'check'");
#endif
    
    lexerr++;
  }
}

<DEFAULT>"clear state honors all returngroups" {
  // Subsequent alert plans clear state honoring all returngroups

  if(yylaststate == SCHEDCF)
  {
    _defnoclearrg = new List(newcf->get_all_returngroups());

    if(_defnoclearrg)
    {
#if defined(DEBUG)
      dlog->log_lex(LEXLOG,
		    "Setting default clear state to honor returngroups");

      for(int i = 0;i < _defnoclearrg->entries();i++)
	dlog->log_lex(LEXLOG, " - %s", _defnoclearrg->retrieve(i));
#endif
    }

    BEGINX(SCHEDCF);
  }
  else
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Unexpected keyword 'clear state'");
#endif
    
    lexerr++;
  }  
}

<DEFAULT>"clear state honors returngroups" {
  // Subsequent alert plans clear state honoring specified returngroups

  if(yylaststate == SCHEDCF)
  {
    BEGINX(LIST);
  }
  else
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Unexpected keyword 'clear state'");
#endif
    
    lexerr++;
  }  
}

<DEFAULT>"clear state ignores returngroups" {
  // Subsequent alert plans clear state ignoring returngroups

  if(yylaststate == SCHEDCF)
  {
    // Just reset the List
    
#if defined(DEBUG)
    dlog->log_lex(LEXLOG,
		  "Setting default clear state to ignore returngroups");
#endif
    
    xdelete(_defnoclearrg);
    BEGINX(SCHEDCF);
  }
  else
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Unexpected keyword 'clear state'");
#endif
    
    lexerr++;
  }  
}

<DEFAULT>"do not clear state on return values"[ \t]*"{" {
  if(yylaststate == SCHEDCF)
  {
    // Toss any previous default value, ints not pointers, so no xadelete

    xdelete(_defnoclearon);

    _defnoclearon = new Array<int>();

    if(_defnoclearon)
    {    
      BEGINXN(NUMBER, 'c');
    }
    else
    {
      wlog->warn("lex failed to allocate int Array");
      lexerr++;
    }
  }
  else
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Unexpected keyword 'do'");
#endif
    
    lexerr++;
  }  
}

<DEFAULT>"do not notify on clear" {
  // Clear default noc

  if(yylaststate == SCHEDCF)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOG, "Unsetting default notify on clear");
#endif
    
    _defnoc = false;
    _defnocsched = NULL;
    BEGINX(SCHEDCF);
  }
  else
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Unexpected keyword 'do'");
#endif
    
    lexerr++;
  }  
}

<DEFAULT>"expire fix lock after" {
  // Set default lock timeout

  if(yylaststate == CHECKCF)
  {
    // Pass to EVERY to parse

    BEGINXN(EVERY, 'l');
  }
  else
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Unexpected keyword 'expire'");
#endif
    
    lexerr++;
  }
}

<DEFAULT>"fix via" {
  if(yylaststate == CHECKCF)
  {
    BEGINXN(NAME, 'f');
  }
  else
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Unexpected keyword 'fix'");
#endif
    
    lexerr++;
  }
}

<DEFAULT>"fix via scheduler" {
  // Clear default transport

  if(yylaststate == CHECKCF)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOG, "Setting default fix transport to scheduler");
#endif
  
    _deffixtm = NULL;
    BEGINX(CHECKCF);
  }
  else
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Unexpected keyword 'fix'");
#endif
    
    lexerr++;
  }
}

<DEFAULT>"notify" {
  // Begin default notify on clear.  Make sure the schedule is NULL, since
  // we might not set it.

  if(yylaststate == SCHEDCF)
  {
    _defnocsched = NULL;
    BEGINXN(NOTIFY, 'd');
  }
  else
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Unexpected keyword 'notify'");
#endif
    
    lexerr++;
  }
}

<DEFAULT>"timeout" {
  // Just pass to EVERY to parse

  if(yylaststate == CHECKCF)
  {
    BEGINXN(EVERY, 't');
  }
  else
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Unexpected keyword 'timeout'");
#endif
    
    lexerr++;
  }
}

<DEPENDENCYCF>"depend"/[{]? {
  // Begin dependency definition
  //  _check = Check module that is depended upon
  //  _rchecks = Check modules that depend on _check
  //  _fixes = Fix modules that depend on _check
  //  _all = All hosts depend on _check
  //  _list1 = Temporary buffer
  //  _list2 = If _all, exceptions; if !_all, hosts to be checked
  //  _name = Host depended upon, if not the host being checked
  //  _tout = Dependency type

  if(_check || _all || _list2 || _tout != -1)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Dependent check already named");
#endif
    
    lexerr++;
  }
  else
  {
    // Start off with Type II, unless status keyword is seen

    _tout = 2;
  }
}

<DEPENDENCYCF>"{" {
  // Begin depend stanza definition.
  
  BEGINX(DEPENDSTANZA);
}
  
<DEPENDENCYCF>"dependencies" {
  // This is really optional
}

<DEPENDENCYCF>"maximum" {
  // Maximum number of dependencies

  if(_freq != -1)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Number of dependencies already defined");
#endif
    
    lexerr++;
  }
  else
  {
    BEGINX(NUMBER);
  }
}

<DEPENDSTANZA>"all checks" {
  // Copy all Checks into the list

  if(!_rchecks)
  {
    _rchecks = new Array<Check>();

    if(_rchecks)
    {
      Array<Check> *cfca = newcf->get_all_checks();

      if(cfca)
      {
	for(int i = 0;i < cfca->entries();i++)
	  _rchecks->add(cfca->retrieve(i));
      }
    }
    else
    {
      wlog->warn("lex failed to allocate Check Array");
      lexerr++;
    }
  }
  else
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Depending checks already specified");
#endif
    
    lexerr++;
  }
}

<DEPENDSTANZA>"all checks except"/[{]? {
  // Copy all Checks except the exceptions into the list

  if(!_rchecks)
  {
    BEGINXN(LIST, 'C');
  }
  else
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Depending checks already specified");
#endif
    
    lexerr++;
  }
}

<DEPENDSTANZA>"all fixes" {
  // Copy all Fixes into the list

  if(!_fixes)
  {
    _fixes = new Array<Fix>();

    if(_fixes)
    {
      Array<Fix> *cffa = newcf->get_all_fixes();

      if(cffa)
      {
	for(int i = 0;i < cffa->entries();i++)
	  _fixes->add(cffa->retrieve(i));
      }
    }
    else
    {
      wlog->warn("lex failed to allocate Fix Array");
      lexerr++;
    }
  }
  else
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Depending fixes already specified");
#endif
    
    lexerr++;
  }
}

<DEPENDSTANZA>"all fixes except"/[{]? {
  // Copy all Fixes except the exceptions into the list

  if(!_fixes)
  {
    BEGINXN(LIST, 'F');
  }
  else
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Depending fixes already specified");
#endif
    
    lexerr++;
  }
}

<DEPENDSTANZA>"checks"/[{]? {
  // Begin parsing check modules that depend on this dependency.

  if(!_rchecks)
  {
    BEGINXN(LIST, 'c');
  }
  else
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Depending checks already specified");
#endif
    
    lexerr++;
  }
}

<DEPENDSTANZA>"fixes"/[{]? {
  // Begin parsing fix modules that depend on this dependency.

  if(!_fixes)
  {
    BEGINXN(LIST, 'f');
  }
  else
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Depending fixes already specified");
#endif
    
    lexerr++;
  }
}

<DEPENDSTANZA>"for all hosts" {
  // Flag that all hosts depend on the dependency

  if(!_all)
  {
    _all = true;
  }
  else
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Depending hosts already specified");
#endif
    
    lexerr++;
  }
}

<DEPENDSTANZA>"for all hosts except"/[{]? {
  // Flag that all hosts except those specified depend on the dependency.
  // Hand off to list processor.

  if(!_all)
  {
    _all = true;
    BEGINXN(LIST, 'H');
  }
  else
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Depending hosts already specified");
#endif
    
    lexerr++;
  }
}

<DEPENDSTANZA>"for all hosts except groups"/[{]? {
  // Flag that all hosts except those specified depend on the dependency.
  // Hand off to list processor.

  if(!_all)
  {
    _all = true;
    BEGINXN(LIST, 'G');
  }
  else
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Depending hosts already specified");
#endif
    
    lexerr++;
  }
}

<DEPENDSTANZA>"for groups"/[{]? {
  // Hand off group list to list processor.

  if(!_list2)
  {
    BEGINXN(LIST, 'g');
  }
  else
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Depending hosts already specified");
#endif
    
    lexerr++;
  }
}

<DEPENDSTANZA>"for hosts"/[{]? {
  // Hand off host list to list processor.

  if(!_list2)
  {
    BEGINXN(LIST, 'h');
  }
  else
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Depending hosts already specified");
#endif
    
    lexerr++;
  }
}

<DEPENDSTANZA>"on" {
  // Parse service or service@host

  if(_check)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Dependent Check already specified");
#endif
    
    lexerr++;
  }
  else
  {
    BEGINX(NAME);
  }
}

<DEPENDSTANZA>"status" {
  // Type I if status keyword is present.  _check must already be
  // specified.

  if(_check)
  {
    // If _name is specified, it must be a member of the group for
    // _check.

    if(_name)
    {
      List *l = newcf->find_group(_check->name());

      if(!l || l->find(_name)==-1)
      {
#if defined(DEBUG)
	dlog->log_lex(LEXLOGERR,
		      "Host '%s' specified in Type I Dependency must be member of group '%s'",
		      _name, _check->name());
#endif  

	lexerr++;
      }
    }

    // Type I Dependencies are only useful with Checks, not Fixes.

    if(_fixes)
    {
#if defined(DEBUG)
      dlog->log_lex(LEXLOGERR,
		    "Fixes cannot be dependent on Type I Dependencies");
#endif
	
      lexerr++;
    }
    
    _tout = 1;
  }
  else
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR,
		  "Cannot specify 'status' before dependent Check");
#endif  

    lexerr++;
  }
}

<DEPENDSTANZA>"}" {
  // Store the dependency information in newcf now.

  if(_check && (_rchecks || _fixes) && (_all || _list2)
     && (_tout == 1 || _name))
  {
#if defined(DEBUG)
    if(_rchecks)
    {
      dlog->log_lex(LEXLOG, "Checks:");

      for(int i = 0;i < _rchecks->entries();i++)
      {
	Check *c = _rchecks->retrieve(i);

	if(c)
	  dlog->log_lex(LEXLOG, "- %s", c->name());
      }
    }
    
    if(_fixes)
    {
      dlog->log_lex(LEXLOG, "Fixes:");

      for(int i = 0;i < _fixes->entries();i++)
      {
	Fix *f = _fixes->retrieve(i);

	if(f)
	  dlog->log_lex(LEXLOG, "- %s", f->name());
      }
    }

    if(_all)
      dlog->log_lex(LEXLOG, "For all hosts%s", (_list2 ? ", except" : ""));
    else
      dlog->log_lex(LEXLOG, "For hosts");

    if(_list2)
    {
      for(int i = 0;i < _list2->entries();i++)
	dlog->log_lex(LEXLOG, "- %s", _list2->retrieve(i));
    }

    dlog->log_lex(LEXLOG, "Depend on %s %s%s%s",
		  (_tout == 1 ? "no outstanding error for" :
		   "successful completion of"),
		  _check->name(), (_name ? "@" : ""), (_name ? _name : ""));
#endif

    Dependency *newd = new Dependency(_check, _tout, _name,
				      (_all ? NULL : _list2),
				      (_all ? _list2 : NULL));

    if(newd)
    {
      // Store the Dependency in the Configuration object to make it
      // easier to clean up, then attach copies to the relevant Check
      // and Fix objects.

      if(newcf->add_dependency(newd))
      {
	if(_rchecks)
	{
	  for(int i = 0;i < _rchecks->entries();i++)
	  {
	    Check *c = _rchecks->retrieve(i);

	    if(c)
	    {
	      if(_tout == 1)
	      {
		// Type I Dependencies cannot be mutually dependent.

		Array<Dependency> *deps = c->dependencies();

		if(deps)
		{
		  for(int j = 0;j < deps->entries();j++)
		  {
		    Dependency *xd = deps->retrieve(j);

		    if(xd && xd->dependency() == _check)
		    {
#if defined(DEBUG)
		      dlog->log_lex(LEXLOGERR,
				    "Mutual Type I Dependency between '%s' and '%s' not permitted",
				    _check->name(), c->name());
#endif
		      lexerr++;
		      break;
		    }
		  }
		}
	      }

	      c->add_dependency(newd);
	    }
	  }
	}

	if(_fixes)
	{
	  for(int i = 0;i < _fixes->entries();i++)
	  {
	    Fix *f = _fixes->retrieve(i);

	    if(f)
	      f->add_dependency(newd);
	  }
	}
      }
      else
      {
	wlog->warn("lex failed to add Dependency to Configuration");
	lexerr++;
      }
    }
    else
    {
      wlog->warn("lex failed to allocate Dependency");
      lexerr++;
    }
  }
  else
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Dependency definition incomplete");

    if(!_rchecks && !_fixes)
      dlog->log_lex(LEXLOGERR, "At least one Check or Fix is required");

    if(!_all && !_list2)
      dlog->log_lex(LEXLOGERR, "At least one depending host required");
    
    if(_tout == 2 && !_name)
      dlog->log_lex(LEXLOGERR,
		    "Type II Dependency requires dependent hostname");
#endif
    
    lexerr++;
  }

  // Clean up before returning
  
  _check = NULL;
  xdelete(_rchecks);
  xdelete(_fixes);
  _all = false;
  xdelete(_list2);
  xdelete(_name);
  _tout = -1;

  BEGINX(DEPENDENCYCF);
}
  
<EVERY>[0-9]+ {
  // This is a frequency.  0 is meaningless for SCHEDSTANZA, so we trip it
  // as an error.

  if(((yylaststate == SCHEDSTANZA) && (_freq > 0))
     || _freq >= 0)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Frequency or time interval already defined");
#endif
    
    lexerr++;
  }
  else
  {
    _freq = atoi(yytext);
	
    if(_freq < 1)
    {
#if defined(DEBUG)
      dlog->log_lex(LEXLOGERR,
		     "Frequency or time interval must be at least 1");
#endif
      
      lexerr++;
    }
  }
}

<EVERY>"second"|"seconds"|"minute"|"minutes"|"hour"|"hours"|"day"|"days"|"week"|"weeks" {
  // For SCHEDSTANZA, a frequency of 0 here means no frequency was specified,
  // so we assume a frequency of 1.  Otherwise, it is an error.

  if(yylaststate == SCHEDSTANZA)
  {
    if(_freq < 1)
      _freq = 1;

    // Create a new Schedule object and add it to the array.  Similar
    // functionality is performed in SCHEDSTANZA "never".
    
    if(_sarray)
    {
#if defined(DEBUG)
      dlog->log_lex(LEXLOG, " - from %s@%s until %s@%s every %d %s",
		     IONULL(_fromtime), IONULL(_fromday), IONULL(_untiltime),
		     IONULL(_untilday), _freq, yytext);
#endif
    
      Schedule *s = new Schedule(_fromtime, _fromday, _untiltime, _untilday,
				 _freq, yytext);
      
      if(s)
      {
	if(!_sarray->add(s))
	  delete s;
      }
      else
      {
	wlog->warn("lex unable to allocate new Schedule object");
	lexerr++;
      }
    }
#if defined(DEBUG)
    else
      dlog->log_lex(LEXLOGERR, "Schedule array not defined");
#endif
    
    // Clean up

    xdelete(_fromtime);
    xdelete(_fromday);
    xdelete(_untiltime);
    xdelete(_untilday);

    _freq = -1;

    BEGINX(SCHEDSTANZA);
  }
  else
  {
    if(_freq < 0)
    {
#if defined(DEBUG)
      dlog->log_lex(LEXLOGERR, "Interval defined without numeric value");
#endif
      
      lexerr++;
    }
    else
    {
      // Compute time(out) in seconds
      
      int t = 0;

      switch(tolower(yytext[0]))
      {
      case 's':
	t = _freq;
	break;
      case 'm':
	t = _freq * 60;
	break;
      case 'h':
	t = _freq * 3600;
	break;
      case 'd':
	t = _freq * 86400;
	break;
      case 'w':
	t = _freq * 604800;
	break;
      default:
#if defined(DEBUG)
	dlog->log_lex(LEXLOGERR, "programmer error reached");
#endif
	lexerr++;
	break;
      }

      switch(yylaststate)
      {
      case DEFAULT:
	// No, it is not at all confusing to have a DEFAULT case as
	// opposed to default
	switch(yynote)
	{
	case 'l':
#if defined(DEBUG)
	  dlog->log_lex(LEXLOG,
			 "Setting default lock expiration to %d seconds", t);
#endif
	  _deflock = t;
	  break;
	case 't':
#if defined(DEBUG)
	  dlog->log_lex(LEXLOG,
			 "Setting default check module timeout to %d seconds",
			 t);
#endif
	  _deftimeout = t;
	  break;
	default:
#if defined(DEBUG)
	  dlog->log_lex(LEXLOGERR, "Unknown state parsing time");
#endif
	  lexerr++;
	  break;
	}
	_freq = -1;
	BEGINX(CHECKCF);
	break;
      case FIXSTANZA:
	// Since EVERY parses into _freq, we cannot simply use it
	switch(yynote)
	{
	case 'e':
	  // expiry for fix lock
	  _lock = _freq;
	  break;
	case 't':
	  // timeout for fix module
	  _tout = _freq;
	  break;
	default:
#if defined(DEBUG)
	  dlog->log_lex(LEXLOGERR, "Unknown state parsing time");
#endif
	  lexerr++;
	  break;
	}
	_freq = -1;
	BEGINX(FIXSTANZA);
	break;
      case SCHEDCF:
#if defined(DEBUG)
	dlog->log_lex(LEXLOG, "Setting alert shift to %d seconds", t);
#endif
	
	if(!newcf->set_alert_shift(t))
	{
#if defined(DEBUG)
	  dlog->log_lex(LEXLOGERR, "Alert shift already specified");
#endif
	  
	  lexerr++;
	}	
		
	_freq = -1;
	
	BEGINX(SCHEDCF);
	break;
      default:
	// Leave _freq as is
	BEGINX(CHECKSTANZA);
	break;
      }
    }
  }
}

<FILENAME>[a-zA-Z0-9\/\.+%@_-]+ {
  // Used for filenames in instance definitions
  
  switch(yylaststate)
  {
  case INSTANCESTANZA:
    // We only do this if the _name matches the name of the Instance
    if(newi && strcmp(_name, newi->name())==0)
    {
      switch(yynote)
      {
      case 'c':
	newi->set_config(yytext);
	break;
      case 'h':
	newi->set_history(yytext);
	break;
      case 'H':
	newi->set_help(yytext);
	break;
      case 's':
	newi->set_state(yytext);
	break;
      case 't':
	newi->set_tmp(yytext);
	break;
      default:
#if defined(DEBUG)
	dlog->log_lex(LEXLOGERR, "Unknown note parsing filename");
#endif
	lexerr++;
	break;
      }
    }
    // else fail silently
    BEGINX(INSTANCESTANZA);
    break;
  default:
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Unknown state parsing filename");
#endif
    lexerr++;
    break;
  }
}

<FIXSTANZA>"expire lock after" {
  // Override lock expiry.  EVERY handles our error checking.

  BEGINXN(EVERY, 'e');
}

<FIXSTANZA>"module" {
  // Parse module name and args

  if(_via)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Module already specified");
#endif
    
    lexerr++;
  }
  else
  {
    BEGINXN(NAME, 'm');
  }
}

<FIXSTANZA>"require host locking" {
  // Flag host level locking
  
  if(_lockt != standard_lock)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Locking level already requested");
#endif
    
    lexerr++;
  }
  else
    _lockt = host_lock;
}

<FIXSTANZA>"require service locking" {
  // Flag service level locking

  if(_lockt != standard_lock)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Locking level already requested");
#endif
    
    lexerr++;
  }
  else
    _lockt = service_lock;
}

<FIXSTANZA>"timeout" {
  // Override lock expiry.  EVERY handles our error checking.

  BEGINXN(EVERY, 't');
}

<FIXSTANZA>"via" {
  // Find a transport module

  if(_notransport || _transport)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Transport already specified");
#endif
    
    lexerr++;
  }
  else
  {
    BEGINXN(NAME, 't');
  }    
}

<FIXSTANZA>"via scheduler" {
  // Use scheduler (no remote)

  if(_notransport || _transport)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Transport already specified");
#endif
    
    lexerr++;
  }
  else
    _notransport = true;
}

<FIXSTANZA>"}" {
  // fix stanza definition complete, allocate an object and stuff it in cf

  if(_name && _via)
  {
    // If no timeout was specified, use the default
    if(_tout == -1)
      _tout = _deftimeout;

    // If no expiry was specified, use the default
    if(_lock == -1)
      _lock = _deflock;
    
    // If no transport was specified, use the default
    if(!_transport && !_notransport)
      _transport = _deffixtm;
    
#if defined(DEBUG)
    dlog->log_lex(LEXLOG, "   Module: %s", _via);

    if(_ararray)
      for(int i = 0;i < _ararray->entries();i++)
      {
	Argument *a = _ararray->retrieve(i);
	
	if(a)
	  dlog->log_lex(LEXLOG, "     %s=%s",
			IONULL(a->name()), IONULL(a->value()));
      }

    dlog->log_lex(LEXLOG, "   Timeout: %d seconds", _tout);
    dlog->log_lex(LEXLOG, "   Lock expiry: %d seconds", _lock);

    if(_transport)
      dlog->log_lex(LEXLOG, "   Via transport: %s",
		     IONULL(_transport->name()));
    else
      dlog->log_lex(LEXLOG, "   Via scheduler");

    if(_lockt == host_lock)
      dlog->log_lex(LEXLOG, "   Requires host level locking");
    else if(_lockt == service_lock)
      dlog->log_lex(LEXLOG, "   Requires service level locking");
#endif

    if(!newcf->find_fix(_name))
    {
      Fix *f = new Fix(_name, _via, _ararray, _transport, _tout, _lock,
		       _lockt);
      
      if(f)
      {
	if(!newcf->add_fix(f))
	{
	  wlog->warn("lex unable to add Fix");
	  lexerr++;
	  
	  xdelete(f);
	  xadelete(_ararray, Argument);
	}
      }
      else
      {
	wlog->warn("lex unable to allocate new Fix object");
	lexerr++;
      }
    }
    else
    {
#if defined(DEBUG)
      dlog->log_lex(LEXLOGERR, "Fix module '%s' redefined", _name);
#endif
      
      lexerr++;
    }
  }
  else
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Fix module not fully defined");
#endif
    
    lexerr++;
  }
  
  xdelete(_name);
  xdelete(_via);

  // Just reset _ararray since the Fix will manage it
  _ararray = NULL;

  _lock = -1;
  _tout = -1;
  _lockt = standard_lock;
  _transport = NULL;
  _notransport = false;
  
  BEGINX(CHECKCF);
}

<GATEWAYCF>"debug level" {
  // Enable debugging.  This will be done immediately, so no need to
  // muck with data structures.

  BEGINX(NUMBER);
}

<GATEWAYCF>"relay all domains" {
  // Accept any domain for relay

  if(newgcf->set_relay_all_domains())
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOG, "Setting relay domains to all");
#endif
  }
  else
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Failed to set 'relay all domains'");
#endif
    
    lexerr++;
  }
}
			   
<GATEWAYCF>"relay domain"/[{]? {
  // Parse accepted relay domains

  if(_list1)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "List already allocated");
#endif

    lexerr++;
  }
  else
  {
    BEGINX(LISTX);
  }
}
  
<GATEWAYCF>"relay localhost" {
  // Accept local host for relay

  if(newgcf->set_relay_localhost())
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOG, "Setting relay domains to local host");
#endif
  }
  else
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Failed to set 'relay localhost'");
#endif
    
    lexerr++;
  }
}

<GATEWAYCF>"relay user" {
  // Begin parsing relay target
  //  _name = name
  //  _inst = target instance
  //  _group = target CallList
  //  _alert = target Person
  //  _via = module when _person also set
  
  if(_name)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Relay definition already in progress");
#endif
    
    lexerr++;
  }
  else
  {
    BEGINXN(NAMEDEF, 'R');
  }
}

<GROUP>[a-zA-Z0-9\.+%@_-]+/[{]? {
  // Parse group name
  
#if defined(DEBUG)
  dlog->log_lex(LEXLOG, "New group definition: %s", yytext);
#endif

  _group = xstrdup(yytext);

  if(!_group)
  {
    wlog->warn("lex unable to strdup yytext");
    lexerr++;
  }
  
  BEGINX(LIST);
}

<GROUP>"{" {
  // Begin group definition
  
#if defined(DEBUG)
  dlog->log_lex(LEXLOGERR, "Group name not specified");
#endif
  
  lexerr++;
}

<GROUPALIAS>"to" {
  // Read the name of the alias and then allocate the new group

  BEGINX(NAME);
}

<HOSTCF>"alias" {
  // Begin group alias definition
  //  _name = new name for group (not used)
  //  _list1 = group being aliased

  if(_list1)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Group alias definition already in progress");
#endif

    lexerr++;
  }
  else
  {
    BEGINX(NAME);
  }
}

<HOSTCF>"class" {
  // Begin class definition:
  //  _name = classname
  //  _schedule = default check schedule
  //  _ap = default alertplan

  if(_name || _sarray || _ap)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "class definition in progress");
#endif

    lexerr++;
  }
  else
  {
    BEGINXN(NAMEDEF, 'c');
  }
}

<HOSTCF>"group" {
  // Begin group definition:
  //  _group = groupname

  if(_group)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "group definition in progress");
#endif

    lexerr++;
  }
  else
  {
    BEGINX(GROUP);
  }
}

<INSTANCECF>"instance" {
  // Begin instance definition
  //  _name = name of call list, for comparison with newi

  if(_name)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Instance name already read");
#endif

    lexerr++;
  }
  else
  {
    BEGINXN(NAMEDEF, 'i');
  }
}

<INSTANCESTANZA>"allow"[ ]+"archive" {
  // Set the flag in the new instance and continue in this state.

  if(newi)
    newi->set_cli_allow_archive();
}

<INSTANCESTANZA>"allow"[ ]+"root" {
  // Set the flag in the new instance and continue in this state.

  if(newi)
    newi->set_cli_allow_root();
}

<INSTANCESTANZA>"configdir" {
  BEGINXN(FILENAME, 'c');
}

<INSTANCESTANZA>"helpdir" {
  BEGINXN(FILENAME, 'H');
}

<INSTANCESTANZA>"historydir" {
  BEGINXN(FILENAME, 'h');
}

<INSTANCESTANZA>"require"[ ]+"comment" {
  // Set the flag in the new instance and continue in this state.

  if(newi)
    newi->set_cli_require_comment();
}

<INSTANCESTANZA>"statedir" {
  BEGINXN(FILENAME, 's');
}

<INSTANCESTANZA>"tmpdir" {
  BEGINXN(FILENAME, 't');
}

<INSTANCESTANZA>"}" {
  // Clean up

  xdelete(_name);

  BEGINX(INSTANCECF);
}

<LIST>"{" {
  // Begin list definition, for groups it must be named
  
  if(_list1)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "_list1 already allocated");
#endif
    
    lexerr++;
  }
  else
  {
    if(yylaststate == GROUP)
      _list1 = new List(_group);
    else
      _list1 = new List();

    if(!_list1)
    {
      wlog->warn("lex unable to allocate new List");
      lexerr++;
    }
  }
}

<LIST>[a-zA-Z0-9\.+%@_-]+ {
  // An entry in the list
  
  if(_list1)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOG, " - %s", yytext);
#endif

    HostClass *hc = NULL;

    switch(yylaststate)
    {
    case APSTANZA:
    case DEFAULT:
      // Returngroups must be defined, complain if not

      if(!newcf->find_returngroup(yytext))
      {
#if defined(DEBUG)
	dlog->log_lex(LEXLOGERR, "No return group '%s' defined", yytext);
#endif

	lexerr++;
      }
      else
	_list1->add(yytext);
      break;
    case CLASSSTANZA:
      // Hosts can only be defined in one class

      hc = newcf->find_class(yytext);
      
      if(hc)
      {
#if defined(DEBUG)
	dlog->log_lex(LEXLOGERR, "%s is already a member of class '%s'",
		       yytext, IONULL(hc->name()));
#endif

	lexerr++;
      }
      else
	_list1->add(yytext);
      break;
    case GROUP:
      // Members names should be defined a class, warn if not

      hc = newcf->find_class(yytext);

      if(hc)
	_list1->add(yytext);
#if defined(DEBUG)
      else
	dlog->log_lex(LEXLOGMORE,
		       "WARNING: %s is not defined as a member of any class, skipping",
		       yytext);
#endif
      break;
    default:
      _list1->add(yytext);
      break;
    }
  }
  else
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "List not allocated");
#endif
    lexerr++;
  }
}

<LIST>"}" {
  // List complete
  
  switch(yylaststate)
  {
  case APSTANZA:
#if defined(DEBUG)
    dlog->log_lex(LEXLOG,
		  " - Will not clear state when old and new return codes are both in return groups");
    
    for(int i = 0;i < _list1->entries();i++)
      dlog->log_lex(LEXLOG, " -- %s", _list1->retrieve(i));
#endif
    BEGINX(APSTANZA);
    break;
  case CALLLISTSTANZA:
    BEGINX(CALLLISTSTANZA);
    break;
  case DEFAULT:
    // Store _list1 as the default value
    _defnoclearrg = _list1;
    _list1 = NULL;
#if defined(DEBUG)
    dlog->log_lex(LEXLOG,
		  "Setting default clear state to honor returngroups");
    
    for(int i = 0;i < _defnoclearrg->entries();i++)
      dlog->log_lex(LEXLOG, " - %s", _defnoclearrg->retrieve(i));
#endif
    BEGINX(SCHEDCF);
    break;
  case DEPENDSTANZA:
    // Translate _list1 into _rchecks or _fixes, then toss _list1
    if(_list1)
    {
      switch(yynote)
      {
      case 'c':
	// Specific Check list
	_rchecks = new Array<Check>();

	if(!_rchecks)
	{
	  wlog->warn("lex failed to allocate Check Array");
	  lexerr++;
	}
	else
	{
	  for(int i = 0;i < _list1->entries();i++)
	  {
	    char *xc = _list1->retrieve(i);
	    Check *xch = newcf->find_check(xc);

	    if(xch)
	    {
	      if(!_rchecks->add(xch))
	      {
		wlog->warn("lex failed to add Check to Array");
		lexerr++;
	      }
	    }
	    else
	    {
#if defined(DEBUG)
	      dlog->log_lex(LEXLOGERR,
			    "Requested check module '%s' is not defined", xc);
#endif
	      
	      lexerr++;
	    }
	  }
	}
	break;
      case 'C':
	// Check exception list
	_rchecks = new Array<Check>();

	if(_rchecks)
	{
	  Array<Check> *cfca = newcf->get_all_checks();

	  if(cfca)
	  {
	    for(int i = 0;i < cfca->entries();i++)
	    {
	      Check *xc = cfca->retrieve(i);

	      // We add a pointer if we do not find the name in the
	      // exception list
	      
	      if(xc && _list1->find(xc->name())==-1)
		_rchecks->add(xc);
	    }
	  }
	}
	else
	{
	  wlog->warn("lex failed to allocate Check Array");
	  lexerr++;
	}
	break;
      case 'f':
	// Specific Fix list
	_fixes = new Array<Fix>();

	if(!_fixes)
	{
	  wlog->warn("lex failed to allocate Fix Array");
	  lexerr++;
	}
	else
	{
	  for(int i = 0;i < _list1->entries();i++)
	  {
	    char *xf = _list1->retrieve(i);
	    Fix *xfx = newcf->find_fix(xf);

	    if(xfx)
	    {
	      if(!_fixes->add(xfx))
	      {
		wlog->warn("lex failed to add Fix to Array");
		lexerr++;
	      }
	    }
	    else
	    {
#if defined(DEBUG)
	      dlog->log_lex(LEXLOGERR,
			    "Requested fix module '%s' is not defined", xf);
#endif
	      
	      lexerr++;
	    }
	  }
	}
	break;
      case 'F':
	// Fix exception list
	_fixes = new Array<Fix>();

	if(_fixes)
	{
	  Array<Fix> *cffa = newcf->get_all_fixes();

	  if(cffa)
	  {
	    for(int i = 0;i < cffa->entries();i++)
	    {
	      Fix *xf = cffa->retrieve(i);

	      // We add a pointer if we do not find the name in the
	      // exception list
	      
	      if(xf && _list1->find(xf->name())==-1)
		_fixes->add(xf);
	    }
	  }
	}
	else
	{
	  wlog->warn("lex failed to allocate Fix Array");
	  lexerr++;
	}
	break;
      case 'g':
      case 'G':
	// List of groups, translate into hosts
	if(_list1)
	{
	  _list2 = new List();
	  
	  if(_list2)
	  {
	    for(int i = 0;i < _list1->entries();i++)
	    {
	      List *g = newcf->find_group(_list1->retrieve(i));

	      if(g)
	      {
		for(int j = 0;j < g->entries();j++)
		  _list2->add(g->retrieve(j));
	      }
	      else
	      {
#if defined(DEBUG)
		dlog->log_lex(LEXLOGERR, "Group '%s' not defined",
			      IONULL(_list1->retrieve(i)));
#endif
		lexerr++;
	      }
	    }
	  }
	  else
	  {
	    wlog->warn("lex failed to allocate List");
	    lexerr++;
	  }
	}
	break;
      case 'h':
      case 'H':
	// List of hosts: verify them and then just move the pointer
	if(_list1)
	{
	  for(int i = 0;i < _list1->entries();i++)
	  {
	    if(!newcf->find_groups(_list1->retrieve(i)))
	    {
#if defined(DEBUG)
	      dlog->log_lex(LEXLOGERR, "Host '%s' not defined",
			    IONULL(_list1->retrieve(i)));
#endif
	    
	      lexerr++;
	    }
	  }

	  _list2 = _list1;
	  _list1 = NULL;  // So xdelete will not delete the List
	}
	break;
      }
      
      xdelete(_list1);
    }
    BEGINX(DEPENDSTANZA);
    break;
  case GROUP:
    // _group was used to name _list1, so it is not needed
    xdelete(_group);
    if(_list1)
    {
      if(_list1->name())
      {
	if(!newcf->find_group(_list1->name()))
	{
	  if(!newcf->add_group(_list1))
	  {
	    wlog->warn("lex unable to add Group");
	    lexerr++;
	    
	    delete _list1;  // Clean up
	  }
	}
	else
	{
#if defined(DEBUG)
	  dlog->log_lex(LEXLOGERR, "Group '%s' redefined", _list1->name());
#endif

	  delete _list1;
	  lexerr++;
	}
      }
      else
      {
#if defined(DEBUG)
	dlog->log_lex(LEXLOGERR, "Group list has no name");
#endif

	delete _list1;
	lexerr++;
      }

      // Do not delete the list since either we added it successfully or we
      // already deleted it

      _list1 = NULL;
    }
    BEGINX(HOSTCF);
    break;
  default:
    BEGINX(CLASSSTANZA);
    break;
  }
}

<LISTX>"{" {
  // Begin list definition for lists with more promiscuous OK characters

  if(_list1)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "_list1 already allocated");
#endif
    
    lexerr++;
  }
  else
  {
    _list1 = new List();
  
    if(!_list1)
    {
      wlog->warn("lex unable to allocate new List");
      lexerr++;
    }
  }
}

<LISTX>[^ \n\t}][^\n]* {
  // Accept anything up to the newline as as the value, skipping past
  // the initial whitespace.
  
  if(_list1)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOG, " - %s", yytext);
#endif

    _list1->add(yytext);
  }
  else
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "List not allocated");
#endif
    lexerr++;
  }
}

<LISTX>"}" {
  // List complete
  
  switch(yylaststate)
  {
  case GATEWAYCF:
    // We were parsing "relay domain"
    if(!newgcf->set_relay_domains(_list1))
    {
#if defined(DEBUG)
      dlog->log_lex(LEXLOGERR, "'relay domain' already defined");
#endif
      
      delete _list1;
      lexerr++;
    }
#if defined(DEBUG)
    else
    {
      dlog->log_lex(LEXLOG, "Setting relay domains to:");

      for(int i = 0;i < _list1->entries();i++)
	dlog->log_lex(LEXLOG, " - %s", _list1->retrieve(i));
    }
#endif
    
    // Reset
    _list1 = NULL;
    
    BEGINX(GATEWAYCF);
    break;
  }
}
			    
<MODULE>"{"|"{}" {
  // Set up for argument parsing.  We need to explicitly look for {} to
  // work around lex parsing too much.
  
  if(_ararray || _arg)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Module argument already started");
#endif
   
    lexerr++;
  }
  else
  {
    if(strlen(yytext)==2)
      unput('}');
    
    _ararray = new Array<Argument>();

    if(!_ararray)
    {
      wlog->warn("lex unable to allocate new Argument Array");
      lexerr++;
    }
    
    BEGINXN(MODULEARG, yynote);
  }
}

<MODULEARG>"}" {
  switch(yynote)
  {
  case 'a':
    // We are storing an auth module for cgi.cf
    if(_name && _ararray)
    {
#if defined(DEBUG)
      dlog->log_lex(LEXLOG, "Setting auth module to %s (%d options)",
		    _name, _ararray->entries());
#endif

      CGIAuthModule *cam = new CGIAuthModule(_name, _ararray);

      if(cam)
      {
	if(!newccf->add_authmodule(cam))
	{
	  wlog->warn("lex unable to add AuthModule");
	  lexerr++;
	  
	  xdelete(cam);
	}
      }
      else
      {
	wlog->warn("lex failed to allocate CGIAuthModule");
	lexerr++;
	
	xadelete(_ararray, Argument);
      }
    }
    else
    {
#if defined(DEBUG)
      dlog->log_lex(LEXLOGERR, "AuthModule definition incompleted");
#endif
	
      xadelete(_ararray, Argument);
      lexerr++;
    }
    // Clean up for next turn
    xdelete(_name);
    _ararray = NULL;
    BEGINX(CGICF);
    break;
  case 'c':
    // Simply drop back to the appropriate stanza.  _ararray will be
    // stored there.
    BEGINX(CHECKSTANZA);
    break;
  case 'f':
    // Simply drop back to the appropriate stanza.  _ararray will be
    // stored there.
    BEGINX(FIXSTANZA);
    break;
  case 't':
    // Simply drop back to the appropriate stanza.  _ararray will be
    // stored there.
    BEGINX(TRANSPORTSTANZA);
    break;
  default:
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Unknown note parsing modargs");
#endif
    lexerr++;
    break;
  }
}

<MODULEARG>[a-zA-Z0-9\._-]+ {
  // This is the name of the argument

  if(_arg)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Argument name already specified");
#endif
   
    lexerr++;
  }
  else
  {
    _arg = xstrdup(yytext);
    BEGINXN(MODULEVALUE, yynote);
  }
}

<MODULEVALUE>[^ \n\t][^\n]* {
  // Accept anything up to the newline as as the value, skipping past
  // the initial whitespace.

  if(_arg && _ararray)
  {
    // We now have the name of the argument in _arg and the value in
    // yytext.  Allocate an Argument and store them.

    Argument *newarg = new Argument(_arg, yytext, false);

    if(newarg)
    {
      // We do not bother checking if name is already defined since
      // some modules accept lists of arguments.  We do not know
      // which modules, since that is determined at module run time,
      // so we leave it to the module to complain.

      if(!_ararray->add(newarg))
      {
	xdelete(newarg);
	lexerr++;
      }
    }
    else
    {
      wlog->warn("lex failed to allocate Argument");
      lexerr++;
    }
    
    // Delete _arg so we can parse more

    xdelete(_arg);
  }
  else
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Argument name undefined or programmer error");
#endif
    
    lexerr++;
  }

  // Return to module state
  
  BEGINXN(MODULEARG, yynote);
}

<NAME>[a-zA-Z0-9\.+%@_-]+/[{]?[}]? {
  // Begin name parsing, according to our current state

  switch(yylaststate)
  {
  case APALIAS:
    switch(yynote)
    {
    case 'r':
      // "replacing" = calllist to replace (_call)
      _call = newcf->find_calllist(yytext);

      if(!_call)
      {
#if defined(DEBUG)
	dlog->log_lex(LEXLOGERR, "Undefined call list '%s'", yytext);
#endif
	
	lexerr++;
      }
      BEGINX(APALIAS);
      break;
    case 't':
      // "to" = alias name (_name)
      _name = xstrdup(yytext);
      BEGINX(APALIAS);
      break;
    case 'a':
    case 'w':
      {
	CallList *c2 = newcf->find_calllist(yytext);

	if(!c2)
	{
#if defined(DEBUG)
	  dlog->log_lex(LEXLOGERR, "Undefined call list '%s'", yytext);
#endif
	  
	  lexerr++;
	}
	else
	{
	  // Create the new AlertPlan with the alias information

	  if(_name && _ap)
	  {
	    if(!newcf->find_alertplan(_name))
	    {
#if defined(DEBUG)
	      if(_call)
		dlog->log_lex(LEXLOG,
			      "AlertPlan %s aliased to %s replacing %s with %s",
			      IONULL(_ap->name()), IONULL(_name),
			      IONULL(_call->name()), IONULL(c2->name()));
	      else
		dlog->log_lex(LEXLOG,
			      "AlertPlan %s aliased to %s adding %s",
			      IONULL(_ap->name()), IONULL(_name),
			      IONULL(c2->name()));
#endif
	      
	      if(!newcf->add_alertplan_alias(_ap, _name, _call, c2))
	      {
		wlog->warn("lex unable to create AlertPlan alias");
		lexerr++;
	      }
	    }
	    else
	    {
#if defined(DEBUG)
	      dlog->log_lex(LEXLOGERR,
			     "AlertPlan '%s' already defined", _name);
#endif
	    
	      lexerr++;
	    }

	    // Clean up
	      
	    _ap = NULL;
	    _call = NULL;
	    c2 = NULL;

	    xdelete(_name);
	  }
	  else
	  {
#if defined(DEBUG)
	    dlog->log_lex(LEXLOGERR,
			   "AlertPlan alias missing name, original, or list");
#endif
	    
	    lexerr++;
	  }
	}

	BEGINX(SCHEDCF);
      }
      break;
    default:
#if defined(DEBUG)
      dlog->log_lex(LEXLOGERR, "Unknown state reached parsing NAME");
#endif
      lexerr++;
     }
    break;
  case APSTANZA:
    _rvs = newcf->find_returngroup(yytext);
    
    if(_rvs)
    {
      BEGINX(APSTANZA);
    }
    else
    {
#if defined(DEBUG)
      dlog->log_lex(LEXLOGERR, "Undefined returngroup '%s'", yytext);
#endif
      
      lexerr++;
    }
    break;
  case CALLLISTALERT:
    switch(yynote)
    {
    case 'f':
      _arg = xstrdup(yytext);
      break;
    case 't':
      _via = xstrdup(yytext);
      break;
    default:
#if defined(DEBUG)
      dlog->log_lex(LEXLOGERR, "Unknown state reached parsing NAME");
#endif
      lexerr++;
      break;
    }
    BEGINX(CALLLISTALERT);
    break;
  case CALLLISTALIAS:
    switch(yynote)
    {
    case 't':
      if(_call && _via)
      {
	if(!newcf->find_calllist(yytext))
	{
#if defined(DEBUG)
	  dlog->log_lex(LEXLOG, "CallList %s via %s aliased to %s",
			 IONULL(_call->name()), _via, yytext);
#endif

	  AlertModule *am = newcf->find_alertmodule(_via);

	  if(am)
	  {	  
	    CallList *c = new CallList(yytext, _call, am);
	  
	    if(c)
	    {
	      if(!newcf->add_calllist(c))
	      {
		wlog->warn("lex unable to add CallList");
		lexerr++;
		
		delete c;
	      }
	    }
	    else
	    {
	      wlog->warn("lex unable to allocate new CallList");
	      lexerr++;
	    }
	  }
	  else
	  {
#if defined(DEBUG)
	    dlog->log_lex(LEXLOGERR,
			  "Requested alert module '%s' is not defined",
			  _via);
#endif
	    
	    lexerr++;
	  }
	}
	else
	{
#if defined(DEBUG)
	  dlog->log_lex(LEXLOGERR, "CallList '%s' already defined", yytext);
#endif
	  
	  lexerr++;
	}
	
	// Clean up

	_call = NULL;

	xdelete(_via);
      }
      else
      {
#if defined(DEBUG)
	dlog->log_lex(LEXLOGERR, "CallList alias missing list or via");
#endif
	
	lexerr++;
      }
      
      BEGINX(CALLLISTCF);
      break;
    case 'v':
      // "via" = alert module (_via)
      _via = xstrdup(yytext);
      BEGINX(CALLLISTALIAS);
      break;
    default:
#if defined(DEBUG)
      dlog->log_lex(LEXLOGERR, "Unknown state reached parsing NAME");
#endif
      lexerr++;
    }
    break;
  case CALLLISTCF:
    // See if yytext exists as a calllist to alias to
    _call = newcf->find_calllist(yytext);

    if(!_call)
    {
#if defined(DEBUG)
      dlog->log_lex(LEXLOGERR, "Undefined call list '%s'", yytext);
#endif
      
      lexerr++;
    }
    BEGINX(CALLLISTALIAS);
    break;
  case CALLLISTSTANZA:
    switch(yynote)
    {
    case 'r':
      // "rotates using" = (as yet undefined) schedule (_sched)
      _sched = xstrdup(yytext);
      BEGINX(CALLLISTSTANZA);
      break;
    case 'v':
      // "via" = alert module (_via)
      _via = xstrdup(yytext);
      BEGINX(CALLLISTSTANZA);
      break;
    default:
#if defined(DEBUG)
      dlog->log_lex(LEXLOGERR, "Unknown state parsing NAME");
#endif
      lexerr++;
      break;
    }
    break;
  case CGICF:
    switch(yynote)
    {
    case 'a':
      // name of alert module
      _name = xstrdup(yytext);
      BEGINXN(MODULE, 'a');
      break;
    default:
#if defined(DEBUG)
      dlog->log_lex(LEXLOGERR, "Unknown state parsing NAME");
#endif
      lexerr++;
      break;
    }
    break;
  case CHECKSTANZA:
  case CLASSSTANZA:
    switch(yynote)
    {
    case 'a':
      // alertplan
      _ap = newcf->find_alertplan(yytext);

      if(!_ap)
      {
#if defined(DEBUG)
	dlog->log_lex(LEXLOGERR, "Undefined alertplan '%s'", yytext);
#endif
	
	lexerr++;
      }
      break;
    case 'c':
      // check schedule
      _sarray = newcf->find_schedule(yytext);

      if(!_sarray)
      {
#if defined(DEBUG)
	dlog->log_lex(LEXLOGERR, "Undefined schedule '%s'", yytext);
#endif
	
	lexerr++;
      }
      break;
    case 'f':
      // fix module
      if(yylaststate == CHECKSTANZA)
      {
	_fix = newcf->find_fix(yytext);

	if(!_fix)
	{
#if defined(DEBUG)
	  dlog->log_lex(LEXLOGERR, "Undefined fix module '%s'", yytext);
#endif	  
	  lexerr++;
	}
      }
      else
      {
#if defined(DEBUG)
	dlog->log_lex(LEXLOGERR, "Programmer error parsing NAME");
#endif
	lexerr++;
      }
      break;
    case 'h':
      // helpfile
      if(yylaststate == CHECKSTANZA)
      {
	_help = xstrdup(yytext);
	if(!_help)
	{
	  wlog->warn("lex failed to duplicate helpfile");
	  lexerr++;
	}
      }
      else
      {
#if defined(DEBUG)
	dlog->log_lex(LEXLOGERR, "Programmer error parsing NAME");
#endif
	lexerr++;
      }
      break;
    case 'm':
      // module name
      if(yylaststate == CHECKSTANZA)
      {
	_via = xstrdup(yytext);
	
	if(!_via)
	{
	  wlog->warn("lex failed to duplicate module name");
	  lexerr++;
	}
      }
      else
      {
#if defined(DEBUG)
	dlog->log_lex(LEXLOGERR, "Programmer error parsing NAME");
#endif
	lexerr++;
      }
      break;
    case 't':
      // transport module name
      _transport = newcf->find_transport(yytext);

      if(!_transport)
      {
#if defined(DEBUG)
	dlog->log_lex(LEXLOGERR, "Undefined transport '%s'", yytext);
#endif
	lexerr++;
      }
      break;
    default:
#if defined(DEBUG)
      dlog->log_lex(LEXLOGERR, "Unknown state parsing NAME");
#endif
      lexerr++;
      break;
    }
    if(yylaststate == CLASSSTANZA)
    {
      BEGINX(CLASSSTANZA);
    }
    else
    {
      if(yynote == 'm')
      {
	BEGINXN(MODULE, 'c');
      }
      else
      {
	BEGINX(CHECKSTANZA);
      }
    }
    break;
  case DEFAULT:
    switch(yynote)
    {
    case 'c':
      // Default check transport
      _defchecktm = newcf->find_transport(yytext);

      if(_defchecktm)
      {
#if defined(DEBUG)
	dlog->log_lex(LEXLOG, "Setting default check transport to '%s'",
		       yytext);
#endif
      }
      else
      {
#if defined(DEBUG)
	dlog->log_lex(LEXLOGERR, "Undefined transport '%s'", yytext);
#endif	

	lexerr++;
      }
      BEGINX(CHECKCF);
      break;
    case 'f':
      // Default fix transport
      _deffixtm = newcf->find_transport(yytext);

      if(_deffixtm)
      {
#if defined(DEBUG)
	dlog->log_lex(LEXLOG, "Setting default fix transport to '%s'",
		       yytext);
#endif
      }
      else
      {
#if defined(DEBUG)
	dlog->log_lex(LEXLOGERR, "Undefined transport '%s'", yytext);
#endif	

	lexerr++;
      }
      BEGINX(CHECKCF);
      break;
    default:
#if defined(DEBUG)
      dlog->log_lex(LEXLOGERR, "Unknown state parsing NAME");
#endif
      lexerr++;
      break;
    }
    break;
  case DEPENDSTANZA:
    // Parse service or service@host
    {
      char *c = xstrdup(yytext);

      if(c)
      {
	char *p = strchr(c, '@');
      
	if(p)
	{
	  // service@host, store host in _name

	  *p = '\0';
	  _name = xstrdup(p+1);
	}

	// We need to at least have a check defined

	_check = newcf->find_check(c);

	if(!_check)
	{
#if defined(DEBUG)
	  dlog->log_lex(LEXLOGERR, "Check '%s' is not defined", c);
#endif
	  lexerr++;
	}
      
	xdelete(c);
      }
    }
    BEGINX(DEPENDSTANZA);
    break;
  case FIXSTANZA:
    switch(yynote)
    {
    case 'm':
      // module name
      _via = xstrdup(yytext);

      if(!_via)
      {
	wlog->warn("lex failed to duplicate module name");
	lexerr++;
      }
      else
      {
	BEGINXN(MODULE, 'f');
      }
      break;
    case 't':
      // transport module name
      _transport = newcf->find_transport(yytext);

      if(!_transport)
      {
#if defined(DEBUG)
	dlog->log_lex(LEXLOGERR, "Undefined transport '%s'", yytext);
#endif
	lexerr++;
      }
      BEGINX(FIXSTANZA);
      break;
    default:
#if defined(DEBUG)
      dlog->log_lex(LEXLOGERR, "Unknown state parsing NAME");
#endif
      lexerr++;
      break;
    }
    break;
  case GROUPALIAS:
    if(_list1)
    {
      if(!newcf->find_group(yytext))
      {
#if defined(DEBUG)
	dlog->log_lex(LEXLOG, "Group %s aliased to %s",
		       IONULL(_list1->name()), yytext);
#endif
	
	List *g = new List(_list1, yytext);
	
	if(g)
	{
	  if(!newcf->add_group(g))
	  {
	    wlog->warn("lex failed to add Group");
	    lexerr++;
	    
	    delete g;
	  }
	}
	else
	{
	  wlog->warn("lex unable to allocate new List");
	  lexerr++;
	}
      }
      else
      {
#if defined(DEBUG)
	dlog->log_lex(LEXLOGERR, "Group '%s' already defined", yytext);
#endif
	
	lexerr++;
      }

      // Clean up

      _list1 = NULL;
    }
    else
    {
#if defined(DEBUG)
      dlog->log_lex(LEXLOGERR, "Group alias missing group");
#endif
      
      lexerr++;
    }
    
    BEGINX(HOSTCF);
    break;
  case HOSTCF:
    // See if yytext exists as a group to alias to
    _list1 = newcf->find_group(yytext);

    if(!_list1)
    {
#if defined(DEBUG)
      dlog->log_lex(LEXLOGERR, "Undefined group '%s'", yytext);
#endif
      
      lexerr++;
    }
    BEGINX(GROUPALIAS);
    break;
  case NOTIFY:
    // "using" schedule name (_nsched)
    _nsched = newcf->find_schedule(yytext);

    if(!_nsched)
    {
#if defined(DEBUG)
      dlog->log_lex(LEXLOGERR, "Undefined schedule '%s'", yytext);
#endif
      
      lexerr++;
    }
    BEGINXN(NOTIFY, yynote);
    break;
  case PERSONSTANZA:
    // Store either an address or a module
    switch(yynote)
    {
    case 'a':
      // Address
      _alert = xstrdup(yytext);
      break;
    case 'm':
      // Module
      if(_alert)
      {
	// Create a new contact and add it to the Contact Array

	AlertModule *am = newcf->find_alertmodule(yytext);

	if(am)
	{
	  Contact *c = new Contact(_alert, am);

	  if(c)
	  {
#if defined(DEBUG)
	    dlog->log_lex(LEXLOG, "   notifies %s via %s", _alert, yytext);
#endif
	    
	    if(!_person || !_person->add_contact(c))
	    {
	      wlog->warn("lex unable to insert Contact into Array");
	      lexerr++;
	      
	      delete c;
	    }
	  }
	  else
	  {
	    wlog->warn("lex unable to allocate new Contact");
	    lexerr++;
	  }
	}
	else
	{
#if defined(DEBUG)
	  dlog->log_lex(LEXLOGERR,
			"Requested alert module '%s' is not defined",
			yytext);
#endif
	  lexerr++;
	}

	// Clean up for next iteration
	xdelete(_alert);
      }
      break;
    default:
#if defined(DEBUG)
      dlog->log_lex(LEXLOGERR, "Unknown state parsing NAME");
#endif
      lexerr++;
      break;
    }
    BEGINX(PERSONSTANZA);
    break;
  case RELAYTARGETSTANZA:
    switch(yynote)
    {
    case 'c':
      _group = xstrdup(yytext);
      break;
    case 'i':
      _inst = xstrdup(yytext);
      break;
    case 'p':
      _alert = xstrdup(yytext);
      break;
    case 'v':
      _via = xstrdup(yytext);
      break;
    default:
#if defined(DEBUG)
      dlog->log_lex(LEXLOGERR, "Unknown note parsing NAME");
#endif
      lexerr++;
      break;
    }
    BEGINX(RELAYTARGETSTANZA);
    break;
  case RETURNSTANZA:
    // "using" = schedule name(_sarray)
    _sarray = newcf->find_schedule(yytext);

    if(!_sarray)
    {
#if defined(DEBUG)
      dlog->log_lex(LEXLOGERR, "Undefined schedule '%s'", yytext);
#endif
      
      lexerr++;
    }
#if defined(DEBUG)
    else
      dlog->log_lex(LEXLOG, " - Using '%s' schedule", yytext);
#endif
    BEGINX(RETURNSTANZA);
    break;
  case SCHEDCF:
    // See if yytext exists as an alertplan to alias to
    _ap = newcf->find_alertplan(yytext);

    if(!_ap)
    {
#if defined(DEBUG)
      dlog->log_lex(LEXLOGERR, "Undefined AlertPlan '%s'", yytext);
#endif
      
      lexerr++;
    }
    BEGINX(APALIAS);
    break;
  case TRANSPORTSTANZA:
    // module name
    {
      _via = xstrdup(yytext);

      if(!_via)
      {
	wlog->warn("lex failed to duplicate module name");
	lexerr++;
      }
      else
      {
	BEGINXN(MODULE, 't');
      }
    }
    break;
  case TRYSTANZA:
    switch(yynote)
    {
    case 'a':
    case 't':
      // yynote is a calllist to notify
      _call = newcf->find_calllist(yytext);

      if(!_call)
      {
#if defined(DEBUG)
	dlog->log_lex(LEXLOGERR, "Undefined call list '%s'", yytext);
#endif

	lexerr++;
      }
      else
      {
	if(!_calls->add(_call))
	{
	  wlog->warn("lex failed to add CallList to Array");	  
	  lexerr++;
	}

	_call = NULL;
      }
      break;
    case 'd':
      // find the schedule for degraded mode
      _sarray2 = newcf->find_schedule(yytext);

      if(!_sarray2)
      {
#if defined(DEBUG)
	dlog->log_lex(LEXLOGERR, "Undefined schedule '%s'", yytext);
#endif
	
	lexerr++;
      }
      break;
    default:
#if defined(DEBUG)
      dlog->log_lex(LEXLOGERR, "Unknown state parsing NAME");
#endif
      lexerr++;
      break;
    }
    BEGINX(TRYSTANZA);
    break;
  default:
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Unknown state parsing NAME");
#endif
    lexerr++;
    break;
  }
}

<NAMEDEF>[a-zA-Z0-9\.+%@_-]+/[{]? {
  // Generic state for parsing the name of some definition.  Group definitions
  // do not use this because they have a different naming format.

#if defined(DEBUG)
  char *x = "*unknown*";

  // While we are here, make sure we have do not already have
  // something of the same name.
  
  switch(yynote)
  {
  case 'a':
    x = "alertplan";
    break;
  case 'A':
    x = "alert stanza";
    break;
  case 'c':
    x = "class";
    break;
  case 'f':
    x = "fix";
    break;
  case 'h':
    x = "check";
    break;
  case 'i':
    x = "instance";
    break;
  case 'l':
    x = "call list";
    break;
  case 'p':
    x = "person";
    break;
  case 'r':
    x = "returngroup";
    break;
  case 'R':
    x = "relay target";
    break;
  case 's':
    x = "schedule";
    break;
  case 't':
    x = "transport";
    break;
  }

  dlog->log_lex(LEXLOG, "New %s definition: %s", x, yytext);
#endif
  
  if(_name)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR,
		   "Unexpected token '%s' (name already defined)", yytext);
#endif
    
    lexerr++;
  }
  else
  {
    _name = xstrdup(yytext);

    if(!_name)
    {
      wlog->warn("lex unable to strdup yytext");
      lexerr++;
    }

    if(yynote == 't' && strcmp(_name, "scheduler")==0)
    {
      // transport modules cannot be named scheduler

#if defined(DEBUG)
      dlog->log_lex(LEXLOGERR,
		     "transport stanza cannot be named reserved name 'scheduler'");
#endif
      
      lexerr++;
    }

    if(yynote == 'i' && newil)
    {
      // We are assembling a list of instances, add it here

      newil->add(yytext);
    }
  }
}

<NAMEDEF>"{" {
  // Begin stanza definition.  
  
  if(!_name)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Definition name not yet specified");
#endif
    
    lexerr++;
  }
  else
  {
    switch(yynote)
    {
    case 'a':
      if(_aarray)
      {
#if defined(DEBUG)
	dlog->log_lex(LEXLOGERR, "AlertPlan definition already started");
#endif
	lexerr++;
      }
      else
      {
	_aarray = new Array<AlertReturnGroup>();

	if(_aarray)
	{
	  BEGINX(APSTANZA);
	}
	else
	{
	  wlog->warn("lex unable to allocate AlertReturnGroup Array");
	  lexerr++;
	}
      }
      break;
    case 'A':
      BEGINX(CALLLISTALERT);
      break;
    case 'c':
      BEGINX(CLASSSTANZA);
      break;
    case 'f':
      BEGINX(FIXSTANZA);
      break;
    case 'h':
      BEGINX(CHECKSTANZA);
      break;
    case 'i':
      BEGINX(INSTANCESTANZA);
      break;
    case 'l':
      BEGINX(CALLLISTSTANZA);
      break;
    case 'p':
      if(_person)
      {
#if defined(DEBUG)
	dlog->log_lex(LEXLOGERR, "Person definition already started");
#endif
	
	lexerr++;
      }
      else
      {
	_person = new Person(_name);

	if(_person)
	{
	  BEGINX(PERSONSTANZA);
	}
	else
	{
	  wlog->warn("lex unable to allocate Person");
	  lexerr++;
	}
      }
      break;
    case 'r':
      if(_name)
      {
	// Important distinction here.  The alertplan only deletes Array<int>
	// objects that are not named, since they were allocated by config.l.
	// Named arrays are established here as pointers to objects stored
	// in the Configuration.
	
	_rvs = new Array<int>(_name);
	
	if(_rvs)
	{
	  BEGINX(NUMBER);
	}
	else
	{
	  wlog->warn("lex unable to allocate new int Array");
	  lexerr++;
	}
      }
      else
      {
#if defined(DEBUG)
	dlog->log_lex(LEXLOGERR, "No name specified for return group");
#endif
	
	lexerr++;
      }
      break;
    case 'R':
      BEGINX(RELAYTARGETSTANZA);
      break;
    case 's':
      if(_sarray)
      {
#if defined(DEBUG)
	dlog->log_lex(LEXLOGERR, "Array name already specified");
#endif
	lexerr++;
      }
      else
      {
	_sarray = new Array<Schedule>(_name);

	if(_sarray)
	{
	  BEGINX(SCHEDSTANZA);
	}
	else
	{
	  wlog->warn("lex unable to allocate Schedule Array");
	  lexerr++;
	}
      }
      break;
    case 't':
      BEGINX(TRANSPORTSTANZA);
      break;
    default:
#if defined(DEBUG)
      dlog->log_lex(LEXLOGERR, "Unknown state reached");
#endif
      lexerr++;
      break;
    }
  }
}

<NOTIFIES>"{" {
  // Begin list of persons

  if(_parray)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Call List notification list already defined");
#endif
    
    lexerr++;
  }
  else
  {
    _parray = new Array<Person>();

    if(!_parray)
    {
      wlog->warn("lex unable to allocate Person Array");      
      lexerr++;
    }
  }
}

<NOTIFIES>[a-zA-Z0-9\.+%@_-]+ {
  // Each person must already be defined before they can be added.
  // Furthermore, they must not have already been added.

  if(_parray)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOG, " - %s", yytext);
#endif

    Person *p = newcf->find_person(yytext);

    if(p)
    {
      if(_parray->find(p)==-1)
      {
	if(!_parray->add(p))
	{
	  lexerr++;
	}
      }
      else
      {
#if defined(DEBUG)
	dlog->log_lex(LEXLOGERR, "Person '%s' already a member of list '%s'",
                      yytext, _name);
#endif

	lexerr++;
      }
    }
    else
    {
#if defined(DEBUG)
      dlog->log_lex(LEXLOGERR,
		     "No person '%s' defined for use in notification list",
		     yytext);
#endif

      lexerr++;
    }
  }
  else
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Call List notification list not yet started");
#endif

    lexerr++;
  }
}

<NOTIFIES>"}" {
  // Done reading persons
  
  BEGINX(CALLLISTSTANZA);
}

<NOTIFY>"on clear" {
  // Set global notify on clear.  _nsched may or may not be NULL.

  // We have to use yynote and not yylaststate since we may not get
  // here directly from the last state
  
  switch(yynote)
  {
  case 'a':
    // APSTANZA, just flag noc as true, validation will be handled later
    if(_notify)
    {
#if defined(DEBUG)
      dlog->log_lex(LEXLOGERR, "Notify on clear already specified");
#endif
    
      lexerr++;
    }
    else
    {
#if defined(DEBUG)
      dlog->log_lex(LEXLOG, " - Notifying on clear%s%s%s",
		     _nsched ? " using " : "",
		     _nsched ? IONULL(_nsched->name()) : "",
		     _nsched ? " schedule" : "");
#endif
      
      _notify = true;
    }
    BEGINX(APSTANZA);
    break;
  case 'd':
    // DEFAULT (from SCHEDCF)
#if defined(DEBUG)
    dlog->log_lex(LEXLOG, "Setting default notify on clear%s%s%s",
		   _nsched ? " using " : "",
		   _nsched ? IONULL(_nsched->name()) : "",
		   _nsched ? " schedule" : "");
#endif

    _defnoc = true;
    _defnocsched = _nsched;
    _nsched = NULL;
    BEGINX(SCHEDCF);
    break;
  default:
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Programmer error in NOTIFY");
#endif    
    lexerr++;
    break;
  }
}

<NOTIFY>"schedule" {
  // This is effectively optional
}

<NOTIFY>"using" {
  BEGINXN(NAME, yynote);
}

<NUMBER>[0-9]+ {
  switch(yylaststate)
  {
  case APSTANZA:
  case NAMEDEF:
    // Do not accept return values less than 1 (since 0 means "OK").
    // If we come from NAMEDEF, it is for a returngroup definition.
    // If we come from APSTANZA and yynote is c, we are populating
    // a different Array.  NAMEDEF does not set yynote, so we simplify
    // the logic by not checking for APSTANZA with 'c'.
    if((yynote == 'c' && _ncrvs) || (yynote != 'c' &&_rvs))
    {
      int x = atoi(yytext);
      
      if(x == 0)
      {
#if defined(DEBUG)
	dlog->log_lex(LEXLOGERR, "Return value of 0 not permitted");
#endif
	
	lexerr++;
      }
      else
      {
	if(yynote == 'c')
	  _ncrvs->add((int *)x);
	else
	  _rvs->add((int *)x);
      }
    }
    else
    {
#if defined(DEBUG)
      dlog->log_lex(LEXLOGERR, "_rvs or _rcrvs is unexpectedly NULL");
#endif

      lexerr++;
    }
    // AlertPlans can have more than one return value, so we stay in
    // NUMBER until we see a brace
    break;
  case CGICF:
#if defined(DEBUG)
    dlog->log_lex(LEXLOG, "Setting session length to %d", atoi(yytext));
#endif
    newccf->set_session_length(atoi(yytext));
    BEGINX(CGICF);
    break;
  case DEFAULT:
    switch(yynote)
    {
    case 'a':
      // Set the default check failure value
#if defined(DEBUG)
      dlog->log_lex(LEXLOG, "Setting default check failure value to %d",
		     atoi(yytext));
#endif
      _deffail = atoi(yytext);
      BEGINX(SCHEDCF);
      break;
    case 'c':
      // Do not accept return values less than 1 (since 0 means "OK").
      if(_defnoclearon)
      {
	int x = atoi(yytext);
      
	if(x == 0)
	{
#if defined(DEBUG)
	  dlog->log_lex(LEXLOGERR, "Return value of 0 not permitted");
#endif
	
	  lexerr++;
	}
	else
	  _defnoclearon->add((int *)x);
      }
      else
      {
#if defined(DEBUG)
	dlog->log_lex(LEXLOGERR, "_defnoclearon is unexpectedly NULL");
#endif
	
	lexerr++;
      }
      // We can have more than one return value, so we stay in
      // NUMBER until we see a brace
      break;
    default:
#if defined(DEBUG)
      dlog->log_lex(LEXLOGERR, "Unknown state reached parsing NUMBER");
#endif
      lexerr++;
       break;
    }
    break;
  case DEPENDENCYCF:
    _freq = atoi(yytext);
    if(_freq == -1)
    {
#if defined(DEBUG)
      dlog->log_lex(LEXLOGERR, "Number of dependencies not specified");
#endif
    
      lexerr++;
    }
    else
    {
#if defined(DEBUG)
      dlog->log_lex(LEXLOG, "Setting maximum dependencies to %d", _freq);
#endif
      
      if(!newcf->set_max_dependencies(_freq))
      {
#if defined(DEBUG)
	dlog->log_lex(LEXLOGERR, "Maximum dependencies already set");
#endif
	
	lexerr++;
      }
      
      // Reset value
      
      _freq = -1;
    }
    BEGINX(DEPENDENCYCF);
    break;
  case GATEWAYCF:
#if defined(DEBUG)
    dlog->set_level(atoi(yytext));
#endif
    BEGINX(GATEWAYCF);
    break;
  case RETURNSTANZA:
    _fail = atoi(yytext);
    BEGINX(RETURNSTANZA);
    break;
  case SCHEDCF:
    switch(yynote)
    {
    case 't':
      // Set the alert throttle
#if defined(DEBUG)
      dlog->log_lex(LEXLOG, "Setting alert throttle to %d", atoi(yytext));
#endif
      
      if(!newcf->set_alert_throttle(atoi(yytext)))
      {
#if defined(DEBUG)
	dlog->log_lex(LEXLOGERR, "Alert throttle already set");
#endif
	
	lexerr++;
      }
      break;
    default:
#if defined(DEBUG)
      dlog->log_lex(LEXLOGERR, "Unknown state reached parsing NUMBER");
#endif
      lexerr++;
       break;
    }
    BEGINX(SCHEDCF);
    break;
  case TRYSET:
    _freq = atoi(yytext);
    BEGINX(TRYSET);
    break;
  case TRYSTANZA:
    _degrade = atoi(yytext);
    BEGINX(TRYSTANZA);
    break;
  default:
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Unknown state parsing NUMBER");
#endif
    lexerr++;
    break;
  }
}

<NUMBER>"{" {
  if(yylaststate == APSTANZA)
  {
    // This terminates the list of return values in an alert plan.
    // This stanza is essentially identical to one in APSTANZA.

    if(_asarray)
    {
#if defined(DEBUG)
      dlog->log_lex(LEXLOGERR, "AlertSchedule Array already exists");
#endif
      
      lexerr++;
    }
    else
    {
      _asarray = new Array<AlertSchedule>();

      if(_asarray)
      {
	BEGINX(RETURNSTANZA);
      }
      else
      {
	wlog->warn("lex Unable to allocate AlertSchedule Array");
	lexerr++;
      }
    }
  }
  else
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Unexpected state reached");
#endif

    lexerr++;
  }
}

<NUMBER>"}" {
  switch(yylaststate)
  {
  case APSTANZA:
    // This terminates the list of return values to not clear state for.
    // There is not really anything to do here.

    if(_ncrvs)
    {
#if defined(DEBUG)
      dlog->log_lex(LEXLOG,
		    " - Will not clear state on change to or from return code");
      for(int i = 0;i < _ncrvs->entries();i++)
	dlog->log_lex(LEXLOG, " -- %d", (int)_ncrvs->retrieve(i));
#endif
      
      BEGINX(APSTANZA);
    }
    else
    {
#if defined(DEBUG)
      dlog->log_lex(LEXLOGERR, "Return value list missing");
#endif

      lexerr++;
    }
    break;
  case DEFAULT:
    // The only way we currently get here is from SCHEDCF
    if(_defnoclearon)
    {
#if defined(DEBUG)
      dlog->log_lex(LEXLOG,
		    "Setting default clear state on return value change to values");
      
      for(int i = 0;i < _defnoclearon->entries();i++)
	dlog->log_lex(LEXLOG, " - %d", (int)_defnoclearon->retrieve(i));
#endif
      BEGINX(SCHEDCF);
    }
    else
    {
#if defined(DEBUG)
      dlog->log_lex(LEXLOGERR, "Return value list missing");
#endif

      lexerr++;
    }
    break;
  case NAMEDEF:
    // This terminates the list of return values in a return group.
    if(_rvs)
    {
      // _rvs is already named

      if(!newcf->find_returngroup(_rvs->name()))
      {
	if(!newcf->add_named_returngroup(_rvs))
	{
	  wlog->warn("lex failed to add ReturnGroup");
	  lexerr++;
	  
	  delete _rvs;
	}
      }
      else
      {
#if defined(DEBUG)
	dlog->log_lex(LEXLOGERR, "ReturnGroup '%s' redefined", _rvs->name());
#endif

	delete _rvs;
	lexerr++;
      }	
    }
    else
    {
#if defined(DEBUG)
      dlog->log_lex(LEXLOGERR, "returngroup missing return values");
#endif
      
      lexerr++;
    }

    // Clean up

    xdelete(_name);
    _rvs = NULL;

    BEGINX(SCHEDCF);
    break;
  default:
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Unknown state reached");
#endif
    lexerr++;
    break;
  }
}

<PERSONSTANZA>"notify" {
  // Read a contact
  //  _alert = address of contact

  if(_alert)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Contact definition already in progress");
#endif
    
    lexerr++;
  }
  else
  {
    BEGINXN(NAME, 'a');
  }  
}

<PERSONSTANZA>"otherwise" {
  // No module to use, if in progress just create a new Contact with the
  // address

  if(!_all)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOG, "   notifies %s otherwise", IONULL(_alert));
#endif

    Contact *c = new Contact(_alert);

    if(c)
    {
      if(!_person || !_person->add_contact(c))
      {
	wlog->warn("lex unable to insert Contact into Person");
	lexerr++;
	
	delete c;
      }

      _all = true;
    }
    else
    {
      wlog->warn("lex unable to allocate new Person");
      lexerr++;
    }
  }
  else
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Default contact address already specified");
#endif
    
    lexerr++;
  }

  // Clean up for next iteration
  xdelete(_alert);
}

<PERSONSTANZA>"via" {
  // Read module to use
  
  if(_alert)
  {
    BEGINXN(NAME, 'm');
  }
  else
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Contact definition not yet in progress");
#endif
    
    lexerr++;
  }
}

<PERSONSTANZA>"}" {
  // We are done, add this Person to the Configuration.

  if(_name && _person)
  {
    if(!newcf->find_person(_name))
    {
      if(!newcf->add_person(_person))
      {
	delete _person;  // Clean up
	
	wlog->warn("lex failed to insert Person into configuration");
	lexerr++;
      }
    }
    else
    {
#if defined(DEBUG)
      dlog->log_lex(LEXLOGERR, "Person '%s' redefined", _name);
#endif
    
      delete _person;
      lexerr++;
    }
  }
  else
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Incomplete Person definition");
#endif
    
    lexerr++;
  }
  
  // Clean up for next go

  xdelete(_name);
  _person = NULL;
  _all = false;

  BEGINX(CALLLISTCF);
}

<RELAYTARGETSTANZA>"calllist" {
  // Cannot set if person is set

  if(_group)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Relay calllist already set");
#endif
    
    lexerr++;
  }
  else if(_alert)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Cannot set calllist since person already set");
#endif
    
    lexerr++;
  }
  else
  {
    BEGINXN(NAME, 'c');
  }
}
			      
<RELAYTARGETSTANZA>"instance" {
  // Set instance

  if(_inst)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Relay instance already set");
#endif
    
    lexerr++;
  }
  else
  {
    BEGINXN(NAME, 'i');
  }
}
			      
<RELAYTARGETSTANZA>"person" {
  // Cannot set if calllist is set

  if(_alert)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Relay person already set");
#endif
    
    lexerr++;
  }
  else if(_group)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Cannot set person since calllist already set");
#endif
    
    lexerr++;
  }
  else
  {
    BEGINXN(NAME, 'p');
  }
}
			      
<RELAYTARGETSTANZA>"via" {
  // Via requires person to be set

  if(_via)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Relay via already set");
#endif
    
    lexerr++;
  }
  else if(!_alert)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "person must be defined before via");
#endif
    
    lexerr++;
  }
  else
  {
    BEGINXN(NAME, 'v');
  }
}
			      
<RELAYTARGETSTANZA>"}" {
  // Allocate a new RelayTarget and store it

  if(_name && _inst && (_group || (_alert && _via)))
  {
    RelayTarget *rt = NULL;
    
    if(_group)
    {
#if defined(DEBUG)
      dlog->log_lex(LEXLOG,
		    "New relay target '%s' for calllist '%s' in instance '%s'",
		    _name, _group, _inst);
#endif

      rt = new RelayTarget(_name, _inst, _group);
    }
    else
    {
#if defined(DEBUG)
      dlog->log_lex(LEXLOG,
		    "New relay target '%s' for person '%s' via '%s' in instance '%s'",
		    _name, _alert, _via, _inst);
#endif

      rt = new RelayTarget(_name, _inst, _alert, _via);
    }

    if(rt)
    {
      if(!newgcf->add_relay_target(rt))
      {
	wlog->warn("lex failed to add RelayTarget to GatewayConfiguration");
	xdelete(rt);
	lexerr++;
      }
    }
    else
    {
      wlog->warn("lex failed to allocate RelayTarget");
      lexerr++;
    }
  }
  else
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "relay user stanza not fully specified");
#endif
    
    lexerr++;
  }

  // Clean up

  xdelete(_name);
  xdelete(_inst);
  xdelete(_group);
  xdelete(_alert);
  xdelete(_via);
     
  BEGINX(GATEWAYCF);
}
			      
<RETURNSTANZA>"after" {
  // Read a number

  if(_fail > 0)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Check failures already specified");
#endif
    
    lexerr++;
  }
  else
  {
    BEGINX(NUMBER);
  }
}

<RETURNSTANZA>"check failure"|"check failures"|"schedule" {
  // These are essentially optional
}

<RETURNSTANZA>"using" {
  // This specifies which schedule to use

  if(_sarray)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Schedule already specified");
#endif
    
    lexerr++;
  }
  else
  {
    BEGINX(NAME);
  }
}

<RETURNSTANZA>"{" {
  // Start reading try stanzas
  //  _atarray = Array of AlertTrys

  if(_atarray)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "AlertTry definitions already in progress");
#endif
    
    lexerr++;
  }
  else
  {
    _atarray = new Array<AlertTry>();

    if(_atarray)
    {
      BEGINX(TRYSET);
    }
    else
    {
      wlog->warn("lex unable to allocate AlertTry Array");
      lexerr++;
    }
  }
}

<RETURNSTANZA>"}" {
  // Done with this return stanza, create an AlertReturnGroup and at it to
  // _aarray.

  if(_asarray && _aarray)
  {
    // _fail of 0 indicates to use the default
    
    if(_fail == 0)
      _fail = _deffail;
    
#if defined(DEBUG)
    string s;

    if(_rvs)
    {
      s = "of return value";
      if(_rvs->entries() == 1)
	s += " ";
      else
	s += "s ";

      for(int i = 0;i < _rvs->entries();i++)
      {
	char buf[BUFSIZE];

	sprintf(buf, "%d", (int)_rvs->retrieve(i));

	s += buf;

	if(i + 1 < _rvs->entries())
	  s += ",";
      }
    }
    else
      s = "of any return value";
    
    dlog->log_lex(LEXLOG,
		  "Previous %d schedule block%s after %d check failure%s %s",
		  _asarray->entries(),
		  (_asarray->entries() == 1 ? " applies" : "s apply"),
		  _fail,
		  (_fail == 1 ? "" : "s"),
		  s.c_str());
#endif

    AlertReturnGroup *ag = new AlertReturnGroup(_rvs, _fail, _asarray);

    if(ag)
    {
      if(_aarray->add(ag))
      {
	_asarray = NULL;
	_rvs = NULL;
      }
      else
      {
	delete ag;
	lexerr++;
      }
    }
    else
    {
      wlog->warn("lex unable to allocate new AlertReturnGroup object");
      lexerr++;
    }
  }
  else
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "AlertReturnGroup not fully defined");
#endif

    lexerr++;
  }

  // Clean up.  _asarray and _rvs are defined if we were not successful.

  xadelete(_asarray, AlertSchedule);
  xdelete(_rvs);
  _fail = 0;
  
  BEGINX(APSTANZA);
}

<SCHEDCF>"alertplan" {
  // Begin alertplan definition
  //  _name = alertplan name
  //  _aarray = alertreturngroup array
  //  _rvs = returngroup set
  //  _notify = notify on clear
  //  _nonoc = do not notify on clear (override default)
  //  _ncrvs = do not clear state (override default)
  //  _noncrvs = always clear state (override default)
  //  _list1 = return group clear state equivs (override default)
  //  _nocsrg = no return group clear state equivs (override default)

  if(_name)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "period definition in progress");
#endif

    lexerr++;
  }
  else
  {
    BEGINXN(NAMEDEF, 'a');
  }
}

<SCHEDCF>"alert shift" {
  // Read a time/interval
  //  _freq = interval

  BEGINX(EVERY);
}

<SCHEDCF>"alias" {
  // Begin alertplan alias definition
  //  _name = alias
  //  _ap = original alertplan
  //  _call = replace this calllist
  //  (replacement name is used as soon as read)

  if(_name)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR,
		   "AlertPlan alias definition already in progress");
#endif

    lexerr++;
  }
  else
  {
    BEGINX(NAME);
  }
}

<SCHEDCF>"check failure"|"check failures" {
  // These are currently effectively optional
}

<SCHEDCF>"default" {
  BEGINX(DEFAULT);
}

<SCHEDCF>"returngroup" {
  // Begin return group definition
  //  _name = rg name

  if(_name)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "returngroup definition in progress");
#endif
    
    lexerr++;
  }
  else
  {
    BEGINXN(NAMEDEF, 'r');
  }
}

<SCHEDCF>"schedule" {
  // Begin schedule definition
  //  _name = schedule name

  if(_name)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "period definition in progress");
#endif

    lexerr++;
  }
  else
  {
    BEGINXN(NAMEDEF, 's');
  }
}

<SCHEDCF>"throttle" {
  // Read a number, set it

  BEGINXN(NUMBER, 't');
}

<SCHEDSTANZA>"at"/[{]? {
  // We begin parsing a list of items of the form <day> <HH:MM> or just
  // <HH:MM>.

  BEGINXN(TIME, 'a');
}

<SCHEDSTANZA>"every" {
  // parse an interval label and maybe a number, and stuff the schedule
  // into the array, including _from/untilday/time if set

  BEGINX(EVERY);
}

<SCHEDSTANZA>"from" {
  // parse a time into _fromday and _fromtime

  BEGINXN(TIME, 'f');
}

<SCHEDSTANZA>"never" {
  // This is equivalent to "every 0 intervals".  This functionality is
  // similar to what is performed in EVERY "[intervals]".

  if(_freq == -1)
  {
    if(_sarray)
    {
#if defined(DEBUG)
      dlog->log_lex(LEXLOG, " - from %s@%s until %s@%s never",
		     IONULL(_fromtime), IONULL(_fromday), IONULL(_untiltime),
		     IONULL(_untilday));
#endif

      Schedule *s = new Schedule(_fromtime, _fromday, _untiltime, _untilday,
				 0, "seconds");

      if(s)
      {
	if(!_sarray->add(s))
	  delete s;
      }
      else
      {
	wlog->warn("lex unable to allocate new Schedule object");
	lexerr++;
      }
    }
#if defined(DEBUG)
    else
      dlog->log_lex(LEXLOGERR, "Schedule array not defined");
#endif

    // Clean up

    xdelete(_fromtime);
    xdelete(_fromday);
    xdelete(_untiltime);
    xdelete(_untilday);

    _freq = -1;
  }
  else
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Frequency already specified");
#endif
    
    lexerr++;
  }
}

<SCHEDSTANZA>"until" {
  // parse a time into _untilday and _untiltime

  BEGINXN(TIME, 'u');
}

<SCHEDSTANZA>"}" {
  // Schedule array is now complete, add it to the Configuration.

  if(_sarray)
  {
    if(_sarray->name())
    {
      if(!newcf->find_schedule(_sarray->name()))
      {
	if(!newcf->add_named_schedule(_sarray))
	{
	  wlog->warn("lex failed to add Schedule");
	  lexerr++;
	  
	  delete _sarray;  // Clean up
	}
	
	// Do not delete _sarray since we either added it successfully or we
	// already deleted it
      }
      else
      {
#if defined(DEBUG)
	dlog->log_lex(LEXLOGERR, "Schedule '%s' redefined", _sarray->name());
#endif

 	delete _sarray;
	lexerr++;
      }
    }
    else
    {
#if defined(DEBUG)
      dlog->log_lex(LEXLOGERR, "Schedule Array has no name");
#endif

      lexerr++;
    }

    _sarray = NULL;
  }

  // Clean up _name

  xdelete(_name);

  BEGINX(SCHEDCF);
}

<TIME>"sunday"|"monday"|"tuesday"|"wednesday"|"thursday"|"friday"|"saturday"|"Sunday"|"Monday"|"Tuesday"|"Wednesday"|"Thursday"|"Friday"|"Saturday" {
  // _day = weekday

  if(_day)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Day already specified");
#endif

    lexerr++;
  }
  else
  {
    _day = xstrdup(yytext);

    if(!_day)
    {
      wlog->warn("lex unable to duplicate day");
      lexerr++;
    }
  }
}

<TIME>[0-2][0-9]":"[0-5][0-9] {
  // _day may or may not already be specified

  switch(yylaststate)
  {
  case SCHEDSTANZA:
    switch(yynote)
    {
    case 'a':
      // <at> Create a new Schedule object and add it to the array
#if defined(DEBUG)
      dlog->log_lex(LEXLOG, " - at %s@%s", yytext, IONULL(_day));
#endif
      if(_sarray)
      {
	Schedule *s = new Schedule(yytext, _day);
	
	if(s)
	{
	  if(!_sarray->add(s))
	    delete s;
	}
	else
	{
	  wlog->warn("lex unable to allocate new Schedule object");
	  lexerr++;
	}
      }
#if defined(DEBUG)
      else
	dlog->log_lex(LEXLOGERR, "Schedule array not defined");
#endif
      // We stay in TIME state
      break;
    case 'f':
      // <from> Move _day to _fromday if set, store time in _fromtime
      if(_fromday || _fromtime)
      {
#if defined(DEBUG)
	dlog->log_lex(LEXLOGERR, "From time already specified");
#endif
	
	lexerr++;
      }
      else
      {
	if(_day)
	{
	  _fromday = _day;
	  _day = NULL;
	}

	_fromtime = xstrdup(yytext);

	if(!_fromtime)
	{
	  wlog->warn("lex unable to duplicate from time");
	  lexerr++;
	}
      }
      BEGINX(SCHEDSTANZA);
      break;
    case 'u':
      // <until> Move _day to _untilday if set, store time in _untiltime
      if(_untilday || _untiltime)
      {
#if defined(DEBUG)
	dlog->log_lex(LEXLOGERR, "Until time already specified");
#endif
	
	lexerr++;
      }
      else
      {
	if(_day)
	{
	  _untilday = _day;
	  _day = NULL;
	}

	_untiltime = xstrdup(yytext);

	if(!_untiltime)
	{
	  wlog->warn("lex unable to duplicate until time");
	  lexerr++;
	}
      }
      BEGINX(SCHEDSTANZA);
      break;
    default:
#if defined(DEBUG)
      dlog->log_lex(LEXLOGERR, "Unknown note parsing TIME");
#endif
      break;
    } // end switch(yynote)

    xdelete(_day);
    break;
  default:
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Unknown state parsing TIME");
#endif
    break;
  }
}

<TIME>"{" {
  // Ignore this, it is just from "at {"
}

<TIME>"}" {
  if(yylaststate == SCHEDSTANZA)
  {
    // Done processing times, return to stanza state

    BEGINX(SCHEDSTANZA);
  }
  else
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Unexpected '}'");
#endif

    lexerr++;
  }
}

<TRANSPORTSTANZA>"module" {
  // Parse module name and args

  if(_via)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Module already specified");
#endif
    
    lexerr++;
  }
  else
  {
    BEGINX(NAME);
  }
}

<TRANSPORTSTANZA>"}" {
  // transport stanza definition is complete, allocate an object and stuff
  // it in cf

  if(_name && _via)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOG, "   Module: %s", _via);

    if(_ararray)
      for(int i = 0;i < _ararray->entries();i++)
      {
	Argument *a = _ararray->retrieve(i);
	
	if(a)
	  dlog->log_lex(LEXLOG, "     %s=%s",
			IONULL(a->name()), IONULL(a->value()));
      }
#endif

    if(!newcf->find_transport(_name))
    {
      Transport *t = new Transport(_name, _via, _ararray);

      if(t)
      {
	if(!newcf->add_transport(t))
	{
	  wlog->warn("lex failed to add Transport");
	  lexerr++;
	  
	  xdelete(t);
	  xadelete(_ararray, Argument);
	}
      }
      else
      {
	wlog->warn("lex unable to allocate new Transport object");
	xadelete(_ararray, Argument);
	lexerr++;
      }
    }
    else
    {
#if defined(DEBUG)
      dlog->log_lex(LEXLOGERR, "Transport '%s' redefined", _name);
#endif
      
      lexerr++;
      xadelete(_ararray, Argument);
    }
  }
  else
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Transport module not fully defined");
#endif
    
    lexerr++;
  }

  xdelete(_name);
  xdelete(_via);

  // Just reset _ararray since the Transport will manage it
  _ararray = NULL;

  BEGINX(CHECKCF);
}

<TRYSET>"time"|"times" {
  // These are effectively optional
}

<TRYSET>"try" {
  // Read a repeat counter (-1 = indefinitely)
  //  _freq = repeat counter

  BEGINX(NUMBER);
}

<TRYSET>"try"[ \t]*"{"|"{" {
  // Parse a try stanza
  //  _tryfix = try fix
  //  _calls = whom to notify
  //  _esc = flag escalated
  //  _degrade = number of permitted failed hosts
  //  _sarray2 = schedule for _degrade

  if(_calls)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Try stanza already in progress");
#endif

    lexerr++;
  }
  else
  {
    _calls = new Array<CallList>();

    if(_calls)
    {
      BEGINX(TRYSTANZA);
    }
    else
    {
      wlog->warn("lex unable to allocate CallList Array");
      lexerr++;
    }
  }
}

<TRYSET>"}" {
  // Create an AlertSchedule and stuff it in _asarray

  if(_sarray && _atarray && _asarray)
  {
    AlertSchedule *as = new AlertSchedule(_sarray, _atarray);

    if(as)
    {
      if(_asarray->add(as))
	_atarray = NULL;
      else
      {
	delete as;
	lexerr++;
      }
    }
    else
    {
      wlog->warn("lex unable to allocate new AlertSchedule object");
      lexerr++;
    }
  }
  else
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "AlertSchedule not fully defined");
#endif

    lexerr++;
  }

  // Clean up

  xadelete(_atarray, AlertTry);
  _sarray = NULL;

  BEGINX(RETURNSTANZA);
}

<TRYSTANZA>"alert" {
  // "alert" and "tell" have two different meanings (alert indicates a
  // two-way response could be sent, tell indicates no reply would be
  // appropriate) but are stored the same way.  Nevertheless, we parse
  // them differently to make sure the configuration files are set up
  // correctly for future implementations.

  if(_tryfix != no_fix)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Cannot use 'alert' with 'attempt fix'");
#endif
    
    lexerr++;
  }
  else
  {
    BEGINXN(NAME, 'a');
  }
}

<TRYSTANZA>"allow" {
  // The number of permitted failed hosts

  if(_degrade > 0)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Allowed check failures already specified");
#endif
    
    lexerr++;
  }
  else
  {
    BEGINX(NUMBER);
  }
}

<TRYSTANZA>"attempt fix"|"attempt fix if defined" {
  // Set _tryfix.  This must be set before tell so we can make sure
  // that it is not used with the alert keyword.

  if(_tryfix != no_fix)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Attempt fix already set");
#endif
    
    lexerr++;
  }
  else
  {
    if(_calls && _calls->entries() > 0)
    {
#if defined(DEBUG)
      dlog->log_lex(LEXLOGERR,
		     "'attempt fix' must be specified before call lists to notify");
#endif
      
      lexerr++;
    }
    else
    {
      if(strcmp(yytext, "attempt fix")==0)
	_tryfix = always_fix;
      else
	_tryfix = if_defined_fix;
    }
  }
}

<TRYSTANZA>"during" {
  // schedule for degrade is optional, without one the schedule for
  // this tryset applies

  if(_degrade < 1)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR,
		   "Use of 'during' requires number of failed hosts to allow");
#endif
    
    lexerr++;
  }
  else
  {
    if(_sarray2)
    {
#if defined(DEBUG)
      dlog->log_lex(LEXLOGERR, "Schedule already defined for degraded mode");
#endif
    
      lexerr++;
    }
    else
    {
      BEGINXN(NAME, 'd');
    }
  }
}

<TRYSTANZA>"failed"|"host"|"hosts"|"schedule" {
  // These are essentially optional
}

<TRYSTANZA>"flag escalated" {
  // Set _esc to true

  if(_esc)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Escalated flag already set");
#endif
    
    lexerr++;
  }
  else
    _esc = true;
}

<TRYSTANZA>"tell" {
  // "alert" and "tell" have two different meanings (alert indicates a
  // two-way response could be sent, tell indicates no reply would be
  // appropriate) but are stored the same way.  Nevertheless, we parse
  // them differently to make sure the configuration files are set up
  // correctly for future implementations.

  if(_tryfix == no_fix)
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "Use of 'tell' requires 'attempt fix'");
#endif
    
    lexerr++;
  }
  else
  {
    BEGINXN(NAME, 't');
  }
}

<TRYSTANZA>"}" {
  // Allocate an AlertTry and stuff it in _atarray

  if((_calls || (_tryfix != no_fix)) && _atarray)
  {
    // fix attempts do not need to inform anyone, although it is recommended.
    // If we are not fixing, we need a notification recipient.
    
#if defined(DEBUG)
    if(_freq == -1)
      dlog->log_lex(LEXLOG, "   - Try indefinitely to %s%s%s",
		    (_tryfix != no_fix) ? "attempt fix" : "alert",
		    (_tryfix == if_defined_fix) ? " if defined" : "",
		    (_tryfix != no_fix && _calls) ? ", telling" : "");
    else
      dlog->log_lex(LEXLOG, "   - Try %d time%s to %s%s%s",
		    _freq, _freq == 1 ? "" : "s",
		    (_tryfix != no_fix) ? "attempt fix" : "alert",
		    (_tryfix == if_defined_fix) ? " if defined" : "",
		    (_tryfix != no_fix && _calls) ? ", telling" : "");

    if(_calls)
      for(int i = 0;i < _calls->entries();i++)
      {
	CallList *cl = _calls->retrieve(i);
	
	if(cl && cl->name())
	  dlog->log_lex(LEXLOG, "    - %s", cl->name());
      }

    if(_esc)
      dlog->log_lex(LEXLOG, "    - Flagging as escalated when this try stanza is reached");
    
    if(_degrade > 0)
      dlog->log_lex(LEXLOG, "    - Allowing %d failed host%s%s%s%s",
	     _degrade, _degrade == 1 ? "" : "s",
	     _sarray2 ? " during " : "",
	     _sarray2 ? IONULL(_sarray2->name()) : "",
	     _sarray2 ? " schedule" : "");
#endif

    AlertTry *t = new AlertTry(_freq, _tryfix, _calls, _degrade, _sarray2,
			       _esc);

    if(t)
    {
      if(_atarray->add(t))
	_calls = NULL;
      else
	delete t;
    }
    else
    {
      wlog->warn("lex unable to allocate new AlertTry object");
      lexerr++;
    }
  }
  else
  {
#if defined(DEBUG)
    dlog->log_lex(LEXLOGERR, "AlertTry not fully defined");
#endif

    lexerr++;
  }

  // Clean up

  xdelete(_calls);
  _degrade = 0;
  _esc = false;
  _tryfix = no_fix;
  _freq = -1;
  _sarray2 = NULL;
  
  BEGINX(TRYSET);
}

[^ \n\t]+ {
#if defined(DEBUG)
  dlog->log_lex(LEXLOGERR,
		 "Unexpected token '%s' (state %d)", yytext, YYSTATE);
#endif

  lexerr++;
}

#.*\n {
  lexline++;
}

[ \t]+ {
}

\n {
  lexline++;
}

%%

void lexfinish()
{
  // Perform any post-file parse operations.

  // As lexincomplete() says, we should always end up in a CF state
  // at the end of a file.  (lexincomplete is called more than once,
  // which is why this functionality is not integrated there.)

  switch(YYSTATE)
  {
  case SCHEDCF:
    // Go back and verify the call list rotation schedule names.
    // calllist.cf and schedule.cf are mutually dependent, so this
    // can not be done at parse time.

    if(!lexerr)
    {
      Array<CallList> *cls = newcf->get_all_calllists();

      if(cls)
      {
	for(int i = 0;i < cls->entries();i++)
	{
	  CallList *cl = cls->retrieve(i);

	  if(cl && cl->listtype() == rotating_list)
	  {
	    char *rname = cl->rotatename();
	    Array<Schedule> *sched = newcf->find_schedule(rname);
	    
	    if(sched)
	    {
	      bool atonly = true;
	      
	      for(int j = 0;j < sched->entries();j++)
	      {
		Schedule *s = sched->retrieve(j);

		if(s && !s->at())
		  atonly = false;
	      }

	      if(atonly)
		cl->set_rotatesched(sched);
	      else
	      {
#if defined(DEBUG)
		dlog->log_lex(LEXLOGERR,
			       "Call list '%s' cannot use requested schedule '%s' since schedule includes non-at stanzas (sorry, line number information is unavailable)",
			       IONULL(cl->name()),
			       IONULL(rname));
#endif
		lexerr++;
	      }
	    }
	    else
	    {
#if defined(DEBUG)
	      dlog->log_lex(LEXLOGERR,
			     "Undefined schedule '%s' requested for call list '%s' in calllist.cf (sorry, line number information is unavailable)",
			     IONULL(rname),
			     IONULL(cl->name()));
#endif
	      lexerr++;
	    }
	  }
	}
      }
    }
    break;
  default:
    break;
  }
}

int lexincomplete()
{
  // Determine the completeness of a parse.

  int r = 0;

  // We should always end up in a CF state

  switch(YYSTATE)
  {
  case CALLLISTCF:
  case CGICF:
  case CHECKCF:
  case DEPENDENCYCF:
  case GATEWAYCF:
  case HOSTCF:
  case INSTANCECF:
  case SCHEDCF:
    break;
  default:
    r++;
    break;
  }
  
  return(r);
}

void lexinit(void)
{
  // Reset state

  yyin = stdin;
  lexerr = 0;
  yylaststate = 0;
  lexline = 1;
  newcf = NULL;
  newccf = NULL;
  newgcf = NULL;
  newi = NULL;
  newil = NULL;
  cftype = none_cf;
}

void lexstart(config_file_t cft, FILE *in, Configuration *cfg)
{
  // Reset state

  lexinit();
  yyin = in;
  newcf = cfg;
  cftype = cft;
  
  switch(cft)
  {
  case calllist_cf:
    BEGIN CALLLISTCF;
    break;
  case host_cf:
    BEGIN HOSTCF;
    break;
  case sched_cf:
    BEGIN SCHEDCF;
    break;
  case check_cf:
    BEGIN CHECKCF;
    break;
  case dependency_cf:
    BEGIN DEPENDENCYCF;
    break;
  default:
    BEGIN INITIAL;
    break;
  }
}

void lexiastart(FILE *in, List *l)
{
  // Reset state to parse instance.cf instances into <l>.

  lexinit();
  yyin = in;
  newil = l;
  cftype = instance_cf;

  BEGIN INSTANCECF;
}

void lexistart(FILE *in, Instance *i)
{
  // Reset state to parse instance.cf into <i>.

  lexinit();
  yyin = in;
  newi = i;
  cftype = instance_cf;
  
  BEGIN INSTANCECF;
}

void lexsgstart(FILE *in, GatewayConfiguration *gcf)
{
  // Reset state to parse gateway.cf into <gcf>.

  lexinit();
  yyin = in;
  newgcf = gcf;
  cftype = gateway_cf;

  BEGIN GATEWAYCF;
}

void lexswstart(FILE *in, CGIConfiguration *ccf)
{
  // Reset state to parse cgi.cf into <ccf>.

  lexinit();
  yyin = in;
  newccf = ccf;
  cftype = cgi_cf;
  
  BEGIN CGICF;
}

int get_yy()
{
  int c;

  while((c = getc(yyin)) == '\r' || c == '\0')
    ;

  return(c);
}
