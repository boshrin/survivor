# Survivor::Report.pm
#
# Version: $Revision: 0.2 $
# Author: Benjamin Oshrin
# Date: $Date: 2006/01/23 02:17:44 $
#
# Copyright (c) 2004 - 2006
# The Trustees of Columbia University in the City of New York
# Academic Information Systems
# 
# License restrictions apply, see doc/license.html for details.

package Survivor::Report;

use strict;

use vars qw( @ISA $VERSION @EXPORT %opt $requesttop );
use Exporter;
use Survivor;
use Survivor::XML;
@ISA = qw( Exporter Survivor );

@EXPORT = @Survivor::EXPORT;

=head1 NAME

Survivor::Report contains utilities for Survivor Perl Report modules.

=head1 SYNOPSIS

See src/modules/report for sample implementations.

=cut

=head1 CONSTRUCTOR

=over 4

=item new ()

Creates a C<Survivor::Report>, which is a reference to a newly created
Survivor::Report object. C<new()> takes the following arguments:

  Arguments
  =========
  None.

  Returns
  =======
  A reference to a newly created Survivor::Report object.

=back

=cut

sub new {
    my $class = shift;
    my $self = {};
    bless $self, $class;

    return $self;
}
    
=over 4

=item DataSet()

Subroutine to obtain the provided data set.

  Arguments
  =========
  None.

  Returns
  =======
  An array of hash references, where each hash is of the form
    "host" -> Host name for entries in this hash
    "service" -> Service name for entries in this hash
    "alertdata" -> A reference to an array of hash references of the form
       "time" -> Time of alert
       "alertrc" -> Alert return code
       "checkrc" -> Check return code alert is for
       "recipients" -> Reference to an array of recipient addresses
       "via" -> Module used to transmit alert
    "checkdata" -> A reference to an array of hash references of the form
       "time" -> Time of check
       "checkrc" -> Check return code
       "scalar" -> Scalar value
       "comment" -> Result comment
       "duration" -> Duration, in milliseconds (if available)
    "commanddata" -> A reference to an array of hash references of the form
       "time" -> Time of command execution
       "command" -> Command executed
       "who" -> Who executed it
       "comment" -> Command comment
    "fixdata" -> A reference to an array of hash references of the form
       "time" -> Time of fix execution
       "fixrc" -> Fix return code
       "who" -> Who executed the fix
       "comment" -> Result comment

=back

=cut

