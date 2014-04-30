# Survivor.pm
#
# Version: $Revision: 0.42 $
# Author: Matt Selsky and Benjamin Oshrin, based on survivor.pl 
#   by Benjamin Oshrin
# Date: $Date: 2006/01/26 00:05:03 $
#
# Common definitions and utilities for survivor perl modules.
# To use, add the following magical lines to the beginning of the module:
#
#  use FindBin;
#  use lib "$FindBin::Bin/../common";
#  use Survivor;
#
# Copyright (c) 2002 - 2006
# The Trustees of Columbia University in the City of New York
# Academic Information Systems
# 
# License restrictions apply, see doc/license.html for details.

package Survivor;

use strict;

use vars qw( @ISA $VERSION @EXPORT %accresults %args %opt %format $dlog );

use Exporter;
use Survivor::Debug;
@ISA = qw( Exporter );
'$Revision: 0.42 $ ' =~ m/(\d+)\.(\d+)/ && (( $VERSION ) = sprintf("%d.%03d", $1, $2));

=head1 NAME

Survivor contains common definitions and utilities for Survivor Perl modules.

=head1 SYNOPSIS

See src/modules/check/test/ for examples of how to use the Survivor
modules.

=cut

@EXPORT = qw( MODEXEC_OK MODEXEC_PROBLEM MODEXEC_WARNING MODEXEC_NOTICE
	      MODEXEC_MISCONFIG MODEXEC_TIMEDOUT MODEXEC_DEPEND
	      MODEXEC_NOTYET $dlog );

### PATH adjustments: This is the only item in this file likely to be
  # adjusted locally.

$ENV{'PATH'} = '/opt/local/bin:/usr/local/bin:/usr/sbin:/sbin:/usr/etc:/usr/kvm:/usr/bin:/bin:'.$ENV{'PATH'};

=head1 CONSTRUCTOR

=over 4

=item new ()

Creates a C<Survivor>, which is a reference to a newly created
Survivor object. C<new()> takes no arguments.

=back

=cut

sub new {
    my $class = shift;
    my $self = {};
    bless $self, $class;
    
    return $self;
}

### Exit codes (as defined in include/survivor.H)

=head1 DEFINES

Survivor defines the following exit codes from F<include/survivor.H>:

    MODEXEC_OK
    MODEXEC_PROBLEM
    MODEXEC_WARNING
    MODEXEC_NOTICE
    MODEXEC_MISCONFIG
    MODEXEC_TIMEDOUT
    MODEXEC_DEPEND

These symbols are exported by default.

=cut

sub MODEXEC_OK { 0; }
sub MODEXEC_PROBLEM { 1; }
sub MODEXEC_WARNING { 2; }
sub MODEXEC_NOTICE { 3; }
sub MODEXEC_MISCONFIG { 4; }
sub MODEXEC_TIMEDOUT { 5; }
sub MODEXEC_DEPEND { 6; }

=head1 METHODS

=over 4

=item AccResult (HOST, SCALAR)

Subroutine to generate one result line for the accumulated results.

  Arguments: A host name and a scalar value.
  Returns: The largest return value accumulated.

=back

=cut

sub AccResult {
    my $self = shift;

    my($h, $sv) = @_;

    # $sv2 should be the same as $sv
    my($r, $sv2, $t) = AccResultGet($h, $sv);
    
    $self->Result($h, $r, $sv, $t);

    return($r);
}

=over 4

=item AccResultAdd (HOST, CODE, STRING)

Subroutine to add a subresult to the accumulated results.  Currently,
the following exit codes are accepted: MODEXEC_OK, MODEXEC_WARNING,
MODEXEC_PROBLEM.

  Arguments: A host name, an exit code, and a text string.
  Returns: Nothing.

=back

=cut

sub AccResultAdd {
    my $self = shift;

    my($h, $ex, $s, $key);

    ($h, $ex, $s) = @_;

    if($ex == MODEXEC_PROBLEM) {
	$key = $h.'-p';
    } elsif($ex == MODEXEC_WARNING) {
	$key = $h.'-w';
    } else {
	$key = $h.'-o';
    }
    
    $accresults{$key} .= $s.',';
}

=over 4

=item AccResultGet (HOST, SCALAR[, MODE])

Subroutine to obtain a result tuple for the accumulated results.

  Arguments
  =========
  HOST:   Hostname to retrieve results for
  SCALAR: Scalar value to return
  MODE:   "any" to indicate any failure means overall failure, "all" to
          indicate that all results must be failure for overall failure
          (If not specified, "any" is assumed.)

  Returns
  =======
  An exit code, a scalar value, and a text string.

=back

=cut

sub AccResultGet {
    my $self = shift;

    my($h, $sv, $mode) = @_;

    if(!defined($mode)) {
	$mode = "any";
    }
    
    # First retrieve any stored results for this host and toss the trailing
    # comma.
    my $key;
    $key = $h.'-p';
    my $tp = $accresults{$key};
    defined $tp && chop($tp);

    $key = $h.'-w';
    my $tw = $accresults{$key};
    defined $tw && chop($tw);

    $key = $h.'-o';
    my $to = $accresults{$key};
    defined $to && chop($to);
    
    # Now start assembling t.  Only append PROBLEM, WARNING, or OK
    # if more than one will be used.

    my $t = '';
    my $r = MODEXEC_OK;

    if(defined $tp) {
	if(!defined $tw  && !defined $to) {
	    $t .= $tp.',';
	} else {
	    $t .= 'PROBLEM:'.$tp.',';
	}

	$r = MODEXEC_PROBLEM;
    }

    if(defined $tw) {
	if(!defined $tp && !defined $to) {
	    $t .= $tw.',';
	} else {
	    $t .= 'WARNING:'.$tw.',';
	}

	if($mode eq "all" || $r != MODEXEC_PROBLEM) {
	    $r = MODEXEC_WARNING;
	}
    }
    
    if(defined $to) {
	if(!defined $tp && !defined $tw) {
	    $t .= $to.',';
	} else {
	    $t .= 'OK:'.$to.',';
	}

	if($mode eq "all") {
	    $r = MODEXEC_OK;
	}
    }

    # Toss the trailing comma
    chop($t);

    return($r, $sv, $t);
}

