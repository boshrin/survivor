/*
 * test the Debugger library
 *
 */

#include <survivor.H>

Debugger *debug = NULL;

char *progname = "test";

int main(int argc, char **argv)
{
  debug = new SyslogDebugger();

  if(debug)
  {
    debug->set_level(DEBUG_MAJTRC);
    debug->enable_timestamp();
    debug->log_entry(DEBUG_MAJTRC, "Foo::Foo(%d,%s)", argc, argv[0]);
    
    xdelete(debug);
  }
}
