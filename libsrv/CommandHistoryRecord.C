/*
 * CommandHistoryRecord.C: Object to manage command history records
 *
 * Version: $Revision: 0.1 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2003/11/08 15:36:31 $
 * MT-Level: Unsafe
 *
 * Copyright (c) 2003
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: CommandHistoryRecord.C,v $
 * Revision 0.1  2003/11/08 15:36:31  benno
 * Initial revision
 *
 */

#include "survivor.H"

CommandHistoryRecord::CommandHistoryRecord()
{
  // Allocate and initialize a new CommandHistoryRecord object.

  // Returns: A new CommandHistoryRecord object.

  init(NULL, NULL, NULL);
}

CommandHistoryRecord::CommandHistoryRecord(char *command, char *who,
					   char *comment)
{
  // Allocate and initialize a new CommandHistoryRecord object, holding
  // <command>, <who>, <comment>.

  // Returns: A new CommandHistoryRecord object.

  init(command, who, comment);
}

char *CommandHistoryRecord::command()
{
  // Obtain the command stored within.

  // Returns: A pointer to a string containing the command, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CommandHistoryRecord::command()");
  dlog->log_exit(DEBUG_MINTRC, "CommandHistoryRecord::command = %s",
		 IONULL(cmd));
#endif
  
  return(cmd);
}

char *CommandHistoryRecord::comment()
{
  // Obtain the command comment stored within.

  // Returns: A pointer to a string containing the comment, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CommandHistoryRecord::comment()");
  dlog->log_exit(DEBUG_MINTRC, "CommandHistoryRecord::comment = %s",
		 IONULL(cmt));
#endif
  
  return(cmt);
}

char *CommandHistoryRecord::who()
{
  // Obtain the who identity stored within.

  // Returns: A pointer to a string containing the identity, or NULL.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CommandHistoryRecord::who()");
  dlog->log_exit(DEBUG_MINTRC, "CommandHistoryRecord::who = %s", IONULL(w));
#endif
  
  return(w);
}

CommandHistoryRecord::~CommandHistoryRecord()
{
  // Deallocate the CommandHistoryRecord.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC,
		  "CommandHistoryRecord::~CommandHistoryRecord()");
#endif
  
  xdelete(cmd);
  xdelete(cmt);
  xdelete(m);
  xdelete(w);
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC,
		 "CommandHistoryRecord::~CommandHistoryRecord()");
#endif
}

char *CommandHistoryRecord::make_string()
{
  // Generate a string containing the data portion of the record entry.

  // Returns: A pointer to a string that should not be modified and
  // contains the data portion of the record, or NULL on error.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CommandHistoryRecord::make_string()");
#endif

  // Toss any old data
  xdelete(m);

  CharBuffer *cbuf = new CharBuffer();

  if(cbuf)
  {
    cbuf->append(cmd);
    cbuf->append(':');
    cbuf->append(w);
    cbuf->append(':');
    cbuf->append(cmt);

    m = xstrdup(cbuf->str());
    xdelete(cbuf);
  }
  else
    wlog->warn("CommandHistoryRecord::make_string failed to allocate cbuf");
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CommandHistoryRecord::make_string = %s",
		 IONULL(m));
#endif

  return(m);
}

bool CommandHistoryRecord::parse_string(char *s)
{
  // Parse the contents of <s>.

  // Returns: true if fully successful, false otherwise.

  bool ret = false;

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CommandHistoryRecord::parse_string(%s)",
		  IONULL(s));
#endif

  if(s)
  {
    // Toss any previous data

    xdelete(cmd);
    xdelete(cmt);
    xdelete(w);
    
    // Make a copy of s and run it through strtok

    char *sx = xstrdup(s);

    if(sx)
    {
      toss_eol(sx);
      
      char *last;
      char *r = strtok_r(sx, ":", &last);

      if(r)
      {
	cmd = xstrdup(r);

	r = strtok_r(NULL, ":", &last);

	if(r)
	{
	  w = xstrdup(r);

	  r = strtok_r(NULL, "\n", &last);

	  if(r)
	  {
	    cmt = xstrdup(r);
	    ret = true;
	  }
	  else
	    wlog->warn("CommandHistoryRecord::parse_string did not find comment");
	}
	else
	  wlog->warn("CommandHistoryRecord::parse_string did not find identity");
      }
      else
	wlog->warn("CommandHistoryRecord::parse_string did not find command");
      
      xdelete(sx);
    }
    else
      wlog->warn("CommandHistoryRecord::parse_string failed to copy s");
  }

#if defined(debug)
  dlog->log_exit(DEBUG_MINTRC, "CommandHistoryRecord::parse_string = %s",
		 IOTF(ret));
#endif
  
  return(ret);
}

void CommandHistoryRecord::init(char *command, char *who, char *comment)
{
  // Initializer for CommandHistoryRecord object.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "CommandHistoryRecord::init(%s,%s,%s)",
		  IONULL(command), IONULL(who), IONULL(comment));
#endif

  cmd = xstrdup(command);
  w = xstrdup(who);
  cmt = xstrdup(comment);
  m = NULL;

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "CommandHistoryRecord::init()");
#endif
}
