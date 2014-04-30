/*
 * SurvivorXML.C: Object to parse and generate XML
 *
 * Version: $Revision: 0.18 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2006/11/16 13:23:23 $
 * MT-Level: Safe, except set_ methods.
 *
 * Copyright (c) 2003 - 2006
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: SurvivorXML.C,v $
 * Revision 0.18  2006/11/16 13:23:23  benno
 * Fix BSD signal handling
 *
 * Revision 0.17  2006/10/17 14:00:24  benno
 * Alert,Check,Fix Status documents to v1.1
 *
 * Revision 0.16  2006/01/23 00:47:57  benno
 * SurvivorCheckResult to 1.1
 * SurvivorReportRequest to 1.2
 *
 * Revision 0.15  2005/12/22 04:03:03  benno
 * SurvivorStatus v1.1
 *
 * Revision 0.14  2005/11/14 03:13:43  benno
 * SurvivorReportRequest v1.1
 *
 * Revision 0.13  2005/10/05 01:44:55  benno
 * SurvivorCallListStatus v1.1
 *
 * Revision 0.12  2005/04/30 21:38:08  benno
 * Add read timeout option
 *
 * Revision 0.11  2005/04/07 02:56:55  benno
 * Support Argument objects
 * Add generate(CheckRequest)
 * Add generate(CheckResult)
 * Add generate(FixRequest)
 * Add generate(FixResult)
 * Add generate(TransportRequest)
 * New parse routines for same
 * parse_document from buffer
 *
 * Revision 0.10  2004/12/05 23:37:08  benno
 * Use escape_xml
 *
 * Revision 0.9  2004/08/24 22:39:38  benno
 * Add generate(HistorySet), generate_report_begin, generate_report_end
 *
 * Revision 0.8  2004/06/20 01:06:17  benno
 * Fix typo and a bug in Substitution generation
 *
 * Revision 0.7  2004/06/12 01:02:20  benno
 * Add support for State formatted as XML
 *
 * Revision 0.6  2004/03/01 23:40:01  benno
 * Add generate(CGIAuthRequest) and parse_cgiauthresult
 * Call parse_document and new wrapper functions
 *
 * Revision 0.5  2003/11/02 21:38:53  benno
 * Substitute < and > with &lt; and &gt;
 *
 * Revision 0.4  2003/10/20 00:02:57  benno
 * Add module to generate(Alert) to only output RecipientSet information
 * relevant for transmit module being executed
 *
 * Revision 0.3  2003/10/06 23:19:16  benno
 * Changes to SurvivorAlertData, SurvivorFormattedAlertData
 * generate SurvivorStatus
 * 6 arg begin_element
 *
 * Revision 0.2  2003/07/30 21:41:27  benno
 * Add generate(FormattedAlert)
 *
 * Revision 0.1  2003/05/29 00:38:11  benno
 * Initial revision
 *
 */

#include "survivor.H"

#define MAYBEKNOWN(x) (char *)(x ? x : "UNKNOWN"), (x ? strlen(x) : 7)
#define XMLDOCTAG "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n"

void _SurvivorXML_alarm_catcher(int i) {}

SurvivorXML::SurvivorXML()
{
  // Allocate and initialize a new SurvivorXML object.

  // Returns: A new, initialized SurvivorXML object

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "SurvivorXML::SurvivorXML()");
  dlog->log_exit(DEBUG_MAJTRC, "SurvivorXML::SurvivorXML()");
#endif
  
  rto = 0;  // Default no read timeout
}

bool SurvivorXML::generate(int fd, Acknowledgement *ad)
{
  // Generate an XML SurvivorAlertData document on <fd> based on the
  // contents of <ad>.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "SurvivorXML::generate(%d,%d)", fd, ad);
#endif

  if(ad && fd > -1)
  {
    begin_document(fd);
    begin_element(fd, "SurvivorAlertAcknowledgement", "Version", "1.0");
    
    write_element(fd, "Who", MAYBEKNOWN(ad->who()));

    if(ad->why())
      write_element(fd, "Why", ad->why(), strlen(ad->why()));

    end_element(fd, "SurvivorAlertAcknowledgement");
    end_document(fd);
    
    ret = true;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "SurvivorXML::generate = %s", IOTF(ret));
#endif
  
  return(ret);
}

bool SurvivorXML::generate(int fd, Alert *a, char *module)
{
  // Generate an XML SurvivorAlertData document on <fd> based on the
  // contents of <a>.  If <module> is provided, only generate address
  // information for that module.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "SurvivorXML::generate(%d,%d,%s)",
		  fd, a, IONULL(module));
#endif

  if(a && fd > -1)
  {
    begin_document(fd);
    begin_element(fd, "SurvivorAlertData", "Version", "1.1",
		  "ReplyOK", (char *)(a->fix() ? "no" : "yes"));
    begin_element(fd, "RecipientSet");

    if(a->recipients())
    {
      // At least one module stanza is required, but if we don't have
      // any data it's not clear what we should write.

      for(int i = 0;i < a->recipients()->modules();i++)
      {
	char *mname = a->recipients()->module(i);

	if(mname && (!module || strcmp(mname, module)==0))
	{
	  begin_element(fd, "Module", "Name", mname);
	  
	  List *as = a->recipients()->address_list(i);

	  if(as && as->entries() > 0)
	  {
	    for(int j = 0;j < as->entries();j++)
	    {
	      char *s = as->retrieve(j);
	      
	      if(s)
		write_element(fd, "Address", s, strlen(s));
	    }
	  }

	  List *cs = a->recipients()->calllist_list(i);

	  if(cs && cs->entries() > 0)
	  {
	    for(int j = 0;j < cs->entries();j++)
	    {
	      char *s = cs->retrieve(j);
	      
	      if(s)
		write_element(fd, "CallList", s, strlen(s));
	    }
	  }
	  
	  end_element(fd, "Module");
	}
      }
    }

    end_element(fd, "RecipientSet");

    // Summary is required
    write_element(fd, "Summary", MAYBEKNOWN(a->summary()));

    if(a->helpfile())
      write_element(fd, "HelpFile", a->helpfile(), strlen(a->helpfile()));

    // Host is required, and the dtd allows more than one
    write_element(fd, "Host", MAYBEKNOWN(a->host()));

    write_element(fd, "Instance", MAYBEKNOWN(a->instance()));
    write_element(fd, "Instances", a->instances());

    if(a->fix_summary())
    {
      write_element(fd, "FixSummary", a->fix_summary(),
		    strlen(a->fix_summary()));
      write_element(fd, "FixReturnCode", a->fix_retval());
    }

    write_element(fd, "ReturnCode", a->retval());
    write_element(fd, "Service", MAYBEKNOWN(a->service()));
    write_element(fd, "Time", a->checktime());
    write_element(fd, "Token", MAYBEKNOWN(a->token()));
    
    end_element(fd, "SurvivorAlertData");
    end_document(fd);

    ret = true;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "SurvivorXML::generate = %s", IOTF(ret));
#endif
  
  return(ret);
}

bool SurvivorXML::generate(int fd, AlertStateData *ad)
{
  // Generate an XML SurvivorAlertStatus document on <fd> based
  // on the contents of <ad>.

  // Returns: true if fully successful, false otherwise.
  
  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "SurvivorXML::generate(%d,%d)", fd, ad);
#endif

  if(ad && fd > -1)
  {
    begin_document(fd);

    begin_element(fd, "SurvivorAlertStatus", "Version", "1.1");

    begin_element(fd, "RecipientSet");

    if(ad->recipients())
    {
      // At least one module stanza is required, but if we don't have
      // any data it's not clear what we should write.

      for(int i = 0;i < ad->recipients()->modules();i++)
      {
	char *mname = ad->recipients()->module(i);

	if(mname)
	{
	  begin_element(fd, "Module", "Name", mname);
	  
	  List *as = ad->recipients()->address_list(i);

	  if(as && as->entries() > 0)
	  {
	    for(int j = 0;j < as->entries();j++)
	    {
	      char *s = as->retrieve(j);
	      
	      if(s)
		write_element(fd, "Address", s, strlen(s));
	    }
	  }

	  write_element(fd, "ReturnCode", ad->recipients()->rc(i));
	  
	  end_element(fd, "Module");
	}
      }
    }

    end_element(fd, "RecipientSet");

    write_element(fd, "Instances", ad->instances());
    write_element(fd, "CheckReturnCode", ad->returncode());
    write_element(fd, "CheckSummary", MAYBEKNOWN(ad->summary()));
    write_element(fd, "Time", ad->time());
    write_element(fd, "Since", ad->since());

    end_element(fd, "SurvivorAlertStatus");
    end_document(fd);

    ret = true;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "SurvivorXML::generate = %s", IOTF(ret));
#endif

  return(ret);
}

bool SurvivorXML::generate(int fd, CallListStateData *cd)
{
  // Generate an XML SurvivorCallListStatus document on <fd> based on the
  // contents of <cd>.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "SurvivorXML::generate(%d,%d)", fd, cd);
#endif

  if(cd && fd > -1)
  {
    begin_document(fd);
    begin_element(fd, "SurvivorCallListStatus", "Version", "1.1");

    if(cd->last_notified_person() && cd->last_notified_address() &&
       cd->last_notified_via())
    {
      begin_element(fd, "LastNotify");
      write_element(fd, "Person", cd->last_notified_person(),
		    strlen(cd->last_notified_person()));
      write_element(fd, "Address", cd->last_notified_address(),
		    strlen(cd->last_notified_address()));
      write_element(fd, "Module", cd->last_notified_via(),
		    strlen(cd->last_notified_via()));
      end_element(fd, "LastNotify");
    }
    
    if(cd->last_rotated() > 0)
      write_element(fd, "LastRotate", cd->last_rotated());

    if(cd->oncall_person())
      write_element(fd, "PersonOnCall", cd->oncall_person(),
		    strlen(cd->oncall_person()));

    end_element(fd, "SurvivorCallListStatus");
    end_document(fd);
    
    ret = true;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "SurvivorXML::generate = %s", IOTF(ret));
#endif
  
  return(ret);
}

