# Survivor::Transmit.pm
#
# Version: $Revision: 0.2 $
# Author: Benjamin Oshrin
# Date: $Date: 2003/10/06 22:27:56 $
#
# Copyright (c) 2003
# The Trustees of Columbia University in the City of New York
# Academic Information Systems
# 
# License restrictions apply, see doc/license.html for details.

package Survivor::Transmit;

use strict;

use vars qw( @ISA $VERSION @EXPORT %opt $alerttop );
use Exporter;
use Survivor;
use Survivor::XML;
@ISA = qw( Exporter Survivor );

@EXPORT = @Survivor::EXPORT;

=head1 NAME

Survivor::Transmit contains utilities for Survivor Perl Transmit modules.

=head1 SYNOPSIS

See src/modules/transmit/test/test for sample implementations.

=cut

=head1 CONSTRUCTOR

=over 4

=item new ()

Creates a C<Survivor::Transmit>, which is a reference to a newly created
Survivor::Transmit object. C<new()> takes the following arguments:

  Arguments
  =========
  None.

  Returns
  =======
  A reference to a newly created Survivor::Transmit object.

=back

=cut

sub new {
    my $class = shift;
    my $self = {};
    bless $self, $class;

    return $self;
}

=over 4

=item Data(NAME)

Subroutine to obtain the data stored via C<ParseFormattedAlert()>.
For all data except "Address", a string will be returned containing
the text for NAME.  For "Address", a reference to an array containing
each address will be returned.

  Arguments
  =========
  NAME: The data to obtain.

  Returns
  =======
  A string containing the data, except as described above.

=back

=cut

sub Data() {
    my $self = shift;
    my($name) = (@_);

    if($name eq "Address")
    {
	my @ret = ();

	my $aaref = $alerttop->{"children"}->{"SurvivorFormattedAlertData"}[0]->{"children"}->{"Address"};

	foreach my $a (@{$aaref})
	{
	    push(@ret, $a->{"text"});
	}

	return \@ret;
    }
    else
    {
	my $aref = $alerttop->{"children"}->{"SurvivorFormattedAlertData"}[0]->{"children"}->{$name};

	if(defined $aref)
	{
	    my $ref = @{$aref}[0];

	    if(defined $ref)
	    {
		return $ref->{"text"};
	    }
	}
    }

    # Return undef if nothing was found, to distinguish from an empty string
    return(undef);
}
    
=over 4

=item ParseFormattedAlert()

Subroutine to parse stdin in conformance with the specification,
which is available in doc/spec-tmm.html.

C<Data()> will have the values of the provided data.

  Arguments
  =========
  None.

  Returns
  =======
  MODEXEC_OK if successful, MODEXEC_PROBLEM on error.

=back

=cut

sub ParseFormattedAlert {
    my $self = shift;

    my $sx = new Survivor::XML();

    $alerttop = $sx->ParseStdin();

    if($alerttop) {
	return MODEXEC_OK;
    } else {
	return MODEXEC_PROBLEM;
    }
}

=over 4

=item ReplyOK()

Subroutine to determine if the "ReplyOK" attribute was set in the
SurvivorAlertData object.

  Arguments
  =========
  None.

  Returns
  =======
  1 if set, 0 otherwise.
    
=back

=cut

sub ReplyOK() {
    return($alerttop->{"children"}->{"SurvivorFormattedAlertData"}[0]->{"attrs"}{"ReplyOK"});
}
