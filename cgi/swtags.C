/*
 * swtags.C: Information about the tags sw accepts
 *
 * Version: $Revision: 0.3 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2004/08/24 23:12:05 $
 * MT-Level: Safe
 *
 * Copyright (c) 2003 - 2004
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: swtags.C,v $
 * Revision 0.3  2004/08/24 23:12:05  benno
 * time_tag takes parameters
 *
 * Revision 0.2  2004/04/24 14:11:21  benno
 * Add SPLIT tag
 *
 * Revision 0.1  2004/03/02 17:13:03  benno
 * Initial revision
 *
 */

#include "cgi.H"

SWTagInfo alertplan_tag = {
  "ALERTPLAN",
  parameter_tag,
  NULL,
  read_only_authz
};

SWTagInfo alertstatus_tag = {
  "ALERTSTATUS",
  parameter_tag,
  NULL,
  read_only_authz
};

SWTagInfo authlevel_tag = {
  "AUTHLEVEL",
  token_tag,
  NULL,
  none_authz
};

SWTagInfo checkschedule_tag = {
  "CHECKSCHEDULE",
  parameter_tag,
  NULL,
  read_only_authz
};

SWTagInfo checkstatus_tag = {
  "CHECKSTATUS",
  parameter_tag,
  NULL,
  read_only_authz
};

SWTagInfo clipboard_tag = {
  "CLIPBOARD",
  token_tag,
  NULL,
  clipboard_authz
};

SWTagInfo clipemail_tag = {
  "CLIPEMAIL",
  noarg_tag,
  NULL,
  clipboard_authz
};

SWTagInfo clipphone_tag = {
  "CLIPPHONE",
  noarg_tag,
  NULL,
  clipboard_authz
};

SWTagInfo echo_tag = {
  "ECHO",
  opaque_tag,
  NULL,
  none_authz
};

SWTagInfo error_tag = {
  "ERROR",
  opaque_tag,
  NULL,
  none_authz
};

SWTagInfo fixstatus_tag = {
  "FIXSTATUS",
  parameter_tag,
  NULL,
  read_only_authz
};

SWTagInfo flag_tag = {
  "FLAG",
  token_tag,
  NULL,
  none_authz
};

SWTagInfo foreach_tag = {
  "FOREACH",
  token_tag,
  "ENDEACH",
  none_authz
};

SWTagInfo helpfile_tag = {
  "HELPFILE",
  token_tag,
  NULL,
  read_only_authz
};

SWTagInfo hostclass_tag = {
  "HOSTCLASS",
  token_tag,
  NULL,
  read_only_authz
};

SWTagInfo if_tag = {
  "IF",
  token_tag,
  "ENDIF",
  none_authz
};

// if_else_tag is a special tag.  The tag parser won't automatically
// recognize it (if_tag is for that), but is used to parse out nested
// if/else blocks after an if_tag is recognized.

SWTagInfo if_else_tag = {
  "IF",
  token_tag,
  "ELSE",
  none_authz
};

SWTagInfo include_tag = {
  "INCLUDE",
  opaque_tag,
  NULL,
  none_authz
};

SWTagInfo instance_tag = {
  "INSTANCE",
  noarg_tag,
  NULL,
  none_authz
};

SWTagInfo pageset_tag = {
  "PAGESET",
  noarg_tag,
  NULL,
  none_authz
};

SWTagInfo rctext_tag = {
  "RCTEXT",
  token_tag,
  NULL,
  none_authz
};

SWTagInfo referer_tag = {
  "REFERER",
  noarg_tag,
  NULL,
  none_authz
};

SWTagInfo refresh_tag = {
  "REFRESH",
  noarg_tag,
  NULL,
  none_authz
};

SWTagInfo runningstate_tag = {
  "RUNNINGSTATE",
  parameter_tag,
  NULL,
  read_only_authz
};

SWTagInfo set_tag = {
  "SET",
  opaque_tag,
  NULL,
  none_authz
};

SWTagInfo split_tag = {
  "SPLIT",
  token_tag,
  NULL,
  none_authz
};

SWTagInfo time_tag = {
  "TIME",
  parameter_tag,
  NULL,
  none_authz
};

SWTagInfo uri_tag = {
  "URI",
  parameter_tag,
  NULL,
  none_authz
};

SWTagInfo uritags_tag = {
  "URITAGS",
  parameter_tag,
  NULL,
  none_authz
};

SWTagInfo uritop_tag = {
  "URITOP",
  noarg_tag,
  NULL,
  none_authz
};

SWTagInfo username_tag = {
  "USERNAME",
  noarg_tag,
  NULL,
  none_authz
};

SWTagInfo version_tag = {
  "VERSION",
  parameter_tag,
  NULL,
  none_authz
};

// The number of entries in swtags[]
int swtagslen = 31;

SWTagInfo *swtags[] = {
  &alertplan_tag,
  &alertstatus_tag,
  &authlevel_tag,
  &checkschedule_tag,
  &checkstatus_tag,
  &clipboard_tag,
  &clipemail_tag,
  &clipphone_tag,
  &echo_tag,
  &error_tag,
  
  &fixstatus_tag,
  &flag_tag,
  &foreach_tag,
  &helpfile_tag,
  &hostclass_tag,
  &if_tag,
  &include_tag,
  &instance_tag,
  &pageset_tag,
  &rctext_tag,
  
  &referer_tag,
  &refresh_tag,
  &runningstate_tag,
  &set_tag,  
  &split_tag,
  &time_tag,
  &uri_tag,
  &uritags_tag,
  &uritop_tag,
  &username_tag,
  
  &version_tag
};
