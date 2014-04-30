# Survivor::Check.pm
#
# Version: $Revision: 0.22 $
# Author: Benjamin Oshrin and Matt Selsky
# Date: $Date: 2006/01/23 02:17:24 $
#
# Copyright (c) 2002 - 2006
# The Trustees of Columbia University in the City of New York
# Academic Information Systems
# 
# License restrictions apply, see doc/license.html for details.

package Survivor::Check;

use strict;

use vars qw( @ISA $VERSION @EXPORT @hosts %flags %opt $timeout $doduration );
use Exporter;
use Survivor;
use Survivor::Debug;
use Survivor::XML;
@ISA = qw( Exporter Survivor );

@EXPORT = @Survivor::EXPORT;

use IO::Pipe;
use IO::Select;

=head1 NAME

Survivor::Check contains utilities for Survivor Perl Check modules.

=head1 SYNOPSIS

See src/modules/check/test/ for sample implementations.

=cut

=head1 CONSTRUCTOR

=over 4

=item new ()

Creates a C<Survivor::Check>, which is a reference to a newly created
Survivor::Check object. C<new()> takes the following arguments:

  Arguments
  =========
  None.

  Returns
  =======
  A reference to a newly created Survivor::Check object.

=back

=cut

sub new {
    my $class = shift;
    my $self = {};
    bless $self, $class;

    # Initialize flags
    $flags{"usethreads"} = 1;   # Let Exec try threading if available
    $flags{"usefork"} = 1;      # Let Exec handle forking if not
    $doduration = 1;            # Measure execution duration

    return $self;
}

=over 4

=item GetOpt(FORMAT, VALIDATE)

Subroutine to parse C<-v> in conformance with specification.  This
routine uses the XML Argument Format specification as the format of
the expected arguments provided in the check.cf and dependency.cf
files.  The definition of this format can be found in
doc/spec-xmlarg.html.

This will set C<Timeout()> to contain the timeout, C<Validate()> will
be set to indicate the module should validate itself, and C<Hosts()>
to have the same hosts but in an array fashion.  C<Arg()> can be used
to access named arguments.

If no hosts are provided, this subroutine will call
C<exit(MODEXEC_MISCONFIG)>.  If a usage error is encountered, an error
will be generated and C<exit(MODEXEC_MISCONFIG)> will be called.
If validation is requested, the validation subroutine will be called
and the module will then exit.

  Arguments
  =========
  FORMAT: A reference to an argument format hash.
  VALIDATE: A reference to a validation subroutine.

  Returns
  =======
  If successful.

=back

=cut

sub GetOpt {
    my $self = shift;
    my ($f, $v) = @_;

    my %flags = ("v" => "bool");
    my $err = $self->ParseFlags(\%flags, \%opt);

    if(exists $opt{'v'}) {
	# Perform the test here (before argument parsing, since the
	# test harness shouldn't need to know about each module's
	# arguments) and exit.

	my ($rc, $sv, $cmt) = &$v($self);
	$self->ExitError($rc, $sv, $cmt);
    }

    # Now get the rest of the config via XML on stdin

    my $sx = new Survivor::XML();

    my $requesttop = $sx->ParseStdin();
    
    if($requesttop) {
	# Extract timeout and hosts

	$timeout = $requesttop->{"children"}->{"SurvivorCheckData"}[0]->{"children"}->{"Timeout"}[0]->{"text"};
	
	@hosts = ();

	my $href = $requesttop->{"children"}->{"SurvivorCheckData"}[0]->{"children"}->{"Host"};

	foreach my $h (@{$href})
	{
	    my $xh = $h->{"text"};
	    
	    push(@hosts, $xh);
	}

        # Take any ModuleOptions and parse them as named arguments.
        # In order to do this, extract the provided values and store
        # them in an array, then call ParseNamedArgs

        my @named = ();

        my $oref = $requesttop->{"children"}->{"SurvivorCheckData"}[0]->{"children"}->{"ModuleOption"};
        
        foreach my $o (@{$oref})
        {
            if(defined $o->{"attrs"}{"OptionName"} &&
               defined $o->{"children"}->{"OptionValue"}[0]->{"text"})
            {
               my $xa = $o->{"attrs"}{"OptionName"} . "=" .
                    $o->{"children"}->{"OptionValue"}[0]->{"text"};

                push(@named, $xa);
            }
        }
        
        $self->ParseNamedArgs($f, "exit", @named);
    } else {
	# This probably won't do much since @hosts can't exist
	
        $self->ExitError(MODEXEC_MISCONFIG,
			 0,
			 "Valid SurvivorCheckData not provided");
    }
}

