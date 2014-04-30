/*
 * cmutils.C: Check Module utilities
 *
 * Version: $Revision: 0.14 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2006/01/23 00:47:08 $
 * MT-Level: Unsafe
 *
 * Copyright (c) 2002 - 2006
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: cmutils.C,v $
 * Revision 0.14  2006/01/23 00:47:08  benno
 * Add cm_measure_duration, duration measurement support
 *
 * Revision 0.13  2005/11/14 03:07:20  benno
 * Add cm_relation_comparse, cm_perform_extraction
 *
 * Revision 0.12  2005/04/19 22:43:16  benno
 * Add _cm_fix
 * Use GENERATERESULTXML
 *
 * Revision 0.11  2005/04/09 02:43:21  benno
 * XML based arguments
 *
 * Revision 0.10  2003/06/25 21:29:29  benno
 * Workaround gcc 3.3 compile bug
 *
 * Revision 0.9  2003/04/09 20:04:17  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.8  2003/04/07 16:28:45  benno
 * Use Debugger
 *
 * Revision 0.7  2003/03/04 20:53:11  benno
 * Bump copyright
 *
 * Revision 0.6  2003/01/24 02:54:02  benno
 * Use IONULL, IOTF, xdelete, xadelete
 *
 * Revision 0.5  2002/09/05 21:58:51  benno
 * fix rcs tag
 *
 * Revision 0.4  2002/05/02 23:07:57  toor
 * initialize and deinitialize tids
 *
 * Revision 0.3  2002/04/04 20:01:13  benno
 * copyright
 *
 * Revision 0.2  2002/04/02 22:47:15  benno
 * rcsify date
 *
 * Revision 0.1  2002/04/02 22:46:56  benno
 * initial revision
 *
 */

#include "survivor.H"

// These are global for easy access
CheckResult *(*checkfunc)(int) = NULL;
pthread_t *tids = NULL;
int *tidstates = NULL;   // 0 = none, 1 = working, 2 = finished
sem_t workers;           // Restricts concurrent workers permitted
sem_t finished;          // Indicates threads in state 2
bool _cm_doduration = true;  // Whether to measure duration

// This is a minor hack: A calling program sets this to say that
// we're really working with Fix data
bool _cm_fix = false;

void cm_error(int rc, int scalar, char *msg)
{
  // Display one error line per host in conformance with the specification.
  // <rc> is the return code, <scalar> is the scalar value, and <msg> is
  // the comment string.

  // Returns: Nothing.
  
  if(cmargs && cmargs->hosts())
  {
    SurvivorXML *sxml = new SurvivorXML();

    if(sxml)
    {
      for(int i = 0;i < cmargs->hosts()->entries();i++)
      {
	CheckResult *cr = new CheckResult(cmargs->hosts()->retrieve(i),
					  rc,
					  scalar,
					  (char*)IONULL(msg));

	if(cr)
	{
	  GENERATERESULTXML(cr);
	  xdelete(cr);
	}
      }

      xdelete(sxml);
    }
  }
}

void cm_measure_duration(bool on)
{
  // Enable (if <on> is true) or disable check duration measurement.
  // By default, measurement is enabled.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "cm_measure_duration(%s)", IOTF(on));
  dlog->log_exit(DEBUG_MAJTRC, "cm_measure_duration()");
#endif
  
  _cm_doduration = on;
}

char *cm_perform_extraction(ExtractionArg *earg, char *s)
{
  // Perform the extraction described in <earg> on the string <s>.

  // Returns: A newly allocated string that should be delete'd when no
  // longer required, or NULL on error.

  char *ret = NULL;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "cm_perform_extraction(%d,%s)", earg,
		  IONULL(s));
#endif

  if(earg && s)
  {
    switch(earg->extraction())
    {
    case column_ext:
      // Copy column x (start counting at 0)
      {
	List *l = tokenize(s, " \t");

	if(l)
	{
	  ret = xstrdup(l->retrieve(earg->x()));
	  
	  xdelete(l);
	}
      }
      break;
    case substr_ext:
      // Copy from x of length y
      if(earg->x() < strlen(s))
	ret = xstrncat(NULL, s+earg->x(), earg->y());
      break;
    case no_ext:
    default:
      // Unknown extraction or nothing to do
      break;
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "cm_perform_extraction = %s", IONULL(ret));
#endif

  return(ret);
}

bool cm_relation_compare(RelationArg *rarg, char *s, char *label, char **rtext)
{
  // Compare the value <s> against the relation <rarg>.  <s> will be
  // treated as a number or string appropriately.  If <rtext> is
  // provided, a newly allocated string that should be delete'd when
  // no longer required will be stored there with a text description
  // of the comparison, labelled <label> (if provided).

  // Returns: true if the relation matches, false otherwise.

  bool ret = false;
  CharBuffer *r = new CharBuffer();
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "cm_relation_compare(%d,%s,%s,%d)",
		  rarg, IONULL(s), IONULL(label), rtext);
