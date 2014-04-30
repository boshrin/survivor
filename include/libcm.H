/*
 * libcm.H
 *
 * Version: $Revision: 0.15 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2006/01/23 00:44:53 $
 *
 * Copyright (c) 2002 - 2006
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: libcm.H,v $
 * Revision 0.15  2006/01/23 00:44:53  benno
 * Add cm_measure_duration
 *
 * Revision 0.14  2005/11/14 03:53:38  benno
 * RelationArg cleanup
 * Add cm_perform_extraction, cm_relation_compare
 *
 * Revision 0.13  2005/06/09 02:25:59  benno
 * SRProtocol(int *)
 *
 * Revision 0.12  2005/04/19 22:43:33  benno
 * Add _cm_fix, GENERATERESULTXML
 *
 * Revision 0.11  2005/04/09 02:53:25  benno
 * Add TMArgs, SRProtocol, changes for XML based args
 *
 * Revision 0.10  2004/06/11 22:25:41  benno
 * Add ExtractionArg
 *
 * Revision 0.9  2003/05/04 21:38:52  benno
 * Don't use string type
 *
 * Revision 0.8  2003/03/04 20:56:21  benno
 * Bump copyright
 *
 * Revision 0.7  2002/12/31 04:39:48  benno
 * Add unparsed
 *
 * Revision 0.6  2002/12/16 01:02:51  benno
 * Remove list separator
 *
 * Revision 0.5  2002/12/05 19:00:40  benno
 * add support for default arguments
 *
 * Revision 0.4  2002/10/21 20:42:20  benno
 * add named args
 *
 * Revision 0.3  2002/04/04 19:59:40  benno
 * copyright
 *
 * Revision 0.2  2002/04/02 22:07:37  benno
 * rcsify date
 *
 * Revision 0.1  2002/04/02 22:07:14  benno
 * initial revision
 *
 */

/* To use this library, the check module must define cmargs (see
 * below) and make it globally available.
 *
 * This library is slightly misnamed, since transport modules use it
 * also.  It should probably be renamed libmod.
 *
 */

#define DEFAULT_CMTHREADS 15

// The ArgSpec struct is used to specify the types of arguments the
// module requires.  It is designed to be compiled into a module and
// not need to be assembled dynamically at runtime.

#define INFINITY_ARG MAXFLOAT

enum argtype_t {
  boolean_arg,            // true, false
  directory_arg,          // string that must exist as a directory
  extraction_arg,         // column[x], substr[x,y]
  file_arg,               // string that must exist as a file
  flag_arg,               // string of characters, eg "avVs"
  number_arg,             // [-][0-9]*[.][0-9]+
  password_arg,           // alias for string, may do more later
  relation_arg,           // gt[x], lt[x], eq[x], ne[x], bt[x,y], nb[x,y]
  string_arg              // string of permitted characters
};

// The ArgParsed classes are used to provide the parsed named argument
// information at run time.

enum extraction_t {
  no_ext,                 // No extraction (placeholder)
  column_ext,             // Column
  substr_ext              // Substring
};

enum relation_t {
  no_rel,                 // No relation (placeholder)
  bt_rel,                 // Between
  eq_rel,                 // Equal
  gt_rel,                 // Greater than
  lt_rel,                 // Less than
  nb_rel,                 // Not between
  ne_rel,                 // Not equal
  re_rel,                 // Match regular expression
  rv_rel,                 // Not match regular expression
  tn_rel,                 // Time newer than
  to_rel                  // Time older than
};

// DefaultArg defines default values for named arguments

struct DefaultArg {
  bool bool_val;             // For BooleanArg
  char *char_val;            // For Flag, String, Directory, File, PasswordArg
  char *char_array;          // As above, takes precedence, not for FlagArg
  float float_val;           // For NumberArg, used if float_array is NULL
  Array<float> *float_array; // For NumberArg
  relation_t relation_type;  // For RelationArg
  char *relation_x;          // For RelationArg
  char *relation_y;          // For RelationArg
};

struct ArgSpec {
  char *argname;          // The name of this argument
  argtype_t argtype;      // The type of argument
  char **any;             // NULL terminated list, zero or more required
  float betweenx;         // Lower bound for between test (or INFINITY_ARG)
  float betweeny;         // Upper bound for between test (or INFINITY_ARG)
  bool list;              // If a list is expected
  char **one;             // NULL terminated list, exactly one required
  bool optional;          // Whether or not this argument is required
  DefaultArg *defval;     // Default value for this argument, if any
};

