/*
 * Executor.C: survivor object to handle execution of modules
 *
 * Version: $Revision: 0.39 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2006/01/25 00:33:15 $
 * MT-Level: Safe
 *
 * Copyright (c) 2002 - 2006
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: Executor.C,v $
 * Revision 0.39  2006/01/25 00:33:15  benno
 * Detect non-existent report module before trying to exec
 *
 * Revision 0.38  2006/01/23 02:09:35  benno
 * Fix typo
 *
 * Revision 0.37  2005/11/30 01:37:53  benno
 * Add module serialization
 *
 *
 * Revision 0.36  2005/08/10 01:19:20  benno
 * Block on read_xml_document in composite check exec
 *
 * Revision 0.35  2005/06/09 02:30:25  benno
 * exec_pipe returns a pair of fds
 *
 * Revision 0.34  2005/04/27 01:56:34  benno
 * Use read_xml_document for proper reading of composite check results
 *
 * Revision 0.33  2005/04/19 22:42:23  benno
 * Use pipefds2 for proper execution of composite checks
 *
 * Revision 0.32  2005/04/07 01:12:02  benno
 * Checks and Fixes use XML
 * Toss allocate_argv
 *
 * Revision 0.31  2004/08/24 23:51:53  benno
 * Add support for report modules
 *
 * Revision 0.30  2004/03/01 23:28:54  benno
 * Add exec_webauth and matching result()
 *
 * Revision 0.29  2003/10/19 23:57:35  benno
 * Get rid of unused mindex argument to exec_format_transmit
 * Call generate(alert) with module name
 *
 * Revision 0.28  2003/10/06 23:13:07  benno
 * Use unbuffered I/O
 * Add more error reporting
 *
 * Revision 0.27  2003/07/30 21:42:08  benno
 * Add exec_formatted_alert
 * Call FD_CLR before closing the fd it uses
 *
 * Revision 0.26  2003/06/25 21:32:28  benno
 * ComponentCheckResult for composite checks
 *
 * Revision 0.25  2003/06/17 15:07:28  benno
 * Output composite check results as ready instead of all at once
 *
 * Revision 0.24  2003/05/29 00:34:20  benno
 * Changes for AlertModule, format and transmit modules
 *
 * Revision 0.23  2003/05/05 00:34:45  benno
 * Fix inverted logic bug
 *
 * Revision 0.22  2003/05/04 21:29:18  benno
 * Don't use string type
 *
 * Revision 0.21  2003/04/09 20:23:47  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.20  2003/04/04 22:35:38  benno
 * Use Debugger
 *
 * Revision 0.19  2003/03/31 13:26:53  benno
 * Add sanity checking for data
 * Pass host lists to parse_results
 *
 * Revision 0.18  2003/03/04 17:59:34  benno
 * Use dump_misconfig for better reporting of failures
 * Use select() instead of poll() for better cross-platformability
 *
 * Revision 0.17  2003/02/20 02:50:06  benno
 * Add support for composite checks
 *
 * Revision 0.16  2003/01/24 16:51:06  benno
 * Add setsid for killpg
 * Use IONULL
 *
 * Revision 0.15  2003/01/02 04:44:50  benno
 * Fix tmp_state_filename race condition
 *
 * Revision 0.14  2002/12/31 04:31:26  benno
 * Add fix support
 * Add reset_executor
 *
 * Revision 0.13  2002/12/22 17:17:58  benno
 * Add support for Transport modules
 * Switch to xdelete
 *
 * Revision 0.12  2002/12/16 00:47:05  benno
 * Use RecipientSets to allow multiple calllist notifications
 * Remove token generation to AlertState
 *
 * Revision 0.11  2002/12/10 23:34:02  benno
 * add exec_pipe, allocate_argv for tunneling
 *
 * Revision 0.10  2002/10/21 20:46:38  benno
 * add support for named args
 *
 * Revision 0.9  2002/08/06 14:19:53  selsky
 * Remove embedded null characters from format
 *
 * Revision 0.8  2002/05/22 19:03:27  selsky
 * fix format string for time_t
 *
 * Revision 0.7  2002/04/04 20:10:13  benno
 * copyright
 *
 * Revision 0.6  2002/04/03 18:14:34  benno
 * rcsify date
 *
 * Revision 0.5  2002/04/03 18:14:23  benno
 * Don't regenerate token if it already exists
 *
 * Revision 0.4  2002/04/03 18:13:51  benno
 * Support -I for alert modules
 * Change alert token from long to char[]
 * Provide full path to helpfile
 *
 * Revision 0.3  2002/04/03 18:13:22  benno
 * Clean up
 * Use CheckState for result(char **)
 *
 * Revision 0.2  2002/04/03 18:12:45  benno
 * Return pids instead of ints
 * Become Unsafe and store fds[] locally
 *
 * Revision 0.1  2002/04/03 18:12:03  benno
 * initial revision
 *
 */

#include "survivor.H"

Executor::Executor()
{
  // Allocate and initialize a new Executor object.

  // Returns: A new Executor object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Executor::Executor()");
#endif
  
  a = NULL;
  ar = NULL;
  c = NULL;
  cs = NULL;
  f = NULL;
  h = NULL;
  rep = NULL;
  pid = -1;
  fds[0] = -1;
  fds[1] = -1;
  fds2[0] = -1;
  fds2[1] = -1;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Executor::Executor()");
#endif
}