=over 4

=item AlertGetOpt ()

Subroutine to parse C<-achHiHrsStT> in conformance with specification.
This will set C<Opt('a')> to contain the requested argument,
C<Opt('c')> will contain the call list name, C<Opt('h')> will contain
the host, C<Opt('H')> will contain the path to the helpfile,
C<Opt('i')> will contain the number of instances, C<Opt('I')> will
contain the name of the instance that generated the alert, C<Opt('r')>
will contain the return code, C<Opt('s')> will contain the summary,
C<Opt('S')> will contain the service name, C<Opt('t')> will contain
the time in epoch seconds, and C<Opt('T')> will contain the token ID.
If no arguments are provided, this subroutine will call
C<exit(MODEXEC_MISCONFIG)>.  If a usage error is encountered, an error
will be generated and C<exit(MODEXEC_MISCONFIG)> will be called.

  Arguments: None.
  Returns: If successful.

=back

=cut

sub AlertGetOpt {
    # XXX This should use ParseFlags.  See Check::GetOpt for when this
    # is moved into Survivor::Alert.pm.
    my $self = shift;

    my %allowed = ( 'a' => 1, 'c' => 1, 'h' => 1,
		    'H' => 1, 'i' => 1, 'I' => 1,
		    'r' => 1, 's' => 1, 'S' => 1,
		    't' => 1, 'T' => 1 );

    my $err;
    
    while($_ = shift(@ARGV)) {
	my $arg = $_;
	$arg =~ s/^-//;

	if( /^-[^-]/ && exists $allowed{$arg} ) {
	    $opt{$arg} = shift(@ARGV) || exit(MODEXEC_MISCONFIG);
	}
	else {
	    $err = "Unknown option $_";
	}
    }

    # XXX Host isn't defined
    # XXX does this make any sense anyway?
    if(defined $err) {
	$self->ExitError(MODEXEC_MISCONFIG, 0, $err);
    }

    unless( exists $opt{'a'} ) {
	exit(MODEXEC_MISCONFIG);
    }
}

=over 4

=item Arg (NAME)

Subroutine to access the hash of named arguments.

  Arguments
  =========
  NAME: The name of the argument to retrieve.

  Returns
  =======
  The argument's value.

=back

=cut

sub Arg {
    my $self = shift;
    my $r = shift;

    # We need to know if the item we're looking for is a list or not.
    # We default to no, unless we find yes.

    my $list = 0;

    if(exists $format{$r}) {
	foreach my $kw (split(/ /, $format{$r})) {
	    if($kw eq "list") {
		$list++;
	    }
	}
    }
    
    if(exists $args{$r}) {
	# We return everything as an array.  If the caller is expecting
	# only one argument, it will just work.  If they are expecting
	# a list, it will also just work.  If they are expecting a hash,
	# it will need to be dereferenced.
	
	my $x = $args{$r};
	my @r = @$x;

	if($list) {
	    return(@r);
	} else {
	    return($r[0]);
	}
    } else {
	if($list) {
	    my @r = ();
	    return(@r);
	} else {
	    return(undef);
	}
    }
}

=over 4

=item Glob (STRING)

Replacement for perl C<glob>, which can break for more than 1 or 2
thousand files because of shortcomings in /bin/csh.

  Arguments: A string to be globbed.
  Returns: A list of filenames that match (if any), an error message,
  or the pattern itself, if no matches are found.  Filenames can be
  distinguished from error messages by the lack of a leading /
  character.

=back

=cut

sub Glob {
    my $self = shift;

    my(@candidates, @errors);
    
    my $glob = shift;

    my @gs = split(m!/!, $glob);
    shift(@gs);  # Get rid of leading empty entry

    push(@candidates, '');
    
    foreach my $g (@gs) {
	my @newcandidates;

	foreach my $c (@candidates) {
	    my $dir;
	    # Special case: "" = "/"
	    if($c eq "") { $dir = "/"; }
	    else { $dir = $c; }

	    if(opendir(SURVIVORDIRP, $dir)) {
		my @ds = grep(!/^\.\.?/, readdir(SURVIVORDIRP));

		foreach my $d (@ds) {
		    my $cd = $c."/".$d;   # The thing that exists
		    my $cg = "^".$c."/".$g."\$";  # The regex we want to match
		                               # (exact, not substring)

		    if($cd =~ /$cg/) {
			push(@newcandidates, $c.'/'.$d);
		    }
		}
		
		closedir(SURVIVORDIRP);
	    }
	    else {
		# This is permission denied, since if a directory doesn't
		# exist it will be excluded during the regex comparison.
		push(@errors, "Permission denied opening $dir");
	    }
	}

	@candidates = @newcandidates;
    }

    if(scalar(@errors) > 0) {
	return(@errors);
    } elsif(scalar(@candidates) == 0) {
	return($glob);
    } else {
	return(@candidates);
    }
}