bool SurvivorXML::generate(int fd, CGIAuthRequest *ar)
{
  // Generate an XML SurvivorWebAuthRequest document on <fd> based
  // on the contents of <ar>.

  // Returns: true if fully successful, false otherwise.
  
  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "SurvivorXML::generate(%d,%d)", fd, ar);
#endif

  if(ar && fd > -1)
  {
    begin_document(fd);

    begin_element(fd, "SurvivorWebAuthRequest", "Version", "1.0");

    write_element(fd, "RequestedURI", MAYBEKNOWN(ar->uri()));

    if(ar->ip())
      write_element(fd, "SourceIP", ar->ip(), strlen(ar->ip()));

    if(ar->module() && ar->module()->modopts())
    {
      for(int i = 0;i < ar->module()->modopts()->entries();i++)
      {
	Argument *a = ar->module()->modopts()->retrieve(i);

	if(a && a->name() && a->value())
	{
	  begin_element(fd, "ModuleOption", "OptionName", a->name());
	  write_element(fd, "OptionValue", a->value(), strlen(a->value()));
	  end_element(fd, "ModuleOption");
	}
      }
    }
    
    end_element(fd, "SurvivorWebAuthRequest");
    end_document(fd);

    ret = true;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "SurvivorXML::generate = %s", IOTF(ret));
#endif

  return(ret);
}

bool SurvivorXML::generate(int fd, CheckRequest *cr)
{
  // Generate an XML SurvivorCheckData document on <fd> based
  // on the contents of <cr>.

  // Returns: true if fully successful, false otherwise.
  
  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "SurvivorXML::generate(%d,%d)", fd, cr);
#endif

  if(cr && fd > -1)
  {
    begin_document(fd);
    
    begin_element(fd, "SurvivorCheckData", "Version", "1.0");

    if(cr->hosts())
    {
      // This is required, we should dump something even if no hosts
      // were provided, but since the module can't do anything without
      // hosts an error is OK.

      for(int i = 0;i < cr->hosts()->entries();i++)
	write_element(fd, "Host", MAYBEKNOWN(cr->hosts()->retrieve(i)));
    }

    write_element(fd, "Timeout", cr->timeout());

    if(cr->modargs())
    {
      for(int i = 0;i < cr->modargs()->entries();i++)
      {
	Argument *ar = cr->modargs()->retrieve(i);

	if(ar && ar->name() && ar->value())
	{
	  begin_element(fd, "ModuleOption", "OptionName", ar->name());
	  write_element(fd, "OptionValue", ar->value(), strlen(ar->value()));
	  end_element(fd, "ModuleOption");
	}
      }
    }
    
    end_element(fd, "SurvivorCheckData");
    end_document(fd);

    ret = true;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "SurvivorXML::generate = %s", IOTF(ret));
#endif

  return(ret);
}

bool SurvivorXML::generate(int fd, CheckResult *cr)
{
  // Generate an XML SurvivorCheckData document on <fd> based
  // on the contents of <cr>.

  // Returns: true if fully successful, false otherwise.
  
  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "SurvivorXML::generate(%d,%d)", fd, cr);
#endif

  if(cr && fd > -1)
  {
    begin_document(fd);
    
    begin_element(fd, "SurvivorCheckResult", "Version", "1.1");

    write_element(fd, "Host", MAYBEKNOWN(cr->hostname()));
    write_element(fd, "ReturnCode", cr->rc());
    write_element(fd, "Scalar", cr->scalar());
    write_element(fd, "Comment", MAYBEKNOWN(cr->comment()));

    if(cr->duration() > -1)
      write_element(fd, "Duration", cr->duration());
    
    end_element(fd, "SurvivorCheckResult");
    end_document(fd);

    ret = true;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "SurvivorXML::generate = %s", IOTF(ret));
#endif

  return(ret);
}

bool SurvivorXML::generate(int fd, CheckStateData *cd)
{
  // Generate an XML SurvivorCheckStatus document on <fd> based
  // on the contents of <cd>.

  // Returns: true if fully successful, false otherwise.
  
  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "SurvivorXML::generate(%d,%d)", fd, cd);
#endif

  if(cd && fd > -1)
  {
    begin_document(fd);

    begin_element(fd, "SurvivorCheckStatus", "Version", "1.1");

    write_element(fd, "Instances", cd->instances());
    write_element(fd, "ReturnCode", cd->returncode());
    write_element(fd, "Summary", MAYBEKNOWN(cd->summary()));
    write_element(fd, "Time", cd->time());
    write_element(fd, "Since", cd->since());

    if(cd->duration() > -1)
      write_element(fd, "Duration", cd->duration());

    end_element(fd, "SurvivorCheckStatus");
    end_document(fd);

    ret = true;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "SurvivorXML::generate = %s", IOTF(ret));
#endif

  return(ret);
}

bool SurvivorXML::generate(int fd, Escalation *ed)
{
  // Generate an XML SurvivorAlertEscalation document on <fd> based on
  // the contents of <ed>.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "SurvivorXML::generate(%d,%d)", fd, ed);
#endif

  if(ed && fd > -1)
  {
    begin_document(fd);
    begin_element(fd, "SurvivorAlertEscalation", "Version", "1.0");
    
    write_element(fd, "EscalateTo", ed->escalated_to());

    end_element(fd, "SurvivorAlertEscalation");
    end_document(fd);
    
    ret = true;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "SurvivorXML::generate = %s", IOTF(ret));
#endif
  
  return(ret);
}

bool SurvivorXML::generate(int fd, FixRequest *fr)
{
  // Generate an XML SurvivorFixData document on <fd> based
  // on the contents of <fr>.

  // Returns: true if fully successful, false otherwise.
  
  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "SurvivorXML::generate(%d,%d)", fd, fr);
#endif

  if(fr && fd > -1)
  {
    begin_document(fd);
    
    begin_element(fd, "SurvivorFixData", "Version", "1.0");

    if(fr->modargs())
    {
      write_element(fd, "TimeOut", fr->timeout());
      
      for(int i = 0;i < fr->modargs()->entries();i++)
      {
	Argument *ar = fr->modargs()->retrieve(i);

	if(ar && ar->name() && ar->value())
	{
	  begin_element(fd, "ModuleOption", "OptionName", ar->name());
	  write_element(fd, "OptionValue", ar->value(), strlen(ar->value()));
	  end_element(fd, "ModuleOption");
	}
      }
    }
    
    end_element(fd, "SurvivorFixData");
    end_document(fd);

    ret = true;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "SurvivorXML::generate = %s", IOTF(ret));
#endif

  return(ret);
}

bool SurvivorXML::generate(int fd, FixResult *fr)
{
  // Generate an XML SurvivorFixResult document on <fd> based on the
  // contents of <fr>.

  // Returns: true if fully successful, false otherwise.
  
  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "SurvivorXML::generate(%d,%d)", fd, fr);
#endif

  if(fr && fd > -1)
  {
    begin_document(fd);
    
    begin_element(fd, "SurvivorFixResult", "Version", "1.0");

    write_element(fd, "ReturnCode", fr->rc());
    write_element(fd, "Comment", MAYBEKNOWN(fr->comment()));
    
    end_element(fd, "SurvivorFixResult");
    end_document(fd);

    ret = true;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "SurvivorXML::generate = %s", IOTF(ret));
#endif

  return(ret);
}

bool SurvivorXML::generate(int fd, FixStateData *xd)
{
  // Generate an XML SurvivorFixStatus document on <fd> based
  // on the contents of <xd>.

  // Returns: true if fully successful, false otherwise.
  
  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "SurvivorXML::generate(%d,%d)", fd, xd);
#endif

  if(xd && fd > -1)
  {
    begin_document(fd);

    begin_element(fd, "SurvivorFixStatus", "Version", "1.1");

    write_element(fd, "InitiatedBy", MAYBEKNOWN(xd->initiatedby()));
    write_element(fd, "Instances", xd->instances());
    write_element(fd, "ReturnCode", xd->returncode());
    write_element(fd, "Summary", MAYBEKNOWN(xd->summary()));
    write_element(fd, "Time", xd->time());
    write_element(fd, "Since", xd->since());

    end_element(fd, "SurvivorFixStatus");
    end_document(fd);

    ret = true;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "SurvivorXML::generate = %s", IOTF(ret));
#endif

  return(ret);
}

bool SurvivorXML::generate(int fd, FormattedAlert *f)
{
  // Generate an XML SurvivorFormattedAlertData document on <fd> based
  // on the contents of <f>.

  // Returns: true if fully successful, false otherwise.
  
  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "SurvivorXML::generate(%d,%d)", fd, f);
#endif

  if(f && fd > -1)
  {
    begin_document(fd);

    begin_element(fd, "SurvivorFormattedAlertData", "Version", "2.0",
		  "ReplyOK", (char *)(f->replyok() ? "yes" : "no"));
    
    if(f->addresses())
    {
      // At least one address is required, but if we don't have any
      // it's not clear what we should write.

      for(int i = 0;i < f->addresses()->entries();i++)
      {
	char *e = f->addresses()->retrieve(i);

	if(e)
	  write_element(fd, "Address", e, strlen(e));
      }

      if(f->replyto())
	write_element(fd, "ReplyTo", f->replyto(), strlen(f->replyto()));

      write_element(fd, "Subject", MAYBEKNOWN(f->subject()));
      write_element(fd, "Message", MAYBEKNOWN(f->message()));
    }
    
    end_element(fd, "SurvivorFormattedAlertData");
    end_document(fd);
    
    ret = true;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "SurvivorXML::generate = %s", IOTF(ret));
#endif
  
  return(ret);
}

