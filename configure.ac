# Survivor Autoconf

# Version: $Revision: 0.12 $
# Author: Benjamin Oshrin
# Date: $Date: 2007/01/25 13:11:14 $
# Copyright (c) 2003 - 2007
# The Trustees of Columbia University in the City of New York
# Academic Information Systems
# 
# License restrictions apply, see doc/license.html for details.

# Initialize autoconf

AC_INIT([[SURVIVOR: The Systems Monitor]],
	[[v1.0]],
	[[survivor-dev@lists.columbia.edu]],
	[[survivor]])

# Require a recent version of autoconf

AC_PREREQ(2.57)

### First see if we are only configuring for a remote-only build

AC_ARG_ENABLE([remote-only],
	      AC_HELP_STRING([--enable-remote-only],
			     [Configure for remote installation only, default is no]),
			     [enable_remote_only="yes"],
			     [enable_remote_only="no"])
			     
if test "$enable_remote_only" = "no"; then
### Full build
   echo "Configuring a full build"
   
### Checks for programs

# See if we need to define _GNU_SOURCE
AC_GNU_SOURCE()

if test "$CXXFLAGS" = ""; then
# Don't let configure set -g -O2, which appears to break pthread_join
   CXXFLAGS=-O2
fi

# Find the C++ compiler
AC_PROG_CXX()

# Find ranlib
AC_PROG_RANLIB()

# Find lex
AC_PROG_LEX()

### Checks for C library functions

AC_CHECK_FUNCS([getdomainname])
	
### Checks for libraries

# Do we need -lnsl?
AC_CHECK_LIB([nsl],
	     [gethostbyname],
	     AC_SUBST(LIBNSL, [-lnsl]))

# Do we need -lsocket?
AC_CHECK_LIB([socket],
	     [connect],
	     AC_SUBST(LIBSOCKET, [-lsocket]))

# Do we need -lrt?
AC_CHECK_LIB([rt],
	     [sem_init],
	     AC_SUBST(LIBRT, [-lrt]))

# Do we need -lintl for localization?  Note on AIX we really need this,
# on Solaris we don't, but it causes no harm.
AC_CHECK_LIB([intl],
	     [gettext],
	     AC_SUBST(LIBINTL, [-lintl]))

# Do we need to always link -lpthread since Debugger references pthread_self
# and no stub is defined in libc?
AC_CHECK_LIB([c],
	     [pthread_self],
	     ,
	     AC_SUBST(LIBTHREADCORE, [-lpthread]))
	     
### Checks for header files

# Must have posix threads or don't even bother

AC_CHECK_HEADERS([pthread.h],
		 [],
		 AC_MSG_ERROR([cannot build without posix threads]))

# SSL is optional

AC_ARG_WITH([openssl],
	    AC_HELP_STRING([--with-openssl=DIR],
			   [Use OpenSSL, default is yes]),
	    [ac_cv_enable_ssl=$withval],
	    [ac_cv_enable_ssl=yes])

if test "$ac_cv_enable_ssl" != "no"; then
    if test "$withval" != "yes"; then
	CPPFLAGS="${CPPFLAGS} -I${withval}/include"
	
	AC_CHECK_HEADER([openssl/crypto.h],
			AC_SUBST(SSL_CCFLAGS, "-I${withval}/include -DWITH_SSL")
			AC_SUBST(SSL_LDFLAGS, "-R${withval}/lib -L${withval}/lib -lssl -lcrypto"),
			AC_MSG_ERROR([openssl include directory not found]))
    else
	AC_CHECK_HEADER([openssl/crypto.h],
			AC_SUBST(SSL_CCFLAGS, "-DWITH_SSL")
			AC_SUBST(SSL_LDFLAGS, "-lssl -lcrypto"),
			AC_MSG_ERROR([openssl include directory not found]))
    fi
fi

AC_CHECK_HEADERS([libintl.h])

# libintl isn't everywhere

# gethostby[name|addr]_r have different prototypes
# Based on code by Caolan McNamara <caolan@skynet.ie> at
#  http://www.gnu.org/software/ac-archive/htmldoc/ac_caolan_func_which_gethostbyname_r.html

