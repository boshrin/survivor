/*
 * test: Compiled test module
 *
 * Version: $Revision: 0.2 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2003/01/29 01:30:33 $
 * MT-Level: Safe
 *
 * Copyright (c) 2002 - 2003
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: main.C,v $
 * Revision 0.2  2003/01/29 01:30:33  benno
 * xdelete
 *
 * Revision 0.1  2002/12/31 04:13:52  benno
 * Initial revision
 *
 */

#include "survivor.H"

// Required to use libcm
CMArgs *cmargs = NULL;

// Define our named argument structures
ArgSpec test1 = {
  "test1",        // arg name
  boolean_arg,    // arg type
  NULL,           // any list
  INFINITY_ARG,   // between lower bound
  INFINITY_ARG,   // between upper bound
  false,          // expect a list
  NULL,           // one list
  false,          // optional
  NULL            // default value
};

DefaultArg test2default = {
  false, "foo", NULL, 0.0, NULL, no_rel, NULL, 0.0, 0.0
};

ArgSpec test2 = {
  "test2",        // arg name
  string_arg,     // arg type
  NULL,           // any values list
  INFINITY_ARG,   // between lower bound
  INFINITY_ARG,   // between upper bound
  false,          // expect a list
  NULL,           // one values list
  true,           // optional
  &test2default   // default value
};

char *test3vals[] = {"blue", "white", "red", NULL};

ArgSpec test3 = {
  "test3",        // arg name
  string_arg,     // arg type
  test3vals,      // any values list
  INFINITY_ARG,   // between lower bound
  INFINITY_ARG,   // between upper bound
  true,           // expect a list
  NULL,           // one values list
  true,           // optional
  NULL            // default value
};

ArgSpec test4 = {
  "test4",        // arg name
  directory_arg,  // arg type
  NULL,           // any values list
  INFINITY_ARG,   // between lower bound
  INFINITY_ARG,   // between upper bound
  false,          // expect a list
  NULL,           // one values list
  false,          // optional
  NULL            // default value
};

ArgSpec test5 = {
  "test5",        // arg name
  file_arg,       // arg type
  NULL,           // any values list
  INFINITY_ARG,   // between lower bound
  INFINITY_ARG,   // between upper bound
  false,          // expect a list
  NULL,           // one values list
  true,           // optional
  NULL            // default value
};

char *test6flags[] = {"a", "v", "S", NULL};

ArgSpec test6 = {
  "test6",        // arg name
  flag_arg,       // arg type
  test6flags,     // any values list
  INFINITY_ARG,   // between lower bound
  INFINITY_ARG,   // between upper bound
  false,          // expect a list
  NULL,           // one values list
  true,           // optional
  NULL            // default value
};

DefaultArg test7default = {
  false, NULL, NULL, 100.0, NULL, no_rel, NULL, 0.0, 0.0
};

ArgSpec test7 = {
  "test7",        // arg name
  number_arg,     // arg type
  NULL,           // any values list
  0.0,            // between lower bound
  INFINITY_ARG,   // between upper bound
  false,          // expect a list
  NULL,           // one values list
  true,           // optional
  &test7default   // default value
};

ArgSpec test8 = {
  "test8",        // arg name
  relation_arg,   // arg type
  NULL,           // any values list
  INFINITY_ARG,   // between lower bound
  INFINITY_ARG,   // between upper bound
  false,          // expect a list
  NULL,           // one values list
  true,           // optional
  NULL            // default value
};

// And put them in a NULL terminated array
ArgSpec *argfmt[] = {
  &test1,
  &test2,
  &test3,
  &test4,
  &test5,
  &test6,
  &test7,
  &test8,
  NULL
};

CheckResult *check(int i)
{
  CheckResult *rv = NULL;
  
  // List->retrieve is MT-Safe.  cmargs is set before this is called.

  char *h = cmargs->hosts()->retrieve(i);

  if(h)
  {
    rv = new CheckResult(MODEXEC_MISCONFIG, 0, "");
    
    if(rv)
    {
      // rv->set_comment("xxx");
      // rv->set_rc(0);
    }
  }
    
  return(rv);
}

CheckResult *validate()
{
  CheckResult *rv = new CheckResult(MODEXEC_OK, 0, "Test OK");

  return(rv);
}

int main(int argc, char **argv)
{
  int r = MODEXEC_OK;  

  if(!libsrvinit(argv[0]))
    exit(MODEXEC_MISCONFIG);
    
  cmargs = new CMArgs();

  if(cmargs)
  {
    CheckResult *rv = cmargs->parse_args(argc, argv, argfmt, validate);

    if(rv)
    {
      if(rv->rc() == MODEXEC_OK && !cmargs->validating())
      {
	BooleanArg *a1 = (BooleanArg *)cmargs->arg("test1");
	
	if(a1)
	  cout << "Value of test1 is " << a1->value() << endl;
	else
	  cout << "test1 not provided" << endl;
	
	StringArg *a2 = (StringArg *)cmargs->arg("test2");
	
	if(a2)
	  cout << "Value of test2 is " << a2->value() << endl;
	else
	  cout << "test2 not provided" << endl;
	
	StringArg *a3 = (StringArg *)cmargs->arg("test3");
	
	if(a3 && a3->values())
	{
	  cout << "Value of test3 is";
	  
	  for(int i = 0;i < a3->values()->entries();i++)
	    cout << " " << a3->values()->retrieve(i);
	  
	  cout << endl;
	}
	else
	  cout << "test3 not provided" << endl;
	
	DirectoryArg *a4 = (DirectoryArg *)cmargs->arg("test4");
	
	if(a4)
	  cout << "Value of test4 is " << a4->value() << endl;
	else
	  cout << "test4 not provided" << endl;
	
	FileArg *a5 = (FileArg *)cmargs->arg("test5");
	
	if(a5)
	  cout << "Value of test5 is " << a5->value() << endl;
	else
	  cout << "test5 not provided" << endl;
	
	FlagArg *a6 = (FlagArg *)cmargs->arg("test6");
	
	if(a6)
	{
	  cout << "test6 a => " << a6->flag('a') << endl;
	  cout << "test6 v => " << a6->flag('v') << endl;
	  cout << "test6 S => " << a6->flag('S') << endl;
	}
	else
	  cout << "test6 not provided" << endl;
	
	NumberArg *a7 = (NumberArg *)cmargs->arg("test7");
	
	if(a7)
	  cout << "Value of test7 is " << a7->value() << endl;
	else
	  cout << "test7 not provided" << endl;
	
	RelationArg *a8 = (RelationArg *)cmargs->arg("test8");
	
	if(a8)
	  cout << "Value of test8 is relop=" << a8->relation()
	       << " [" << a8->x() << "," << a8->y() << "]" << endl;
	else
	  cout << "test8 not provided" << endl;
      }
      else
      {
	r = rv->rc();
	cm_error(r, 0, rv->comment());
      }
      
      xdelete(rv);
    }
    else
    {
      r = MODEXEC_MISCONFIG;
      cm_error(r, 0, "failed to parse named args");
    }
    
    xdelete(cmargs);
  }
  else  // No point calling cm_error with no hosts
    r = MODEXEC_PROBLEM;

  libsrvexit();
  
  exit(r);
}