=over 4

=item Exec (CHECK)

Subroutine to properly execute a check routine in a multi-threaded
fashion if threads are available.  If threads are unavailable, the
routine will execute the checks in parallel via fork if possible, or
serially if not.  In order to use this method properly, the calling
code must create a check subroutine that is thread safe.  See
F<modules/check/test/test> for a simple example.

  Arguments
  =========
  CHECK: A reference to a subroutine that performs the check.

  Returns
  =======
  The highest return value obtained.

=back

=cut

sub Exec {
    use Config;

    my $self = shift;
    my $c = shift;

    my $ret = MODEXEC_OK;
    
    # If we're only checking one host, don't bother using threads or
    # forking since it will only slow us down.
    
    if($flags{"usethreads"} == 1 && $] >= 5.008 && $Config{useithreads}
       && scalar($self->Hosts()) > 1) {
	require Survivor::CheckMT;
	import Survivor::CheckMT;
	
	if(defined $dlog) {
	    $dlog->log_progress("$0 execing with threads");
	}
	
	my $smt = Survivor::CheckMT::new();

	$ret = $smt->ExecMT($self, $c);
    } elsif($flags{"usefork"} == 1 && scalar($self->Hosts()) > 1) {
	# If threading is not available, fall back to forking.
	# This basically implements the parallel module without
	# having to configure it as such.
	
	if(defined $dlog) {
	    $dlog->log_progress("$0 execing with fork");
	}
	
	$ret = $self->ExecFork($c);
    } else {
	# If forking is not available, fall back to serial mode.
	
	foreach my $h ($self->Hosts()) {
	    if(defined $dlog) {
		$dlog->log_progress("$0 execing serially", $h);
	    }

	    my $start = -1;
	    my $end = -1;

	    if($doduration) { $start = time(); }
	    
	    my ($rv, $sc, $cmt, $dur) = &$c($self, $h);

	    if($doduration) {
		$end = time();

		$dur = ($end - $start) * 1000;
	    }
	    
	    if($rv > $ret) {
		$ret = $rv;
	    }
	    
	    $self->Result($h, $rv, $sc, $cmt, $dur);
	}
    }

    return($ret);
}	

=over 4

=item ExecFork (CHECK)

Subroutine to properly execute a check routine in a parallel
fashion if threads are not available.

  Arguments
  =========
  CHECK: A reference to a subroutine that performs the check.

  Returns
  =======
  The highest return value obtained.

=back

=cut