ac_gethostr_args=0
AC_MSG_CHECKING([number of arguments to gethostbyname_r])

AC_TRY_COMPILE(
[#include <netdb.h>],
[struct hostent *h;
 struct hostent_data data;
 (void)gethostbyname_r("foo", h, &data);],
ac_gethostr_args=3,
[AC_TRY_COMPILE(
 [#include <netdb.h>],
 [struct hostent *h, *res;
  char buffer[1024];
  int h_errno;
  (void)gethostbyname_r("foo", h, buffer, 1024, &res, &h_errno);],
 ac_gethostr_args=6,
 [AC_TRY_COMPILE(
  [#include <netdb.h>],
  [struct hostent *h;
   char buffer[1024];
   int h_errno;
   (void)gethostbyname_r("foo", h, buffer, 1024, &h_errno);],
  ac_gethostr_args=5,
  AC_MSG_ERROR([usable gethostbyname_r not found]))])])
			
AC_MSG_RESULT([$ac_gethostr_args])			

if test $ac_gethostr_args = 3; then
   AC_DEFINE([HAVE_FUNC_GETHOSTBYNAME_R_3], [3], [gethostbyname_r with 3 args])
elif test $ac_gethostr_args = 5; then
   AC_DEFINE([HAVE_FUNC_GETHOSTBYNAME_R_5], [5], [gethostbyname_r with 5 args])
elif test $ac_gethostr_args = 6; then
   AC_DEFINE([HAVE_FUNC_GETHOSTBYNAME_R_6], [6], [gethostbyname_r with 6 args])
fi
								     	
### Checks for types

### Checks for system services

# XXX This is a temporary hack until the OS dependent stuff can be
# properly ported

OS=`uname -s`
REV=`uname -r`

case $OS in
    AIX)
	AC_DEFINE(AIX, [1], [Temporary definition])
	;;
    Darwin)
	# Must be 6.0 or later.
	case $REV in
	    5.*)
		AC_MSG_ERROR([This package requires Darwin 6.0 or later])
		;;
	    *)
		AC_DEFINE(DARWIN, [1], [Temporary definition])
		;;
	esac
	;;
    Linux)
	AC_DEFINE(LINUX, [1], [Temporary definition])
	;;
    NetBSD)
	AC_MSG_NOTICE([NetBSD port not yet complete.])
	AC_DEFINE(NETBSD, [1], [Temporary definition])
	;;
    SunOS)
		# Must be recent Solaris, not SunOS.
	case $REV in
	    4.*)
		AC_MSG_ERROR([This package will not build on SunOS 4.x])
		;;
	    5.[12345]*)
		AC_MSG_ERROR([This package requires SunOS 5.6 or later])
		;;
	    5.6*)
		AC_MSG_NOTICE([This package has not been tested on SunOS 5.6.])
		AC_DEFINE(SOLARIS6, [1], [Temporary definition])
		;;
	    5.7)
		AC_DEFINE(SOLARIS7, [1], [Temporary definition])
		;;
	    5.8)
		AC_DEFINE(SOLARIS8, [1], [Temporary definition])
		;;
	    *)
		AC_DEFINE(SOLARIS, [1], [Temporary definition])
		;;
	esac
	;;
    *)
	AC_MSG_ERROR([This package is not known to build on this platform])
	;;
esac

# More customizations

# Use _GNU_SOURCE if defined
AH_TEMPLATE([_GNU_SOURCE],
	    [Enable GNU extensions on system that require them.])

## For the following tests, autoconf will do the right thing for --enable
## or --disable.  The fourth argument specifies the default.
   
## Determine if debugging should be compiled in
   
AC_ARG_ENABLE([debug],
	      AC_HELP_STRING([--enable-debug],
			     [Compile in debugging utilities, default is yes]),
			     [],
			     [enable_debug="yes"])

if test "$enable_debug" = "yes"; then
    AC_DEFINE(ENABLE_DEBUG, [1], [Enable debugging.])
fi

## Determine if module serialization should be disabled
	
AC_ARG_ENABLE([module-serialization],
	      AC_HELP_STRING([--enable-module-serialization],
			     [Enable module execution serialization, default is yes]),
			     [],
			     [enable_module_serialization="yes"])