bool Executor::close_pipe()
{
  // Close a pipe originated by exec_pipe.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Executor::close_pipe()");
#endif
  
  if(pid > 0)
  {
    // Wait on the child process(es)

    waitpid(pid, NULL, 0);
    closepipe();
    
    ret = true;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Executor::close_pipe = %s", IOTF(ret));
#endif

  return(ret);
}

pid_t Executor::exec_alert(Alert *alert, AlertModule *module)
{
  // Run the Alert <alert>.  <module> indicates which module in the
  // RecipientSet within <alert> to exec.  (The others are ignored.)

  // Returns: The pid of the exec'd process, whose status may be obtained
  // with result(), or -1 on error.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Executor::exec_alert(%d,%d)",
		  alert, module);
#endif

  if(pid == -1 && alert && module && module->name())
  {
    // Perform a sanity check on the data we receive.  We shouldn't have
    // to do this, but some bad data is making it here.

    if(alert->retval() != MODEXEC_DEPEND && alert->checktime() > 0)
    {
      // First find the module we are supposed to use
      
      RecipientSet *rset = alert->recipients();
      int mindex = -1;
      
      for(int i = 0;i < rset->modules();i++)
      {
	char *mod = rset->module(i);
	
	if(mod && strcmp(mod, module->name())==0)
	{
	  mindex = i;
	  break;
	}
      }
      
      if(mindex > -1 && rset->addresses(mindex))
      {
	// Because we set up a pipe to a format module and then a transmit
	// module, behave like for composite checks: create a new subprocess
	// that can be timed out via killpg, and have it run the modules.
      
	if(pipe(fds)==0)
	{
	  pid = fork();
	  
	  if(pid > -1)
	  {
	    if(pid == 0)
	    {
	      // Child, create a new session for use by killpg().  If this
	      // fails, we might have a few extra defunct processes lying
	      // around.
	      
	      setsid();
	      
	      // Reset stdin and stdout and exec the check.  We don't
	      // really need to reset stdin & stdout as we don't read from
	      // them, but this prevents garbage from spewing.
	      
	      dup2(fds[0], 0);
	      dup2(fds[1], 1);
	      
	      // No need to pass our setgidness to the process we exec.
	      
	      if(getgid() != getegid())
		setegid(getgid());

	      exec_format_transmit(alert, module);

	      // We shouldn't make it here
	      exit(MODEXEC_MISCONFIG);
	    }
	    else
	    {
	      // We're the parent.  Note <alert> and return.
	      
	      a = alert;
	    }
	  }
	  else
	    wlog->warn("fork() failed in Executor::exec_alert");
	}
	else
	  wlog->warn("pipe() failed in Executor::exec_alert");
      }
      else
	wlog->warn("Executor::exec_alert unable to validate request for module %s",
		   module);
    }
    else
      wlog->warn("Executor::exec_alert received implausible values for %s@%s: rv=%d, checktime=%d",
		 IONULL(alert->service()), IONULL(alert->host()),
		 alert->retval(), alert->checktime());
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Executor::exec_alert = %d", pid);
#endif
  
  return(pid);
}

pid_t Executor::exec_check(Check *check, CheckRequest *cr)
{
  // Run the Check <c> as requested in <cr>.  The CheckRequest timeout
  // is advisory to the Check module, and is NOT implemented by the
  // Executor object. <cr> must remain valid until result() is called.
  // If <cptr> is provided to result(), up to one line of comment will
  // be read and stored in <cptr>, which should be delete'd when no
  // longer required.

  // Returns: The pid of the exec'd process, whose status may be obtained
  // with result(), or -1 on error.
  
  return(exec_check(check, NULL, cr));
}

pid_t Executor::exec_check(Check *check, CheckState *cstate, CheckRequest *cr)
{
  // Run the Check <check> as requested in <cr>.  The CheckRequest
  // timeout is advisory to the Check module, and is NOT implemented
  // by the Executor object.  <cr> must remain valid until result() is
  // called.  <cstate> is used to record state.

  // Returns: The pid of the exec'd process, whose status may be obtained
  // with result(), or -1 on error.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Executor::exec_check(%d,%d,%d)",
		  check, cstate, cr);
#endif
  
  if(pid == -1 && check && cr && cr->hosts())
  {
    // Note host list and checkstate if provided

    h = cr->hosts();
    cs = cstate;
    
#if defined(ENABLE_MODULE_SERIALIZATION)
    // Module serialization solves a very subtle bug in threaded execution
    // (as the scheduler does).  The problem is a race condition, when two
    // Worker threads try to exec different checks simultaneously.  The
    // first thread calls pipe() below, and opens 4 filehandles.  The
    // second thread does the same, opening 4 new filehandles.  Each
    // thread then forks.  (Alternately, the first thread calls pipe()
    // but the second thread calls pipe() and forks before the first
    // thread forks.)

    // The children inherit ALL open filehandles, not just the ones
    // intended for it.  This is problematic because XML based arguments
    // expect an EOF on stdin to signal the end of document, but the EOF
    // only gets sent when ALL filehandles have closed.  Since each of the
    // two children have open the other's filehandles (and don't even know
    // what they are), EOF never gets sent.

    // As a workaround, libcm and XML.pm will timeout after 1 second of
    // inactivity once the document has started being read.  Module
    // serialization only forks one child at a time.
    
    if(pthread_mutex_lock(&msmutex)==0)
    {
#endif
      
    // We set up the pipe regardless of whether or not we store the results.
    // We're bidirectional, so we need both fds.

    if(pipe(fds)==0 && pipe(fds2)==0)
    {
      pid = fork();

      if(pid > -1)
      {
	if(pid == 0)
	{
	  // Child, create a new session for use by killpg().
	  
	  setsid();

	  // Reset stdin and stdout.  If <cs> was provided, redirect
	  // stdout to the appropriate temp file, otherwise use the
	  // pipe.

	  // First pair is input
	  dup2(fds[0], 0);
	  close(fds[1]);
	  fds[1] = -1;

	  // Second pair is output
	  close(fds2[0]);
	  fds2[0] = -1;

	  if(cs && cs->tmp_state_filename())
	  {
	    // Unlink the file and then insist on creating the file anew.
	    // Otherwise, renegade modules that do not exit properly can
	    // interleave their belated output with ours.
	    // (O_CREAT | O_TRUNC opens as append the second time.
	    // To test this, unset noclobber, do cat > somefile in two
	    // different shell prompts, and watch the output interleave.

	    unlink(cs->tmp_state_filename());
	    
	    int fd = open(cs->tmp_state_filename(),
			  O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);

	    if(fd > -1)
	      dup2(fd, 1);
	    else
	    {
	      dump_misconfig(cr->hosts(),
			     "exec_check child process failed to open %s",
			     cs->tmp_state_filename());

	      exit(MODEXEC_MISCONFIG);
	    }
	  }
	  else
	    dup2(fds2[1], 1);

	  // See if we have a composite check.  If so, coordinate
	  // those processes.

	  if(check->composite_required() || check->composite_optional())
	    exec_check_components(check, cr);
	  else
	    do_exec_check_child(check);
	  
	  // We shouldn't make it here, since either of the above should
	  // exit for us.
	  
	  exit(MODEXEC_MISCONFIG);
	}
	else
	{
	  // We're the parent.  Note <check> and pass xml over
	  // first fd pair.

	  c = check;

	  close(fds[0]);
	  fds[0] = -1;
	  close(fds2[1]);
	  fds2[1] = -1;

	  if(!check->composite_required() && !check->composite_optional())
	    do_exec_check_parent(fds[1], check, cr);

	  // Close fdout so the module doesn't hang forever on input
	  close(fds[1]);
	  fds[1] = -1;
	}
      }
      else
	wlog->warn("fork() failed in Executor::exec_check");
    }
    else
      wlog->warn("pipe() failed in Executor::exec_check");
    }
  
#if defined(ENABLE_MODULE_SERIALIZATION)
    // We could unlock the mutex right after the fork, flagging
    // filehandles after exec as FD_CLOEXEC, meaning they won't
    // survive subsequent execs.  However, the speedup from that
    // optimization is pretty small, and this way we save the
    // effort of implementing fcntl() calls.
    
    pthread_mutex_unlock(&msmutex);
    }
#endif
    
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Executor::exec_check = %d", pid);
#endif
  
  return(pid);
}

