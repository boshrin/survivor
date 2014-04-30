# Survivor::Debug.pm
#
# Version: $Revision: 0.1 $
# Author: Benjamin Oshrin
# Date: $Date: 2006/01/24 23:39:37 $
#
# Copyright (c) 2005
# The Trustees of Columbia University in the City of New York
# Academic Information Systems
# 
# License restrictions apply, see doc/license.html for details.

package Survivor::Debug;

use strict;

use vars qw( @ISA $VERSION @EXPORT %flags );
use Exporter;
use Survivor;
use Sys::Syslog;
@ISA = qw( Exporter Survivor );

@EXPORT = @Survivor::EXPORT;

=head1 NAME

Survivor::Debug contains handlers for logging debugging information
for Survivor modules.

=head1 SYNOPSIS

Module utilities such as Survivor::Check.pm may automatically call
these routines under appropriate circumstances.  However, it is
possible to supplement those calls or to invoke these routines directly.
    
Call new() with an appropriate argument list to set up how debugging
is handled, then call any of the log_ routines to perform the actual
logging.

    $dbg = new Survivor::Debug("file", "/tmp/debug.log");
    $dbg->log_entry("my_function()");
    $dbg->log_progress("host1", "beginning check");
    $dbg->log_exit("my_function() = 1");

=cut

=head1 CONSTRUCTOR

=over 4

=item new (LOGTYPE[, LOGTARGET])

Creates a C<Survivor::Debug>, which is a reference to a newly created
Survivor::Debug object. C<new()> takes the following arguments:

  Arguments
  =========
  LOGTYPE: The type of logging to perform, "syslog" or "file".
  LOGTARGET: If LOGTYPE is "file", the filename to log to.

  Returns
  =======
  A reference to a newly created Survivor::Debug object.

=back

=cut

sub new {
    my $class = shift;
    my $self = {};
    bless $self, $class;

    # Initialize flags
    $flags{"logtype"} = shift;
    if($flags{"logtype"} eq "file") {
	$flags{"logtarget"} = shift;
    }

    return $self;
}

=over 4

=item log_entry(STRING)

Log an entry point.

  Arguments
  =========
  STRING: The text to log for the entry point.

  Returns
  =======
  Nothing.

=back

=cut

sub log_entry {
    my $self = shift;
    my ($l) = @_;

    write_line("-> " . $l);
}

=over 4

=item log_exit(STRING)

Log an exit point.

  Arguments
  =========
  STRING: The text to log for the exit point.

  Returns
  =======
  Nothing.

=back

=cut

sub log_exit {
    my $self = shift;
    my ($l) = @_;

    write_line("<- " . $l);
}

=over 4

=item log_progress(STRING[, HOST[, ...]])

Log progress.

  Arguments
  =========
  STRING: The progress text to log.
  HOST: Zero or more hostnames that the progress refers to.
    The string will be logged once per host.

  Returns
  =======
  Nothing.

=back

=cut

sub log_progress {
    my $self = shift;
    my $l = shift;

    if(scalar(@_) > 0) {
	foreach my $h (@_) {
	    write_line("|" . $h . "| " . $l);
	}
    } else {
        write_line("|| " . $l);
    }
}

=over 4

=item write_line(STRING)

This subroutine performs the actual write according to the configuration.
This subroutine should not be called directly.

  Arguments
  =========
  STRING: The line to log.

  Returns
  =======
  Nothing.

=back

=cut

sub write_line {
    my $l = shift;

    # We don't hold any filehandles open, which may not be the most
    # efficient approach but should suffice for a first draft.

    if($flags{"logtype"} eq "file") {
	if(defined $flags{"logtarget"} && $flags{"logtarget"} ne "") {
	    if(open(LOG, ">> ".$flags{"logtarget"})) {
		my $t = localtime(time);

		printf(LOG "$t $$ $l\n");  # $$ = pid
		close(LOG);
	    }
	}
    } else {
	# syslog, which will handle the timestamp and pid/tid

	openlog("mod", "pid", "user");
	syslog("debug", $l);
	closelog();
    }
}

1;