=over 4

=item IsBSDish ()

Subroutine to determine if the operating system is BSD-like or SYSV-like.

  Arguments: None.
  Returns: True if operating system is BSD-ish, false otherwise.

=back

=cut

sub IsBSDish {
    if( ! -d '/proc' ) {
	return 1;
    } else {
	return 0;
    }
}

=over 4

=item MountedFilesystems ()

Subroutine to determine currently mounted filesystems.  If an error is
encountered, C<ExitError()> will be called.

  Arguments: None.
  Returns: Reference to a hash of mounted filesystems, where the mount
  point is the key and the device is the value, unless an error is
  encountered.

=back

=cut

sub MountedFilesystems {
    my $self = shift;
    my %mnt;

    # Look for /etc/mnttab or /etc/mtab.  If neither exist, exit error.
    if(-e '/etc/mnttab') {
	open(IN, '/etc/mnttab') ||
	    $self->ExitError(MODEXEC_MISCONFIG, 0, 'Unable to read /etc/mnttab');

	while(<IN>) {
	    chomp;
	    s/#.*$//;

	    # For now, we only check mount point
	    my($dev, $mount, $fstype, $options, $mtime) = split;

	    if($dev && $mount) {
		$mnt{$mount} = $dev;
	    }
	}
	
	close(IN);
    } elsif(-e '/etc/mtab') {
	open(IN, '/etc/mtab') ||
	    $self->ExitError(MODEXEC_MISCONFIG, 0, 'Unable to read /etc/mtab');

	while(<IN>) {
	    chomp;
	    s/#.*$//;

	    # For now, we only check mount point
	    my($dev, $mount, $fstype, $options, $freq, $pass) = split;

	    if($dev && $mount) {
		$mnt{$mount} = $dev;
	    }
	}
	
	close(IN);
    } else {
	$self->ExitError(MODEXEC_MISCONFIG, 0,
		      '/etc/mnttab or /etc/mtab not found');
    }

    return \%mnt;
}

=over 4

=item OperatingSystem ()

Subroutine to get the current operating system.

  Arguments: None.
  Returns: String containing the current operating system (eg 'linux',
  'solaris').

=back

=cut

sub OperatingSystem {
    my $self = shift;

    $^O;
}

=over 4

=item Opt (ARG)

    (This routine is deprecated)

Subroutine to access a command line option.

  Arguments: The argument to access.
  Returns: The value of the argument if found, or undef otherwise.

=back

=cut

sub Opt {
    my $self = shift;

    my $arg = shift;

    if( exists $opt{$arg} ) {
	return $opt{$arg};
    }
    else {
	return undef;
    }
}

=over 4

=item ParseFlags (FLAGSET, FLAGHASH)

Subroutine to parse command line arguments in a reliable fashion.
(The various getopts.pl and GetOpt::Foo routines are not consistently
implemented across multiple releases of Perl, and do not return useful
error information.)

