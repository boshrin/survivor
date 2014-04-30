/*
 * Schedule.C: An object to hold schedule definitions.
 *
 * Version: $Revision: 0.18 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2005/12/22 04:04:39 $
 * MT-Level: Safe
 *
 * Copyright (c) 2002 - 2005
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: Schedule.C,v $
 * Revision 0.18  2005/12/22 04:04:39  benno
 * More robust next()
 * Add next_end()
 *
 * Revision 0.17  2005/10/05 13:33:16  benno
 * Yet another intervals() calculation fix
 *
 * Revision 0.16  2005/10/05 03:50:49  benno
 * Adjust calculations for daylight savings
 * Fix missing parentheses in calculation
 *
 * Revision 0.15  2005/09/26 13:41:41  benno
 * Add next(asof) and sort_key()
 *
 * Revision 0.14  2005/08/26 00:36:21  benno
 * Fix logic computing intervals for daily schedules
 *
 * Revision 0.13  2005/07/29 13:22:41  benno
 * Fix logic computing intervals()
 *
 * Revision 0.12  2003/04/09 20:21:28  benno
 * dlog and wlog Debuggers
 * Casting for localtime calls
 *
 * Revision 0.11  2003/04/06 00:06:06  benno
 * Use Debugger
 *
 * Revision 0.10  2003/01/24 18:12:30  benno
 * Add IONULL and IOTF
 *
 * Revision 0.9  2002/12/31 04:33:30  benno
 * Adjust debugging
 *
 * Revision 0.8  2002/10/28 16:01:31  benno
 * add daylight saving adjustment to now(last)
 *
 * Revision 0.7  2002/10/21 20:46:22  benno
 * add intervals for call list rotation
 *
 * Revision 0.6  2002/04/04 20:11:31  benno
 * copyright
 *
 * Revision 0.5  2002/04/03 18:33:03  benno
 * rcsify date
 *
 * Revision 0.4  2002/04/03 18:32:45  benno
 * Bug fix in now(last)
 *
 * Revision 0.3  2002/04/03 18:32:19  benno
 * Bug fix in now()
 *
 * Revision 0.2  2002/04/03 18:31:54  benno
 * Add "never" frequency
 * ove getday and gettime to utils.C
 *
 * Revision 0.1  2002/04/03 18:31:14  benno
 * initial revision
 *
 */

#include "survivor.H"

Schedule::Schedule(char *attime, char *atday)
{
  // Allocate and initialize a new Schedule object.  This object holds
  // a specific time and optionally a day.  <attime> must be in the
  // form HH:MM.  <atday> is a day of the week in English.

  // Returns: A new Schedule object.

  init(attime, atday, NULL, NULL, NULL, NULL, -1, NULL);
}

Schedule::Schedule(int freq, char *frequnit)
{
  // Allocate and initialize a new Schedule object.  This object holds
  // only a frequency, specified as <freq> <frequnit>s.  <freq> is a
  // positive number, <frequnit> is one of seconds, minutes, hours,
  // days, or weeks, where only the first letter is significant.

  // Returns: A new Schedule object.

  init(NULL, NULL, NULL, NULL, NULL, NULL, freq, frequnit);
}

Schedule::Schedule(char *fromtime, char *fromday, char *untiltime,
		   char *untilday, int freq, char *frequnit)
{
  // Allocate and initialize a new Schedule object.  This object
  // holds only a frequency, specified as <freq> <frequnit>s.  <freq> is a
  // positive number, <frequnit> is one of seconds, minutes, hours,
  // days, or weeks, where only the first letter is significant.
  // <fromtime> and <untiltime> are in the form HH:MM and are required.
  // <fromday> and <untilday> are days of the week in English, and are
  // optional.

  // Returns: A new Schedule object.

  init(NULL, NULL, fromtime, fromday, untiltime, untilday, freq, frequnit);
}

bool Schedule::at()
{
  // Determine if this Schedule is an "at" schedule.

  // Returns: true if this is an "at" schedule, false otherwise.

  bool r = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Schedule::at()");
#endif
  
  if(fhour > -1 && fmin > -1 && uhour == -1 && umin == -1)
    r = true;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Schedule::at = %s", IOTF(r));
#endif
  
  return(r);
}