bool SurvivorXML::generate(int fd, HistorySet *hs)
{
  // Generate a DataSet element for an XML SurvivorReportRequest
  // document on <fd> using the contents of <hs>.
  // generate_report_begin() should be called before this method is
  // invoked, and generate_report_end() should be called when no
  // further generate(HistorySet) invocations will be called for the
  // document.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "SurvivorXML::generate(%d,%d)", fd, hs);
#endif

  if(hs && fd > -1)
  {
    begin_element(fd, "DataSet", "Host", hs->host(), "Service", hs->service());

    // Start with AlertData, then run through the rest

    for(AlertHistoryRecord *ahr = hs->iterate_alerthistory();
	ahr != NULL;
	ahr = hs->iterate_alerthistory())
    {
      char *t = xnumdup(ahr->timestamp());
      char *arc = xnumdup(ahr->alertrv());
      char *crc = xnumdup(ahr->checkrv());

      begin_element(fd, "AlertData");
      write_element(fd, "Time", MAYBEKNOWN(t));
      write_element(fd, "AlertRC", MAYBEKNOWN(arc));
      write_element(fd, "CheckRC", MAYBEKNOWN(crc));
      write_element(fd, "Recipient", MAYBEKNOWN(ahr->who()));
      write_element(fd, "Via", MAYBEKNOWN(ahr->via()));
      end_element(fd, "AlertData");

      xdelete(t);
      xdelete(arc);
      xdelete(crc);
      
      xdelete(ahr);
    }

    for(CheckHistoryRecord *chr = hs->iterate_checkhistory();
	chr != NULL;
	chr = hs->iterate_checkhistory())
    {
      char *t = xnumdup(chr->timestamp());
      char *crc = xnumdup(chr->checkrv());
      char *s = xnumdup(chr->scalar());

      begin_element(fd, "CheckData");
      write_element(fd, "Time", MAYBEKNOWN(t));
      write_element(fd, "CheckRC", MAYBEKNOWN(crc));
      write_element(fd, "Scalar", MAYBEKNOWN(s));
      write_element(fd, "Comment", MAYBEKNOWN(chr->comment()));
      if(chr->duration() > -1)
	write_element(fd, "Duration", chr->duration());
      end_element(fd, "CheckData");

      xdelete(t);
      xdelete(crc);
      xdelete(s);
      
      xdelete(chr);
    }

    for(CommandHistoryRecord *chr = hs->iterate_commandhistory();
	chr != NULL;
	chr = hs->iterate_commandhistory())
    {
      char *t = xnumdup(chr->timestamp());

      begin_element(fd, "CommandData");
      write_element(fd, "Time", MAYBEKNOWN(t));
      write_element(fd, "Command", MAYBEKNOWN(chr->command()));
      write_element(fd, "Who", MAYBEKNOWN(chr->who()));
      write_element(fd, "Comment", MAYBEKNOWN(chr->comment()));
      end_element(fd, "CommandData");

      xdelete(t);
      
      xdelete(chr);
    }

    for(FixHistoryRecord *fhr = hs->iterate_fixhistory();
	fhr != NULL;
	fhr = hs->iterate_fixhistory())
    {
      char *t = xnumdup(fhr->timestamp());
      char *frc = xnumdup(fhr->fixrv());

      begin_element(fd, "FixData");
      write_element(fd, "Time", MAYBEKNOWN(t));
      write_element(fd, "FixRC", MAYBEKNOWN(frc));
      write_element(fd, "Who", MAYBEKNOWN(fhr->who()));
      write_element(fd, "Comment", MAYBEKNOWN(fhr->comment()));
      end_element(fd, "FixData");

      xdelete(t);
      xdelete(frc);
      
      xdelete(fhr);
    }

    end_element(fd, "DataSet");
   
    ret = true;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "SurvivorXML::generate = %s", IOTF(ret));
#endif
  
  return(ret);
}

bool SurvivorXML::generate(int fd, Inhibition *id)
{
  // Generate an XML SurvivorAlertInhibition document on <fd> based on
  // the contents of <id>.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "SurvivorXML::generate(%d,%d)", fd, id);
#endif

  if(id && fd > -1)
  {
    begin_document(fd);
    begin_element(fd, "SurvivorAlertInhibition", "Version", "1.0");
    
    write_element(fd, "Who", MAYBEKNOWN(id->who()));

    if(id->why())
      write_element(fd, "Why", id->why(), strlen(id->why()));

    end_element(fd, "SurvivorAlertInhibition");
    end_document(fd);
    
    ret = true;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "SurvivorXML::generate = %s", IOTF(ret));
#endif
  
  return(ret);
}

bool SurvivorXML::generate(int fd, Array<Substitution> *sa)
{
  // Generate an XML SurvivorCallListSubstitutions document on <fd>
  // based on the contents of <sa>.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "SurvivorXML::generate(%d,%d)", fd, sa);
#endif

  if(sa && fd > -1)
  {
    begin_document(fd);
    begin_element(fd, "SurvivorCallListSubstitutions", "Version", "1.0");

    for(int i = 0;i < sa->entries();i++)
    {
      Substitution *s = sa->retrieve(i);

      if(s)
      {
	begin_element(fd, "Substitution");
	write_element(fd, "Begin", s->begins());
	write_element(fd, "End", s->ends());
	write_element(fd, "Original", MAYBEKNOWN(s->oldname()));
	write_element(fd, "Replacement", MAYBEKNOWN(s->newname()));
	end_element(fd, "Substitution");
      }
    }

    end_element(fd, "SurvivorCallListSubstitutions");
    end_document(fd);
    
    ret = true;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "SurvivorXML::generate = %s", IOTF(ret));
#endif
  
  return(ret);
}

bool SurvivorXML::generate(int fd, SurvivorStatus *s)
{
  // Generate an XML SurvivorStatus document on <fd> based on the
  // contents of <s>.

  // Returns: true if fully successful, false otherwise.
  
  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "SurvivorXML::generate(%d,%d)", fd, s);
#endif

  if(s && fd > -1)
  {
    begin_document(fd);
    begin_element(fd, "SurvivorStatus", "Version", "1.1");

    begin_element(fd, "ConfigurationStatus",
		  "ParseOK", (char *)(s->parse_error() ? "no" : "yes"));

    Array<ModuleConfigError> *mces = s->module_errors();

    if(mces)
    {
      for(int i = 0;i < mces->entries();i++)
      {
	ModuleConfigError *mce = mces->retrieve(i);

	if(mce && mce->modname() && mce->modtype())
	{
	  begin_element(fd, "ModuleError",
			"ModuleName", mce->modname(),
			"ModuleType", mce->modtype());

	  write_element(fd, "ErrorDetail", MAYBEKNOWN(mce->moderr()));
	  
	  end_element(fd, "ModuleError");
	}
      }
    }
    
    if(s->parse_error())
      write_element(fd, "ParserError", s->parse_error(),
		    strlen(s->parse_error()));
    
    end_element(fd, "ConfigurationStatus");

    char *at = xnumdup(s->last_alert_cycle());
    char *ct = xnumdup(s->last_check_cycle());

    begin_element(fd, "SchedulerStatus");
    write_element(fd, "LastAlertCycle", MAYBEKNOWN(at));
    write_element(fd, "LastCheckCycle", MAYBEKNOWN(ct));
    end_element(fd, "SchedulerStatus");

    begin_element(fd, "InstanceStatus");

    Array<StalledCheck> *scks = s->stalled_checks();
    
    if(scks)
    {
      for(int i = 0;i < scks->entries();i++)
      {
	StalledCheck *sck = scks->retrieve(i);

	if(sck && sck->service() && sck->host())
	{
	  char *lc = xnumdup(sck->lastcheck());
	  
	  begin_element(fd, "StalledCheck",
			"Service", sck->service(),
			"Host", sck->host());
	  write_element(fd, "LastCheck", MAYBEKNOWN(lc));
	  end_element(fd, "StalledCheck");

	  xdelete(lc);
	}
      }
    }
    
    end_element(fd, "InstanceStatus");

    xdelete(at);
    xdelete(ct);
    
    end_element(fd, "SurvivorStatus");
    end_document(fd);
    
    ret = true;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "SurvivorXML::generate = %s", IOTF(ret));
#endif
  
  return(ret);
}

bool SurvivorXML::generate(int fd, TransportRequest *tr)
{
  // Generate an XML SurvivorTransportData document on <fd> based
  // on the contents of <tr>.

  // Returns: true if fully successful, false otherwise.
  
  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "SurvivorXML::generate(%d,%d)", fd, tr);
#endif

  if(tr && fd > -1)
  {
    begin_document(fd);
    
    begin_element(fd, "SurvivorTransportData", "Version", "1.0");

    if(tr->hosts())
    {
      // This is required, we should dump something even if no hosts
      // were provided, but since the module can't do anything without
      // hosts an error is OK.

      for(int i = 0;i < tr->hosts()->entries();i++)
	write_element(fd, "Host", MAYBEKNOWN(tr->hosts()->retrieve(i)));
    }

    write_element(fd, "Timeout", tr->timeout());

    if(tr->modargs())
    {
      for(int i = 0;i < tr->modargs()->entries();i++)
      {
	Argument *ar = tr->modargs()->retrieve(i);

	if(ar && ar->name() && ar->value())
	{
	  begin_element(fd, "ModuleOption", "OptionName", ar->name());
	  write_element(fd, "OptionValue", ar->value(), strlen(ar->value()));
	  end_element(fd, "ModuleOption");
	}
      }
    }

    begin_element(fd, "RemoteModule");

    write_element(fd, "Module", MAYBEKNOWN(tr->module()));
    write_element(fd, "ModType", MAYBEKNOWN(tr->modtype()));
    
    if(tr->rmodargs())
    {
      for(int i = 0;i < tr->rmodargs()->entries();i++)
      {
	Argument *ar = tr->rmodargs()->retrieve(i);

	if(ar && ar->name() && ar->value())
	{
	  begin_element(fd, "ModuleOption", "OptionName", ar->name());
	  write_element(fd, "OptionValue", ar->value(), strlen(ar->value()));
	  end_element(fd, "ModuleOption");
	}
      }
    }

    end_element(fd, "RemoteModule");
    
    end_element(fd, "SurvivorTransportData");
    end_document(fd);

    ret = true;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "SurvivorXML::generate = %s", IOTF(ret));
#endif

  return(ret);
}

bool SurvivorXML::generate_report_begin(int fd, ReportFormatting *rf)
{
  // Begin generating an XML SurvivorReportRequest document on <fd>
  // based on the contents of <rf>.

  // Returns: true if fully successful, false otherwise.
  
  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "SurvivorXML::generate_report_begin(%d,%d)",
		  fd, rf);