sub DataSet {
    my @ret = ();
    
    my $dref = $requesttop->{"children"}->{"SurvivorReportRequest"}[0]->{"children"}->{"DataSet"};

    foreach my $d (@{$dref})
    {
	my %h;

	# Meta data
	
	$h{"host"} = $d->{"attrs"}{"Host"};
	$h{"service"} = $d->{"attrs"}{"Service"};
	
	# Assemble Alert data
	
	my $aref = $d->{"children"}->{"AlertData"};
	my @ad;

	foreach my $a (@{$aref})
	{
	    my %ah;
	    
	    $ah{"alertrc"} = $a->{"children"}->{"AlertRC"}[0]->{"text"};
	    $ah{"checkrc"} = $a->{"children"}->{"CheckRC"}[0]->{"text"};
	    $ah{"time"} = $a->{"children"}->{"Time"}[0]->{"text"};
	    $ah{"via"} = $a->{"children"}->{"Via"}[0]->{"text"};

	    my $rref = $a->{"children"}->{"Recipient"};
	    my @rs;

	    foreach my $r (@{$rref})
	    {
		push(@rs, $r->{"text"});
	    }
	    
	    $ah{"recipients"} = \@rs;

	    push(@ad, \%ah);
	}

	$h{"alertdata"} = \@ad;

	# Assemble Check data

	my $cref = $d->{"children"}->{"CheckData"};
	my @cd;

	foreach my $c (@{$cref})
	{
	    my %ch;
	    
	    $ch{"checkrc"} = $c->{"children"}->{"CheckRC"}[0]->{"text"};
	    $ch{"comment"} = $c->{"children"}->{"Comment"}[0]->{"text"};
	    $ch{"scalar"} = $c->{"children"}->{"Scalar"}[0]->{"text"};
	    $ch{"time"} = $c->{"children"}->{"Time"}[0]->{"text"};
	    if(defined $c->{"children"}->{"Duration"}[0]->{"text"} &&
	       defined $c->{"children"}->{"Duration"}[0]->{"text"} > -1)
	    {
		$ch{"duration"} = $c->{"children"}->{"Duration"}[0]->{"text"};
	    }

	    push(@cd, \%ch);
	}

	$h{"checkdata"} = \@cd;

	# Assemble Command data

	my $mref = $d->{"children"}->{"CommandData"};
	my @md;

	foreach my $m (@{$mref})
	{
	    my %mh;
	    
	    $mh{"command"} = $m->{"children"}->{"Command"}[0]->{"text"};
	    $mh{"comment"} = $m->{"children"}->{"Comment"}[0]->{"text"};
	    $mh{"time"} = $m->{"children"}->{"Time"}[0]->{"text"};
	    $mh{"who"} = $m->{"children"}->{"Who"}[0]->{"text"};

	    push(@md, \%mh);
	}

	$h{"commanddata"} = \@md;

	# Assemble Fix data

	my $fref = $d->{"children"}->{"FixData"};
	my @fd;

	foreach my $f (@{$fref})
	{
	    my %fh;
	    
	    $fh{"comment"} = $f->{"children"}->{"Comment"}[0]->{"text"};
	    $fh{"fixrc"} = $f->{"children"}->{"FixRC"}[0]->{"text"};
	    $fh{"time"} = $f->{"children"}->{"Time"}[0]->{"text"};
	    $fh{"who"} = $f->{"children"}->{"Who"}[0]->{"text"};

	    push(@fd, \%fh);
	}

	$h{"fixdata"} = \@fd;

	# We're done with this set
	
	push(@ret, \%h);
    }

    return(@ret);
}
  
=over 4

=item ParseReportRequest()

Subroutine to parse stdin in conformance with the specification,
which is available in doc/spec-rm.html.

  Arguments
  =========
  None.

  Returns
  =======
  MODEXEC_OK on success or an error string on failure.

=back

=cut

sub ParseReportRequest {
    my $self = shift;

    my $sx = new Survivor::XML();

    $requesttop = $sx->ParseStdin();

    if($requesttop) {
	return(MODEXEC_OK);
    } else {
	return("Valid SurvivorReportRequest not provided");
    }
}
  
=over 4

=item Style()

Subroutine to obtain the requested output style.

  Arguments
  =========
  None.

  Returns
  =======
  The requested output style.

=back

=cut

sub Style {
    return $requesttop->{"children"}->{"SurvivorReportRequest"}[0]->{"children"}->{"Formatting"}[0]->{"attrs"}{"Style"};
}
  
=over 4

=item TmpDir()

Subroutine to obtain the directory for temporary files.

  Arguments
  =========
  None.

  Returns
  =======
  The directory path, or undef if none was provided.

=back

=cut

sub TmpDir {
    return $requesttop->{"children"}->{"SurvivorReportRequest"}[0]->{"children"}->{"Formatting"}[0]->{"children"}->{"TmpDir"}[0]->{"text"};
}

=over 4

=item TmpURIPrefix()

Subroutine to obtain the URI prefix for making temporary files available.

  Arguments
  =========
  None.

  Returns
  =======
  The URI prefix, or undef if none was provided.

=back

=cut

sub TmpURIPrefix {
    return $requesttop->{"children"}->{"SurvivorReportRequest"}[0]->{"children"}->{"Formatting"}[0]->{"children"}->{"TmpURIPrefix"}[0]->{"text"};
}
