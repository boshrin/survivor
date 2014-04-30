/*
 * expat_handlers.C: Handlers for use with the Expat XML parser.
 *
 * Version: $Revision: 0.2 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2004/03/01 23:38:34 $
 * MT-Level: Safe
 *
 * Copyright (c) 2003 - 2004
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: expat_handlers.C,v $
 * Revision 0.2  2004/03/01 23:38:34  benno
 * Overhaul wrapper to be more generalized
 *
 * Revision 0.1  2003/05/29 00:38:28  benno
 * Initial revision
 *
 */

#include "survivor.H"

void startXMLNode(void *userData, const char *name, const char **atts)
{
  // Expat handler for open tags.

  // Returns: Nothing.

  XMLParseData *pd = (XMLParseData *)userData;

  if(pd)
  {
    // First, assemble the attribute lists.  We assume they are provided
    // correctly, as alternating name and value attributes, with a
    // terminating NULL in an even slot.

    List *attsn = NULL;
    List *attsv = NULL;
  
    if(atts)
    {
      bool attok = true;  // Assume success unless otherwise noted
      
      attsn = new List();
      attsv = new List();
      
      if(attsn && attsv)
      {
	for(int i = 0;atts[i] != NULL;i += 2)
	{
	  if(!atts[i+1] || !attsn->add((char *)atts[i])
	     || !attsv->add((char *)atts[i+1]))
	    attok = false;
	}
      }
      else
      {
	attok = false;
	wlog->warn("startXMLNode failed to allocate Lists");
      }
      
      if(!attok)
      {
	xdelete(attsn);
	xdelete(attsv);
      }
    }
    
    if(!pd->current)
    {
      // This is a new document, so create a top
      
      pd->top = new XMLNode((char *)name, NULL, attsn, attsv);
      
      if(pd->top)
	pd->current = pd->top;
      else
	wlog->warn("startXMLNode failed to allocate XMLNode");
    }
    else
    {
      // Create a new node, make it a child of the current node, then
      // make it current.
      
      XMLNode *xn = new XMLNode((char *)name, pd->current, attsn, attsv);
      
      if(xn)
      {
	if(pd->current->attach_child(xn))
	  pd->current = xn;
	else
	{
	  wlog->warn("startXMLNode failed to attach child");
	  xdelete(xn);
	}
      }
      else
	wlog->warn("startXMLNode failed to allocate XMLNode");
    }
  }
}

void endXMLNode(void *userData, const char *name)
{
  // Expat handler for close tags.

  // Returns: Nothing.

  XMLParseData *pd = (XMLParseData *)userData;

  if(pd)
  {
    // Make the current be the parent of the current, unless the
    // current is the top.

    if(pd->current && pd->current->parent())
      pd->current = pd->current->parent();
  }
}

void charDataXMLNode(void *userData, const XML_Char *s, int len)
{
  // Expat handler for char data.  <s> is not NULL terminated,
  // so <len> must be used.  This function may be called multiple
  // times before the close tag is read.

  // Returns: Nothing.

  XMLParseData *pd = (XMLParseData *)userData;

  if(pd)
  {
    // Just append to the current node.  If we're not interested
    // in this data (eg: the whitespace between tags within the
    // document's open and close tags), it's OK.  It'll be
    // appended to the node, and then just ignored.

    if(pd->current)
      pd->current->append_data((char *)s, len);
  }
}