#endif

  if(rf && fd > -1)
  {
    begin_document(fd);
    begin_element(fd, "SurvivorReportRequest", "Version", "1.2");

    begin_element(fd, "Formatting", "Style", rf->style());

    if(rf->tmpdir())
      write_element(fd, "TmpDir", rf->tmpdir(), strlen(rf->tmpdir()));

    if(rf->uriprefix())
      write_element(fd, "TmpURIPrefix", rf->uriprefix(),
		    strlen(rf->uriprefix()));

    end_element(fd, "Formatting");
    
    ret = true;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "SurvivorXML::generate_report_begin = %s",
		 IOTF(ret));
#endif
  
  return(ret);
}

bool SurvivorXML::generate_report_end(int fd)
{
  // Finish generating an XML SurvivorReportRequest document on <fd>.

  // Returns: true if fully successful, false otherwise.
  
  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "SurvivorXML::generate_report_end(%d)", fd);
#endif

  if(fd > -1)
  {
    end_element(fd, "SurvivorReportRequest");
    end_document(fd);
    
    ret = true;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "SurvivorXML::generate_report_end = %s",
		 IOTF(ret));
#endif
  
  return(ret);
}

Acknowledgement *SurvivorXML::parse_acknowledgement(int fd)
{
  // Parse an XML SurvivorAlertAcknowledgement document from <fd>.

  // Returns: A newly allocated Acknowledgement object that should be
  // deleted when no longer required if successful, or NULL on error.

  Acknowledgement *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "SurvivorXML::parse_acknowledgement(%d)", fd);
#endif

  if(fd > -1)
  {
    XMLNode *xdoc = parse_document(fd);

    if(xdoc)
    {
      // Make sure we have a document version we understand

      char *xvers = xdoc->value("Version");
      
      if(xdoc->name()
	 && strcmp(xdoc->name(), "SurvivorAlertAcknowledgement")==0
	 && xvers && strcmp(xvers, "1.0")==0)
      {
	char *who = NULL;
	char *why = NULL;
	
	// We use a pointer since we'll dupe the char data before we
	// toss the XMLNode.

	Array<XMLNode> *xels = xdoc->children();
	
	if(xels)
	{
	  for(int i = 0;i < xels->entries();i++)
	  {
	    XMLNode *xel = xels->retrieve(i);
	    
	    if(xel && xel->name())
	    {
	      if(strcmp(xel->name(), "Who")==0)
		who = xel->data();
	      else if(strcmp(xel->name(), "Why")==0)
		why = xel->data();
	    }
	  }
	}

	ret = new Acknowledgement(who, why);

	if(!ret)
	  wlog->warn("SurvivorXML::parse_acknowledgement failed to allocate Acknowledgement");
      }
      else
	wlog->warn("SurvivorXML::parse_acknowledgement encountered version mismatch (found %s %s)", IONULL(xdoc->name()), IONULL(xvers));
      
      xdelete(xdoc);
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "SurvivorXML::parse_acknowledgement = %d", ret);
#endif
  
  return(ret);
}

Alert *SurvivorXML::parse_alert(int fd)
{
  // Parse an XML SurvivorAlertData document from <fd>.

  // Returns: A newly allocated Alert object that should be deleted when
  // no longer required if successful, or NULL on error.

  Alert *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "SurvivorXML::parse_alert(%d)", fd);
#endif

  if(fd > -1)
  {
    XMLNode *xdoc = parse_document(fd);

    if(xdoc)
    {
      // Make sure we have a document version we understand

      char *xvers =  xdoc->value("Version");
      
      if(xdoc->name() && strcmp(xdoc->name(), "SurvivorAlertData")==0
	 && xvers && strcmp(xvers, "1.1")==0)
      {
	// Convert the elements in the XMLNode object into the data
	// required for the Alert object.  We just keep pointers to
	// the xdoc stuff since the Alert will create copies before we
	// toss the XMLNode.
	
	RecipientSet *rset = new RecipientSet();
	time_t checktime = 0;
	int fixretval = 0;
	char *fixsummary = NULL;
	char *host = NULL;
	char *helpfile = NULL;
	char *instance = NULL;
	int instances = 0;
	int retval = 0;
	char *service = NULL;
	char *summary = NULL;
	char *token = NULL;
	
	if(rset)
	{
	  // First, look for the RecipientSet data

	  XMLNode *rchild = xdoc->child("RecipientSet");
	  
	  if(rchild)
	  {
	    // A set of Module children should be attached
	    
	    Array<XMLNode> *rmods = rchild->children();
	    
	    if(rmods)
	    {
	      for(int i = 0;i < rmods->entries();i++)
	      {
		XMLNode *rmod = rmods->retrieve(i);
		
		if(rmod && rmod->name() && strcmp(rmod->name(), "Module")==0)
		{
		  char *modname = rmod->value("Name");
		  
		  Array<XMLNode> *moddata = rmod->children();
		  
		  if(moddata)
		  {
		    for(int j = 0;j < moddata->entries();j++)
		    {
		      XMLNode *modd = moddata->retrieve(j);
		      
		      if(modd && modd->name() && modd->data())
		      {
			if(strcmp(modd->name(), "Address")==0)
			  rset->add_address(modd->data(), modname);
			else if(strcmp(modd->name(), "CallList")==0)
			  rset->add_calllist(modd->data(), modname);
		      }
		    }
		  }
		}
	      }
	    }
	  }
	  
	  // Now assemble everything else.  Since there can be more than
	  // one host, we may as well just iterate through everything
	  // manually.
	  
	  Array<XMLNode> *xadata = xdoc->children();
	  
	  if(xadata)
	  {
	    for(int i = 0;i < xadata->entries();i++)
	    {
	      XMLNode *xd = xadata->retrieve(i);
	      
	      if(xd)
	      {
		char *xn = xd->name();
		char *xdata = xd->data();

		if(xn && xdata)
		{
		  if(strcmp(xn, "Summary")==0)
		    summary = xdata;
		  else if(strcmp(xn, "HelpFile")==0)
		    helpfile = xdata;
		  else if(strcmp(xn, "Host")==0)
		    host = xdata;
		  else if(strcmp(xn, "Instance")==0)
		    instance = xdata;
		  else if(strcmp(xn, "Instances")==0)
		    instances = atoi(xdata);
		  else if(strcmp(xn, "FixSummary")==0)
		    fixsummary = xdata;
		  else if(strcmp(xn, "FixReturnCode")==0)
		    fixretval = atoi(xdata);
		  else if(strcmp(xn, "ReturnCode")==0)
		    retval = atoi(xdata);
		  else if(strcmp(xn, "Service")==0)
		    service = xdata;
		  else if(strcmp(xn, "Time")==0)
		    checktime = atoi(xdata);
		  else if(strcmp(xn, "Token")==0)
		    token = xdata;
		}
	      }
	    }
	  }
	  
	  // Now that we should have seen all relevant tags, allocate
	  // ret and store everything.

	  ret = new Alert(rset, checktime, host, helpfile, instance,
			  instances, retval, service, summary, token, false);

	  if(!ret)
	  {
	    wlog->warn("SurvivorXML::parse_alert failed to allocate Alert");
	    xdelete(rset);
	  }
	}
	else
	  wlog->warn("SurvivorXML::parse_alert failed to allocate RecipientSet");
      }
      else
	wlog->warn("SurvivorXML::parse_alert encountered version mismatch");
      
      xdelete(xdoc);
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "SurvivorXML::parse_alert = %d", ret);
#endif
  
  return(ret);
}

AlertStateData *SurvivorXML::parse_alertstatedata(int fd)
{
  // Parse an XML SurvivorAlertStatus document from <fd>.

  // Returns: A newly allocated AlertStateData object that should be
  // deleted when no longer required if successful, or NULL on error.

  AlertStateData *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "SurvivorXML::parse_alertstatedata(%d)", fd);
#endif

  if(fd > -1)
  {
    XMLNode *xdoc = parse_document(fd);

    if(xdoc)
    {
      // Make sure we have a document version we understand

      char *xvers = xdoc->value("Version");
      
      if(xdoc->name() && strcmp(xdoc->name(), "SurvivorAlertStatus")==0
	 && xvers && (strcmp(xvers, "1.0")==0 || strcmp(xvers, "1.1")==0))
      {
	// Version 1.0 is a subset of 1.1.  We'll accept either to
	// facilitate transition between 0.9.x and 1.0, but we don't
	// need to do this indefinitely.

	RecipientSet *rset = new RecipientSet();
	int insts = 0;
	int rc = 0;
	char *sum = NULL;
	time_t st = 0;
	time_t t = 0;
	
	if(rset)
	{
	  // First, look for the RecipientSet data

	  XMLNode *rchild = xdoc->child("RecipientSet");
	  
	  if(rchild)
	  {
	    // A set of Module children should be attached
	    
	    Array<XMLNode> *rmods = rchild->children();
	    
	    if(rmods)
	    {
	      for(int i = 0;i < rmods->entries();i++)
	      {
		XMLNode *rmod = rmods->retrieve(i);
		
		if(rmod && rmod->name() && strcmp(rmod->name(), "Module")==0)
		{
		  char *modname = rmod->value("Name");
		  
		  Array<XMLNode> *moddata = rmod->children();
		  
		  if(moddata)
		  {
		    // First, get the module return code

		    XMLNode *rcnode = rmod->child("ReturnCode");

		    if(rcnode && rcnode->data())
		    {
		      // Now iterate for all Addresses

		      for(int j = 0;j < moddata->entries();j++)
		      {
			XMLNode *modd = moddata->retrieve(j);
			
			if(modd && modd->name() && modd->data())
			{
			  if(strcmp(modd->name(), "Address")==0)
			    rset->add(modd->data(), modname,
				      atoi(rcnode->data()));
			}
		      }
		    }
		  }
		}
	      }
	    }
	  }

	  // Now look for the rest of the data.
	  
	  // We use a pointer for sum since AlertStateData will
	  // dupe it before we toss the XMLNode.

	  Array<XMLNode> *xels = xdoc->children();
	  
	  if(xels)
	  {
	    for(int i = 0;i < xels->entries();i++)
	    {
	      XMLNode *xel = xels->retrieve(i);
	      
	      if(xel && xel->name() && xel->data())
	      {
		if(strcmp(xel->name(), "Instances")==0 && xel->data())
		  insts = atoi(xel->data());
		else if(strcmp(xel->name(), "CheckReturnCode")==0
			&& xel->data())
		  rc = atoi(xel->data());
		else if(strcmp(xel->name(), "Since")==0)  // v1.1
		  st = atoi(xel->data());
		else if(strcmp(xel->name(), "CheckSummary")==0)
		  sum = xel->data();
		else if(strcmp(xel->name(), "Time")==0 && xel->data())
		  t = atoi(xel->data());
	      }
	    }
	  }
	
	  ret = new AlertStateData(rset, insts, rc, sum, t, st);

	  if(!ret)
	    wlog->warn("SurvivorXML::parse_alertstatedata failed to allocate AlertStateData");

	  // AlertStateData copies rset
	  xdelete(rset);
	}
	else
	  wlog->warn("SurvivorXML::parse_alertstatedata failed to allocate RecipientSet");      }
      else
	wlog->warn("SurvivorXML::parse_alertstatedata encountered version mismatch (found %s %s)", IONULL(xdoc->name()), IONULL(xvers));
      
      xdelete(xdoc);
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "SurvivorXML::parse_alertstatedata = %d", ret);
#endif
  
  return(ret);
}

