/*
 * Variable.C: Variable storage object
 *
 * Version: $Revision: 0.1 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2004/03/02 17:12:44 $
 * MT-Level: Safe
 *
 * Copyright (c) 2003
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: Variable.C,v $
 * Revision 0.1  2004/03/02 17:12:44  benno
 * Initial revision
 *
 */

#include "cgi.H"

Variable::Variable(char *name, char *value)
{
  // Allocate and initialize a new Variable object, holding the
  // variable <name> with contents <value>.

  // Returns: A new, initialized Variable.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Variable::Variable(%s,%s)",
		  IONULL(name), IONULL(value));
#endif

  n = xstrdup(name);
  v = xstrdup(value);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Variable::Variable()");
#endif
}

char *Variable::name()
{
  // Retrieve the variable name.

  // Returns: The variable name, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Variable::name()");
  dlog->log_exit(DEBUG_MINTRC, "Variable::name = %s", IONULL(n));
#endif
  
  return(n);
}

bool Variable::set_value(char *value)
{
  // Set the value of this Variable to <value>, replacing any previous
  // value.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Variable::set_value(%s)", IONULL(value));
#endif

  if(value)
  {
    xdelete(v);
    v = xstrdup(value);

    ret = true;
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Variable::set_value = %s", IOTF(ret));
#endif

  return(ret);
}

char *Variable::value()
{
  // Retrieve the variable value.

  // Returns: The variable value, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Variable::value()");
  dlog->log_exit(DEBUG_MINTRC, "Variable::value = %s", IONULL(v));
#endif
  
  return(v);
}

Variable::~Variable()
{
  // Deallocate the Variable object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "Variable::~Variable()");
#endif

  xdelete(n);
  xdelete(v);

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "Variable::~Variable()");
#endif
}
