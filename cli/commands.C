/*
 * commands.C: Information about the commands sc accepts
 *
 * Version: $Revision: 0.7 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2006/10/14 03:08:20 $
 * MT-Level: Safe
 *
 * Copyright (c) 2003 - 2006
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: commands.C,v $
 * Revision 0.7  2006/10/14 03:08:20  benno
 * Add checkcf
 *
 * Revision 0.6  2005/11/24 18:27:32  benno
 * Tweak status_cmd for match version
 *
 * Revision 0.5  2005/09/26 13:59:34  benno
 * Add clset
 *
 * Revision 0.4  2004/08/25 02:02:37  benno
 * Add report_cmd
 *
 * Revision 0.3  2004/06/20 01:05:31  benno
 * Add clunsub
 *
 * Revision 0.2  2004/06/12 02:16:53  benno
 * ack/unack take implied args
 *
 * Revision 0.1  2003/11/29 05:42:09  benno
 * Initial revision
 *
 */

#include "cli.H"

CommandInfo acknowledge_cmd = {
  "acknowledge",
  2,
  1,
  -1,
  sh_implied_args,
  true
};

CommandInfo alerthistory_cmd = {
  "alerthistory",
  6,
  1,
  -1,
  sh_implied_args,
  false
};

CommandInfo archivehistory_cmd = {
  "archivehistory",
  2,
  0,
  -1,
  sh_implied_args,
  false
};

CommandInfo check_cmd = {
  "check",
  2,
  1,
  -1,
  sh_implied_args,
  false
};

CommandInfo checkcf_cmd = {
  "checkcf",
  6,
  0,
  0,
  no_args,
  false
};

CommandInfo checkhistory_cmd = {
  "checkhistory",
  6,
  1,
  -1,
  sh_implied_args,
  false
};

CommandInfo clcal_cmd = {
  "clcal",
  3,
  1,
  -1,
  opaque_args,
  false
};

CommandInfo clprune_cmd = {
  "clprune",
  3,
  1,
  -1,
  opaque_args,
  false
};

CommandInfo clset_cmd = {
  "clset",
  4,
  1,
  1,
  opaque_args,
  false
};

CommandInfo clstat_cmd = {
  "clstat",
  4,
  1,
  -1,
  opaque_args,
  false
};

CommandInfo clsub_cmd = {
  "clsub",
  4,
  1,
  1,
  opaque_args,
  false
};

CommandInfo clunsub_cmd = {
  "clunsub",
  3,
  1,
  1,
  opaque_args,
  false
};

CommandInfo commandhistory_cmd = {
  "commandhistory",
  8,
  1,
  -1,
  sh_implied_args,
  false
};

#if defined(DEBUG)
CommandInfo dtest_cmd = {
  "dtest",
  5,
  0,
  0,
  no_args,
  false
};
#endif

CommandInfo escalate_cmd = {
  "escalate",
  1,
  1,
  -1,
  sh_explicit_args,
  true
};

CommandInfo fix_cmd = {
  "fix",
  1,
  1,
  -1,
  sh_explicit_args,
  true
};

CommandInfo fixhistory_cmd = {
  "fixhistory",
  4,
  1,
  -1,
  sh_implied_args,
  false
};

CommandInfo inhibit_cmd = {
  "inhibit",
  1,
  1,
  -1,
  sh_implied_args,
  true
};

CommandInfo report_cmd = {
  "report",
  3,
  1,
  -1,
  sh_list_args,
  false
};

CommandInfo reschedule_cmd = {
  "reschedule",
  3,
  1,
  -1,
  sh_implied_args,
  false
};

CommandInfo status_cmd = {
  "status",
  1,
  0,
  -1,
  sh_implied_args,
  false
};

CommandInfo trip_cmd = {
  "trip",
  1,
  1,
  -1,
  sh_explicit_args,
  false
};

CommandInfo unacknowledge_cmd = {
  "unacknowledge",
  3,
  1,
  -1,
  sh_implied_args,
  true
};

CommandInfo uninhibit_cmd = {
  "uninhibit",
  3,
  1,
  -1,
  sh_implied_args,
  true
};

CommandInfo *commands[] = {
  &acknowledge_cmd,
  &alerthistory_cmd,
  &archivehistory_cmd,
  &check_cmd,
  &checkcf_cmd,
  &checkhistory_cmd,
  &clcal_cmd,
  &clprune_cmd,
  &clset_cmd,
  &clstat_cmd,
  &clsub_cmd,
  &clunsub_cmd,
  &commandhistory_cmd,
#if defined(DEBUG)
  &dtest_cmd,
#endif
  &escalate_cmd,
  &fix_cmd,
  &fixhistory_cmd,
  &inhibit_cmd,
  &report_cmd,
  &reschedule_cmd,
  &status_cmd,
  &trip_cmd,
  &unacknowledge_cmd,
  &uninhibit_cmd
};

#if defined(DEBUG)
int commandslen = 24;
#else
int commandslen = 23;
#endif