CallListStateData *SurvivorXML::parse_callliststatedata(int fd)
{
  // Parse an XML SurvivorCallListStatus document from <fd>.

  // Returns: A newly allocated CallListStateData object that should be
  // deleted when no longer required if successful, or NULL on error.

  CallListStateData *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "SurvivorXML::parse_callliststatedata(%d)",
		  fd);
#endif

  if(fd > -1)
  {
    XMLNode *xdoc = parse_document(fd);

    if(xdoc)
    {
      // Make sure we have a document version we understand
      // 1.0 is a subset of 1.1, so accept it to ease upgrading

      char *xvers = xdoc->value("Version");
      
      if(xdoc->name() && strcmp(xdoc->name(), "SurvivorCallListStatus")==0
	 && xvers && (strcmp(xvers, "1.1")==0 || strcmp(xvers, "1.0")==0))
      {
	char *laddr = NULL;
	char *lperson = NULL;
	char *lvia = NULL;
	time_t rotate = 0;
	char *operson = NULL;
	
	// We use a pointer since we'll dupe the char data before we
	// toss the XMLNode.

	XMLNode *lnel = xdoc->child("LastNotify");

	if(lnel)
	{
	  Array<XMLNode> *lnels = lnel->children();

	  if(lnels)
	  {
	    for(int i = 0;i < lnels->entries();i++)
	    {
	      XMLNode *xel = lnels->retrieve(i);

	      if(xel && xel->name())
	      {
		if(strcmp(xel->name(), "Person")==0)
		  lperson = xel->data();
		else if(strcmp(xel->name(), "Address")==0)
		  laddr = xel->data();
		else if(strcmp(xel->name(), "Module")==0)
		  lvia = xel->data();
	      }
	    }
	  }
	}
	
	Array<XMLNode> *xels = xdoc->children();
	
	if(xels)
	{
	  for(int i = 0;i < xels->entries();i++)
	  {
	    XMLNode *xel = xels->retrieve(i);
	    
	    if(xel && xel->name() && xel->data())
	    {
	      if(strcmp(xel->name(), "LastRotate")==0 && xel->data())
		rotate = atoi(xel->data());
	      if(strcmp(xel->name(), "PersonOnCall")==0 && xel->data())
		operson = xel->data();
	    }
	  }
	}

	ret = new CallListStateData(lperson, laddr, lvia, rotate, operson);

	if(!ret)
	  wlog->warn("SurvivorXML::parse_callliststatedata failed to allocate SurvivorCallListStatus");
      }
      else
	wlog->warn("SurvivorXML::parse_callliststatedata encountered version mismatch (found %s %s)", IONULL(xdoc->name()), IONULL(xvers));
      
      xdelete(xdoc);
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "SurvivorXML::parse_callliststatedata = %d", ret);
#endif
  
  return(ret);
}

CGIAuthResult *SurvivorXML::parse_cgiauthresult(int fd)
{
  // Parse an XML SurvivorWebAuthResult document from <fd>.

  // Returns: A newly allocated CGIAuthResult object that should be
  // deleted when no longer required if successful, or NULL on error.

  CGIAuthResult *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "SurvivorXML::parse_cgiauthresult(%d)", fd);
#endif

  if(fd > -1)
  {
    XMLNode *xdoc = parse_document(fd);

    if(xdoc)
    {
      // Make sure we have a document version we understand

      char *xvers = xdoc->value("Version");
      
      if(xdoc->name() && strcmp(xdoc->name(), "SurvivorWebAuthResult")==0
	 && xvers && strcmp(xvers, "1.0")==0)
      {
	// Convert the elements in the XMLNode object into the data
	// required for the CGIAuthResult object.

	char *xar = xdoc->value("AuthOK");

	if(xar)
	{
	  authn_result_t res = no_authn;
	  char *user = NULL;
	  List *groups = new List();
	  char *deferral = NULL;
	  char *error = NULL;
	  List *skipflags = new List();
	  
	  if(strcmp(xar, "yes")==0)
	    res = yes_authn;
	  else if(strcmp(xar, "deferred")==0)
	    res = deferred_authn;

	  if(groups && skipflags)
	  {
	    // We use a pointer for user, etc since CGIAuthResult will
	    // dupe them before we toss the XMLNode.

	    Array<XMLNode> *xels = xdoc->children();
	    
	    if(xels)
	    {
	      for(int i = 0;i < xels->entries();i++)
	      {
		XMLNode *xel = xels->retrieve(i);
		
		if(xel && xel->name() && xel->data())
		{
		  if(strcmp(xel->name(), "Username")==0)
		    user = xel->data();
		  else if(strcmp(xel->name(), "Deferral")==0)
		    deferral = xel->data();
		  else if(strcmp(xel->name(), "Error")==0)
		    error = xel->data();
		  else if(strcmp(xel->name(), "MemberOf")==0)
		    groups->add(xel->data());
		  else if(skipflags && strcmp(xel->name(), "SkipFlag")==0)
		    skipflags->add(xel->data());
		}
	      }
	    }
	  }
	  else
	    wlog->warn("SurvivorXML::parse_cgiauthresult failed to allocate Lists");

	  ret = new CGIAuthResult(res, user, groups, deferral, error,
				  skipflags);

	  if(!ret)
	  {
	    xdelete(groups);
	    xdelete(skipflags);

	    wlog->warn("SurvivorXML::parse_cgiauthresult failed to allocate CGIAuthResult");
	  }
	}
      }
      else
	wlog->warn("SurvivorXML::parse_cgiauthresult encountered version mismatch (found %s %s)", IONULL(xdoc->name()), IONULL(xvers));

      xdelete(xdoc);
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "SurvivorXML::parse_cgiauthresult = %d", ret);
#endif
  
  return(ret);
}

CheckRequest *SurvivorXML::parse_checkrequest(int fd)
{
  // Parse an XML SurvivorCheckData document from <fd>.

  // Returns: A newly allocated CheckRequest object that should be
  // deleted when no longer required if successful, or NULL on error.

  CheckRequest *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "SurvivorXML::parse_checkrequest(%d)", fd);
#endif

  if(fd > -1)
  {
    XMLNode *xdoc = parse_document(fd);

    if(xdoc)
    {
      // Make sure we have a document version we understand

      char *xvers = xdoc->value("Version");
      
      if(xdoc->name() && strcmp(xdoc->name(), "SurvivorCheckData")==0
	 && xvers && strcmp(xvers, "1.0")==0)
      {
	// Convert the elements in the XMLNode object into the data
	// required for the CheckRequest object.

	List *hosts = new List();
	int timeout = 0;
	Array<Argument> *modargs = new Array<Argument>();

	if(hosts && modargs)
	{
	  Array<XMLNode> *xels = xdoc->children();
	    
	  if(xels)
	  {
	    for(int i = 0;i < xels->entries();i++)
	    {
	      XMLNode *xel = xels->retrieve(i);
	      
	      if(xel && xel->name() && xel->data())
	      {
		if(strcmp(xel->name(), "Host")==0)
		  hosts->add(xel->data());
		else if(strcmp(xel->name(), "Timeout")==0)
		  timeout = atoi(xel->data());
		else if(strcmp(xel->name(), "ModuleOption")==0)
		{
		  char *oname = xel->value("OptionName");
		  XMLNode *ovalue = xel->child("OptionValue");

		  if(oname && ovalue->data())
		  {
		    Argument *arg = new Argument(oname, ovalue->data(), true);

		    if(arg)
		    {
		      if(!modargs->add(arg))
		      {
			xdelete(arg);
		      }
		    }
		    else
		      wlog->warn("SurvivorXML::parse_checkrequest failed to allocate arg");
		  }
		}
	      }
	    }
	  }
	}
	else
	{
	  xdelete(hosts);
	  xdelete(modargs);
	  
	  wlog->warn("SurvivorXML::parse_checkrequest failed to allocate List or Array");
	}

	ret = new CheckRequest(hosts, timeout, modargs, true);

	if(!ret)
	{
	  xdelete(hosts);
	  xadelete(modargs, Argument);

	  wlog->warn("SurvivorXML::parse_checkrequest failed to allocate CheckRequest");
	}
      }
      else
	wlog->warn("SurvivorXML::parse_checkrequest encountered version mismatch (found %s %s)", IONULL(xdoc->name()), IONULL(xvers));

      xdelete(xdoc);
    }
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "SurvivorXML::parse_checkrequest = %d", ret);
#endif
  
  return(ret);
}

CheckResult *SurvivorXML::parse_checkresult(int fd)
{
  // Parse an XML SurvivorCheckResult document from <fd>.

  // Returns: A newly allocated CheckResult object that should be
  // deleted when no longer required if successful, or NULL on error.

  CheckResult *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "SurvivorXML::parse_checkresult(%d)", fd);
#endif

  if(fd > -1)
  {
    XMLNode *xdoc = parse_document(fd);

    ret = parse_checkresult_common(xdoc);
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "SurvivorXML::parse_checkresult = %d", ret);
#endif
  
  return(ret);
}

