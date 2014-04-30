/*
 * typedefs.H
 *
 * Version: $Revision: 0.13 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2007/03/29 12:16:37 $
 *
 * Copyright (c) 2002 - 2007
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: typedefs.H,v $
 * Revision 0.13  2007/03/29 12:16:37  benno
 * Add timeout_unit queueunit_t
 *
 * Revision 0.12  2005/12/22 04:02:27  benno
 * Add match_stalled
 *
 * Revision 0.11  2005/11/24 18:25:21  benno
 * Add matchstate_t
 *
 * Revision 0.10  2004/11/27 00:56:14  benno
 * Add gateway.cf
 *
 * Revision 0.9  2004/08/25 01:52:47  benno
 * Add exec_authz, sort_t
 *
 * Revision 0.8  2004/03/02 03:49:36  benno
 * Add auth[nz] types
 * Add cgi.cf as a config_file
 *
 * Revision 0.7  2003/10/06 23:23:51  benno
 * Add fix_attempt_t
 *
 * Revision 0.6  2003/03/04 20:58:22  benno
 * New _cf, _reply
 *
 * Revision 0.5  2002/12/31 04:41:24  benno
 * fix_lock_t
 *
 * Revision 0.4  2002/05/24 20:55:01  benno
 * add version_t
 *
 * Revision 0.3  2002/04/04 20:00:17  benno
 * copyright
 *
 * Revision 0.2  2002/04/02 22:14:12  benno
 * rcsify date
 *
 * Revision 0.1  2002/04/02 22:13:54  benno
 * initial revision
 *
 */

// The types of authentication results

enum authn_result_t {
  no_authn,
  yes_authn,
  deferred_authn
};

// The types of authorization identifiers

enum authz_type_t {
  user_authz_id,
  group_authz_id
};

// The levels of authorization

enum authz_level_t {
  none_authz,
  read_only_authz,
  exec_authz,
  read_write_authz,
  clipboard_authz,
  admin_authz
};

// The various config files

// Update this in utils.C if config_file_t is also updated.
extern char *_config_files[9];

enum config_file_t {
  none_cf,
  instance_cf,
  calllist_cf,
  sched_cf,
  host_cf,
  check_cf,
  dependency_cf,
  cgi_cf,
  gateway_cf
};

// Types of calllists

enum calllist_t {
  simple_list,
  broadcast_list,
  rotating_list,
  alias_list
};

// Whether to attempt a fix

enum fix_attempt_t {
  no_fix,           // Don't try a fix, even if defined
  if_defined_fix,   // Try a fix if defined, otherwise don't
  always_fix        // Always try a fix, complain if not defined
};

// Types of fix locks

enum fix_lock_t {
  standard_lock,   // lock service@host
  host_lock,       // lock @host
  service_lock     // lock service@
};

// For retrieve_matching_state, the types of matches

enum matchstate_t {
  match_all,            // Match all states
  match_addressed,      // Match ack'd/inhibited
  match_error,          // Match items in error state
  match_escalated,      // Match escalated only
  match_stalled         // Match stalled checks
};

// QueueUnit states

enum queueunit_t {
  unassigned_unit,
  assigned_unit,
  completed_unit,
  timedout_unit
};

// Reply types

enum reply_t {
  unknown_reply,
  acknowledge_reply,
  escalate_reply,
  inhibit_reply
};

// Sort orders

enum sort_t {
  no_sort,
  ascending_sort,
  ascending_caseless_sort,
  descending_sort,
  descending_caseless_sort
};

// Version requests

enum version_t {
  package_version,
  build_version
};