#endif

  // For some relations, we need numbers.

  double sn = s ? atof(s) : 0;
  double sx = rarg->x() ? atof(rarg->x()) : 0;
  double sy = rarg->y() ? atof(rarg->y()) : 0;
  
  if(rarg && r)
  {
    r->append(label ? label : (char *)"Value");
    r->append(" ");

    switch(rarg->relation())
    {
    case bt_rel:
      // Treat s as a number
      if(sn >= sx && sn <= sy)
      {
	ret = true;
	
	r->append("'");
	r->append(sn);
	r->append("' is between '");
	r->append(sx);
	r->append("' and '");
	r->append(sy);
	r->append("'");
      }
      else
      {
	r->append("'");
	r->append(sn);
	r->append("' is not between '");
	r->append(sx);
	r->append("' and '");
	r->append(sy);
	r->append("'");
      }
      break;
    case eq_rel:
      // Treat s as a number
      if(sn == sx)
      {
	ret = true;
	
	r->append("is '");
	r->append(sx);
	r->append("'");
      }
      else
      {
	r->append("'");
	r->append(sn);
	r->append("' is not '");
	r->append(sx);
	r->append("'");
      }
      break;
    case gt_rel:
      // Treat s as a number
      if(sn > sx)
      {
	ret = true;

	r->append("'");
	r->append(sn);
	r->append("' is greater than '");
	r->append(sx);
	r->append("'");
      }
      else
      {
	r->append("'");
	r->append(sn);
	r->append("' is not greater than '");
	r->append(sx);
	r->append("'");
      }
      break;
    case lt_rel:
      // Treat s as a number
      if(sn < sx)
      {
	ret = true;
	
	r->append("'");
	r->append(sn);
	r->append("' is less than '");
	r->append(sx);
	r->append("'");
      }
      else
      {
	r->append("'");
	r->append(sn);
	r->append("' is not less than '");
	r->append(sx);
	r->append("'");
      }
      break;
    case nb_rel:
      // Treat s as a number
      if(sn < sx || sn > sy)
      {
	ret = true;
	
	r->append("'");
	r->append(sn);
	r->append("' is not between '");
	r->append(sx);
	r->append("' and '");
	r->append(sy);
	r->append("'");
      }
      else
      {
	r->append("'");
	r->append(sn);
	r->append("' is between '");
	r->append(sx);
	r->append("' and '");
	r->append(sy);
	r->append("'");
      }
      break;
    case ne_rel:
      // Treat s as a number
      if(sn != sx)
      {
	ret = true;
	
	r->append("'");
	r->append(sn);
	r->append("' is not '");
	r->append(sx);
	r->append("'");
      }
      else
      {
	r->append("is '");
	r->append(sx);
	r->append("'");
      }
      break;
    case re_rel:
      // Treat s as a string
      if(compare_regex(s, rarg->x()))
      {
	ret = true;
	
	r->append("'");
	r->append(s);
	r->append("' matches '");
	r->append(rarg->x());
	r->append("'");
      }
      else
      {
	r->append("'");
	r->append(s);
	r->append("' does not match '");
	r->append(rarg->x());
	r->append("'");
      }
      break;
    case rv_rel:
      // Treat s as a string
      if(!compare_regex(s, rarg->x()))
      {
	ret = true;
	
	r->append("'");
	r->append(s);
	r->append("' does not match '");
	r->append(rarg->x());
	r->append("'");
      }
      else
      {
	r->append("'");
	r->append(s);
	r->append("' matches '");
	r->append(rarg->x());
	r->append("'");
      }
      break;
    case tn_rel:
      // Treat s as a number
      {
	struct timeval tp;
	
	if(gettimeofday(&tp, NULL)==0)
	{
	  if(sn > (tp.tv_sec - sx))
	  {
	    ret = true;
	    
	    r->append("'");
	    r->append(sn);
	    r->append("' is newer than '");
	    r->append(sx);
	    r->append("' seconds");
	  }
	  else
	  {
	    r->append("'");
	    r->append(sn);
	    r->append("' is not newer than '");
	    r->append(sx);
	    r->append("' seconds");
	  }
	}
	else
	  r->append("Failed to get time of day");
      }
      break;
    case to_rel:
      // Treat s as a number
      {
	struct timeval tp;
	
	if(gettimeofday(&tp, NULL)==0)
	{
	  if(sn < (tp.tv_sec - sx))
	  {
	    ret = true;
	    
	    r->append("'");
	    r->append(sn);
	    r->append("' is older than '");
	    r->append(sx);
	    r->append("' seconds");
	  }
	  else
	  {
	    r->append("'");
	    r->append(sn);
	    r->append("' is not older than '");
	    r->append(sx);
	    r->append("' seconds");
	  }
	}
	else
	  r->append("Failed to get time of day");
      }
      break;
    case no_rel:
    default:
      r->append("Unknown relation or nothing to do");
      break;
    }
  }

  if(rtext)
    *rtext = xstrdup(r->str());
  
  xdelete(r);
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "cm_relation_compare = %s", IOTF(ret));
#endif

  return(ret);  
}