CheckResult *SurvivorXML::parse_checkresult(char *buf, int len)
{
  // Parse an XML SurvivorCheckResult document from <buf> of size <len>.

  // Returns: A newly allocated CheckResult object that should be
  // deleted when no longer required if successful, or NULL on error.

  CheckResult *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "SurvivorXML::parse_checkresult(%d,%d)",
		  strlen(buf), len);
#endif

  if(buf && len > 0)
  {
    XMLNode *xdoc = parse_document(buf, len);

    ret = parse_checkresult_common(xdoc);
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "SurvivorXML::parse_checkresult = %d", ret);
#endif
  
  return(ret);
}

CheckStateData *SurvivorXML::parse_checkstatedata(int fd)
{
  // Parse an XML SurvivorCheckStatus document from <fd>.

  // Returns: A newly allocated CheckStateData object that should be
  // deleted when no longer required if successful, or NULL on error.

  CheckStateData *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "SurvivorXML::parse_checkstatedata(%d)", fd);
#endif

  if(fd > -1)
  {
    XMLNode *xdoc = parse_document(fd);

    if(xdoc)
    {
      // Make sure we have a document version we understand

      char *xvers = xdoc->value("Version");
      
      if(xdoc->name() && strcmp(xdoc->name(), "SurvivorCheckStatus")==0
	 && xvers && (strcmp(xvers, "1.0")==0 || strcmp(xvers, "1.1")==0))
      {
	// Version 1.0 is a subset of 1.1.  We'll accept either to
	// facilitate transition between 0.9.x and 1.0, but we don't
	// need to do this indefinitely.

	int dur = -1;
	int insts = 0;
	int rc = 0;
	char *sum = NULL;
	time_t st = 0;
	time_t t = 0;
	
	// We use a pointer for sum since CheckStateData will
	// dupe it before we toss the XMLNode.

	Array<XMLNode> *xels = xdoc->children();
	
	if(xels)
	{
	  for(int i = 0;i < xels->entries();i++)
	  {
	    XMLNode *xel = xels->retrieve(i);
	    
	    if(xel && xel->name() && xel->data())
	    {
	      if(strcmp(xel->name(), "Duration")==0 && xel->data()) // v1.1
		dur = atoi(xel->data());
	      else if(strcmp(xel->name(), "Instances")==0 && xel->data())
		insts = atoi(xel->data());
	      else if(strcmp(xel->name(), "ReturnCode")==0 && xel->data())
		rc = atoi(xel->data());
	      else if(strcmp(xel->name(), "Since")==0)  // v1.1
		st = atoi(xel->data());
	      else if(strcmp(xel->name(), "Summary")==0)
		sum = xel->data();
	      else if(strcmp(xel->name(), "Time")==0 && xel->data())
		t = atoi(xel->data());
	    }
	  }
	}

	ret = new CheckStateData(insts, rc, sum, t, st, dur);

	if(!ret)
	  wlog->warn("SurvivorXML::parse_checkstatedata failed to allocate CheckStateData");
      }
      else
	wlog->warn("SurvivorXML::parse_checkstatedata encountered version mismatch (found %s %s)", IONULL(xdoc->name()), IONULL(xvers));
      
      xdelete(xdoc);
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "SurvivorXML::parse_checkstatedata = %d", ret);
#endif
  
  return(ret);
}

Escalation *SurvivorXML::parse_escalation(int fd)
{
  // Parse an XML SurvivorAlertEscalation document from <fd>.

  // Returns: A newly allocated Escalation object that should be
  // deleted when no longer required if successful, or NULL on error.

  Escalation *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "SurvivorXML::parse_escalation(%d)", fd);
#endif

  if(fd > -1)
  {
    XMLNode *xdoc = parse_document(fd);

    if(xdoc)
    {
      // Make sure we have a document version we understand

      char *xvers = xdoc->value("Version");
      
      if(xdoc->name()
	 && strcmp(xdoc->name(), "SurvivorAlertEscalation")==0
	 && xvers && strcmp(xvers, "1.0")==0)
      {
	int escto = 0;
	
	// We use a pointer since we'll dupe the char data before we
	// toss the XMLNode.

	Array<XMLNode> *xels = xdoc->children();
	
	if(xels)
	{
	  for(int i = 0;i < xels->entries();i++)
	  {
	    XMLNode *xel = xels->retrieve(i);
	    
	    if(xel && xel->name() && xel->data())
	    {
	      if(strcmp(xel->name(), "EscalateTo")==0 && xel->data())
		escto = atoi(xel->data());
	    }
	  }
	}

	ret = new Escalation(escto);

	if(!ret)
	  wlog->warn("SurvivorXML::parse_escalation failed to allocate Escalation");
      }
      else
	wlog->warn("SurvivorXML::parse_escalation encountered version mismatch (found %s %s)", IONULL(xdoc->name()), IONULL(xvers));
      
      xdelete(xdoc);
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "SurvivorXML::parse_escalation = %d", ret);
#endif
  
  return(ret);
}

FixResult *SurvivorXML::parse_fixresult(int fd)
{
  // Parse an XML SurvivorFixResult document from <fd>.

  // Returns: A newly allocated FixResult object that should be
  // deleted when no longer required if successful, or NULL on error.

  FixResult *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "SurvivorXML::parse_fixresult(%d)", fd);
#endif

  if(fd > -1)
  {
    XMLNode *xdoc = parse_document(fd);

    ret = parse_fixresult_common(xdoc);
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "SurvivorXML::parse_fixresult = %d", ret);
#endif
  
  return(ret);
}

FixResult *SurvivorXML::parse_fixresult(char *buf, int len)
{
  // Parse an XML SurvivorFixResult document from <buf> of size <len>.

  // Returns: A newly allocated FixResult object that should be
  // deleted when no longer required if successful, or NULL on error.

  FixResult *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "SurvivorXML::parse_fixresult(%d,%d)",
		  strlen(buf), len);
#endif

  if(buf && len > 0)
  {
    XMLNode *xdoc = parse_document(buf, len);

    ret = parse_fixresult_common(xdoc);
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "SurvivorXML::parse_fixresult = %d", ret);
#endif
  
  return(ret);
}

FixStateData *SurvivorXML::parse_fixstatedata(int fd)
{
  // Parse an XML SurvivorFixStatus document from <fd>.

  // Returns: A newly allocated FixStateData object that should be
  // deleted when no longer required if successful, or NULL on error.

  FixStateData *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "SurvivorXML::parse_fixstatedata(%d)", fd);
#endif

  if(fd > -1)
  {
    XMLNode *xdoc = parse_document(fd);

    if(xdoc)
    {
      // Make sure we have a document version we understand

      char *xvers = xdoc->value("Version");
      
      if(xdoc->name() && strcmp(xdoc->name(), "SurvivorFixStatus")==0
	 && xvers && (strcmp(xvers, "1.0")==0 || strcmp(xvers, "1.1")==0))
      {
	// Version 1.0 is a subset of 1.1.  We'll accept either to
	// facilitate transition between 0.9.x and 1.0, but we don't
	// need to do this indefinitely.

	char *initby = NULL;
	int insts = 0;
	int rc = 0;
	char *sum = NULL;
	time_t st = 0;
	time_t t = 0;
	
	// We use a pointer for initby, etc since FixStateData will
	// dupe them before we toss the XMLNode.

	Array<XMLNode> *xels = xdoc->children();
	
	if(xels)
	{
	  for(int i = 0;i < xels->entries();i++)
	  {
	    XMLNode *xel = xels->retrieve(i);
	    
	    if(xel && xel->name() && xel->data())
	    {
	      if(strcmp(xel->name(), "InitiatedBy")==0)
		initby = xel->data();
	      else if(strcmp(xel->name(), "Instances")==0 && xel->data())
		insts = atoi(xel->data());
	      else if(strcmp(xel->name(), "ReturnCode")==0 && xel->data())
		rc = atoi(xel->data());
	      else if(strcmp(xel->name(), "Since")==0)  // v1.1
		st = atoi(xel->data());
	      else if(strcmp(xel->name(), "Summary")==0)
		sum = xel->data();
	      else if(strcmp(xel->name(), "Time")==0 && xel->data())
		t = atoi(xel->data());
	    }
	  }
	}

	ret = new FixStateData(initby, insts, rc, sum, t, st);

	if(!ret)
	  wlog->warn("SurvivorXML::parse_fixstatedata failed to allocate FixStateData");
      }
      else
	wlog->warn("SurvivorXML::parse_fixstatedata encountered version mismatch (found %s %s)", IONULL(xdoc->name()), IONULL(xvers));
      
      xdelete(xdoc);
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "SurvivorXML::parse_fixstatedata = %d", ret);
#endif
  
  return(ret);
}

Inhibition *SurvivorXML::parse_inhibition(int fd)
{
  // Parse an XML SurvivorAlertInhibition document from <fd>.

  // Returns: A newly allocated Inhibition object that should be
  // deleted when no longer required if successful, or NULL on error.

  Inhibition *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "SurvivorXML::parse_inhibition(%d)", fd);
#endif

  if(fd > -1)
  {
    XMLNode *xdoc = parse_document(fd);

    if(xdoc)
    {
      // Make sure we have a document version we understand

      char *xvers = xdoc->value("Version");
      
      if(xdoc->name()
	 && strcmp(xdoc->name(), "SurvivorAlertInhibition")==0
	 && xvers && strcmp(xvers, "1.0")==0)
      {
	char *who = NULL;
	char *why = NULL;
	
	// We use a pointer since we'll dupe the char data before we
	// toss the XMLNode.

	Array<XMLNode> *xels = xdoc->children();
	
	if(xels)
	{
	  for(int i = 0;i < xels->entries();i++)
	  {
	    XMLNode *xel = xels->retrieve(i);
	    
	    if(xel && xel->name())
	    {
	      if(strcmp(xel->name(), "Who")==0)
		who = xel->data();
	      else if(strcmp(xel->name(), "Why")==0)
		why = xel->data();
	    }
	  }
	}

	ret = new Inhibition(who, why);

	if(!ret)
	  wlog->warn("SurvivorXML::parse_inhibition failed to allocate Inhibition");
      }
      else
	wlog->warn("SurvivorXML::parse_inhibition encountered version mismatch (found %s %s)", IONULL(xdoc->name()), IONULL(xvers));
      
      xdelete(xdoc);
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "SurvivorXML::parse_inhibition = %d", ret);
#endif
  
  return(ret);
}

