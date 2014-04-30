/*
 * Test libparsexml stuff
 *
 * Usage: test (alert|authresult) [xmlfile]
 *
 */

#include "survivor.H"

int main(int argc, char **argv)
{
  if(!libsrvinit(argv[0]))
    exit(1);

  int fd = -1;
  
  if(argc < 3)
    fd = 0;
  else
    fd = open(argv[2], O_RDONLY);

  if(fd > -1)
  {
    SurvivorXML *s = new SurvivorXML();
    
    if(s)
    {
      if(strcmp(argv[1], "alert")==0)
      {
	Alert *a = s->parse_alert(fd);
	
	if(a)
	{
	  cout << "alert parsed: " << IONULL(a->service()) << endl;
	  
	  if(!s->generate(1, a, NULL))
	    cout << "generate failed" << endl;
	  
	  xdelete(a);
	}
	else
	  cout << "parse_alert failed" << endl;
      }
      else if(strcmp(argv[1], "authresult")==0)
      {
	CGIAuthResult *r = s->parse_cgiauthresult(fd);

	if(r)
	{
	  cout << "auth result: " << r->authok() << endl;

	  if(r->authok() == yes_authn)
	  {
	    cout << "username: " << r->username() << endl;

	    List *g = r->groups();

	    if(g)
	    {
	      for(int i = 0;i < g->entries();i++)
		cout << "member of: " << g->retrieve(i) << endl;
	    }
	  }

	  xdelete(r);
	}
	else
	  cout << "parse_cgiauthresult failed" << endl;
      }
      
      xdelete(s);
    }
    
    close(fd);
  }
  else
    cout << "unable to read " << (argc == 3 ? argv[2] : "-") << endl;

  libsrvexit();

  exit(1);
}