class ArgParsed {
public:
  ArgParsed(char *name, argtype_t argtype);
  argtype_t argtype();
  char *name();
  ~ArgParsed();

private:
  argtype_t a;            // Type of this argument
  char *n;                // The name of this argument
};

class BooleanArg : public ArgParsed {
public:
  BooleanArg(char *name, bool value);
  bool value();
  ~BooleanArg();

private:
  bool v;                 // The value of this argument
};

class ExtractionArg : public ArgParsed {
public:
  ExtractionArg(char *name, extraction_t extraction, int x);
  ExtractionArg(char *name, extraction_t extraction, int x, int y);
  extraction_t extraction();
  int x();
  int y();
  ~ExtractionArg();

private:
  extraction_t ext;       // The type of extraction
  int xval;               // The x value
  int yval;               // The y value
};

class FlagArg : public ArgParsed {
public:
  FlagArg(char *name, char *flags);
  bool flag(char f);
  ~FlagArg();

private:
  char *v;                // String containing provided flags
};

class NumberArg : public ArgParsed {
public:
  NumberArg(char *name, float value);
  NumberArg(char *name, Array<float> *values);
  float value();
  Array<float> *values();
  ~NumberArg();

private:
  float v;                // The single value of this argument
  Array<float> *vs;       // Or for a list of values
};

class RelationArg : public ArgParsed {
public:
  RelationArg(char *name, relation_t relation, char *x);
  RelationArg(char *name, relation_t relation, char *x, char *y);
  relation_t relation();
  char *x();
  char *y();
  ~RelationArg();
  
private:
  relation_t rel;         // Type of relation
  char *xval;             // Value 1
  char *yval;             // Value 2
};

class StringArg : public ArgParsed {
public:
  StringArg(char *name, char *value);
  StringArg(char *name, List *values);
  char *value();
  List *values();
  ~StringArg();

private:
  char *v;
  List *vs;
};

typedef StringArg DirectoryArg;
typedef StringArg FileArg;
typedef StringArg PasswordArg;

// CMArgs is the Check Module Argument class.

class CMArgs {
public:
  CMArgs();
  ArgParsed *arg(char *name);
  List *hosts();
  CheckResult *parse_args(int argc, char **argv, ArgSpec **argfmt,
			  CheckResult *(*validate)());
  int timeout();
  bool validating();
  ~CMArgs();

protected:
  bool tm;                   // Is this a TMArgs object?
  // If so, the following are here so parse_named_args can set them
  Array<Argument> *rmargs;   // Arguments for remote module
  char *mod;                 // Remote module
  char *mtype;               // Remote module type

private:
  List *h;                   // host list
  int t;                     // timeout
  bool v;                    // -v (validate)
  Hashtable<ArgParsed> *ht;  // Hashtable of parsed named arguments

  CheckResult *parse_named_args(ArgSpec **argfmt);
  char *validate_float(ArgSpec *arg, char *f, bool list);
  char *validate_string(ArgSpec *arg, char *value, bool list);
};

// TMArgs is the Transport Module Argument class, which is pretty
// similar to CMArgs.

class TMArgs : public CMArgs {
public:
  TMArgs() { tm = true; }
  char *modtype();
  char *module();
  Array<Argument> *rmodargs();
  ~TMArgs() {}

private:
};

// A class for implementing the SR protocol

class SRProtocol {
public:
  SRProtocol(int fd);
  SRProtocol(int *fds);
  CheckResult *check(char *module, CheckRequest *cr);
  bool exit();
  CheckResult *fix(char *module, FixRequest *fr);
  char *version();
  ~SRProtocol();

private:
  FileHandler *fin;   // Easier manipulation of fds
  FileHandler *fout;  // Easier manipulation of fds
  bool invalid;       // If we see any invalid syntax, stop trying
  int f;              // Pointer to f
  int *fs;            // Pointer to fds
};

/* From cmutils.C */
extern bool _cm_fix;
void cm_error(int rc, int scalar, char *msg);
void cm_measure_duration(bool on);
char *cm_perform_extraction(ExtractionArg *earg, char *s);
bool cm_relation_compare(RelationArg *rarg, char *s, char *label,
			 char **rtext);
int  cm_thread_exec(CheckResult *(*check)(int));

extern CMArgs *cmargs;

// Macro to dump FixResult or CheckResult.  We need the full if/else
// to allow for proper method binding at compile time.
#define GENERATERESULTXML(c) if(_cm_fix) \
  sxml->generate(STDOUT_FILENO, (FixResult *)c); \
 else \
  sxml->generate(STDOUT_FILENO, c);
