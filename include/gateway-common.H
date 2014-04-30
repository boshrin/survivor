/*
 * gateway-common.H: Items that are specific to the gateway, but are
 *                   implemented in libsrv to take advantage of existing
 *                   code.
 *
 * Version: $Revision: 0.1 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2004/11/26 22:04:04 $
 *  
 * Copyright (c) 2004
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: gateway-common.H,v $
 * Revision 0.1  2004/11/26 22:04:04  benno
 * Initial revision
 *
 */

#if !defined(GATEWAY_COMMON_H)
#define GATEWAY_COMMON_H

/*
 * RelayTarget is used by GatewayConfiguration.  Names are used instead
 * of objects for CallList and Person because the main cfs are parsed
 * after gateway.cf.
 *
 */

class RelayTarget {
public:
  RelayTarget(char *name, char *instance, char *calllist);
  RelayTarget(char *name, char *instance, char *person, char *via);
  RelayTarget(char *name, RelayTarget *rt);
  char *calllist();
  char *instance();
  char *name();
  char *person();
  char *via();
  ~RelayTarget();

private:
  char *cl;   // The CallList to relay to
  char *in;   // The instance of this RelayTarget
  char *n;    // The name of this RelayTarget
  char *p;    // The Person to relay to
  char *v;    // For Persons, how to relay to them

  void init(char *name, char *instance, char *calllist, char *person,
	    char *via);
};

/*
 * GatewayConfiguration is filled out by lex, which is built in libsrv.
 *
 */

class GatewayConfiguration {
public:
  GatewayConfiguration();
  bool add_relay_target(RelayTarget *rt);
  bool parse_cf();
  bool relay_domain(char *domain);
  RelayTarget *relay_target(char *user);
  bool set_relay_all_domains();
  bool set_relay_domains(List *regexs);
  bool set_relay_localhost();
  ~GatewayConfiguration();

private:
  Hashtable<RelayTarget> *rts;   // Hash of defined RelayTargets
  List *rlydmns;                 // List of regexs of domains we accept
  bool rlyall;                   // Relay regardless of destination domain?
  bool rlylcl;                   // Relay only if destination is local host
};

#endif
