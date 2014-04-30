/*
 * cgi-common.H: Items that are specific to the cgi, but are implemented
 *               in libsrv to take advantage of existing code.
 *
 * Version: $Revision: 0.2 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2005/04/09 03:02:04 $
 *  
 * Copyright (c) 2004
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: cgi-common.H,v $
 * Revision 0.2  2005/04/09 03:02:04  benno
 * Argument Arrays instead of Lists
 *
 * Revision 0.1  2004/03/02 03:22:52  benno
 * Initial revision
 *
 */

#if !defined(CGI_COMMON_H)
#define CGI_COMMON_H

/*
 * CGIAuthModule is parsed by lex into CGIConfiguration.
 *
 */

class CGIAuthModule {
public:
  CGIAuthModule(char *module, Array<Argument> *modopts);
  char *module();
  Array<Argument> *modopts();
  ~CGIAuthModule();

private:
  char *mname;             // The name of the module to run
  Array<Argument> *mopts;  // The options to pass to the module
};

/*
 * CGIAuthRequest is passed to Executor.
 *
 */

class CGIAuthRequest {
public:
  CGIAuthRequest(CGIAuthModule *module, char *uri, char *ip);
  char *ip();
  CGIAuthModule *module();
  char *uri();
  ~CGIAuthRequest();

private:
  CGIAuthModule *mod;      // The auth module to use for this request
  char *oi;                // The originating ip
  char *u;                 // The requested URI
};

/*
 * CGIAuthResult is received from Executor.
 *
 */

class CGIAuthResult {
public:
  CGIAuthResult(authn_result_t authok, char *username, List *groups,
		char *deferral, char *error, List *skipflags);
  authn_result_t authok();
  char *deferral();
  char *error();
  List *groups();
  List *skipflags();
  char *username();
  ~CGIAuthResult();

private:
  authn_result_t a;    // Authentication result
  List *g;             // Groups of authenticated user
  List *sf;            // Flags to skip when generating new uris
  char *d;             // Deferral document
  char *e;             // Error on auth failure
  char *u;             // Username of authenticated user
};

/*
 * CGIAuthorization is stored in CGIConfiguration
 *
 */

class CGIAuthorization {
public:
  CGIAuthorization(authz_type_t aztype, authz_level_t azlevel, char *name);
  authz_level_t authz_level();
  authz_type_t authz_type();
  char *name();
  ~CGIAuthorization();

private:
  authz_level_t al;    // The authorization level
  authz_type_t at;     // The authorization type
  char *n;             // The identifier for this authorization
};

/*
 * CGIConfiguration is filled out by lex, which is built in libsrv.
 *
 */

class CGIConfiguration {
public:
  CGIConfiguration();
  bool add_authmodule(CGIAuthModule *authmod);
  bool add_authorization(CGIAuthorization *authz);
  authz_level_t authorized(char *user, List *groups);
  Array<CGIAuthModule> *authmodules();
  char *clip_email();
  char *clip_phone();
  char *pageset();
  char *pageset_directory();
  bool parse_cf();
  int session_length();
  bool set_clip_email(char *email);
  bool set_clip_phone(char *phone);
  bool set_pageset(char *pageset);
  bool set_pageset_directory(char *srcdir);
  bool set_session_length(int len);
  bool set_state_directory(char *statedir);
  char *statedir();
  ~CGIConfiguration();

private:
  Array<CGIAuthorization> *authzs; // Authorizations
  Array<CGIAuthModule> *ams;       // WebAuth modules to use, in order
  char *ce;                        // Default clipboard email address
  char *cp;                        // Default clipboard phone number
  char *ps;                        // Default pageset
  char *psdir;                     // Where the pagesets are to be found
  char *sdir;                      // State directory
  int stime;                       // Session timeout
};

#endif