if test "$enable_module_serialization" = "yes"; then
    AC_DEFINE(ENABLE_MODULE_SERIALIZATION, [1], [Enable module execution serialization.])
fi

else
### Remote-only Build
   echo "Configuring a remote-only build"
fi

### Items in this section are common to full and remote-only build

# Use bsd style install
AC_PROG_INSTALL()

## Determine user and group to install and run as.  We can't test user
## here since we probably don't have permission to chown files.

AC_ARG_ENABLE([user],
	      AC_HELP_STRING([--enable-user=x],
			     [Set the installation user to x, default is survivor]),
	      [ac_cv_enable_user=$enableval],
	      [ac_cv_enable_user=survivor])

AC_ARG_ENABLE([group],
	      AC_HELP_STRING([--enable-group=x],
			     [Set the installation group to x, default is staff]),
	      [ac_cv_enable_group=$enableval],
	      [ac_cv_enable_group=staff])

# Write user and group to install and run as
AC_SUBST(INST_USER, $ac_cv_enable_user)
AC_SUBST(INST_GROUP, $ac_cv_enable_group)

### Write out autoconf'd files

AC_CONFIG_HEADERS([include/config.H])

# Dynamically determine the set of modules to create makefiles for

CHECKMODS=`(cd $srcdir ; echo modules/check/*/Makefile.in | sed 's/modules\/check\/common\/Makefile.in//' | sed 's/Makefile.in/Makefile/g')`
FIXMODS=`(cd $srcdir ; echo modules/fix/*/Makefile.in | sed 's/modules\/fix\/common\/Makefile.in//' | sed 's/Makefile.in/Makefile/g')`
FORMATMODS=`(cd $srcdir ; echo modules/format/*/Makefile.in | sed 's/modules\/format\/common\/Makefile.in//' | sed 's/Makefile.in/Makefile/g')`
REPORTMODS=`(cd $srcdir ; echo modules/report/*/Makefile.in | sed 's/modules\/report\/common\/Makefile.in//' | sed 's/Makefile.in/Makefile/g')`
TRANSMITMODS=`(cd $srcdir ; echo modules/transmit/*/Makefile.in | sed 's/modules\/transmit\/common\/Makefile.in//' | sed 's/Makefile.in/Makefile/g')`
TRANSPORTMODS=`(cd $srcdir ; echo modules/transport/*/Makefile.in | sed 's/modules\/transport\/common\/Makefile.in//' | sed 's/Makefile.in/Makefile/g')`
WEBAUTHMODS=`(cd $srcdir ; echo modules/webauth/*/Makefile.in | sed 's/modules\/webauth\/common\/Makefile.in//' | sed 's/Makefile.in/Makefile/g')`

if test "$enable_remote_only" = "no"; then
   # Only do this for full build
   AC_CONFIG_SUBDIRS(libparsexml/expat)
fi

AC_CONFIG_FILES([Makefile
		 Makefile.inc
		 cgi/Makefile
		 cli/Makefile
		 doc/Makefile
		 gateway/Makefile
		 init.d/survivor
		 libcm/Makefile
		 libdebug/Makefile
		 libparse822/Makefile
		 libparsecgi/Makefile
		 libparsexml/Makefile
		 libsrv/Makefile
		 libui/Makefile
		 modules/check/Makefile
		 modules/check/Makefile.generic
		 $CHECKMODS
		 modules/common/Makefile
		 modules/fix/Makefile
		 modules/fix/Makefile.generic
		 $FIXMODS
		 modules/format/Makefile
		 modules/format/Makefile.generic
		 $FORMATMODS
		 modules/report/Makefile
		 modules/report/Makefile.generic
		 $REPORTMODS
		 modules/transmit/Makefile
		 modules/transmit/Makefile.generic
		 $TRANSMITMODS
		 modules/transport/Makefile
		 modules/transport/Makefile.generic
		 $TRANSPORTMODS
		 modules/webauth/Makefile
		 modules/webauth/Makefile.generic
		 $WEBAUTHMODS
		 remote/Makefile
		 scheduler/Makefile])

AC_OUTPUT