sub ExecFork {    
    my $self = shift;
    my $c = shift;

    # MAX_FORKED is similar in intent (but not in implementation) to
    # DEFAULT_CMTHREADS (include/libcm.H).
    my $MAX_FORKED = 10;

    my $ret = MODEXEC_OK;
    my $todo = scalar($self->Hosts());
    my @tofork = $self->Hosts();
    my @running = ();
    my %children;
    my $readset = new IO::Select();

    while($todo > 0) {
	# Fire up new children until we have the maximum number running

	while(scalar(@running) < $MAX_FORKED && scalar(@tofork) > 0) {
	    my $h = pop(@tofork);

	    if(defined $dlog) {
		$dlog->log_progress("$0 execing via fork", $h);
	    }
	    
	    # Storing piped filehandles for later reference by the parent
	    # is a bit tricky.  Create local filehandles here (so new
	    # ones are used each time through), then pipe them.  Store
	    # them in the array.  To use them, they must be assigned to
	    # a scalar from the array.

	    my $pipe = new IO::Pipe;

	    my $pid = fork();

	    if(defined $pid) {
		if($pid == 0) {
		    # Child: Call subroutine, pass back results, and exit
		    $pipe->writer();
		    my $start = -1;
		    my $end = -1;
		    if($doduration) { $start = time(); }
		    my ($rv, $sc, $cmt, $dur) = &$c($self, $h);
		    if($doduration) {
			$end = time();			
			$dur = ($end - $start) * 1000;
		    }
		    $self->ResultFH($pipe, $h, $rv, $sc, $cmt, $dur);
		    exit($rv);
		} else {
		    $pipe->reader();
		    $children{$h} = $pid;
		    $readset->add($pipe);
		    push(@running, $h);
		}
	    } else {
		# If we fail to fork, generate an appropriate error for
		# that host and consider it finished.

		$self->Result($h, MODEXEC_PROBLEM, 0,
			      "Fork failure in ExecFork");
		$todo--;
	    }
	}

	# Wait for any subset of results from the currently running children

	my @ready = $readset->can_read();

	foreach my $rh (@ready) {
	    my $reading = 1;
	    my $answer = "";

	    # Read the result XML document on the filehandle $rh

	    my $sx = new Survivor::XML();

	    my $requesttop = $sx->ParseFH($rh);

	    # Check the data we got

	    my $host = $requesttop->{"children"}->{"SurvivorCheckResult"}[0]->{"children"}->{"Host"}[0]->{"text"};
	    my $rv = $requesttop->{"children"}->{"SurvivorCheckResult"}[0]->{"children"}->{"ReturnCode"}[0]->{"text"};
	    my $sc = $requesttop->{"children"}->{"SurvivorCheckResult"}[0]->{"children"}->{"Scalar"}[0]->{"text"};
	    my $cmt = $requesttop->{"children"}->{"SurvivorCheckResult"}[0]->{"children"}->{"Comment"}[0]->{"text"};
	    my $dur = $requesttop->{"children"}->{"SurvivorCheckResult"}[0]->{"children"}->{"Duration"}[0]->{"text"};
	    
	    if(defined $host && defined($children{$host})
	       && $children{$host} > -1) {
		$self->Result($host, $rv, $sc, $cmt, $dur);

		# See if this is our new return value
		if($rv > $ret) {
		    $ret = $rv;
		}

		# If this doesn't close the filehandle, we'll run out
		# for large numbers of hosts
		$readset->remove($rh);

		$children{$host} = -1;
		$todo--;

		# Wait to clean up the defunct process
		wait();
	    }
	    # else we got some random junk
	}

	# Rebuild running based on pids still outstanding

	@running = ();

	foreach my $k (keys %children) {
	    if($children{$k} > -1) {
		push(@running, $k);
	    }
	}
    }

    return($ret);
}

=over 4

=item ExitError (CODE, VALUE, STRING)

Subroutine to produce specification compliant error messages and then
exit.  This is intended for a generic error that is to be produced for
each host, such as if the module is misconfigured. C<Hosts()> is used
to generate the list of hosts.

  Arguments
  =========
  CODE: The exit code.
  VALUE: The scalar value.
  STRING: The descriptive string.

  Returns
  =======
  Nothing.

=back

=cut

sub ExitError {
    my $self = shift;

    my($ex, $sv, $err) = @_;

    if(!defined $ex or $ex eq '') {
	$ex = 0;
    }
    
    foreach my $h (@hosts) {
	$self->Result($h, $ex, $sv, $err);
    }

    # check for non-integer exit values
    if( $ex =~ /^\d+$/ ) { exit($ex) }
    else { exit }
}

=over 4

=item Hosts ()

Subroutine to access the list of hosts to be checked.

  Arguments
  =========
  None.

  Returns
  =======
  A list of hosts to be checked.

=back

=cut

sub Hosts {
    my $self = shift;
    
    @hosts;
}

=over 4

=item MeasureDuration (ON)

Enable or disable automatic execution duration measurement.

  Arguments
  =========
  ON: 1 to enable (the default), 0 to disable.

  Returns
  =======
  Nothing.

=back

=cut

sub MeasureDuration {
    my $self = shift;
    
    $doduration = shift;
}

=over 4

=item Result (HOST, CODE, VALUE, STRING[, DURATION])
    
Subroutine to display result document.

  Arguments
  =========
  HOST: The host name.
  CODE: The exit code.
  VALUE: The scalar value.
  STRING: The descriptive string.
  DURATION: Execution time in milliseconds.

  Returns
  =======
  Nothing.

=back

=cut

sub Result {
    my $self = shift;

    my($h, $ex, $sv, $s, $d) = @_;

    $self->ResultFH(\*STDOUT, $h, $ex, $sv, $s, $d);
}
    
=over 4

=item ResultFH (FH, HOST, CODE, VALUE, STRING[, DURATION])

Subroutine to display result document on a File Handle.

  Arguments
  =========
  FH: The file handle.
  HOST: The host name.
  CODE: The exit code.
  VALUE: The scalar value.
  STRING: The descriptive string.
  DURATION: Execution time in milliseconds.

  Returns
  =======
  Nothing.

