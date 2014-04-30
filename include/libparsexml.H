/*
 * libparsexml.H
 *
 * Version: $Revision: 0.9 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2005/04/30 21:40:19 $
 *
 * Copyright (c) 2003 - 2005
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: libparsexml.H,v $
 * Revision 0.9  2005/04/30 21:40:19  benno
 * Add SurvivorXML::set_read_timeout
 *
 * Revision 0.8  2005/04/09 02:45:39  benno
 * Changes for XML based module arguments
 *
 * Revision 0.7  2004/08/25 01:51:31  benno
 * Add generate(HistorySet) and generate_report_*
 *
 * Revision 0.6  2004/06/11 22:26:40  benno
 * Support for XMLification of state data
 *
 * Revision 0.5  2004/03/02 03:47:45  benno
 * Delete ParseData, add XMLNode, related changes
 *
 * Revision 0.4  2003/10/20 00:04:06  benno
 * generate(Alert, module)
 *
 * Revision 0.3  2003/10/06 23:21:56  benno
 * Add generate(SurvivorStatus), 6 arg begin_element
 *
 * Revision 0.2  2003/07/30 21:40:21  benno
 * Add generate(FormattedAlert)
 *
 * Revision 0.1  2003/05/29 00:40:22  benno
 * Initial revision
 *
 */

// This version of libparsexml is wrapped around Expat version 1.95.6.
// The XML parser used could change in a future release, so nothing
// related to it should be made visible.

// Get the expat includes.  expat is written in C, but the include file
// already extern "C"s itself.

#include "expat.h"

// The handlers and objects defined here should not be considered
// visible outside of libparsexml.

class XMLNode {
public:
  XMLNode(char *name);
  XMLNode(char *name, XMLNode *parent);
  XMLNode(char *name, XMLNode *parent, List *attns, List *attvs);
  bool append_data(char *s, int len);
  bool attach_child(XMLNode *c);
  XMLNode *child(char *name);
  Array<XMLNode> *children();
  char *data();
  char *name();
  XMLNode *parent();
  char *value(char *attribute);
  ~XMLNode();

private:
  Array<XMLNode> *cs;  // The children of this node
  char *n;             // The name of this node
  char *d;             // The data in the node
  List *ans;           // Attribute names
  List *avs;           // Attribute values, corresponding with attribute names
  XMLNode *p;          // The parent of this node, or NULL if this is the top

  void init(char *name, XMLNode *parent, List *attns, List *attvs);
};

struct XMLParseData {
  XMLNode *top;        // The document top
  XMLNode *current;    // The current point of the parse
};

void startXMLNode(void *userData, const char *name, const char **atts);
void endXMLNode(void *userData, const char *name);
void charDataXMLNode(void *userData, const XML_Char *s, int len);

// The objects defined below can be considered visible outside of
// libparsexml.

class SurvivorXML {
public:
  SurvivorXML();
  bool generate(int fd, Acknowledgement *ad);
  bool generate(int fd, Alert *a, char *module);
  bool generate(int fd, AlertStateData *ad);
  bool generate(int fd, CallListStateData *cd);
  bool generate(int fd, CGIAuthRequest *ar);
  bool generate(int fd, CheckRequest *cr);
  bool generate(int fd, CheckResult *cr);
  bool generate(int fd, CheckStateData *cd);
  bool generate(int fd, Escalation *ed);
  bool generate(int fd, FixRequest *fr);
  bool generate(int fd, FixResult *fr);
  bool generate(int fd, FixStateData *xd);
  bool generate(int fd, FormattedAlert *f);
  bool generate(int fd, HistorySet *hs);
  bool generate(int fd, Inhibition *id);
  bool generate(int fd, Array<Substitution> *sa);
  bool generate(int fd, SurvivorStatus *s);
  bool generate(int fd, TransportRequest *fr);
  bool generate_report_begin(int fd, ReportFormatting *rf);
  bool generate_report_end(int fd);
  Acknowledgement *parse_acknowledgement(int fd);
  Alert *parse_alert(int fd);
  AlertStateData *parse_alertstatedata(int fd);
  CallListStateData *parse_callliststatedata(int fd);
  CGIAuthResult *parse_cgiauthresult(int fd);
  CheckRequest *parse_checkrequest(int fd);
  CheckResult *parse_checkresult(int fd);
  CheckResult *parse_checkresult(char *buf, int len);
  CheckStateData *parse_checkstatedata(int fd);
  Escalation *parse_escalation(int fd);
  FixResult *parse_fixresult(int fd);
  FixResult *parse_fixresult(char *buf, int len);
  FixStateData *parse_fixstatedata(int fd);
  Inhibition *parse_inhibition(int fd);
  Array<Substitution> *parse_substitutions(int fd);
  TransportRequest *parse_transportrequest(int fd);
  bool set_read_timeout(int t);
  ~SurvivorXML();

private:
  void begin_document(int fd);
  void begin_element(int fd, char *element);
  void begin_element(int fd, char *element, char *attname, char *attvalue);
  void begin_element(int fd, char *element, char *attname, char *attvalue,
		     char *attname2, char *attvalue2);
  void end_document(int fd);
  void end_element(int fd, char *element);
  CheckResult *parse_checkresult_common(XMLNode *xdoc);
  FixResult *parse_fixresult_common(XMLNode *xdoc);
  XMLNode *parse_document(int fd);
  XMLNode *parse_document(char *dbuf, int len);
  void write_element(int fd, char *element, char *text, int len);
  void write_element(int fd, char *element, int text);

  int rto;         // Read timeout, 0 = none
};
