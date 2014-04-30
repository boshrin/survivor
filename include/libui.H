/*
 * libui.H
 *
 * Version: $Revision: 0.12 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2005/11/24 18:24:46 $
 *
 * Copyright (c) 2002 - 2005
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: libui.H,v $
 * Revision 0.12  2005/11/24 18:24:46  benno
 * Add ui_retrieve_matching_state
 *
 * Revision 0.11  2005/11/14 03:54:17  benno
 * Update ui_execute_report
 *
 * Revision 0.10  2005/10/05 01:45:17  benno
 * Add tmod to _notifies()
 *
 * Revision 0.9  2004/11/27 00:55:43  benno
 * Add ui_*_notifies
 *
 * Revision 0.8  2004/08/25 01:51:45  benno
 * Add UI_REPORT_*, ui_execute_report
 *
 * Revision 0.7  2004/03/02 03:48:12  benno
 * Add composite_check_status
 * Pass error pointers
 *
 * Revision 0.6  2003/11/29 05:31:47  benno
 * Changes for who/why (more history recording)
 *
 * Revision 0.5  2003/06/17 15:12:25  benno
 * Add ui_rc_to_text
 *
 * Revision 0.4  2003/04/21 20:28:00  benno
 * Add ui_unquiet_resched
 *
 * Revision 0.3  2003/03/04 20:57:01  benno
 * Add ui_acknowledge
 *
 * Revision 0.2  2003/01/02 04:47:22  benno
 * Add ui_execute_fix
 *
 * Revision 0.1  2002/12/31 04:38:47  benno
 * Initial revision
 *
 */

/* For ui_execute_report, which histories to report on */
#define UI_REPORT_ALERT    1
#define UI_REPORT_CHECK    2
#define UI_REPORT_COMMAND  4
#define UI_REPORT_FIX      8

/* from uiutils.C */
bool ui_acknowledge(Check *check, char *host, char *who, char *why,
		    char **err);
List *ui_calllist_notifies(char *instance, char *calllist, char **tmod,
			   char **err);
int ui_composite_check_status(Check *check);
int ui_composite_check_status(char *host);
int ui_composite_check_status(List *hosts);
int ui_escalate_to(Check *check, char *host, char *who, char *why,
		   char **err);
int ui_execute_fix(Check *check, char *host, CheckState *cs,
		   FixState *fs, char *who, char *why, char **err);
bool ui_execute_report(CharHandler *chout, ReportFormatting *rf, List *hosts,
                       List *checks, int which, time_t from, time_t until,
		       bool backwards, char **err);
bool ui_inhibit(Check *check, char *host, char *who, char *why, char **err);
char *ui_person_notifies(char *instance, char *person, char *via, char **tmod,
			 char **err);
char *ui_rc_to_text(int rc);
char *ui_rc_to_text(int rc, time_t lastcheck, int instances);
bool ui_record_command(Check *check, char *host, char *command,
		       char *who, char *comment);
List *ui_retrieve_matching_state(matchstate_t match);
bool ui_unquiet_resched(Check *check, char *host, bool uninh, char *who,
			char *why, char **err);
