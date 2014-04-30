/*
 * FileDebugger.C: File Debugging Object
 *
 * Note this debugger is not guaranteed to perform atomic writes, and so
 * should not be used when such a requirement is in effect.
 *
 * Version: $Revision: 0.1 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2003/07/17 20:56:41 $
 * MT-Level: Safe, although data written may be interleaved.
 *
 * Copyright (c) 2003
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: FileDebugger.C,v $
 * Revision 0.1  2003/07/17 20:56:41  benno
 * Initial revision
 *
 */

#include "survivor.H"

FileDebugger::FileDebugger()
{
  // Allocate and initialize a new FileDebugger object that logs to
  // "/tmp/filedebugger.out".

  // Returns: A new FileDebugger object.

  f = xstrdup("/tmp/filedebugger.out");
}

FileDebugger::FileDebugger(char *file)
{
  // Allocate and initialize a new FileDebugger object that logs to <file>.

  // Returns: A new FileDebugger object.

  f = xstrdup(file);
}

FileDebugger::~FileDebugger()
{
  // Deallocate the FileDebugger object.

  // Returns: Nothing.

  xdelete(f);
}

bool FileDebugger::do_log(bool warn, char *s)
{
  // Output <s>.  If <warn> is true, treat <s> as a warning, otherwise
  // as a log message.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;

  if(s && f)
  {
    FILE *out = fopen(f, "a");

    if(f)
    {
      if(warn)
	fprintf(out, "WARN: ");

      fprintf(out, "%s\n", s);

      fflush(out);
      fclose(out);
      
      ret = true;
    }
  }
  
  return(ret);
}