=back

=cut

sub ResultFH {
    my $self = shift;

    my($fh, $h, $ex, $sv, $s, $d) = @_;

    if(defined $dlog) {
	$dlog->log_progress("Generating result document (rc=$ex)", $h);
    }

    my %xmltree = ("name" => "xml",
                   "text" => "",
                   "attrs" => {},
                   "children" => {},
                   "parent" => undef);

    my %scrdata = ("name" => "SurvivorCheckResult",
		   "text" => "",
		   "attrs" => {"Version" => "1.1"},
		   "children" => {},
		   "childorder" => ["Host", "ReturnCode", "Scalar", "Comment",
				    "Duration"],
		   "parent" => \%xmltree);
    $xmltree{"children"}{"SurvivorCheckResult"} = [\%scrdata];

    my %crhdata = ("name" => "Host",
		   "text" => $h,
		   "attrs" => {},
		   "children" => {},
		   "parent" => \%scrdata);
    $scrdata{"children"}{"Host"} = [\%crhdata];
    
    my %crrdata = ("name" => "ReturnCode",
		   "text" => ($ex eq '' ? 0 : $ex),
		   "attrs" => {},
		   "children" => {},
		   "parent" => \%scrdata);
    $scrdata{"children"}{"ReturnCode"} = [\%crrdata];
    
    my %crsdata = ("name" => "Scalar",
		   "text" => ($sv eq '' ? 0 : $sv),
		   "attrs" => {},
		   "children" => {},
		   "parent" => \%scrdata);
    $scrdata{"children"}{"Scalar"} = [\%crsdata];
    
    my %crcdata = ("name" => "Comment",
		   "text" => (defined $s ? $s : ''),
		   "attrs" => {},
		   "children" => {},
		   "parent" => \%scrdata);
    $scrdata{"children"}{"Comment"} = [\%crcdata];

    if(defined $d) {
	my %crddata = ("name" => "Duration",
		       "text" => $d,
		       "attrs" => {},
		       "children" => {},
		       "parent" => \%scrdata);
	$scrdata{"children"}{"Duration"} = [\%crddata];
    }
    
    my $sx = new Survivor::XML();

    $sx->Generate($fh, \%xmltree);
    
    if(defined $dlog) {
	$dlog->log_progress("Done generating result document", $h);
    }
}

=over 4

=item Timeout ()

Subroutine to determine the recommended timeout value.  0 indicates no
timeout was provided.

  Arguments
  =========
  None.

  Returns
  =======
  The timeout value.

=back

=cut

sub Timeout {
    my $self = shift;

    return(defined($timeout) ? $timeout : 0);
}

=over 4

=item UseFork (SET)

Subroutine to enable or disable the use of forking to parallelize
a module.  If enabled (default), Exec will automatically run a
module in parallel using fork() if threading is not available.
If disabled, Exec will run the module serially if threading is
not available, suitable for use with the compiled "parallel" module.

Check modules that wish to use Exec but wish to be executed serially
should set this flag to 0.  See also UseThreads().

  Arguments
  =========
  SET: 1 to allow the use of fork(), 0 to disable it.

  Returns
  =======
  Nothing.

=back

=cut

sub UseFork {
    my $self = shift;
    my $v = shift;

    $flags{"usefork"} = $v;
}

=over 4

=item UseThreads (SET)

Subroutine to enable or disable the use of threads to parallelize
a module.  If enabled (default), Exec will use threading if
available.  If disabled, Exec will not use threading even if
available.

Check modules that are not thread safe but wish to use use Exec should
set this flag to 0.  However, use of this flag for that reason is not
recommended, and modules should be written to be thread safe if they wish to
use Exec.  Alternately, a module may wish to verify that a threads-safe
version of a library is installed, and if not disable threading.

  Arguments
  =========
  SET: 1 to allow the use of threading, 0 to disable it.

  Returns
  =======
  Nothing.

=back

=cut

sub UseThreads {
    my $self = shift;
    my $v = shift;

    $flags{"usethreads"} = $v;
}

=over 4

=item Validate ()

Subroutine to determine if this module should validate and exit.

  Arguments
  =========
  None.

  Returns
  =======
  1 if validate was requested, 0 otherwise.

=back

=cut

sub Validate {
    my $self = shift;

    return(exists $opt{'v'} ? $opt{'v'} : 0);
}

1;
