/*
 * SurvivorStatus.C: survivor status object
 *
 * Version: $Revision: 0.3 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2005/12/22 04:05:06 $
 * MT-Level: Safe
 *
 * Copyright (c) 2003 - 2005
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: SurvivorStatus.C,v $
 * Revision 0.3  2005/12/22 04:05:06  benno
 * Add stalled_check support
 *
 * Revision 0.2  2003/10/06 23:16:55  benno
 * Fix typo
 *
 * Revision 0.1  2003/09/29 13:51:08  benno
 * Initial revision
 *
 */

#include "survivor.H"

SurvivorStatus::SurvivorStatus()
{
  // Allocate and initialize a new SurvivorStatus object.

  // Returns: A new SurvivorStatus object.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "SurvivorStatus::SurvivorStatus()");
  dlog->log_exit(DEBUG_MINTRC, "SurvivorStatus::SurvivorStatus()");
#endif

  merrs = NULL;
  scks = NULL;
  perr = NULL;
  atime = 0;
  ctime = 0;
}

bool SurvivorStatus::add_parse_error(char *err)
{
  // Append <err> to the current parse error string, or set the string
  // if not yet defined.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "SurvivorStatus::add_parse_error(%s)",
		  IONULL(err));
#endif

  if(err)
  {
    perr = xstrcat(perr, err);

    if(perr)
      ret = true;
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "SurvivorStatus::add_parse_error = %s",
		 IOTF(ret));
#endif
  
  return(ret);
}

bool SurvivorStatus::add_module_error(char *modname, char *modtype, char *err)
{
  // Add the error <err> for the module <modname> of type <modtype> to
  // the list of module errors.

  // Returns: true if fully successful, false otherwise.
  
  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "SurvivorStatus::add_module_error(%s,%s,%s)",
		  IONULL(modname), IONULL(modtype), IONULL(err));
#endif

  if(modname && modtype && err)
  {
    if(!merrs)
    {
      // Allocate the holding array if it doesn't exist yet
      
      merrs = new Array<ModuleConfigError>();
    }

    if(merrs)
    {
      ModuleConfigError *mce = new ModuleConfigError(modname, modtype, err);

      if(mce)
      {
	if(merrs->add(mce))
	  ret = true;
	else
	{
	  xdelete(mce);
	}
      }
      else
	wlog->warn("SurvivorStatus::add_module_error failed to allocate MCE");
    }
    else
      wlog->warn("SurvivorStatus::add_module_error failed to allocate Array");
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "SurvivorStatus::add_module_error = %s",
		 IOTF(ret));
#endif
  
  return(ret);
}

bool SurvivorStatus::add_stalled_check(char *service, char *host,
				       time_t lastcheck)
{
  // Add the stalled check <service>@<host> with last check time <lastcheck>
  // to the list of stalled checks.

  // Returns: true if fully successful, false otherwise.
  
  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "SurvivorStatus::add_stalled_check(%s,%s,%d)",
		  IONULL(service), IONULL(host), lastcheck);
#endif

  if(service && host && lastcheck)
  {
    if(!scks)
    {
      // Allocate the holding array if it doesn't exist yet
      
      scks = new Array<StalledCheck>();
    }

    if(scks)
    {
      StalledCheck *sck = new StalledCheck(service, host, lastcheck);

      if(sck)
      {
	if(scks->add(sck))
	  ret = true;
	else
	{
	  xdelete(sck);
	}
      }
      else
	wlog->warn("SurvivorStatus::add_stalled_check failed to allocate sck");
    }
    else
      wlog->warn("SurvivorStatus::add_stalled_check failed to allocate Array");
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "SurvivorStatus::add_stalled_check = %s",
		 IOTF(ret));
#endif
  
  return(ret);
}

time_t SurvivorStatus::last_alert_cycle()
{
  // Obtain the time of the last alert scheduler cycle.

  // Returns: The time of the last alert scheduler cycle.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "SurvivorStatus::last_alert_cycle()");
  dlog->log_exit(DEBUG_MINTRC, "SurvivorStatus::last_alert_cycle = %d",
		 atime);
#endif
  
  return(atime);
}

time_t SurvivorStatus::last_check_cycle()
{
  // Obtain the time of the last check scheduler cycle.

  // Returns: The time of the last check scheduler cycle.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "SurvivorStatus::last_check_cycle()");
  dlog->log_exit(DEBUG_MINTRC, "SurvivorStatus::last_check_cycle = %d",
		 ctime);
#endif
  
  return(ctime);
}

Array<ModuleConfigError> *SurvivorStatus::module_errors()
{
  // Obtain the set of module configuration errors.

  // Returns: An Array of ModuleConfigError objects that should not
  // be modified, or NULL if no configuration errors were found.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "SurvivorStatus::module_errors()");
  dlog->log_exit(DEBUG_MINTRC, "SurvivorStatus::module_errors = %d",
		 merrs);
#endif
  
  return(merrs);
}

char *SurvivorStatus::parse_error()
{
  // Obtain the configuration file parser error(s).

  // Returns: A string containing the parse errors, or NULL if no
  // parse errors were encountered.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "SurvivorStatus::parse_error()");
  dlog->log_exit(DEBUG_MINTRC, "SurvivorStatus::parse_error = %s",
		 IONULL(perr));
#endif
  
  return(perr);
}

bool SurvivorStatus::set_last_cycles(time_t alert, time_t check)
{
  // Set the time of the scheduler runs to <alert> seconds for the
  // alert scheduler and <check> seconds for the <check> scheduler.

  // Returns: true if fully successful, false otherwise.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "SurvivorStatus::set_last_cycles(%d,%d)",
		  alert, check);
#endif
  
  atime = alert;
  ctime = check;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "SurvivorStatus::set_last_cycles = %s",
		 IOTF(true));
#endif
  
  return(true);
}

Array<StalledCheck> *SurvivorStatus::stalled_checks()
{
  // Obtain the set of stalled checks.

  // Returns: An Array of StalledCheck objects that should not
  // be modified, or NULL if no stalled checks were found.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "SurvivorStatus::stalled_checks()");
  dlog->log_exit(DEBUG_MINTRC, "SurvivorStatus::stalled_checks = %d",
		 scks);
#endif
  
  return(scks);
}

SurvivorStatus::~SurvivorStatus()
{
  // Deallocate the SurvivorStatus object.

  // Returns: Nothing.

  xadelete(merrs, ModuleConfigError);
  xadelete(scks, StalledCheck);
  xdelete(perr);
  atime = 0;
  ctime = 0;
}