This subroutine is primarily intended to be called by a GetOpt routine
within Survivor/*.pm, but could be used elsewhere.

  Arguments
  =========
  FLAGSET: A pointer to a hash consisting of the single letter flags to
           be accepted as keys, and "bool" or "arg" to indicate the type of
           flag.
  FLAGHASH: A pointer to a hash in which the matched flags and their values
            will be stored.  Additional arguments will be stored in a pointer
            to an array accessible via the hash key "args".

  Returns
  =======
  An error string, which will be empty if successful.

=back

=cut

sub ParseFlags {
    my ($self, $flagset, $flaghash) = @_;
    my $done = 0;
    my $err = "";
    my @args = ();
    
    while(($a = shift(@ARGV))) {
	if(!$done && $a =~ /^-/) {
	    if($a eq "--") {
		# Special flag to indicate end of flags
		$done = 1;
	    } else {
		$a =~ s/^-//;

		if(!defined $$flagset{$a}) {
		    $err .= "Unknown option '-$a',";
		} elsif($$flagset{$a} eq "arg") {
		    # Flag takes an argument
		    $b = shift(@ARGV);
		    
		    if(defined $b) {
			$$flaghash{$a} = $b;
		    } else {
			$err .= "Option '-$a' requires an argument,";
		    }
		} else {
		    # Boolean flag
		    $$flaghash{$a} = 1;
		}
	    }
	} else {
	    push(@args, $a);
	    $done = 1;  # No more -flags accepted
	}
    }

    # Any arguments get stuffed into an array and returned
    $$flaghash{"args"} = \@args;

    chop($err);

    return($err);
}
    
=over 4

=item ParseNamedArgs (FORMAT, ERRMODE, ARGS)

Subroutine to parse an array consisting of named arguments of the form
"name=value".  The format is defined by the ArgFormat specification
passed to the constructor.

If there is an error while parsing the arguments, the appropriate
error routines will be called.

  Arguments
  =========
  FORMAT: Reference to hash of format specification
  ERRMODE: "exit" to exit on error or "return" to return an error string
  ARGS: An array of name=value strings.

  Returns
  =======
  An empty string if successful, or for "return" ERRMODE an error
  string if not.

=back

=cut

sub ParseNamedArgs {
    my $self = shift;
    my $a = shift;
    my $em = shift;
    
    # Argument format specification, global so Arg() can see it
    %format = %$a;  # Format

    # Arguments that were provided by GetOpt
    my %pargs;         # Provided (actual)

    # Arguments that have been successfully verified
    my %ret;

    # Errors encountered, comma separated
    my $err = "";
    
    # Parse provided arguments
    my @named = @_;
    
    foreach my $na (@named) {
	if($na =~ /[A-Za-z0-9_\.-]+=/) {
	    my ($n, $t) = split(/=/, $na, 2);

	    # Make sure this is a known argument
	    if(exists $format{$n}) {
		if(exists $pargs{$n}) {
		    # Add this to the list of values for this argument

		    my $xl = $pargs{$n};
		    push(@$xl, $t);
		} else {
		    # Create a new array for this argument

		    my @xl = ($t);
		    $pargs{$n} = \@xl;
		}
	    } elsif($n eq "debugfile" || $n eq "debugsyslog") {
		# Special debugging arguments, valid in all modules

		if(!defined $dlog) {
		    if($n eq "debugfile") {
			$dlog = new Survivor::Debug("file", $t);
		    } else {
			$dlog = new Survivor::Debug("syslog");
		    }
		} else {
		    $err .= "'$n' is invalid: debugging already enabled,";
		}
	    } else {
		$err .=
		    "Provided argument '$n' is neither required nor optional,";
	    }
	} else {
	    # Args provided must be x=y or we won't know what to do with them
	    $err .= "Ambiguous argument '$na' not of name=value form,";
	}
    }

    # Make sure each required argument has been specified, assign default
    # values for optional arguments that haven't been provided.

    foreach my $k (keys %format) {
	if(!exists $pargs{$k}) {
	    # Only test for arguments that weren't provided
	    
	    if($format{$k} =~ /optional/) {
		# Optional argument, look for default by means of a hack
		my $kw = $format{$k};
		if($kw =~ /default\(/) {
		    $kw =~ s/.*default\(//;
		    $kw =~ s/\).*//;
		    if($kw ne "") {
			my @xl = ($kw);
			$pargs{$k} = \@xl;
		    }
		}
	    } else {
		# Required argument, complain
		$err .= "Required argument '$k' not provided,";
	    }
	}
    }
    
    # Iterate through each provided argument and verify its type.

    foreach my $k (keys %pargs) {
	# Keywords can be specified in any order so long as they are
	# separated by a space.

	# For each item, we look for several flags.  The difference between
	# "between" and "relation" is that we verify the value in between
	# here, whereas for relation we're just passing back the relation
	# specifier.
	
	my $list = 0;        # Expect a list of arguments
	my $argtype = "";    # The actual type
	my @one;       # List of items, one must be specified
	my @any;       # List of items, any can be specified
	my $betweenx = "inf";# Lower bound for between
	my $betweeny = "inf";# Upper bound for between
	my @argvalues; # Tokenized values passed as the argument

	foreach my $kw (split(/ /, $format{$k})) {
	    if($kw eq "list") {
		$list++;
	    } elsif($kw eq "boolean"
		    || $kw eq "directory"
		    || $kw eq "extraction"
		    || $kw eq "file"
		    || $kw eq "flags"
		    || $kw eq "number"
		    || $kw eq "password"
		    || $kw eq "relation"
		    || $kw eq "string") {
		if($argtype eq "") { $argtype = $kw; }
		else {
		    $err .= "Argument specifier for '$k' wants but cannot have both types '$argtype' and '$kw',";
		}
	    } elsif($kw =~ /^any\(/) {
		$kw =~ s/^any\(//;
		$kw =~ s/\)$//;
		@any = split(/,/, $kw);
	    } elsif($kw =~ /^between\(/) {
		$kw =~ s/^between\(//;
		$kw =~ s/\)$//;
		($betweenx, $betweeny) = split(/,/, $kw);
	    } elsif($kw =~ /^one\(/) {
		$kw =~ s/^one\(//;
		$kw =~ s/\)$//;
		@one = split(/,/, $kw);
	    } elsif($kw =~ /^default\(/) {
		# Ignore default, we've already checked it
	    } elsif($kw ne "optional") {
		# Ignore optional, we've already checked it, but anything
		# else is an error

		$err .= "Unknown argument specifier '$kw' for argument '$k',";
	    }
	}

	# Now that we have all the potential bits,  make sure we have the
	# corresponding actual bits as well.  First check the type.

	# file and directory are just strings that we stat.

	my $xl = $pargs{$k};
	@argvalues = @$xl;
	my $firstarg = $argvalues[0];  # for convenience
	my $totalargs = scalar(@argvalues);
	
	if($argtype eq "file") {
	    foreach my $f (@argvalues) {
		if(!-f $f &&
		   !-l $f &&
		   !-p $f &&
		   !-S $f) {
		    $err .= "File '$f' does not exist for argument '$k',";
		}
	    }
	} elsif ($argtype eq "directory") {
	    foreach my $f (@argvalues) {
		if(!-d $f) {
		    $err .= "Directory '$f' does not exist for argument '$k',";
		}
	    }
	}

	# flag check is a variation on string.
	# Note continuation of previous if/else clause.
	
	elsif($argtype eq "flags") {
	    # We can't accept lists because we create one

	    if(!$list) {
		# Break up the string into a list for any/one parsing,
		# but don't set $list so that the return value is the
		# original string.

		@argvalues = split(/ */, $firstarg);
	    } else {
		$err .= "Flag argument '$k' cannot accept lists,";
	    }
	}

	if(!$list && $totalargs > 1) {
	    $err .= "Argument '$k' does not permit multiple values,";
	}
    
	# Note termination of previous continuing else clause.
	
	if($argtype eq "string" || $argtype eq "password"
	   || $argtype eq "file" || $argtype eq "directory"
	   || $argtype eq "flags") {
	    # string and its aliases require no validation, just store it
	    # (directory and file behave like string after stat)
	    
	    $ret{$k} = $pargs{$k};
	} elsif($argtype eq "number") {
	    # Make sure what we were passed looks vaguely like a number,
	    # then verify between if set.

	    foreach my $num (@argvalues) {
		if($num =~ /^[0-9\.-]+$/) {
		    if($betweenx ne "inf" && $num < $betweenx) {
	    $err .= "Value '$num' for argument '$k' must be at least $betweenx,";
		    } elsif($betweeny ne "inf" && $num > $betweeny) {
       $err .= "Value '$num' for argument '$k' must not be more than $betweeny,";
		    }
		} else {
		    $err .= "Value '$num' for argument '$k' is not a number,";
		}
	    }
	    
	    $ret{$k} = $pargs{$k};
	} elsif($argtype eq "boolean") {
	    # Check for known boolean type values, standardize by setting
	    # the return value to 0 or 1.  No lists are accepted since
	    # having a list of booleans defeats having named arguments.
	    # arg1=true,false,false,true is unreadable.

	    if(!$list) {
		my $b = lc($firstarg);
		my @xl;
		
		if($b eq "true" || $b eq "t" || $b eq "yes" || $b eq "y"
		   || $b eq "1") {
		    @xl = (1);
		    $ret{$k} = \@xl;
		} elsif($b eq "false" || $b eq "f" || $b eq "no" || $b eq "n"
		   || $b eq "0") {
		    @xl = (0);
		    $ret{$k} = \@xl;
		} else {
		   $err .=
		       "Value '$firstarg' for argument '$k' is not boolean,";
		}
	    } else {
		$err .= "Boolean argument '$k' cannot accept lists,";
	    }
	} elsif($argtype eq "extraction") {
	    # Check for known extractions and break up the answer
	    # into an array.  No lists accepted for the same reason
	    # as boolean.
	    
	    if(!$list) {
		my @xl;
		my $ext = $firstarg;
		$ext =~ s/\[.*$//;

		if($ext eq "column" || $ext eq "substr") {
		    if($ext eq "column") {
			# We expect one number
			if(!($firstarg =~ /\[[0-9-]+\]/)) {
			    $err .= "Extraction '$ext' for argument '$k' requires one numeric value,";
			}
		    } else {
			# We expect two numbers
			if(!($firstarg =~ /\[[0-9-]+,[0-9-]+\]/)) {
			    $err .= "Extraction '$ext' for argument '$k' requires two numeric values,";
			}
		    }

		    # Split and store.  For one argument extractions,
		    # the y value will be undefined.

		    my $extnum = $firstarg;
		    $extnum =~ s/^.*\[//;
		    $extnum =~ s/\].*$//;
		    my ($extx, $exty) = split(/,/, $extnum);
		    
		    my %extret;
		    $extret{"ext"} = $ext;
		    $extret{"x"} = $extx;
		    $extret{"y"} = $exty;

		    @xl = (\%extret);
		    $ret{$k} = \@xl;
		} else {
		    $err .= "Invalid extraction '$ext' for argument '$k',";
		}
	    } else {
		$err .= "Extraction argument '$k' cannot accept lists,";
	    }
	} elsif($argtype eq "relation") {
	    # Check for known relations and break up the answer into an array.
	    # No lists accepted for the same reason as boolean.

	    if(!$list) {
		my @xl;
		my $rel = $firstarg;
		$rel =~ s/\[.*$//;

		if($rel eq "lt" || $rel eq "gt" || $rel eq "eq"
		   || $rel eq "ne" || $rel eq "bt" || $rel eq "nb"
		   || $rel eq "tot" || $rel eq "tnt") {
		    # Simple check for number of arguments

		    if($rel eq "bt" || $rel eq "nb") {
			if(!($firstarg =~ /\[[0-9-]+,[0-9-]+\]/)) {
			    $err .= "Relation '$rel' for argument '$k' requires two numeric values,";
			}
		    } else {
			if(!($firstarg =~ /\[[0-9-]+\]/)) {
			    $err .= "Relation '$rel' for argument '$k' requires one numeric value,";
			}
		    }

		    # Split and store.  For one argument relations,
		    # the y value will be undefined.

		    my $relnum = $firstarg;
		    $relnum =~ s/^.*\[//;
		    $relnum =~ s/\].*$//;
		    my ($relx, $rely) = split(/,/, $relnum);

		    my %relret;
		    $relret{"rel"} = $rel;
		    $relret{"x"} = $relx;
		    $relret{"y"} = $rely;

		    @xl = (\%relret);
		    $ret{$k} = \@xl;
		} elsif($rel eq "reg" || $rel eq "regv") {
		    # Just store the whole thing.

		    my $relreg = $firstarg;
		    $relreg =~ s/^.*\[//;
		    $relreg =~ s/\].*$//;
		    
		    my %relret;
		    $relret{"rel"} = $rel;
		    $relret{"s"} = $relreg;

		    @xl = (\%relret);
		    $ret{$k} = \@xl;
		} else {
		    $err .= "Invalid relation '$rel' for argument '$k',";
		}
	    } else {
		$err .= "Relation argument '$k' cannot accept lists,";
	    }
	} else {
	    $err .= "Unknown argument type '$argtype' due to programmer error,";
	}
	
	# Then check any restrictions on content that apply to more than
	# one type.

	if(scalar(@one) > 0) {
	    # Only one value is accepted, and it must be in the list
	    
	    my $found = 0;

	    if(scalar(@argvalues) == 1) {
		my $a = pop(@argvalues);
		
		foreach my $o (@one) {
		    if($o eq $a) { $found++; }
		}

		if(!$found) {
		    $err .= "Value '$a' for argument '$k' is not permitted,";
		}
	    } else {
		$err .=
	    "Multiple values are not permitted to be assigned to argument '$k',";
	    }
	}

	if(scalar(@any) > 0) {
	    # Any number of values are accepted, but they must be in the list
	    
	    if(scalar(@argvalues) > 0) {
		foreach my $a1 (@argvalues) {
		    my $found = 0;
		    
		    foreach my $a2 (@any) {
			if($a1 eq $a2) { $found++; }
		    }

		    if(!$found) {
		       $err .= "Value '$a1' for argument '$k' is not permitted,";
		    }
		}
	    }
	}
    }
    
    # Exit if something went awry and error mode is exit

    if($err ne "") {
	chop($err);

	if($em eq "exit") {
	    $self->ExitError(MODEXEC_MISCONFIG, 0, $err);
	}
    }

    # Otherwise return the error string (potentially empty) and set args

    %args = %ret;

    return($err);
}

=over 4

=item ParsePS (LOOKUPUSERS, PROCESSNAME)

Obtain information about the current processes by parsing the output
from "ps" as appropriate for the local platform.

  Arguments
  =========
  LOOKUPUSERS: If 1, lookup usernames for process owners (can be expensive)
  PROCESSNAME: Zero or more of flags
                a: keep all arguments
                i: discard interpreter (first arg)

  Returns
  =======
  A reference to a hash of the form
    ret{"cpu"}{pid}: CPU usage in seconds for process pid
    ret{"mem"}{pid}: Memory usage in KB for process pid
    ret{"name"}{pid}: Process name and perhaps arguments
    ret{"time"}{pid}: Real run time in seconds for process pid
                      (not supported on BSD)
    ret{"uid"}{pid}: UID that owns pid (on BSD, may be -1 if LOOKUPUSERS
					is true)
    ret{"user"}{pid}: Username that owns pid (will be uid if LOOKUPUSERS
					      is false)
  or undef on error.

=back

=cut

sub ParsePS {
    my $self = shift;
    my $lookupuser = shift;
    my $processname = shift;
    my %ret;

    # Figure out which ps to use

    my $psx = "";

    if(!$self->IsBSDish())
    {
	# If we don't need username, use uid instead of user for faster
	# lookups.
	
	if(!$lookupuser) {
	    $psx = '/bin/ps -e -o uid,uid,pid,vsz,time,etime,args |';
	} else {
	    $psx = '/bin/ps -e -o user,uid,pid,vsz,time,etime,args |';
	}
    }
    else
    {
	# If ps supports -L, it'll work similarly

	my $x = `ps -L`;

	if($? == 0)
	{
	    # If we don't need username, use uid instead of user for faster
	    # lookups.

	    # BSD ps doesn't have etime, so we fill in the field with the
	    # cpu time so we can keep one parsing routine, below
	    
	    if(!$lookupuser) {
		$psx = '/bin/ps -ax -o uid,uid,pid,vsz,time,time,command |';
	    } else {
		$psx = '/bin/ps -ax -o user,uid,pid,vsz,time,time,command |';
	    }
	}
    }
    
    if($psx ne "") {
	open(ANSWER, $psx) || return(undef);
    
	# skip header
	<ANSWER>;
	
	while(<ANSWER>) {
	    chomp;
	    # Toss initial spaces
	    s/^\s+//;
	    my($psuser,$psuid,$pspid,$psmem,$pscpu,$pstime,$psname) =
		split(/\s+/, $_, 7);
	    
	    $psname = process_name($psname, $processname);
	    
	    next unless(defined $psname && $psname ne "");

	    $ret{"cpu"}{$pspid} = time_to_seconds($pscpu);
	    $ret{"mem"}{$pspid} = $psmem;
	    $ret{"name"}{$pspid} = $psname;
	    if(!$self->IsBSDish()) {
		$ret{"time"}{$pspid} = time_to_seconds($pstime);
	    } else {
		$ret{"time"}{$pspid} = -1;
	    }
	    $ret{"uid"}{$pspid} = $psuid;
	    $ret{"user"}{$pspid} = $psuser;
	}
	
	close(ANSWER);
    } else {
	# BSD implementations may not give reliable info or formatting,
	# especially on ancient SunOS

	if($lookupuser) {
	    $psx = '/bin/ps -aux |';
	} else {
	    $psx = '/bin/ps -anux |';
	}
    
	open(ANSWER, $psx) || return(undef);
	
	# skip header
	<ANSWER>;
	
	while(<ANSWER>) {
	    chomp;
	    
	    my($psuser, $pspid, $psmem, $pscpu, $psname) =
		(split(/\s+/, $_, 11))[0,1,4,9,10];
	    
	    $psname = process_name($psname, $processname);
	    
	    next unless(defined $psname && $psname ne "");
	    
	    $ret{"cpu"}{$pspid} = time_to_seconds($pscpu);
	    $ret{"mem"}{$pspid} = $psmem;
	    $ret{"name"}{$pspid} = $psname;
	    $ret{"time"}{$pspid} = -1;
	    $ret{"user"}{$pspid} = $psuser;

	    if(!$lookupuser) {
		$ret{"uid"}{$pspid} = $psuser;
	    } else {
		# uid unavailable
		$ret{"uid"}{$pspid} = -1;
	    }
	}
    
	close(ANSWER);
    }

    return(\%ret);

    sub time_to_seconds {
	# Normalize time to seconds

	my ($pscpu) = @_;

	if($pscpu eq "-")
	{
	    # defunct process
	    return(0);
	}
	
	my $cpu = 0;
	my (@cl) = split(/:/, $pscpu);
	
	my $seconds = pop(@cl);

	if(defined $seconds)
	{
	    $cpu += $seconds;
	}

	my $minutes = pop(@cl);

	if(defined $minutes)
	{
	    $cpu += ($minutes * 60);
	}

	my $hours = pop(@cl);
	
	if(defined $hours)
	{
	    if($hours =~ /[0-9]+-[0-9]+/) {
		# Form is d-hh

		my ($d, $h) = split(/-/, $hours);

		$hours = ($d * 24) + $h;
	    }
	    
	    $cpu += ($hours * 3600);
	}
	
	return($cpu);
    }

    sub process_name {
	# Adjust name according to flags

	my($name, $flags) = @_;
	
	# determine whether if we're looking for an interpreted program 
	# or a program with args
	my @cmd = split(/\s+/, $name);
	my $psname = "";
	
	if($flags eq 'ia' || $flags eq 'ai') {
	    shift @cmd; # ditch interpreter
	    $psname = join(' ', @cmd);
	}
	elsif($flags eq 'a') {
	    $psname = join(' ', @cmd);
	}
	elsif($flags eq 'i') {
	    shift @cmd; # ditch interpreter
	    $psname = shift @cmd;
	}
	else {
	    $psname = shift @cmd;
	}

	return($psname);
    }
}

=over 4

=item PerformExtraction (EXTHASH, VALUE)

Subroutine to compare a value to a relationship.

  Arguments
  =========
  RELHASH: A reference to an extraction hash.
  VALUE: Value to extract from.

  Returns
  =======
  The extracted value.

=back

=cut

sub PerformExtraction {
    my $self = shift;
    my $e = shift;
    my $v = shift;
    my $ret = "";

    my $op = $e->{'ext'};

    if ($op eq 'column') {
	my (@tokens) = split(/[ \t]/, $v);

	if(defined($tokens[$e->{'x'}])) {
	    $ret = $tokens[$e->{'x'}];
	}
    } elsif ($op eq 'substr') {
	$ret = substr($v, $e->{'x'}, $e->{'y'});
    }

    return($ret);
}

=over 4

=item RelationCompare (RELHASH, VALUE, LABEL)

Subroutine to compare a value to a relationship.

  Arguments
  =========
  RELHASH: A reference to a relationship hash.
  VALUE: Value to compare.
  LABEL: Optional label for value, default is 'Value'.

  Returns
  =======
  A list consisting of the boolean value of the relationship and a
  string describing the relationship.

=back

=cut

sub RelationCompare {
    my $self = shift;
    my $r = shift;
    my $v = shift;
    my $l = shift || 'Value';
    
    my $op = $r->{'rel'};
    
    if ($op eq 'gt') {
	if ( ($v =~ /[-?]\D/ && $v gt $r->{'x'}) ||
	     ($v !~ /[-?]\D/ && $v > $r->{'x'}) ) {
	    return(1, "$l '$v' is greater than '".$r->{'x'}."'");
	} else {
	    return(0, "$l '$v' is not greater than '".$r->{'x'}."'");
	}
    } elsif ($op eq 'lt') {
	if ( ($v =~ /[-?]\D/ && $v lt $r->{'x'}) ||
	     ($v !~ /[-?]\D/ && $v < $r->{'x'}) ) {
	    return(1, "$l '$v' is less than '".$r->{'x'}."'");
	} else {
	    return(0, "$l '$v' is not less than '".$r->{'x'}."'");
	}
    } elsif ($op eq 'eq') {
	if ( ($v =~ /[-?]\D/ && $v eq $r->{'x'}) ||
	     ($v !~ /[-?]\D/ && $v == $r->{'x'}) ) {
	    return(1, "$l is '$v'");
	} else {
	    return(0, "$l '$v' is not '".$r->{'x'}."'");
	}
    } elsif ($op eq 'ne') {
	if ( ($v =~ /[-?]\D/ && $v ne $r->{'x'}) ||
	     ($v !~ /[-?]\D/ && $v != $r->{'x'}) ) {
	    return(1, "$l '$v' is not '".$r->{'x'}."'");
	} else {
	    return(0, "$l is '$v'");
	}
    } elsif ($op eq 'bt') {
	if ( ($v =~ /[-?]\D/ && ($v ge $r->{'x'} && $v le $r->{'y'})) ||
	     ($v !~ /[-?]\D/ && ($v >= $r->{'x'} && $v <= $r->{'y'})) ) {
	    return(1, "$l '$v' is between '".$r->{'x'}."' and '".$r->{'y'}."'");
	} else {
	    return(0, "$l '$v' is not between '".$r->{'x'}."' and '".$r->{'y'}."'");
	}
    } elsif ($op eq 'nb') {
	if ( ($v =~ /[-?]\D/ && ($v lt $r->{'x'} || $v gt $r->{'y'})) ||
	     ($v !~ /[-?]\D/ && ($v < $r->{'x'} || $v > $r->{'y'})) ) {
	    return(1, "$l '$v' is not between '".$r->{'x'}."' and '".$r->{'y'}."'");
	} else {
	    return(0, "$l '$v' is between '".$r->{'x'}."' and '".$r->{'y'}."'");
	}
    } elsif ($op eq 'reg') {
	if ( $v =~ $r->{'s'} ) {
	    return(1, "$l '$v' matches '".$r->{'s'}."'");
	} else {
	    return(0, "$l '$v' does not match '".$r->{'s'}."'");
	}
    } elsif ($op eq 'regv') {
	if ( $v !~ $r->{'s'} ) {
	    return(1, "$l '$v' does not match '".$r->{'s'}."'");
	} else {
	    return(0, "$l '$v' matches '".$r->{'s'}."'");
	}
    } elsif ($op eq 'tot' || $op eq 'tnt') {
	# Time comparisons

	my $now = time();

	if ($op eq 'tot') {
	    if ($v < ($now - $r->{'x'})) {
		return(1, "$l '$v' is older than ".$r->{'x'}." seconds");
	    } else {
		return(0, "$l '$v' is not older than ".$r->{'x'}." seconds");
	    }
	} elsif ($op eq 'tnt') {
	    if ($v > ($now - $r->{'x'})) {
		return(1, "$l '$v' is newer than ".$r->{'x'}." seconds");
	    } else {
		return(0, "$l '$v' is not newer than ".$r->{'x'}." seconds");
	    }
	}
    }
}

=over 4

=item ShouldBeMountedFilesystems (TYPE)

Subroutine to determine filesystems that should be mounted as listed
in system files, like /etc/vfstab or /etc/fstab.  If an error is
encountered, C<ExitError()> will be called.

  Arguments: 'local', 'remote', or 'all'.
  Returns: List of mount points.  Routine will call ExitError if it
  encounters an error.

=back

=cut

sub ShouldBeMountedFilesystems {
    my $self = shift;
    my $type = shift;
    my @should;
    my %seen;

    my %allowed_types = ( 'all' => 1,
			  'local' => 1,
			  'remote' => 1,
			 );

    unless(defined $type && exists $allowed_types{$type}) {
	$self->ExitError(MODEXEC_MISCONFIG, 0,
			 'Bad argument to ShouldBeMountedFileSystems()');
    } 

    # non-local filesystem types
    my %non_local_fs = ( NFS => 1, nfs => 1, afs => 1, smbfs => 1, 
			 autofs => 1, auto => 1, iso9660 => 1, pcfs => 1 );

    # not really filesystems
    my %ignore_fs = ( devpts => 1, swap => 1, ignore => 1, proc => 1,
		      tmpfs => 1, usbfs => 1, ramfs => 1, fd => 1,
		      sysfs => 1 );

    # Look for /etc/vfstab or /etc/fstab.  If neither exist, exit error.
    if(-e '/etc/vfstab') {
	open(IN, '/etc/vfstab') ||
	    $self->ExitError(MODEXEC_MISCONFIG, 0, 'Unable to read /etc/vfstab');

	while(<IN>) {
	    chomp;
	    s/#.*$//;

	    my($dev, $rdev, $mount, $fstype, $fspass, $boot, $options) =
		split;

	    # Not an ignorable FS
	    if($fstype && $mount && !exists $ignore_fs{$fstype}) {
		if(defined($seen{$mount})) {
		    next;
		}
		if(($type eq 'local' && !exists $non_local_fs{$fstype}) ||
		   ($type eq 'remote' && exists $non_local_fs{$fstype}) ||
		   $type eq 'all') {
		    push @should, $mount;
		    $seen{$mount} = 1;
		}
	    }
	}

	close(IN);
    } elsif(-e '/etc/fstab') {
	open(IN, '/etc/fstab') ||
	    $self->ExitError(MODEXEC_MISCONFIG, 0, 'Unable to read /etc/fstab');

	while(<IN>) {
	    chomp;
	    s/#.*$//;

	    my ($dev, $mount, $fstype, $options, $freq, $pass) = split;

	    # Not an ignorable FS and should be automatically mounted at boot...
	    if($fstype && $mount && !exists $ignore_fs{$fstype}) {
		if(defined $options && $options =~ /\bnoauto\b/) {
		    next;
		}
		if(defined($seen{$mount})) {
		    next;
		}
		if(($type eq 'local' && !exists $non_local_fs{$fstype}) ||
		   ($type eq 'remote' && exists $non_local_fs{$fstype}) ||
		   $type eq 'all') {
		    push(@should, $mount);
		    $seen{$mount} = 1;
		}
	    }
	}
	
	close(IN);
    } else {
	$self->ExitError(MODEXEC_MISCONFIG, 0,
		      '/etc/vfstab or /etc/fstab not found');
    }

    return @should;
}

=over 4

=item Which (EXECUTABLE)

Subroutine to find an executable in C<$PATH>.  This is a little more
efficient than forking which.

  Arguments: The name of an executable to find.
  Returns: A path to an executable if found, or undef otherwise.

=back

=cut

sub Which {
    my $self = shift;

    my $exec = shift;

    my @p = split(':', $ENV{"PATH"});

    foreach my $p (@p) {
	my $x = $p.'/'.$exec;
	
	if(-x $x) {
	    return $x;
	}
    }

    undef;
}

=head1 BUGS

No known bugs.

=head1 AUTHORS

Matt Selsky E<lt>F<selsky@columbia.edu>E<gt> and
Benjamin Oshrin E<lt>F<benno@columbia.edu>E<gt>

=head1 COPYRIGHT

Copyright (c) 2002 - 2005
The Trustees of Columbia University in the City of New York
Academic Information Systems

License restrictions apply, see F<doc/license.html> for details.

=cut

1;