pid_t Executor::exec_fix(Check *check, FixRequest *fr)
{
  // Run the Fix for (and attached to) <check> as described in <fr>.

  // Returns: The pid of the exec'd process, whose status may be obtained
  // with result(FixState), or -1 on error.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Executor::exec_fix(%d,%d)", check, fr);
#endif

  Fix *fix = check->fix();
  
  if(pid == -1 && check && fix && fr && fr->host())
  {
    // We're bidirectional, so we need both fds.
    
    if(pipe(fds)==0 && pipe(fds2)==0)
    {
      pid = fork();

      if(pid > -1)
      {
	if(pid == 0)
	{
	  // Child, create a new session for use by killpg().

	  setsid();

	  // Reset stdin and stdout and exec the fix.

	  // First pair is input
	  dup2(fds[0], 0);
	  close(fds[1]);
	  fds[1] = -1;

	  // Second pair is output
	  dup2(fds2[1], 1);
	  close(fds2[0]);
	  fds2[0] = -1;

	  // No need to pass our setgidness to the process we exec.
	  
	  if(getgid() != getegid())
	    setegid(getgid());
	  
	  char modexec[PATH_MAX];
	  memset(modexec, 0, PATH_MAX);

	  if(fix->transport() && fix->transport()->module())
	  {
	    // If a Transport was defined, we exec that instead.
	    // This should be the standard case.

	    if((strlen(args->moddir()) + strlen(fix->transport()->module())
		+ 12) < PATH_MAX)
	    {
	      sprintf(modexec, "%s/transport/%s", args->moddir(),
		      fix->transport()->module());
	      execl(modexec, fix->transport()->module(), NULL);
	    }
	    else
	      wlog->warn("Executor::exec_fix exec string exceeds PATH_MAX");
	  }
	  else
	  {
	    // Otherwise, exec the Fix directly, which is usually
	    // not very helpful.
	  
	    if((strlen(args->moddir()) + strlen(fix->module()) + 8) <
	       PATH_MAX)
	    {
	      sprintf(modexec, "%s/fix/%s", args->moddir(), fix->module());
	      execl(modexec, fix->module(), NULL);
	    }
	    else
	      wlog->warn("Executor::exec_fix exec string exceeds PATH_MAX");
	  }
	  
#if defined(DEBUG)
	  dlog->log_progress(DEBUG_THREAD, "Failed to exec %s", modexec);
#endif
	  
	  // This is a minor hack.  We output a Result string so that
	  // sc (and anything else) doesn't hang while waiting to read
	  // input.  This is pretty similar to dump_misconfig().

	  FixResult *fr = new FixResult(MODEXEC_MISCONFIG,
					"Executor::exec_fix child process failed to exec module");
	  SurvivorXML *sxml = new SurvivorXML();

	  if(fr && sxml)
	    sxml->generate(STDOUT_FILENO, fr);

	  xdelete(fr);
	  xdelete(sxml);

	  exit(MODEXEC_MISCONFIG);
	}
	else
	{
	  // We're the parent, pass the appropriate XML to the child
	  // over the first fd pair.

	  close(fds[0]);
	  fds[0] = -1;
	  close(fds2[1]);
	  fds2[1] = -1;
	  
	  SurvivorXML *sxml = new SurvivorXML();

	  if(sxml)
	  {	  
	    if(fix->transport() && fix->transport()->module())
	    {
	      // Generate a transport request

	      // We need to put the host into a list to make a request

	      List *h = new List();

	      if(h)
	      {
		h->add(fr->host());
		
		TransportRequest *tr =
		  new TransportRequest(h,
				       fix->fix_timeout(),
				       fix->transport()->modargs(),
				       fix->module(),
				       "fix",
				       fr->modargs(),
				       false);
		
		if(tr)
		{
		  if(!sxml->generate(fds[1], tr))
		    wlog->warn("Executor::exec_fix failed to generate XML");
		  
		  xdelete(tr);
		}
		else
		  wlog->warn("Executor::exec_fix failed to allocate TransportRequest");

		xdelete(h);
	      }
	      else
		wlog->warn("Executor::exec_fix failed to allocate List");
	    }
	    else
	    {
	      // Generate a fix request directly

	      if(!sxml->generate(fds[1], fr))
		wlog->warn("Executor::exec_fix failed to generate XML");
	    }

	    xdelete(sxml);

	    // Close fdout so the module doesn't hang forever on input
	    close(fds[1]);
	    fds[1] = -1;
	  }
	  else
	    wlog->warn("Executor::exec_fix failed to allocate SurvivorXML");
	}
      }
      else
	wlog->warn("fork() failed in Executor::exec_fix");
    }
    else
      wlog->warn("pipe() failed in Executor::exec_fix");
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Executor::exec_fix = %d", pid);
#endif

  return(pid);
}

pid_t Executor::exec_formatted_alert(FormattedAlert *falert, char *transmit)
{
  // Send the contents of <falert> using the specified <transmit> module.

  // Returns: The pid of the exec'd process, whose status may be obtained
  // with result(), or -1 on error.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Executor::exec_formatted_alert(%d,%s)",
		  falert, IONULL(transmit));
#endif

  if(pid == -1 && falert && transmit)
  {
    if(pipe(fds)==0)
    {
      pid = fork();

      if(pid > -1)
      {
	if(pid == 0)
	{
	  // Child, create a new session for use by killpg() (although
	  // these processes may not be timed out).  If this fails, we
	  // might have a few extra defunct processes lying around.

	  setsid();
	  
	  // Reset stdin and stdout and exec the module.

	  dup2(fds[0], 0);
	  close(fds[1]);

	  // No need to pass our setgidness (if any) to the process we exec.

	  if(getgid() != getegid())
	    setegid(getgid());
	    
	  // Exec the transmit module

	  char modexec[PATH_MAX];
	  memset(modexec, 0, PATH_MAX);
	  
	  if(transmit &&
	     (strlen(args->moddir()) + strlen(transmit) + 11) < PATH_MAX)
	  {
	    sprintf(modexec, "%s/transmit/%s", args->moddir(), transmit);
	    execl(modexec, transmit, NULL);
	  }

	  // We shouldn't make it here
	  exit(MODEXEC_MISCONFIG);
	}
	else
	{
	  // Parent, pass xml, note <falert>, and return.

	  close(fds[0]);
  
	  SurvivorXML *sxml = new SurvivorXML();

	  if(sxml)
	  {
	    if(!sxml->generate(fds[1], falert))
	      wlog->warn("Executor::exec_formatted_alert failed to generate XML");

	    xdelete(sxml);
	  }
	  else
	    wlog->warn("Executor::exec_formatted_alert failed to allocated SurvivorXML");

	  // Close stdout so the module doesn't hang forever on input
	  close(fds[1]);
	  
	  f = falert;
  	}
      }
      else
	wlog->warn("fork() failed in Executor::exec_formatted_alert");
    }
    else
      wlog->warn("pipe() failed in Executor::exec_formatted_alert");
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Executor::exec_formatted_alert = %d", pid);
#endif
  
  return(pid);
}
				     
int *Executor::exec_pipe(char *command)
{
  // Open the process described by <command>, similar to popen() but
  // thread safe.  <command> may not begin with a space.

  // Returns: An array of two file descriptors (input, output) if
  // successful, NULL otherwise.  The array should be deleted when
  // no longer required, the file descriptors should be closed by
  // close_pipe().

  int *ret = NULL;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Executor::exec_pipe(%s)", IONULL(command));
#endif

  if(pid == -1 && command && command[0] != ' ')
  {
    // We're bidirectional, so we need both fds
    
    if(pipe(fds)==0 && pipe(fds2)==0)
    {
      pid = fork();

      if(pid > -1)
      {
	if(pid == 0)
	{
	  // First pair is input to child
	  dup2(fds[0], 0);
	  close(fds[1]);
	  fds[1] = -1;

	  // Second pair is output from child
	  dup2(fds2[1], 1);
	  close(fds2[0]);
	  fds2[0] = -1;

	  // We need to get a path and an argument array from command.
	  // Do all this within the child so we don't have to worry about
	  // cleaning it up.
	  
	  char *path = xstrdup(command);
	  
	  if(path)
	  {
	    // Truncate at the first unescaped space
	    
	    char *p = strchr(path, ' ');
	    
	    while(p)
	    {
	      if(*(p-1) != '\\')
	      {
		*p = '\0';
		p = NULL;
	      }
	      
	      else
		p = strchr(p+1, ' ');
	    }

	    char **argarray = allocate_argv(command);

	    if(argarray)
	    {
	      // Don't worry about cleaning it up, we're about to exec

	      execv(path, argarray);
	    }
	    else
	      wlog->warn("Executor::exec_pipe failed to allocate argument array");
	  }
	  else
	    wlog->warn("Executor::exec_pipe failed to obtain command path");

	  // We're not necessarily (or at all) running a module here,
	  // so exit with generic failure.
	  
	  exit(1);
	}
	else
	{
	  // Parent, set ret and return

	  ret = new int[2];

	  if(ret)
	  {
	    // Send output to the first pair
	    close(fds[0]);
	    fds[0] = -1;
	    ret[1] = fds[1];

	    // Read input from the second pair
	    close(fds2[1]);
	    fds2[1] = -1;
	    ret[0] = fds2[0];
	  }
	  else
	    wlog->warn("Executor::exec_pipe failed to allocate ret");
	}
      }
      else
	wlog->warn("fork() failed in Executor::exec_pipe");
    }
    else
      wlog->warn("pipe() failed in Executor::exec_pipe");
  }

  if(!ret)
  {
    // Make sure the pipe is closed.

    closepipe();
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Executor::exec_pipe = %d", ret);
#endif

  return(ret);
}

pid_t Executor::exec_report_begin(ReportFormatting *rf)
{
  // Execute the report module as described in <rf>.  This method must
  // be followed by zero or more calls to exec_report_continue() and
  // then exec_report_end();

  // Returns: The pid of the exec'd process, whose status may be obtained
  // with result(), or -1 on error.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Executor::exec_report_begin(%d)", rf);
#endif

  if(pid == -1 && rf && rf->module())
  {
    // Verify the requested module exists

    char modexec[PATH_MAX];
    memset(modexec, 0, PATH_MAX);
    
    if(strlen(args->moddir()) + strlen(rf->module()) + 9 < PATH_MAX)
      sprintf(modexec, "%s/report/%s", args->moddir(), rf->module());
    else
      return(-1);

    if(access(modexec, X_OK)==-1)
      return(-1);
    
    if(pipe(fds)==0 && pipe(fds2)==0)
    {
      pid = fork();

      if(pid > -1)
      {
	if(pid == 0)
	{
	  // Child, create a new session for use by killpg() (although
	  // these processes may not be timed out).  If this fails, we
	  // might have a few extra defunct processes lying around.

	  setsid();

	  // We use both sets of file descriptors because we have
	  // bidirectional data flow.

	  // First pair is input
	  dup2(fds[0], 0);
	  close(fds[1]);
	  fds[1] = -1;
	  
	  // Second pair is output
	  dup2(fds2[1], 1);
	  close(fds2[0]);
	  fds2[0] = -1;

	  // No need to pass our setgidness (if any) to the process we exec.

	  if(getgid() != getegid())
	    setegid(getgid());
	  
	  // Exec the report module
	  execl(modexec, rf->module(), NULL);
	  
	  // We shouldn't make it here
	  exit(MODEXEC_MISCONFIG);
	}
	else
	{
	  // Parent, pass xml over first fd pair and return.

	  close(fds[0]);
	  fds[0] = -1;
	  close(fds2[1]);
	  fds2[1] = -1;
	  
	  SurvivorXML *sxml = new SurvivorXML();

	  if(sxml)
	  {
	    if(!sxml->generate_report_begin(fds[1], rf))
	      wlog->warn("Executor::exec_report_begin failed to generate XML");

	    xdelete(sxml);
	  }
	  else
	    wlog->warn("Executor::exec_report_begin failed to allocated SurvivorXML");

	  // Note report request
	  rep = rf;
	}
      }
      else
	wlog->warn("fork() failed in Executor::exec_report_begin");
    }
    else
      wlog->warn("pipe() failed in Executor::exec_report_begin");
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Executor::exec_report_begin = %d", pid);
#endif
  
  return(pid);
}

bool Executor::exec_report_continue(HistorySet *hs)
{
  // Output the contents of <hs> to the currently exec'ing report
  // module.  Multiple calls to this method may be made, but the first
  // must be preceeded by a call to exec_report_begin() and the last
  // followed by exec_report_end().

  // Returns: true if fully successful, false otherwise.

  bool ret = false;

  if(rep && pid > -1 && hs)
  {
    SurvivorXML *sxml = new SurvivorXML();

    if(sxml)
    {
      if(sxml->generate(fds[1], hs))
	ret = true;
      else
	wlog->warn("Executor::exec_report_continue failed to generate XML");
      
      xdelete(sxml);
    }
    else
      wlog->warn("Executor::exec_report_continue failed to allocated SurvivorXML");
  }
  
  return(ret);
}

int Executor::exec_report_end()
{
  // Finish exec'ing a report module.  This method must be preceeded
  // by a call to exec_report_begin().

  // Returns: A file descriptor from which results can be read, or -1
  // on error.

  int ret = -1;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Executor::exec_report_end()");
#endif

  if(rep && pid > -1)
  {
    SurvivorXML *sxml = new SurvivorXML();

    if(sxml)
    {
      if(sxml->generate_report_end(fds[1]))
	ret = fds2[0];
      else
	wlog->warn("Executor::exec_report_end failed to generate XML");
      
      xdelete(sxml);
    }
    else
      wlog->warn("Executor::exec_report_end failed to allocated SurvivorXML");
    
    // Close stdout so the module doesn't hang forever on input
    close(fds[1]);
    fds[1] = -1;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Executor::exec_report_end = %d", ret);
#endif
  
  return(ret);
}

pid_t Executor::exec_webauth(CGIAuthRequest *wreq)
{
  // Execute the web authentication module specified in <wreq>.

  // Returns: The pid of the exec'd process, whose status may be obtained
  // with result(CGIAuthResult), or -1 on error.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Executor::exec_webauth(%d)", wreq);
#endif

  if(pid == -1 && wreq)
  {
    if(pipe(fds)==0 && pipe(fds2)==0)
    {
      pid = fork();

      if(pid > -1)
      {
	if(pid == 0)
	{
	  // Child, create a new session for use by killpg() (although
	  // these processes may not be timed out).  If this fails, we
	  // might have a few extra defunct processes lying around.

	  setsid();

	  // We use both sets of file descriptors because we have
	  // bidirectional data flow, with the module knowing when its
	  // input ends by seeing EOF.

	  // First pair is input
	  dup2(fds[0], 0);
	  close(fds[1]);
	  fds[1] = -1;

	  // Second pair is output
	  dup2(fds2[1], 1);
	  close(fds2[0]);
	  fds2[0] = -1;

	  // No need to pass our setgidness (if any) to the process we exec.

	  if(getgid() != getegid())
	    setegid(getgid());
	  
	  // Exec the webauth module

	  char modexec[PATH_MAX];
	  memset(modexec, 0, PATH_MAX);
	  
	  if(wreq->module() && wreq->module()->module() &&
	     (strlen(args->moddir()) + strlen(wreq->module()->module()) + 10)
	     < PATH_MAX)
	  {
	    sprintf(modexec, "%s/webauth/%s", args->moddir(),
		    wreq->module()->module());
	    execl(modexec, wreq->module()->module(), NULL);
	  }

	  // We shouldn't make it here
	  exit(MODEXEC_MISCONFIG);
	}
	else
	{
	  // Parent, pass xml over first fd pair and return.
	  
	  close(fds[0]);
	  fds[0] = -1;
	  close(fds2[1]);
	  fds2[1] = -1;
	  
	  SurvivorXML *sxml = new SurvivorXML();

	  if(sxml)
	  {
	    if(!sxml->generate(fds[1], wreq))
	      wlog->warn("Executor::exec_webauth failed to generate XML");

	    xdelete(sxml);
	  }
	  else
	    wlog->warn("Executor::exec_webauth failed to allocated SurvivorXML");

	  // Close stdout so the module doesn't hang forever on input
	  close(fds[1]);
	  fds[1] = -1;
	  
	  // Note the request
	  
	  ar = wreq;
  	}
      }
      else
	wlog->warn("fork() failed in Executor::exec_webauth");
    }
    else
      wlog->warn("pipe() failed in Executor::exec_webauth");
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Executor::exec_webauth = %d", pid);
#endif
  
  return(pid);
}
				     
int Executor::result()
{
  // Obtain the result from the child process.
  
  // Returns: The return code from the child process, or -1 on error.

  return(result(true));
}

int Executor::result(bool reset)
{
  // Obtain the result from the child process.  <reset> indicates
  // whether or not reset_executor() should automatically be called.

  // Returns: The return code from the child process, or -1 on error.

  int r = -1;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Executor::result(%s)", IOTF(reset));
#endif
  
  if(pid > 0)
  {
    waitpid(pid, &r, 0);
    r = r >> 8;

    closepipe();

    if(reset)
      reset_executor();
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Executor::result = %d", r);
#endif
  
  return(r);
}

int Executor::result(AlertState *as, char *module)
{
  // Obtain the result from the child processes invoked by exec_alert
  // (for <module>) and store it in <as>.  <a> contains information
  // about the alert exec'd.

  // Returns: The return code from the subchild (transmit) process, or
  // -1 on error.

  int r = -1;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Executor::result(%d,%s)",
		   as, IONULL(module));
#endif
  
  if(pid > 0 && as && a && module)
  {
    // Parent, read result.  Unlike Check modules, Alert modules
    // (really Format and Transmit modules) only produce return codes,
    // with 0 = success and 1 = failure.

    r = result(false);
    
    // Write out results as requested

    RecipientSet *rset = a->recipients();

    if(rset)
    {
      for(int i = 0;i < rset->modules();i++)
      {
	if(rset->module(i) && strcmp(module, rset->module(i))==0)
	{
	  as->results_accumulate(r, module, rset->addresses(i));
	  break;
	}
      }
    }

    reset_executor();
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Executor::result = %d", r);
#endif

  return(r);
}

int Executor::result(CGIAuthResult **arptr)
{
  // Obtain the result from the child process invoked by exec_webauth
  // and store it in a newly allocated CGIAuthResult object pointed
  // to by <arptr>.

  // Returns: The return code from the child process, or -1 on error.

  int ret = -1;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Executor::result(%d)", arptr);
#endif

  if(pid > 0 && arptr)
  {
    // Use an XML parser to parse the results into a CGIAuthResult

    SurvivorXML *sxml = new SurvivorXML();

    if(sxml)
    {
      *arptr = sxml->parse_cgiauthresult(fds2[0]);
      
      xdelete(sxml);
    }
    else
      wlog->warn("Executor::result failed to allocate SurvivorXML");

    ret = result(true);
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Executor::result = %d", ret);
#endif

  return(ret);
}

int Executor::result(CheckResult **crptr)
{
  // Obtain one result from the child process invoked by exec_check,
  // and store it in a newly allocated CheckResult object pointed
  // to be <crptr>.

  // Returns: The return code from the child process, or -1 on error.

  int r = -1;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Executor::result(%d)", crptr);
#endif
  
  if(pid > 0 && crptr && c)
  {
    // Use an XML parser to parse the results into a CheckResult

    SurvivorXML *sxml = new SurvivorXML();

    if(sxml)
    {
      *crptr = sxml->parse_checkresult(fds2[0]);
      
      xdelete(sxml);
    }
    else
      wlog->warn("Executor::result failed to allocate SurvivorXML");

    // Get the return value and close the pipe

    r = result(true);
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Executor::result = %d", r);
#endif

  return(r);
}

int Executor::result(FixState *fs)
{
  // Obtain the result from the child process invoked by exec_fix
  // and store it in <fs>.

  // Returns: The return code from the child process, or -1 on error.

  int r = -1;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Executor::result(%d)", fs);
#endif

  if(pid > 0 && fs)
  {
    // parse_results will close fds2[0]
    fs->parse_results(fds2[0]);
    
    waitpid(pid, &r, 0);
    r = r >> 8;
    
    reset_executor();
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Executor::result = %d", r);
#endif

  return(r);
}

Executor::~Executor()
{
  // Deallocate the Executor object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Executor::~Executor()");
#endif

  reset_executor();

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Executor::~Executor()");
#endif
}

char **Executor::allocate_argv(char *cmd)
{
  // Convert the command line <cmd> into an array suitable for passing
  // to execv.  <cmd> must begin with a fully specified path to an
  // executable, and contain optional additional arguments separated
  // by spaces.  (Spaces within an argument may be escaped with a '\'.)

  // Returns: A newly allocated array of newly allocated char pointers
  // if successful, NULL otherwise.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Executor::allocate_argv(%s)", IONULL(cmd));
#endif

  char **ret = NULL;

  if(cmd)
  {
    // To make things easier, duplicate <cmd> but use \001 as a separator.

    char *ncmd = new char[strlen(cmd)+1];

    if(ncmd)
    {
      memset(ncmd, 0, strlen(cmd)+1);

      int i = 0;
      int j = 0;
      int slots = 2;  // argv[0] and terminating NULL

      // While we are counting, also track how many separators we insert.
      
      while(i < strlen(cmd))
      {
	if(cmd[i]=='\\' && cmd[i+1]==' ')
	{
	  ncmd[j] = ' ';
	  i++;
	}
	else if(cmd[i]==' ')
	{
	  ncmd[j] = 001;
	  slots++;
	}
	else
	  ncmd[j] = cmd[i];
	
	i++;
	j++;
      }

      // Now we can allocate the array

      ret = (char **)malloc(sizeof(char *) * slots);

      if(ret)
      {
	char *lasts;
	i = 0;

	char *p = strtok_r(ncmd, "\001", &lasts);

	if(p)
	{
	  // argv[0] is a special case, since we want only the basename
	  // of the command.

	  char *x = strrchr(p, '/');

	  if(x)
	    x++;
	  else
	    x = p;

	  ret[i] = xstrdup(x);
	  i++;

	  // The rest are easy, just keep cycling through.

	  while((p = strtok_r(NULL, "\001", &lasts)) != NULL)
	  {
	    ret[i] = xstrdup(p);
	    i++;
	  }

	  // Add the terminating NULL
	  ret[i] = NULL;
	}
	else
	{
	  free(ret);
	  ret = NULL;
	}
      }

      xdelete(ncmd);
    }
  }
 
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Executor::allocate_argv = %d", ret);
#endif

  return(ret);
}

void Executor::closepipe()
{
  // Close the actual pipe created by any method of Executor.

  // Returns: Nothing.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Executor::closepipe()");
#endif
  
  for(int i = 0;i < 2;i++)
  {
    if(fds[i] > -1)
    {
      close(fds[i]);
      fds[i] = -1;
    }
    if(fds2[i] > -1)
    {
      close(fds2[i]);
      fds2[i] = -1;
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Executor::closepipe()");
#endif
}

void Executor::do_exec_check_child(Check *check)
{
  // Handle the child portion of exec_check, executing a check or
  // transport as described in <check>.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Executor::do_exec_check_child(%d)", check);
#endif

  // Exec the check or the transport

  // No need to pass our setgidness to the process we exec.
	    
  if(getgid() != getegid())
    setegid(getgid());
	    
  char modexec[PATH_MAX];
  memset(modexec, 0, PATH_MAX);
  
  if(check->transport() && check->transport()->module())
  {
    // If a Transport was defined, we exec that instead
    
    if((strlen(args->moddir()) + strlen(check->transport()->module()) + 12)
       < PATH_MAX)
    {
      sprintf(modexec, "%s/transport/%s", args->moddir(),
	      check->transport()->module());
      execl(modexec, check->transport()->module(), NULL);
    }
    else
      wlog->warn("Executor::do_exec_check_child exec string exceeds PATH_MAX");
  }
  else
  {
    // Otherwise, exec the Check directly
	      
    if((strlen(args->moddir()) + strlen(check->module()) + 8) < PATH_MAX)
    {
      sprintf(modexec, "%s/check/%s", args->moddir(), check->module());
      execl(modexec, check->module(), NULL);
    }
    else
      wlog->warn("Executor::exec_check exec string exceeds PATH_MAX");
  }

  wlog->warn("Executor::do_exec_check_child failed to exec %s", modexec);
	    
  exit(MODEXEC_MISCONFIG);
}

void Executor::do_exec_check_parent(int xmlfd, Check *check, CheckRequest *cr)
{
  // Handle the parent portion of exec_check, generating XML on <xmlfd> as
  // appropriate for <check> and <cr>.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Executor::do_exec_check_parent(%d,%d,%d)",
		  xmlfd, check, cr);
#endif

  // Pass the appropriate XML to the child

  SurvivorXML *sxml = new SurvivorXML();

  if(sxml)
  {	  
    if(check->transport() && check->transport()->module())
    {
      // Generate a transport request

      TransportRequest *tr =
	new TransportRequest(cr->hosts(),
			     check->timeout(),
			     check->transport()->modargs(),
			     check->module(),
			     "check",
			     cr->modargs(),
			     false);

      if(tr)
      {
	if(!sxml->generate(xmlfd, tr))
	  wlog->warn("Executor::do_exec_check_parent failed to generate XML");
		  
	xdelete(tr);
      }
      else
	wlog->warn("Executor::do_exec_check_oarent failed to allocate TransportRequest");
    }
    else
    {
      // Generate a check request directly
      
      if(!sxml->generate(xmlfd, cr))
	wlog->warn("Executor::exec_check failed to generate XML");
    }

    xdelete(sxml);
  }
  else
    wlog->warn("Executor::do_exec_check_parent failed to allocate SurvivorXML");

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Executor::do_exec_check_parent()");
#endif
}

void Executor::dump_misconfig(List *hosts, char *msg1, char *msg2)
{
  // Dump a misconfiguration message to stdout suitable for parsing as
  // a check result.  <msg1> and <msg2> are concatenated together and
  // output for <hosts>.

  // Returns: Nothing.

  if(hosts && msg1)
  {
    SurvivorXML *sxml = new SurvivorXML();
    CharBuffer *cb = new CharBuffer();

    if(sxml && cb)
    {
      for(int i = 0;i < hosts->entries();i++)
      {
	if(hosts->retrieve(i))
	{
	  cb->clear();
	  cb->append(msg1);
	  if(msg2)
	    cb->append(msg2);

	  CheckResult *cr = new CheckResult(hosts->retrieve(i),
					    MODEXEC_MISCONFIG,
					    0,
					    cb->str());

	  if(cr)
	  {
	    sxml->generate(STDOUT_FILENO, cr);
	    xdelete(cr);
	  }
	}
      }
    }

    xdelete(sxml);
    xdelete(cb);
  }
}

void Executor::exec_check_components(Check *check, CheckRequest *creq)
{
  // Execute the component Checks that make up the composite <check>
  // and assemble the results into one output line as requested in
  // <cr>.  The CheckRequest timeout is advisory to all the component
  // Checks and is NOT implemented by the Executor object.  Input and
  // output pipes should be set up before this method is called.

  // Returns: This method does not return.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC,
		  "Executor::exec_check_components(%d,%d)", check, creq);
#endif

  // We set up all the forking and piping that we need to coordinate
  // the results.

  // We don't have access to a timeout mechanism here (eg: we may be
  // running via the command line and not the scheduler) so there is
  // no point trying to honor the timeouts of the component checks.

  // It might make sense to throttle the check creation to avoid
  // choking the scheduler host, but composite checks are expected not
  // to consist of large numbers of components, so in this
  // implementation we simply execute all the components in parallel.
  // Note we have a file handle limit to worry about.  We can't
  // iterate serially, since a single failed check on a single failed
  // host could cause the entire composite check to fail for all
  // hosts.

  int erc = MODEXEC_MISCONFIG;

  // Set up a lot of structures to handle the state.

  int numchecks = (check->composite_required() ?
		   check->composite_required()->entries() : 0)
    +
    (check->composite_optional() ? check->composite_optional()->entries() : 0);

  if(numchecks > 0)
  {
    ComponentResults *crs = new ComponentResults(check->composite_required(),
						 check->composite_optional(),
						 creq->hosts()->entries());

    // pids tracks the process IDs of our children
    pid_t *pids = new pid_t[numchecks];
    // Pipes to communicate with our children
    int **pipefds = new int *[numchecks];
    int **pipefds2 = new int *[numchecks];
#if defined(COMP_CHECK_USE_POLL)
    // Poll structures to poll over the pipes to see when data is ready
    struct pollfd *pfds = new struct pollfd[numchecks];
#else
    // fdset used to select over the pipes to see when data is ready
    fd_set fdset;
#endif
    // The Checks we look at, for easy access
    Check **cptrs = new Check *[numchecks];
    // cr is used to read and parse the check results
    ComponentCheckResult *cr = new ComponentCheckResult();

    // This tracks how far we go
    int index = 0;

    // And any errors we see
    char *errors = NULL;

    if(pids && pipefds
#if defined(COMP_CHECK_USE_POLL)
       && pfds
#endif
       && cptrs && cr)
    {
      // Fire off the component checks
      
      for(int i = 0;i < 2;i++)
      {
	Array<Check> *carray = (i == 0 ? check->composite_required() :
				check->composite_optional());

	if(carray)
	{
	  for(int j = 0;j < carray->entries();j++)
	  {
	    Check *c = carray->retrieve(j);

	    if(c)
	    {
	      // Initialize in case we fail at anything
	      pids[index] = -1;
	      pipefds[index] = NULL;
	      pipefds2[index] = NULL;
#if defined(COMP_CHECK_USE_POLL)
	      pfds[index].fd = -1;
	      pfds[index].events = 0;
	      pfds[index].revents = 0;
#endif
	      cptrs[index] = c;

	      // Allocate a pair of ints for the pipe
	      
	      pipefds[index] = new int[2];
	      pipefds2[index] = new int[2];

	      // Sanity check in case something failes
	      if(pipefds[index])
	      {
		pipefds[index][0] = -1;
		pipefds[index][1] = -1;
	      }
	      if(pipefds2[index])
	      {
		pipefds2[index][0] = -1;
		pipefds2[index][1] = -1;
	      }

	      if(pipefds[index] && pipefds2[index])
	      {
		// Pipe the ints
		
		if(pipe(pipefds[index])==0 && pipe(pipefds2[index])==0)
		{
		  // Fork off a child process to run the module
		  
		  pids[index] = fork();

		  if(pids[index] > -1)
		  {
		    if(pids[index] == 0)
		    {
		      // Child, reset stdin and stdout and exec the check
		      // or transport

		      // First pair is input
		      dup2(pipefds[index][0], 0);
		      close(pipefds[index][1]);
		      pipefds[index][1] = -1;

		      // Second pair is output
		      dup2(pipefds2[index][1], 1);
		      close(pipefds2[index][0]);
		      pipefds2[index][0] = -1;

		      do_exec_check_child(c);
		    }
		    else
		    {
		      // Parent, close the parts of the pipe we don't need

		      close(pipefds[index][0]);
		      pipefds[index][0] = -1;
		      close(pipefds2[index][1]);
		      pipefds2[index][1] = -1;
		      
#if defined(COMP_CHECK_USE_POLL)
		      // Create polling structure
		      pfds[index].fd = pipefds2[index][0];
		      pfds[index].events = POLLIN;
		      pfds[index].revents = 0;
#endif

		      // We want the timeout for the composite check,
		      // but the argument set from the component

		      CheckRequest *cr2 = new CheckRequest(creq->hosts(),
							   creq->timeout(),
							   c->modargs(),
							   false);

		      if(cr2)
		      {
			do_exec_check_parent(pipefds[index][1], c, cr2);
			xdelete(cr2);

			// Close fd so module doesn't hang forever on input
			close(pipefds[index][1]);
			pipefds[index][1] = -1;
		      }
		      else
			wlog->warn("Executor::exec_check_components failed to allocate CheckRequest");
		    }
		  }
		  else
		    errors = xstrcat(errors, "fork() failed,");
		}
		else
		  errors = xstrcat(errors, "pipe() failed,");
	      }
	      else
		errors = xstrcat(errors, "Failed to allocate int array,");

	      // Increment for the next go
	      index++;
	    }
	  }
	}
      }

      // Now that we've finished forking off the children, start collecting
      // the results.
      
      if(index > 0)
      {
	int togo = index;

	erc = MODEXEC_OK;

	while(togo > 0)
	{
#if defined(COMP_CHECK_USE_POLL)
	  if(poll(pfds, index, 10) > 0)
	  {
	    for(int i = 0;i < index;i++)
	    {
	      if(pfds[i].revents > 0)
	      {
		char *svc = cptrs[i]->name();
		
		// This is tricky because we aren't guaranteed a full
		// documented is available to read and we don't know
		// how many documents will become available.  So we
		// assume exactly one is ready, if there are more we
		// can get them next time around, if a full document
		// isn't ready we'll block for a second trying to read it.

		char *xmldoc = read_xml_document(pipefds2[i][0],
						 "</SurvivorCheckResult>",
						 1);

		if(xmldoc)
		{
		  if(cr->parse(xmldoc))
		  {
		    crs->add(cr, svc);
		    cr->reset();  // Don't do this until after the add!
		    pfds[i].revents = 0;
		  }

		  xdelete(xmldoc);
		}

		if(crs->finished(svc))
		{
		  close(pipefds2[i][0]);
		  pipefds2[i][0] = -1;
		  togo--;
		}
	      }	      
	    }
	  }
#else
	  // Set up the fdset each time

	  FD_ZERO(&fdset);
	  int maxfd = -1;

	  for(int i = 0;i < index;i++)
	  {
	    if(pipefds2[i][0] > -1)
	    {
	      FD_SET(pipefds2[i][0], &fdset);
	
	      if(pipefds2[i][0] > maxfd)
		maxfd = pipefds2[i][0];
	    }
	  }

	  // Now read over the available results
	  
	  int r = select(maxfd+1, &fdset, NULL, NULL, NULL);

	  if(r > 0)
	  {
	    for(int i = 0;i < index;i++)
	      if(pipefds2[i][0] > -1 && FD_ISSET(pipefds2[i][0], &fdset))
	      {
		char *svc = cptrs[i]->name();

		// This is tricky because we aren't guaranteed a full
		// documented is available to read and we don't know
		// how many documents will become available.  So we
		// assume exactly one is ready, if there are more we
		// can get them next time around, if a full document
		// isn't ready we'll block for a second trying to read it.

		char *xmldoc = read_xml_document(pipefds2[i][0],
						 "</SurvivorCheckResult>",
						 1);

		if(xmldoc)
		{
		  if(cr->parse(xmldoc))
		  {
		    crs->add(cr, svc);
		    cr->reset();  // Don't do this until after the add!
		  }
		  
		  xdelete(xmldoc);
		}

		// FD_CLR can only be called on valid fds, so do this
		// before closing anything.
		
		FD_CLR(pipefds2[i][0], &fdset);

		if(crs->finished(svc))
		{
		  close(pipefds2[i][0]);
		  pipefds2[i][0] = -1;
		  togo--;
		}
	      }
	  }
#endif
	  // Each time through, see if we have any ready results and
	  // render them if found.  Skip this if there were any errors.
	  // We output here to avoid blocking any results on one timed
	  // out host.

	  if(!errors)
	  {
	    // For each host, see if it is ready.  If so, output its
	    // data and mark it as completed so we don't dump it again.

	    SurvivorXML *sxml = new SurvivorXML();

	    if(sxml)
	    {	    
	      for(int i = 0;i < creq->hosts()->entries();i++)
	      {
		if(crs->ready(creq->hosts()->retrieve(i), true))
		{
		  CheckResult *res = crs->result(creq->hosts()->retrieve(i));
		
		  if(res)
		  {
		    // Note exit code if it's the largest one we've seen
		    
		    if(res->rc() > erc)
		      erc = res->rc();
		  }
		  else
		  {
		    res = new CheckResult(creq->hosts()->retrieve(i),
					  MODEXEC_MISCONFIG,
					  0,
					  "Result not found");
		    
		    if(erc < MODEXEC_MISCONFIG)
		      erc = MODEXEC_MISCONFIG;
		  }

		  if(res)
		    sxml->generate(STDOUT_FILENO, res);
		  
		  // We delete res whether we created it or crs did

		  xdelete(res);
		}
	      }

	      xdelete(sxml);
	    }
	    else
	      errors = xstrcat(errors,
			       "Memory allocation error in Executor::exec_check_components");
	  }
	}
      }
      else
	errors = xstrcat(errors, "Nothing productive accomplished");
    }
    else
      errors = xstrcat(errors,
		       "Memory allocation error in Executor::exec_check_components");

    // Output errors if there were any
    
    if(errors)
    {
      // Chop off the last character
			   
      xstrchop(errors);
      dump_misconfig(creq->hosts(), errors, NULL);
    }
    
    // Don't forget to toss all the pipe pointers and close any open
    // file handles (some may already be closed)

    for(int i = 0;i < index;i++)
    {
      if(pipefds[i])
      {
	close(pipefds[i][0]);
	close(pipefds[i][1]);
	      
	xdelete(pipefds[i]);
      }
      if(pipefds2[i])
      {
	close(pipefds2[i][0]);
	close(pipefds2[i][1]);
	      
	xdelete(pipefds2[i]);
      }
    }
    
    xdelete(crs);
    xdelete(pids);
    xdelete(pipefds);
    xdelete(pipefds2);
#if defined(COMP_CHECK_USE_POLL)
    xdelete(pfds);
#endif
    xdelete(cptrs);
    xdelete(cr);
    xdelete(errors);
  }
  else
    dump_misconfig(creq->hosts(), "No component checks defined for ",
		   check->name());
  
  exit(erc);
}

void Executor::exec_format_transmit(Alert *alert, AlertModule *module)
{
  // Using the information in <alert>, run the Format and Transmit
  // modules for <module>.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "Executor::exec_format_transmit(%d,%d)",
		  alert, module);
#endif

  int erc = MODEXEC_MISCONFIG;

  if(alert && module)
  {
    char modexec[PATH_MAX];
    memset(modexec, 0, PATH_MAX);
    
    // Set up a pipe chain

    int fds[2][2];
    pid_t pids[2];
    int r = 0;     // We'll only check the value of the transmit module,
                   // since if format fails transmit won't have much to do

    if(pipe(fds[0])==0   // Used to talk to the format module
       && pipe(fds[1])==0)  // Used to talk to the transmit module
    {
      pids[0] = fork();

      switch(pids[0])
      {
      case 0:
	// Format module reads from fds[0] and writes to fds[1].
	close(fds[0][1]);
	close(fds[1][0]);
	dup2(fds[0][0], 0);
	dup2(fds[1][1], 1);
	if(module->format() &&
	   (strlen(args->moddir()) + strlen(module->format()) + 9) < PATH_MAX)
	{
	  sprintf(modexec, "%s/format/%s", args->moddir(), module->format());
	  execl(modexec, module->format(), NULL);
	}
	exit(MODEXEC_MISCONFIG);
	break;
      case -1:
	wlog->warn("Executor::exec_format_transmit failed to fork for format");
	break;
      default:
	break;
      }

      pids[1] = fork();
      
      switch(pids[1])
      {
      case 0:
	// Transmit module reads from fds[1] and writes (though it
	// doesn't) to stdout
	close(fds[0][0]);
	close(fds[0][1]);
	close(fds[1][1]);
	dup2(fds[1][0], 0);
	if(module->transmit() &&
	   (strlen(args->moddir()) + strlen(module->transmit()) + 11)
	   < PATH_MAX)
	{
	  sprintf(modexec, "%s/transmit/%s", args->moddir(),
		  module->transmit());
	  execl(modexec, module->transmit(), NULL);
	}
	exit(MODEXEC_MISCONFIG);
	break;
      case -1:
	wlog->warn("Executor::exec_format_transmit failed to fork for transmit");
	break;
      default:
	break;
      }

      // We're the parent.  Close off everything we don't need.

      close(fds[0][0]);
      close(fds[1][0]);
      close(fds[1][1]);
      
      // Write out SurvivorAlertData XML, then close fds[0][1] to send EOF

      SurvivorXML *sxml = new SurvivorXML();
      
      if(sxml)
      {
	if(!sxml->generate(fds[0][1], alert, module->name()))
	  wlog->warn("Executor::exec_format_transmit failed to generate XML");

	xdelete(sxml);
      }
      else
	wlog->warn("Executor::exec_format_transmit failed to allocated SurvivorXML");
      
      close(fds[0][1]);

      // Wait for format
      waitpid(pids[0], &r, 0);
      r = r >> 8;
      // Wait for transmit
      waitpid(pids[1], &r, 0);
      r = r >> 8;
      
      erc = r;
    }
    else
      wlog->warn("pipe() failed in Executor::exec_format_transmit");
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "Executor::exec_format_transmit = %d", erc);
#endif
  
  exit(erc);
}

void Executor::reset_executor()
{
  // Reset the Executor for another exec.  This method must be called
  // for an Executor to be reused.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Executor::reset()");
#endif

  a = NULL;
  ar = NULL;
  c = NULL;
  cs = NULL;
  f = NULL;
  h = NULL;
  rep = NULL;
  pid = -1;

  closepipe();

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Executor::reset()");
#endif  
}