Array<Substitution> *SurvivorXML::parse_substitutions(int fd)
{
  // Parse an XML SurvivorCallListSubstitutions document from <fd>.

  // Returns: A newly allocated Array of Substitutions that should be
  // deleted (both the Array and the Substitutions) when no longer
  // required if successful, or NULL on error.

  Array<Substitution> *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "SurvivorXML::parse_substitutions(%d)", fd);
#endif

  if(fd > -1)
  {
    XMLNode *xdoc = parse_document(fd);

    if(xdoc)
    {
      // Make sure we have a document version we understand

      char *xvers = xdoc->value("Version");
      
      if(xdoc->name()
	 && strcmp(xdoc->name(), "SurvivorCallListSubstitutions")==0
	 && xvers && strcmp(xvers, "1.0")==0)
      {
	ret = new Array<Substitution>();

	if(ret)
	{
	  Array<XMLNode> *xels = xdoc->children();
	
	  if(xels)
	  {
	    for(int i = 0;i < xels->entries();i++)
	    {
	      XMLNode *xel = xels->retrieve(i);

	      // The only element we accept at this level is "Substitution",
	      // and there may be more than one.
	      
	      if(xel && xel->name() && strcmp(xel->name(), "Substitution")==0
		 && xel->data())
	      {
		time_t begin = 0;
		time_t end = 0;
		char *newname = NULL;
		char *oldname = NULL;

		Array<XMLNode> *xsels = xel->children();

		if(xsels)
		{
		  for(int j = 0;j < xsels->entries();j++)
		  {
		    XMLNode *xsel = xsels->retrieve(j);

		    if(xsel && xsel->name() && xsel->data())
		    {
		      if(strcmp(xsel->name(), "Begin")==0 && xsel->data())
			begin = atoi(xsel->data());
		      else if(strcmp(xsel->name(), "End")==0 && xsel->data())
			end = atoi(xsel->data());
		      else if(strcmp(xsel->name(), "Original")==0)
			oldname = xsel->data();
		      else if(strcmp(xsel->name(), "Replacement")==0)
			newname = xsel->data();
		    }
		  }

		  Substitution *s = new Substitution(begin, end,
						     newname, oldname);

		  if(s)
		  {
		    if(!ret->add(s))
		    {
		      xdelete(s);
		    }
		  }
		  else
		    wlog->warn("SurvivorXML::parse_substitutions failed to allocate Substitution");
		}
	      }
	    }
	  }
	}
	else
	  wlog->warn("SurvivorXML::parse_substitutions failed to allocate Array");
      }
      else
	wlog->warn("SurvivorXML::parse_substitutions encountered version mismatch (found %s %s)", IONULL(xdoc->name()), IONULL(xvers));
      
      xdelete(xdoc);
    }
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "SurvivorXML::parse_substitutions = %d", ret);
#endif
  
  return(ret);
}

TransportRequest *SurvivorXML::parse_transportrequest(int fd)
{
  // Parse an XML SurvivorTransportData document from <fd>.

  // Returns: A newly allocated TransportRequest object that should be
  // deleted when no longer required if successful, or NULL on error.

  TransportRequest *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "SurvivorXML::parse_transportrequest(%d)", fd);
#endif

  if(fd > -1)
  {
    XMLNode *xdoc = parse_document(fd);

    if(xdoc)
    {
      // Make sure we have a document version we understand

      char *xvers = xdoc->value("Version");
      
      if(xdoc->name() && strcmp(xdoc->name(), "SurvivorTransportData")==0
	 && xvers && strcmp(xvers, "1.0")==0)
      {
	// Convert the elements in the XMLNode object into the data
	// required for the CheckRequest object.

	List *hosts = new List();
	int timeout = 0;
	Array<Argument> *modargs = new Array<Argument>();
	char *module = NULL;
	char *modtype = NULL;
	Array<Argument> *rmodargs = new Array<Argument>();

	if(hosts && modargs && rmodargs)
	{
	  Array<XMLNode> *xels = xdoc->children();
	    
	  if(xels)
	  {
	    for(int i = 0;i < xels->entries();i++)
	    {
	      XMLNode *xel = xels->retrieve(i);
	      
	      if(xel && xel->name() && xel->data())
	      {
		if(strcmp(xel->name(), "Host")==0)
		  hosts->add(xel->data());
		else if(strcmp(xel->name(), "Timeout")==0)
		  timeout = atoi(xel->data());
		else if(strcmp(xel->name(), "ModuleOption")==0)
		{
		  char *oname = xel->value("OptionName");
		  XMLNode *ovalue = xel->child("OptionValue");

		  if(oname && ovalue->data())
		  {
		    Argument *arg = new Argument(oname, ovalue->data(), true);

		    if(arg)
		    {
		      if(!modargs->add(arg))
		      {
			xdelete(arg);
		      }
		    }
		    else
		      wlog->warn("SurvivorXML::parse_transportrequest failed to allocate arg");
		  }
		}
		else if(strcmp(xel->name(), "RemoteModule")==0)
		{
		  Array<XMLNode> *xels2 = xel->children();

		  if(xels2)
		  {
		    for(int j = 0;j < xels2->entries();j++)
		    {
		      XMLNode *xel2 = xels2->retrieve(j);

		      if(xel2 && xel2->name())
		      {
			if(strcmp(xel2->name(), "Module")==0)
			  module = xel2->data();
			else if(strcmp(xel2->name(), "ModType")==0)
			  modtype = xel2->data();
			else if(strcmp(xel2->name(), "ModuleOption")==0)
			{
			  char *oname = xel2->value("OptionName");
			  XMLNode *ovalue = xel2->child("OptionValue");

			  if(oname && ovalue->data())
			  {
			    Argument *arg = new Argument(oname,
							 ovalue->data(),
							 true);

			    if(arg)
			    {
			      if(!rmodargs->add(arg))
			      {
				xdelete(arg);
			      }
			    }
			    else
			      wlog->warn("SurvivorXML::parse_checkrequest failed to allocate arg");
			  }
			}
		      }
		    }
		  }
		}
	      }
	    }
	  }
	}
	else
	{
	  xdelete(hosts);
	  xdelete(modargs);
	  xdelete(rmodargs);
	  
	  wlog->warn("SurvivorXML::parse_transportrequest failed to allocate List or Array");
	}

	ret = new TransportRequest(hosts, timeout, modargs, module, modtype,
				   rmodargs, true);

	if(!ret)
	{
	  xdelete(hosts);
	  xadelete(modargs, Argument);
	  xadelete(rmodargs, Argument);

	  wlog->warn("SurvivorXML::parse_transportrequest failed to allocate TransportRequest");
	}
      }
      else
	wlog->warn("SurvivorXML::parse_transportrequest encountered version mismatch (found %s %s)", IONULL(xdoc->name()), IONULL(xvers));

      xdelete(xdoc);
    }
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "SurvivorXML::parse_transportrequest = %d",
		 ret);
#endif
  
  return(ret);
}

bool SurvivorXML::set_read_timeout(int t)
{
  // Set the read timeout to <t> seconds, or disable if <t> is 0.
  // The default read timeout is 0.  The read timeout requires the
  // use of alarm() and changes the current signal disposition for
  // SIGALRM.

  // Returns: true if fully successful, false otherwise.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "SurvivorXML::set_read_timeout(%d)", t);
  dlog->log_exit(DEBUG_MAJTRC, "SurvivorXML::set_read_timeout = %s",
		 IOTF(true));
#endif
  
  rto = t;
  return(true);
}

SurvivorXML::~SurvivorXML()
{
  // Deallocate the SurvivorXML object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "SurvivorXML::~SurvivorXML()");
  dlog->log_exit(DEBUG_MAJTRC, "SurvivorXML::~SurvivorXML()");
#endif
  
  rto = 0;
}

void SurvivorXML::begin_document(int fd)
{
  // Generate XML on <fd> to indicate the beginning of a document.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "SurvivorXML::begin_document(%d)", fd);
#endif

  write(fd, XMLDOCTAG, strlen(XMLDOCTAG));
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "SurvivorXML::begin_document()");
#endif
}

void SurvivorXML::begin_element(int fd, char *element)
{
  // Generate XML on <fd> to indicate the beginning of the <element>.

  // Returns: Nothing.

  return(begin_element(fd, element, NULL, NULL, NULL, NULL));
}

void SurvivorXML::begin_element(int fd, char *element, char *attname,
				char *attvalue)
{
  // Generate XML on <fd> to indicate the beginning of the <element>,
  // with attribute <attname>=<attvalue>.

  // Returns: Nothing.

  return(begin_element(fd, element, attname, attvalue, NULL, NULL));
}

void SurvivorXML::begin_element(int fd, char *element,
				char *attname, char *attvalue,
				char *attname2, char *attvalue2)
{
  // Generate XML on <fd> to indicate the beginning of the <element>,
  // with attribute <attname>=<attvalue> and <attname2>=<attvalue2>.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC,
		  "SurvivorXML::begin_element(%d,%s,%s,%s,%s,%s)",
		  fd, IONULL(element), IONULL(attname), IONULL(attvalue),
		  IONULL(attname2), IONULL(attvalue2));
#endif

  if(element)
  {
    write(fd, "<", 1);
    write(fd, element, strlen(element));
    
    if(attname && attvalue)
    {
      write(fd, " ", 1);
      write(fd, attname, strlen(attname));
      write(fd, "=\"", 2);
      write(fd, attvalue, strlen(attvalue));
      write(fd, "\"", 1);

      if(attname2 && attvalue2)
      {
	write(fd, " ", 1);
	write(fd, attname2, strlen(attname2));
	write(fd, "=\"", 2);
	write(fd, attvalue2, strlen(attvalue2));
	write(fd, "\"", 1);
      }
    }
    
    write(fd, ">", 1);
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "SurvivorXML::begin_element()");
#endif
}

