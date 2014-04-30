/*
 * XMLNode.C: Object to hold data parsed from an XML document
 *
 * Version: $Revision: 0.1 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2004/03/01 23:41:29 $
 * MT-Level: Safe
 *
 * Copyright (c) 2004
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: XMLNode.C,v $
 * Revision 0.1  2004/03/01 23:41:29  benno
 * Initial revision
 *
 */

#include "survivor.H"

XMLNode::XMLNode(char *name)
{
  // Allocate and initialize a new, top level XMLNode, named <name>.

  // Returns: A new, initialized XMLNode.
  
  init(name, NULL, NULL, NULL);
}

XMLNode::XMLNode(char *name, XMLNode *parent)
{
  // Allocate and initialize a new XMLNode, named <name>, a child
  // of <parent>.

  // Returns: A new, initialized XMLNode.
  
  init(name, parent, NULL, NULL);
}

XMLNode::XMLNode(char *name, XMLNode *parent, List *attns, List *attvs)
{
  // Allocate and initialize a new, top level XMLNode, named <name>,
  // with a set of attributes named <attns> and corresponding values
  // <attvs>.  <attns> and <attvs> are maintained by the XMLNode
  // object and will be deleted when no longer required.

  // Returns: A new, initialized XMLNode.
  
  init(name, parent, attns, attvs);
}

bool XMLNode::append_data(char *s, int len)
{
  // Append <len> characters of <s> to the data of this XMLNode.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;

#if defined(DEBUG)
  // We don't know that <s> is NULL terminated, so don't echo it
  dlog->log_entry(DEBUG_MINTRC, "XMLNode::append_data(%d,%d)", s, len);
#endif

  if(s && len > 0)
  {
    d = xstrncat(d, s, len);

    if(d)
      ret = true;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "XMLNode::append_data = %s", IOTF(ret));
#endif

  return(ret);
}

bool XMLNode::attach_child(XMLNode *c)
{
  // Attach <c> as a child of this XMLNode.  <c> will be maintained
  // by the XMLNode and deleted when no longer required.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "XMLNode::attach_child(%d)", c);
#endif

  if(c)
  {
    if(!cs)
      cs = new Array<XMLNode>();

    if(cs)
      ret = cs->add(c);
    else
      wlog->warn("XMLNode::attach_child failed to allocate Array");
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "XMLNode::attach_child = %s", IOTF(ret));
#endif

  return(ret);
}

XMLNode *XMLNode::child(char *name)
{
  // Obtain the child node <name>.  If more than one child node is
  // named <name>, only the first will be returned.  Use children()
  // to access multiple nodes of the same name.

  // Returns: A pointer to a child node, or NULL if not found.

  XMLNode *ret = NULL;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "XMLNode::child(%s)", IONULL(name));
#endif

  if(cs && name)
  {
    for(int i = 0;i < cs->entries();i++)
    {
      XMLNode *x = cs->retrieve(i);

      if(x && x->name() && strcmp(x->name(), name)==0)
      {
	ret = x;
	break;
      }
    }
  }
    
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "XMLNode::child = %d", ret);
#endif

  return(ret);
}

Array<XMLNode> *XMLNode::children()
{
  // Obtain the children of this node.
  
  // Returns: A pointer to an array of XMLNodes, or NULL on error.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "XMLNode::children()");
  dlog->log_exit(DEBUG_MINTRC, "XMLNode::children = %d", cs);
#endif

  return(cs);
}

char *XMLNode::data()
{
  // Obtain the data of this node.
  
  // Returns: The data, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "XMLNode::data()");
  dlog->log_exit(DEBUG_MINTRC, "XMLNode::data = %s", IONULL(d));
#endif

  return(d);
}

char *XMLNode::name()
{
  // Obtain the name of this node.
  
  // Returns: The name, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "XMLNode::name()");
  dlog->log_exit(DEBUG_MINTRC, "XMLNode::name = %s", IONULL(n));
#endif

  return(n);
}

XMLNode *XMLNode::parent()
{
  // Obtain the parent of this node.
  
  // Returns: A pointer to the parent, or NULL if this is the top node.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "XMLNode::parent()");
  dlog->log_exit(DEBUG_MINTRC, "XMLNode::parent = %d", p);
#endif

  return(p);
}

char *XMLNode::value(char *attribute)
{
  // Obtain the value of <attribute>.
  
  // Returns: The value, or NULL if not found.

  char *ret = NULL;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "XMLNode::value(%s)", IONULL(attribute));
#endif

  if(ans && avs)
  {
    int i = ans->find(attribute);

    if(i > -1)
      ret = avs->retrieve(i);
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "XMLNode::value = %s", IONULL(ret));
#endif

  return(ret);
}

XMLNode::~XMLNode()
{
  // Deallocate the XMLNode.  This will also deallocate any children
  // of this node.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "XMLNode::~XMLNode()");
#endif

  // This will recursively delete all the nodes stored within the Array.
  xadelete(cs, XMLNode);

  xdelete(n);
  xdelete(d);
  xdelete(ans);
  xdelete(avs);

  // Don't delete this, just reset it.
  p = NULL;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "XMLNode::~XMLNode()");
#endif
}

void XMLNode::init(char *name, XMLNode *parent, List *attns, List *attvs)
{
  // Function for constructors.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "XMLNode::init(%s,%d,%d,%d)", IONULL(name),
		  parent, attns, attvs);
#endif

  cs = NULL;
  n = xstrdup(name);
  d = NULL;
  ans = attns;
  avs = attvs;
  p = parent;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "XMLNode::init()");
#endif
}