int Schedule::frequency()
{
  // Determine the frequency specified by this schedule.

  // Returns: The frequency in seconds, or -1 on error.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Schedule::frequency()");
  dlog->log_exit(DEBUG_MINTRC, "Schedule::frequency = %d", sfreq);
#endif
  
  return(sfreq);
}

int Schedule::intervals(time_t start, time_t finish)
{
  // For at schedules only, determine how many intervals of the
  // schedule occur starting from <start> through <finish>.

  // Returns: The number of intervals, or 0 on error, including
  // if this is not an at schedule.

  int ret = 0;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Schedule::intervals(%d,%d)", start, finish);
#endif

  if(start > 0 && finish > start && at())
  {
    // This is initially implemented to support call list rotation.
    // To compute who is on call, it is necessary to determine how
    // many rotation periods have passed since the last rotation, and
    // to skip forward that many entries in the list.  For example, if
    // we rotate daily at 12:30, the last rotation was two days ago at
    // 13:30, and it is now 11:30, then we move forward only one
    // because all we missed was yesterday's rotation.

    struct tm lasttm, *lasttp;
    struct tm nowtm, *nowtp;

    nowtp = localtime_r(&finish, &nowtm);
    lasttp = localtime_r(&start, &lasttm);

    if(nowtp && lasttp)
    {
      // First compute the beginning of the last interval.
      // In the above example, the last interval began yesterday
      // at 12:30 and it is now 11:30, so the size of the interval
      // is 23 hours or 82800 seconds.

      int lastint = 0;

      if(fday == -1)
      {
	// Daily rotation, so last interval is simply between now and
	// rotation time.

	if(nowtp->tm_hour > fhour ||
	   (nowtp->tm_hour == fhour && nowtp->tm_min >= fmin))
	{
	  // The last interval was earlier today.

	  // A negative value for nowtp->tm_min - fmin is OK, all
	  // it indicates is a partial hour.

	  lastint = (60 * ((60 * (nowtp->tm_hour - fhour)) +
			   (nowtp->tm_min - fmin)))
	    + nowtp->tm_sec;
	}
	else
	{
	  // The last interval was yesterday, so count to and from midnight

	  lastint = (60 * ((nowtp->tm_hour * 60) + nowtp->tm_min))
	    + nowtp->tm_sec;
	  lastint += 60 * (((23 - fhour) * 60) + (60 - fmin));
	}
      }
      else
      {
	// Weekly rotation.  If we rotate on Friday at 12:00 and it
	// is now Monday at 9:00, then the size of the last interval
	// is 2 days and 21 hours, or 248400 seconds.  86400 sec = 1
	// day.

	// First compute the number of full days in the interval.

	if(nowtp->tm_wday > fday)
	  lastint = (nowtp->tm_wday - fday - 1) * 86400;
	else
	{
	  if(fday > nowtp->tm_wday)
	    lastint = (nowtp->tm_wday - (fday - 7) - 1) * 86400;
	  // else lastint = 0
	}

	if(fday == nowtp->tm_wday)
	{
	  if(nowtp->tm_hour > fhour ||
	     (nowtp->tm_hour == fhour && nowtp->tm_min >= fmin))
	  {
	    // The last interval was earlier today

	    lastint = (60 * ((60 * (nowtp->tm_hour - fhour)) +
			   (nowtp->tm_min - fmin)))
	      + nowtp->tm_sec;
	  }
	  else
	  {
	    // The last interval was almost a week ago
	    
	    lastint = 604800 -
	      ((60 * ((60 * (fhour - nowtp->tm_hour)) +
		      (fmin - nowtp->tm_min)))
	       - nowtp->tm_sec);
	  }
	}
	else
	{
	  // Next, compute the time from fhour:fmin to midnight and
	  // the time from midnight to nowhour:nowmin and add them in.
	
	  lastint += (((23 - fhour) * 60) + (60 - fmin)) * 60;
	  lastint += (((nowtp->tm_hour * 60) + nowtp->tm_min) * 60)
	    + nowtp->tm_sec;
	}
      }

      // If we are calculating across daylight savings, adjust for the
      // hour gained or lost
      
      if(nowtp->tm_isdst && !lasttp->tm_isdst)
	lastint -= 3600;
      else if(!nowtp->tm_isdst && lasttp->tm_isdst)
	lastint += 3600;
      
      // If now minus the last interval is before start, then
      // we don't advance.  Otherwise, starting at now minus the
      // last interval, repeatedly subtract the number of seconds
      // in a day or a week until the remaining value is less than
      // start.  The number of subtractions is the number of
      // positions to advance in the list.

      int x = finish - lastint;

      while(x >= start)
      {
	ret++;
	    
	if(fday == -1)
	  x -= 86400;
	else
	  x -= 604800;
      }
      
      // We now have the number of intervals in ret.
    }
    else
      wlog->warn("Schedule::intervals unable to get localtimes");
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Schedule::intervals = %d", ret);
#endif
 
  return(ret);
}

int Schedule::next(time_t asof)
{
  // Determine the interval from <asof> until the next occurrence of
  // the schedule.

  // Returns: The length of time until the next occurrence, or -1 for
  // no next occurrence.

  int ret = 0;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Schedule::next(%d)", asof);
#endif

  if(!at())
  {
    // If this isn't an at schedule, we first try to compute the next
    // by adding the frequency to <asof> and then verifying that the
    // result is in effect.  If it isn't, we then behave just like an
    // at schedule since the next occurrence is at the next occurrence
    // of the from time.

    if(sfreq < 1)
    {
      // Never

#if defined(DEBUG)
      dlog->log_exit(DEBUG_MINTRC, "Schedule::next = -1");
#endif
 
      return(-1);
    }
    else
    {
      int nextint = asof + sfreq;

      // This is something of a hack because of the design of the
      // now() methods, which were originally designed to always be
      // called "now".

      struct timeval tv;

      if(gettimeofday(&tv, NULL)==0)
      {
	if(now(nextint - tv.tv_sec))
	{
	  // The next interval is the scheduled frequency
	  
#if defined(DEBUG)
	  dlog->log_exit(DEBUG_MINTRC, "Schedule::next = %d", sfreq);
#endif
	  
	  return(sfreq);
	}
	// else fall through
      }
      else
	wlog->warn("Schedule::next failed to gettimeofday");
    }
  }

  // If we make it here, we either have an at schedule or a regular
  // schedule that is not currently in effect.
  
  struct tm nowtm, *nowtp;

  if((nowtp = localtime_r(&asof, &nowtm)) != NULL)
  {
    int nextint = 0;
    
    if(fday == -1)
    {
      // Daily rotation
      
      if(fhour > nowtp->tm_hour ||
	 (fhour == nowtp->tm_hour && fmin > nowtp->tm_min))
      {
	// The next interval is later today
	
	nextint = ((fhour * 3600) + (fmin * 60)) -
	  ((nowtp->tm_hour * 3600) + (nowtp->tm_min * 60) +
	   (nowtp->tm_sec));
      }
      else if(fhour == nowtp->tm_hour && fmin == nowtp->tm_min)
      {
	// The next interval is either now or in exactly 24 hours.
	// We'll go with 24 hours since it is more useful for
	// reporting future occurrences.  (ie: via "sc clcal")

	nextint = 86400;
      }
      else
      {
	// The next interval is tomorrow, calculate from now
	// until midnight and then midnight until the interval
	
	nextint = 86400 - ((nowtp->tm_hour * 3600) + (nowtp->tm_min * 60) +
			   (nowtp->tm_sec));
	nextint += (fhour * 3600) + (fmin * 60);
      }
    }
    else
    {
      // Weekly rotation
      
      // First, compute the number of full days in the interval

      if(nowtp->tm_wday > fday)
	nextint = (fday - (nowtp->tm_wday - 7) - 1) * 86400;
      else
      {
	if(fday > nowtp->tm_wday)
	  nextint = (fday - nowtp->tm_wday - 1) * 86400;
      }

      if(fday == nowtp->tm_wday)
      {
	if(fhour > nowtp->tm_hour ||
	   (fhour == nowtp->tm_hour && fmin > nowtp->tm_min))
	{
	  // The next interval is later today
	  
	  nextint = ((fhour * 3600) + (fmin * 60)) -
	    ((nowtp->tm_hour * 3600) + (nowtp->tm_min * 60) +
	     (nowtp->tm_sec));
	}
	else if(fhour == nowtp->tm_hour && fmin == nowtp->tm_min)
	{
	  // The next interval is either now or in exactly 1 week.
	  // We'll go with 1 week for the same reason as above
	  // (daily rotation calculation).
	  
	  nextint = 604800;
	}
	else
	{
	  // The next interval is in almost a week

	  nextint = 604800 - 
	    (((nowtp->tm_hour * 3600) + (nowtp->tm_min * 60) +
	      (nowtp->tm_sec)) -
	     ((fhour * 3600) + (fmin * 60)));
	}
      }
      else
      {
	// Next, compute the time from now to midnight and then
	// from midnight until the interval

	nextint += 86400 - ((nowtp->tm_hour * 3600) + (nowtp->tm_min * 60) +
			    (nowtp->tm_sec));
	nextint += (fhour * 3600) + (fmin * 60);
      }
    }

    // If we're crossing over daylight savings, add or subtract an hour
    
    struct tm newtm, *newtp;
    time_t newtime = nextint + asof;
    
    if((newtp = localtime_r((time_t*)&newtime, &newtm)) != NULL)
    {
      if(nowtp->tm_isdst && !newtp->tm_isdst)
	nextint += 3600;
      else if(!nowtp->tm_isdst && newtp->tm_isdst)
	nextint -= 3600;
    }
    
    ret = nextint;
  }
  else
    wlog->warn("Schedule::next unable to get time or localtime");

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Schedule::next = %d", ret);
#endif
 
  return(ret);
}

int Schedule::next_end(time_t asof)
{
  // Determine the interval from <asof> until the occurrence of the
  // next end of the schedule.  Note that if a schedule definition ends
  // at 11:59, it is really in effect until 11:59:59, which is the time
  // used for the basis of this calculation.  This method cannot be used
  // with at schedules.

  // Returns: The length of time until the next occurrence, or -1 for
  // no next occurrence.

  int ret = 0;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Schedule::next_end(%d)", asof);
#endif

  if(!at())
  {
    // This is pretty similar to next(), except based on until time
    // instead of from time

    struct tm nowtm, *nowtp;

    if((nowtp = localtime_r(&asof, &nowtm)) != NULL)
    {
      int nextint = 0;
    
      if(uday == -1)
      {
	// Daily rotation
      
	if(uhour > nowtp->tm_hour ||
	   (uhour == nowtp->tm_hour && umin > nowtp->tm_min))
	{
	  // The next interval is later today
	
	  nextint = ((uhour * 3600) + (umin * 60)) -
	    ((nowtp->tm_hour * 3600) + (nowtp->tm_min * 60) +
	     (nowtp->tm_sec));
	}
	else if(uhour == nowtp->tm_hour && umin == nowtp->tm_min)
	{
	  // The next interval is either now or in exactly 24 hours.
	  // We'll go with 24 hours since it is more useful for
	  // reporting future occurrences.  (ie: via "sc clcal")

	  nextint = 86400;
	}
	else
	{
	  // The next interval is tomorrow, calculate from now
	  // until midnight and then midnight until the interval
	  
	  nextint = 86400 - ((nowtp->tm_hour * 3600) + (nowtp->tm_min * 60) +
			     (nowtp->tm_sec));
	  nextint += (uhour * 3600) + (umin * 60);
	}
      }
      else
      {
	// Weekly rotation
	
	// First, compute the number of full days in the interval
	
	if(nowtp->tm_wday > uday)
	  nextint = (uday - (nowtp->tm_wday - 7) - 1) * 86400;
	else
	{
	  if(uday > nowtp->tm_wday)
	    nextint = (uday - nowtp->tm_wday - 1) * 86400;
	}

	if(uday == nowtp->tm_wday)
	{
	  if(uhour > nowtp->tm_hour ||
	     (uhour == nowtp->tm_hour && umin > nowtp->tm_min))
	  {
	    // The next interval is later today
	  
	    nextint = ((uhour * 3600) + (umin * 60)) -
	      ((nowtp->tm_hour * 3600) + (nowtp->tm_min * 60) +
	       (nowtp->tm_sec));
	  }
	  else if(uhour == nowtp->tm_hour && umin == nowtp->tm_min)
	  {
	    // The next interval is either now or in exactly 1 week.
	    // We'll go with 1 week for the same reason as above
	    // (daily rotation calculation).
	  
	    nextint = 604800;
	  }
	  else
	  {
	    // The next interval is in almost a week

	    nextint = 604800 - 
	      (((nowtp->tm_hour * 3600) + (nowtp->tm_min * 60) +
		(nowtp->tm_sec)) -
	       ((uhour * 3600) + (umin * 60)));
	  }
	}
	else
	{
	  // Next, compute the time from now to midnight and then
	  // from midnight until the interval

	  nextint += 86400 - ((nowtp->tm_hour * 3600) + (nowtp->tm_min * 60) +
			      (nowtp->tm_sec));
	  nextint += (uhour * 3600) + (umin * 60);
	}
      }

      // Add 59 seconds to the result since we count through the end
      // of the minute

      nextint += 59;

      // If we're crossing over daylight savings, add or subtract an hour
    
      struct tm newtm, *newtp;
      time_t newtime = nextint + asof;
    
      if((newtp = localtime_r((time_t*)&newtime, &newtm)) != NULL)
      {
	if(nowtp->tm_isdst && !newtp->tm_isdst)
	  nextint += 3600;
	else if(!nowtp->tm_isdst && newtp->tm_isdst)
	  nextint -= 3600;
      }
      
      ret = nextint;
    }
    else
      wlog->warn("Schedule::next unable to get time or localtime");
  }
  else
  {
    // at schedules don't have end times
    
    ret = -1;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Schedule::next_end = %d", ret);
#endif
  
  return(ret);
}

bool Schedule::now(int offset)
{
  // Determine if this Schedule is currently in effect.  A Schedule is
  // currently in effect is (1) it is an "every" schedule whose "from"
  // and "until" components are current, (2) it is an "every" schedule
  // with no "from" component, or (3) it is an "at" schedule.  This
  // method only determines if the Schedule is in effect -- it does
  // not determine if the associated check or alert should be
  // executed.  <offset> is used to adjust when "now" is: a value of -2
  // indicates now should really be two seconds ago, while a value of +60
  // indicates now should really be a minute from now.  An offset of 0
  // means to test for real now.

  // Returns: true if the current time is within this schedule's parameters,
  // false otherwise.

  bool r = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Schedule::now(%d)", offset);
#endif

  // Determine what time it is.
    
  struct timeval tv;
  struct tm tm, *tp;
  
  if(gettimeofday(&tv, NULL)==0)
  {
    tv.tv_sec += offset;
    
    tp = localtime_r((const time_t *)&tv.tv_sec, &tm);
    
    if(tp)
    {
      if(uhour == -1 && umin == -1)
      {
	// "every" schedules with no delimiters are always in effect.
	// For "at" schedules and "every" schedules with no delimiters,
	// until times are not provided.  These schedules are always in
	// effect.
	
	r = true;
      }

      if(fhour > -1 && fmin > -1 && uhour > -1 && umin > -1)
      {
	// To determine if it is now or not, we compute what
	// intervals this schedule refers to, and what time it is
	// now, using minute 0 to mean Sunday midnight.  If the
	// schedule refers to a disjoint period (eg: Friday through
	// Monday), we note that and invert the answer.  We need to
	// recompute this each time if fday is -1, since "today"
	// could change between invocations.
	
	// There are 1440 minutes in a day.
	  
	int fromtime = 0;
	int untiltime = 0;
	  
	if(fday == -1)
	{
	  // This means "daily", so use today
	  
	  fromtime = 1440 * tp->tm_wday;
	}
	else
	  fromtime = 1440 * fday;

	// Set untiltime to the same base as fromtime before adding hours
	// and minutes.
	untiltime = fromtime;
	
	// We don't sanity check the data here because we expect gethour()
	// and getmin() to do it.
	fromtime += (60 * fhour) + fmin;
	
	if(uday != -1)
	  untiltime = 1440 * uday;
	// else
	// If fday is not -1, we were specified as "at".  Otherwise, this
	// is a daily block.
	
	untiltime += (60 * uhour) + umin;
	
	int curtime = (1440 * tp->tm_wday) + (60 * tp->tm_hour) + tp->tm_min;
	
	// Now, if curtime falls between fromtime and untiltime, we match
	
	if((curtime >= fromtime && curtime <= untiltime) ||
	   (curtime <= fromtime && curtime >= untiltime))
	  r = true;
	
	// But if fromtime is greater than untiltime, we negate the answer.
	
	if(fromtime > untiltime)
	  r = !r;
      }
    }
    else
      wlog->warn("Schedule unable to get local time");
  }
  else
    wlog->warn("Schedule unable to get time of day");
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Schedule::now = %s", IOTF(r));
#endif

  return(r);
}

bool Schedule::now(time_t last, int offset)
{
  // Determine if the check or alert associated with this Schedule
  // should be executed.  This method does not determine if this
  // Schedule is in effect.  This Schedule should be executed if (1)
  // it is an "every" schedule and (now - <last>) is greater than the
  // interval specified, or (2) it is an "at" schedule and <last> is
  // prior to the specified time and that time has passed. <offset> is
  // used to adjust when "now" is: a value of -2 indicates now should
  // really be two seconds ago, while a value of +60 indicates now
  // should really be a minute from now.  An offset of 0 means to test
  // for real now.

  // Returns: true if this Schedule should be executed, false otherwise.

  bool r = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_SCHEDS, "Schedule::now(%d,%d)", last, offset);
#endif

  // Determine what time it is.
    
  struct timeval tv;
  struct tm tm, *tp;
  
  if(gettimeofday(&tv, NULL)==0)
  {
    tv.tv_sec += offset;
    
    tp = localtime_r((const time_t *)&tv.tv_sec, &tm);
    
    if(tp)
    {
#if defined(DEBUG)
      dlog->log_progress(DEBUG_SCHEDS, "localtime + offset = %d", tv.tv_sec);
#endif
      
      if(fhour > -1 && fmin > -1 && uhour == -1 && umin == -1)
      {
	// For "at" schedules, return true if (last < next <= now).
	// To do this, figure out the most recent time described by the
	// schedule and use that as next.  If last is before next, return
	// true.  For example, if the schedule says "Mondays @ 12:30" and
	// it is now Monday @ 11:30, the most recent time is last Monday,
	// which we compare to <last>.

	// Note that for "at" schedules we need to operate on a per-minute,
	// not a per-second basis.  Most importantly, tv has to be rounded
	// down (never up) to the minute.  Optionally, last may also be
	// rounded down.

	time_t next = tv.tv_sec - tp->tm_sec;

	if((fhour < tp->tm_hour)
	   || (fhour == tp->tm_hour && fmin < tp->tm_min))
	{
	  // Time described is before now, so subtract appropriate time

	  next -= ((tp->tm_hour - fhour) * 3600);
	  next -= ((tp->tm_min - fmin) * 60);

	  if(fday > -1 && fday != tp->tm_wday)
	  {
	    // Adjust for days depending on whether or not the week has wrapped
	    
	    if(fday < tp->tm_wday)
	      next -= ((tp->tm_wday - fday) * 86400);
	    else
	    {
	      if(tp->tm_wday < fday)
		next -= ((tp->tm_wday - (fday - 7)) * 86400);
	      // else diff = 0
	    }
	  }
	}
	else
	{
	  // Time described is after now, so add the time difference and
	  // then subtract an additional day if next is > now.

	  next += ((fhour - tp->tm_hour) * 3600);
	  next += ((fmin - tp->tm_min) * 60);

	  if(fday > -1 && fday != tp->tm_wday)
	  {
	    if(fday < tp->tm_wday)
	      next -= ((tp->tm_wday - fday) * 86400);
	    else
	    {
	      if(tp->tm_wday < fday)
		next -= ((tp->tm_wday - (fday - 7)) * 86400);
	      // else diff = 0
	    }
	  }

	  if(next > tv.tv_sec)
	  {
	    // Subtract a day if daily or a week if weekly

	    if(fday == -1)
	      next -= 86400;
	    else
	      next -= 604800;
	  }
	}

	// Adjust for daylight savings

	struct tm nexttm, *nexttp;

	nexttp = localtime_r(&next, &nexttm);

	if(nexttp->tm_isdst == 0 && tp->tm_isdst > 0)
	  next += 3600;
	else if(nexttp->tm_isdst > 0 && tp->tm_isdst == 0)
	  next -= 3600;
	
	if(last < next)
	  r = true;
      }
      else
      {
	// For "every" schedules, see if the time since the last check exceeds
	// the frequency.  A frequency of 0 means never, not always.

	if(sfreq > 0 && (tv.tv_sec - last > sfreq))
	  r = true;
      }      
    }
    else
      wlog->warn("Schedule unable to get local time");
  }
  else
    wlog->warn("Schedule unable to get time of day");

#if defined(DEBUG)
  dlog->log_exit(DEBUG_SCHEDS, "Schedule::now = %s", IOTF(r));
#endif

  return(r);
}

int Schedule::sort_key()
{
  // Return a sort key for this Schedule.  next() is used to generate
  // the sort key, so only at schedules may be sorted.

  // Returns: An integer useful for sorting.
  
  // Calculating next() is somewhat expensive, but the complications
  // of caching it aren't worth it since we base on the current time.

  struct timeval tp;

  gettimeofday(&tp, NULL);
  
  return(next(tp.tv_sec));
}

Schedule::~Schedule()
{
  // Deallocate the Schedule object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Schedule::~Schedule()");
#endif

  fday = -1;
  fhour = -1;
  fmin = -1;
  sfreq = -1;
  uday = -1;
  uhour = -1;
  umin = -1;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Schedule::~Schedule");
#endif
}

int Schedule::computefreq(int freq, char *frequnit)
{
  // Compute the frequency specified by <freq> <frequnit>s in terms of
  // seconds.

  // Returns: The frequency in seconds, or -1 on error.

  int r = -1;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Schedule::computefreq(%d,%s)",
		   freq, IONULL(frequnit));
#endif

  if(freq >= 0 && frequnit)
  {
    int m = 1;

    switch(tolower(frequnit[0]))
    {
    case 'm':
      m = 60;
      break;
    case 'h':
      m = 3600;
      break;
    case 'd':
      m = 86400;
      break;
    case 'w':
      m = 604800;
      break;
    case 's':
    default:
      m = 1;
      break;
    }
    
    r = freq * m;
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Schedule::computefreq = %d", r);
#endif

  return(r);
}

void Schedule::init(char *attime, char *atday, char *fromtime, char *fromday,
		    char *untiltime, char *untilday, int freq, char *frequnit)
{
  // Initializer for Constructors.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Schedule::init(%s,%s,%s,%s,%s,%s,%d,%s)",
		   IONULL(attime), IONULL(atday), IONULL(fromtime),
		   IONULL(fromday), IONULL(untiltime), IONULL(untilday),
		   freq, IONULL(frequnit));
#endif

  if(attime || atday)
  {
    // This is the first constructor
    
    fday = getday(atday);
    fhour = gettime(attime, true);
    fmin = gettime(attime, false);
    sfreq = -1;
    uday = -1;
    uhour = -1;
    umin = -1;
  }
  else
  {
    if(fromtime || fromday || untiltime || untilday)
    {
      // This is the third constructor
      
      fday = getday(fromday);
      fhour = gettime(fromtime, true);
      fmin = gettime(fromtime, false);
      sfreq = computefreq(freq, frequnit);
      uday = getday(untilday);
      uhour = gettime(untiltime, true);
      umin = gettime(untiltime, false);
    }
    else
    {
      // This is the second constructor
      
      fday = -1;
      fhour = -1;
      fmin = -1;
      sfreq = computefreq(freq, frequnit);
      uday = -1;
      uhour = -1;
      umin = -1;
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Schedule::init()");
#endif
}