void *cm_runcheck(void *x)
{
  // This function should not be publically visible.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "cm_runcheck(%d)", x);
#endif

  struct timeval tpstart, tpend;

  // Get permission before proceeding
  sem_wait(&workers);

  if(_cm_doduration)
    gettimeofday(&tpstart, NULL);

  // Call the worker
  CheckResult *rv = checkfunc((int)x);

  if(_cm_doduration)
  {
    gettimeofday(&tpend, NULL);

    // Replace rv with a new CheckResult with the duration

    CheckResult *newrv = new CheckResult(rv->hostname(),
					 rv->rc(),
					 rv->scalar(),
					 rv->comment(),
					 (((tpend.tv_sec - tpstart.tv_sec)
					   * 1000000) + 
					  tpend.tv_usec - tpstart.tv_usec)
					 / 1000);

    if(newrv)
    {
      xdelete(rv);
      rv = newrv;
    }
  }
  
  // Allow other threads to proceed
  sem_post(&workers);

  // And indicate that we are done
  tidstates[(int)x] = 2;
  sem_post(&finished);
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "cm_runcheck = %d", rv);
#endif
  
  pthread_exit((void *)rv);
}

int cm_thread_exec(CheckResult *(*check)(int))
{
  // Execute a check in a multithreaded fashion and display the results.
  // <check> is a function that takes an index from 0 to
  // cmargs->hosts->entries() - 1 and performs the check for the host at
  // that index.  It returns a newly allocated CheckResult object containing
  // the information to display.  Needless to say, <check> must be MT Safe.
  // Timeouts are not currently handled.  If the module times out, it will
  // be terminated by the scheduler.

  // Returns: The highest return value obtained.

  int r = MODEXEC_OK;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "cm_thread_exec(%s)",
		   (check ? "<check>" : "(null)"));
#endif

  if(check && cmargs && cmargs->hosts())
  {
    checkfunc = check;
    
    if(sem_init(&workers, 0, DEFAULT_CMTHREADS) != -1)
    {
      if(sem_init(&finished, 0, 0) != -1)
      {
	tids = new pthread_t[cmargs->hosts()->entries()];
	tidstates = new int[cmargs->hosts()->entries()];
	SurvivorXML *sxml = new SurvivorXML();

	if(tids && tidstates && sxml)
	{
	  for(int i = 0;i < cmargs->hosts()->entries();i++)
	  {
	    tidstates[i] = 1;
	    tids[i] = (pthread_t)-1;

	    if(pthread_create(&tids[i], NULL, cm_runcheck, (void *)i) != 0)
	    {
	      if(r < MODEXEC_PROBLEM)
		r = MODEXEC_PROBLEM;

	      CheckResult *cr = new CheckResult(cmargs->hosts()->retrieve(i),
						MODEXEC_PROBLEM,
						0,
						"pthread_create failed");
	      
	      if(cr)
	      {
		GENERATERESULTXML(cr);
		xdelete(cr);
	      }
	    }
	  }

	  // Now that all the threads are running, wait for each one to
	  // finish

	  for(int i = 0;i < cmargs->hosts()->entries();i++)
	  {
	    sem_wait(&finished);

	    // Look through tidstates for a state of 2

	    for(int j = 0;j < cmargs->hosts()->entries();j++)
	    {
	      if(tidstates[j] == 2)
	      {
		void *rvp = NULL;

		tidstates[j] = 0;		
		pthread_join(tids[j], &rvp);

		CheckResult *rv = (CheckResult *)rvp;
		tids[j] = (pthread_t)-1;

		if(rv)
		{
		  if(rv->rc() > r)
		    r = rv->rc();

		  CheckResult *cr =
		    new CheckResult(cmargs->hosts()->retrieve(j),
				    rv->rc(),
				    rv->scalar(),
				    rv->comment(),
				    rv->duration());

		  if(cr)
		  {
		    GENERATERESULTXML(cr);
		    xdelete(cr);
		  }

		  xdelete(rv);
		}
		else
		{
		  if(r < MODEXEC_PROBLEM)
		    r = MODEXEC_PROBLEM;

		  CheckResult *cr =
		    new CheckResult(cmargs->hosts()->retrieve(j),
				    MODEXEC_PROBLEM,
				    0,
				    "No exit information received");

		  if(cr)
		  {
		    GENERATERESULTXML(cr);
		    xdelete(cr);
		  }
		}

		break;
	      }
	    }
	  }
	}
	else
	{
	  r = MODEXEC_PROBLEM;
	  cm_error(r, 0, "module failed to allocate structures");
	}

	xdelete(tids);
	xdelete(tidstates);
	xdelete(sxml);
	
	sem_destroy(&finished);
      }
      else
      {
	r = MODEXEC_PROBLEM;
	cm_error(r, 0, "module failed to sem_init finished");
      }

      sem_destroy(&workers);
    }
    else
    {
      r = MODEXEC_PROBLEM;
      cm_error(r, 0, "module failed to sem_init workers");
    }
  }
  else
  {
    r = MODEXEC_MISCONFIG;
    cm_error(r, 0, "No check function provided or no hosts provided");
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "cm_thread_exec = %d", r);
#endif
  
  return(r);
}