void SurvivorXML::end_document(int fd)
{
  // Generate XML on <fd> to indicate the end of the document.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "SurvivorXML::end_document()");
#endif
  
  write(fd, "\n", 1);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "SurvivorXML::end_document()");
#endif
}

void SurvivorXML::end_element(int fd, char *element)
{
  // Generate XML on <fd> to indicate the end of the <element>.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "SurvivorXML::end_element(%d,%s)",
		  fd, IONULL(element));
#endif

  if(element)
  {
    write(fd, "</", 2);
    write(fd, element, strlen(element));
    write(fd, ">\n", 2);
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "SurvivorXML::end_element()");
#endif
}

CheckResult *SurvivorXML::parse_checkresult_common(XMLNode *xdoc)
{
  // Handle post XML parse <xdoc> interpretation of CheckResult.

  // Returns: A newly allocated CheckResult object that should be
  // deleted when no longer required if successful, or NULL on error.

  CheckResult *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "SurvivorXML::parse_checkresult_common(%d)",
		  xdoc);
#endif

  if(xdoc)
  {
    // Make sure we have a document version we understand (v1.1 and v1.0
    // are pretty similar, so we accept either to make upgrading easier)

    char *xvers = xdoc->value("Version");
    
    if(xdoc->name() && strcmp(xdoc->name(), "SurvivorCheckResult")==0
       && xvers && (strcmp(xvers, "1.1")==0 || strcmp(xvers, "1.0")==0))
    {
      char *host = NULL;
      int duration = -1;
      int rc = MODEXEC_MISCONFIG;  // A reasonable fall back
      int scalar = 0;
      char *comment = NULL;

      // We use pointers since we'll dupe the char data before we
      // toss the XMLNode.

      Array<XMLNode> *xels = xdoc->children();

      if(xels)
      {
	for(int i = 0;i < xels->entries();i++)
	{
	  XMLNode *xel = xels->retrieve(i);

	  if(xel && xel->name() && xel->data())
	  {
	    if(strcmp(xel->name(), "Host")==0)
	      host = xel->data();
	    else if(strcmp(xel->name(), "ReturnCode")==0)
	      rc = atoi(xel->data());
	    else if(strcmp(xel->name(), "Scalar")==0)
	      scalar = atoi(xel->data());
	    else if(strcmp(xel->name(), "Comment")==0)
	      comment = xel->data();
	    else if(strcmp(xel->name(), "Duration")==0)
	      duration = atoi(xel->data());
	  }
	}
      }
      
      ret = new CheckResult(host, rc, scalar, comment, duration);

      if(!ret)
	wlog->warn("SurvivorXML::parse_checkresult_common failed to allocate CheckResult");
    }
    else
	wlog->warn("SurvivorXML::parse_checkresult_common encountered version mismatch (found %s %s)", IONULL(xdoc->name()), IONULL(xvers));
      
    xdelete(xdoc);
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "SurvivorXML::parse_checkresult_common = %d",
		 ret);
#endif
  
  return(ret);
}

FixResult *SurvivorXML::parse_fixresult_common(XMLNode *xdoc)
{
  // Handle post XML parse <xdoc> interpretation of FixResult.

  // Returns: A newly allocated FixResult object that should be
  // deleted when no longer required if successful, or NULL on error.

  FixResult *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "SurvivorXML::parse_fixresult_common(%d)",
		  xdoc);
#endif

  if(xdoc)
  {
    // Make sure we have a document version we understand

    char *xvers = xdoc->value("Version");
      
    if(xdoc->name() && strcmp(xdoc->name(), "SurvivorFixResult")==0
       && xvers && strcmp(xvers, "1.0")==0)
    {
      int rc = MODEXEC_MISCONFIG;  // A reasonable fall back
      char *comment = NULL;

      // We use pointers since we'll dupe the char data before we
      // toss the XMLNode.
      
      Array<XMLNode> *xels = xdoc->children();

      if(xels)
      {
	for(int i = 0;i < xels->entries();i++)
	{
	  XMLNode *xel = xels->retrieve(i);
	  
	  if(xel && xel->name() && xel->data())
	  {
	    if(strcmp(xel->name(), "ReturnCode")==0)
	      rc = atoi(xel->data());
	    else if(strcmp(xel->name(), "Comment")==0)
	      comment = xel->data();
	  }
	}
      }
      
      ret = new FixResult(rc, comment);
      
      if(!ret)
	wlog->warn("SurvivorXML::parse_fixresult failed to allocate FixResult");
    }
    else
      wlog->warn("SurvivorXML::parse_fixresult encountered version mismatch (found %s %s)", IONULL(xdoc->name()), IONULL(xvers));
      
    xdelete(xdoc);
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "SurvivorXML::parse_fixresult_common = %d",
		 ret);
#endif
  
  return(ret);
}

XMLNode *SurvivorXML::parse_document(int fd)
{
  // Parse the XML document on <fd>.  This method honors the current
  // read timeout.

  // Returns: A newly allocated XMLNode object representing the top of
  // the parse tree and that should be deleted when no longer
  // required, or NULL on error.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "SurvivorXML::parse_document(%d)", fd);
#endif

  XMLParseData pd = { NULL, NULL };

  // Allocate a parser and attach the ParseData as userdata,
  // and the appropriate handlers.

  XML_Parser parser = XML_ParserCreate(NULL);

  if(parser)
  {
    int done = 0;

    // Local read timeout: we set this after the first read as a workaround
    // to EOF sometimes not being received
    int lrto = 0;
    
    XML_SetUserData(parser, (void *)&pd);
    XML_SetElementHandler(parser, startXMLNode, endXMLNode);
    XML_SetCharacterDataHandler(parser, charDataXMLNode);

#if defined(_BSD_SIGNALS)
    signal(SIGALRM, _SurvivorXML_alarm_catcher);
#else
    sigset(SIGALRM, _SurvivorXML_alarm_catcher);
#endif
    
    while(!done)
    {
      // Use Expat's buffers to avoid double buffering.  Expat requires
      // GetBuffer calls prior to ParseBuffer calls, which suggests
      // it frees the buffer.
      
      void *buf = XML_GetBuffer(parser, BUFSIZE);
      
      if(buf)
      {
	memset(buf, 0, BUFSIZE);

	// Set read timeout?
	if(lrto > 0)
	  alarm(lrto);
	
	int r = read(fd, buf, BUFSIZE);

	// Unset timeout or maybe prepare timeout for next loop
	if(lrto > 0)
	  alarm(0);
	else
	{
	  if(rto > 0)
	    lrto = rto;
	}

	switch(r)
	{
	case -1:
	  if(errno != EINTR)  // EINTR means alarm was probably triggered
	    wlog->warn("SurvivorXML::parse_document read error: %d", errno);
	  done = 1;
	  break;
	case 0:
	  done = 1;
	  // Intentionally fall through
	default:
	  XML_ParseBuffer(parser, r, done);
	  // Ignore errors from XML_ParseBuffer since they seem to
	  // be generated spuriously, before the document is fully
	  // parsed.
	  //wlog->warn("SurvivorXML::parse_document failed in ParseBuffer");
	  break;
	}
      }
      else
	wlog->warn("SurvivorXML::parse_document failed to GetBuffer");
    }

#if defined(_BSD_SIGNALS)
    signal(SIGALRM, SIG_DFL);
#else
    sigrelse(SIGALRM);
#endif
	
    XML_ParserFree(parser);
  }
  else
    wlog->warn("SurvivorXML::parse_document failed to allocate parser");
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "SurvivorXML::parse_document = %d", pd.top);
#endif
  
  return(pd.top);
}

XMLNode *SurvivorXML::parse_document(char *dbuf, int len)
{
  // Parse the XML document in <dbuf> of size <len>.

  // Returns: A newly allocated XMLNode object representing the top of
  // the parse tree and that should be deleted when no longer
  // required, or NULL on error.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "SurvivorXML::parse_document(%d,%d)",
		  strlen(dbuf), len);
#endif

  XMLParseData pd = { NULL, NULL };

  // Allocate a parser and attach the ParseData as userdata,
  // and the appropriate handlers.
  
  XML_Parser parser = XML_ParserCreate(NULL);

  if(parser)
  {
    XML_SetUserData(parser, (void *)&pd);
    XML_SetElementHandler(parser, startXMLNode, endXMLNode);
    XML_SetCharacterDataHandler(parser, charDataXMLNode);

    // Use Expat's buffers to avoid double buffering.  Expat requires
    // GetBuffer calls prior to ParseBuffer calls, which suggests
    // it frees the buffer.
      
    void *buf = XML_GetBuffer(parser, len);

    if(buf)
    {
      memcpy(buf, dbuf, len);
      XML_ParseBuffer(parser, len, 1);
    }
    else
      wlog->warn("SurvivorXML::parse_document failed to GetBuffer");

    XML_ParserFree(parser);
  }
  else
    wlog->warn("SurvivorXML::parse_document failed to allocate parser");
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "SurvivorXML::parse_document = %d", pd.top);
#endif
  
  return(pd.top);
}

void SurvivorXML::write_element(int fd, char *element, char *text, int len)
{
  // Generate XML on <fd> for the <element>, containing <len> characters
  // of <text> between the open and close tags.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "SurvivorXML::write_element(%d,%s,%s,%d)",
		  fd, IONULL(element), IONULL(text), len);
#endif

  if(element && text && len > 0)
  {
    begin_element(fd, element);

    // Escape < and > to not confuse the parsers.  We wouldn't have to
    // do this if we enforced the DTDs.  See similar workarounds in
    // Survivor::XML.pm

    char *etext = escape_xml(text, len);

    if(etext)
    {
      write(fd, etext, strlen(etext));
      xdelete(etext);
    }
    else
    {
      // Default back to original text
      write(fd, text, len);
    }
    
    end_element(fd, element);
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "SurvivorXML::write_element()");
#endif
}

void SurvivorXML::write_element(int fd, char *element, int text)
{
  // Generate XML on <fd> for the <element>, containing the number <text>
  // as the text between the open and close tags.

  // Returns: Nothing.

  char buf[INTBUFSIZE];

  snprintf(buf, INTBUFSIZE, "%d", text);
  return(write_element(fd, element, buf, strlen(buf)));
}
