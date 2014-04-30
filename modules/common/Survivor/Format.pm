# Survivor::Fix.pm
#
# Version: $Revision: 0.2 $
# Author: Benjamin Oshrin
# Date: $Date: 2003/10/06 22:28:15 $
#
# Copyright (c) 2003
# The Trustees of Columbia University in the City of New York
# Academic Information Systems
# 
# License restrictions apply, see doc/license.html for details.

package Survivor::Format;

use strict;

use vars qw( @ISA $VERSION @EXPORT %opt $alerttop );
use Exporter;
use Survivor;
use Survivor::XML;
@ISA = qw( Exporter Survivor );

@EXPORT = @Survivor::EXPORT;

=head1 NAME

Survivor::Format contains utilities for Survivor Perl Format modules.

=head1 SYNOPSIS

See src/modules/format/test/test for sample implementations.

=cut

=head1 CONSTRUCTOR

=over 4

=item new ()

Creates a C<Survivor::Format>, which is a reference to a newly created
Survivor::Format object. C<new()> takes the following arguments:

  Arguments
  =========
  None.

  Returns
  =======
  A reference to a newly created Survivor::Format object.

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

Subroutine to obtain the data stored via C<ParseAlert()>.  For all
data except "RecipientSet" and "Host", a string will be returned
containing the text for NAME.  For "RecipientSet", an array of named
hashes will be returned, each of which contains the following:

  name: The module name.
  addresses: The list of addresses for that module.
  calllists: The set of calllists used to generate addresses.

For "Host", an array of hostnames will be returned.
    
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

    if($name eq "RecipientSet")
    {
	my @ret = ();

	my $maref = $alerttop->{"children"}->{"SurvivorAlertData"}[0]->{"children"}->{"RecipientSet"}[0]->{"children"}->{"Module"};

	foreach my $m (@{$maref})
	{
	    # There could be more than one module in a Recipient set,
	    # so iterate through them all and store the data in ret.

	    my %newmod = ("name" => $m->{"name"},
			  "addresses" => [],
			  "calllists" => []);

	    foreach my $a (@{$m->{"children"}->{"Address"}})
	    {
		push(@{$newmod{"addresses"}}, $a->{"text"});
	    }	    

	    foreach my $c (@{$m->{"children"}->{"CallList"}})
	    {
		push(@{$newmod{"calllists"}}, $c->{"text"});
	    }	    

	    push(@ret, \%newmod);
	}
	
	return \@ret;
    }
    elsif($name eq "Host")
    {
	my @ret = ();

	my $haref = $alerttop->{"children"}->{"SurvivorAlertData"}[0]->{"children"}->{"Host"};

	foreach my $h (@{$haref})
	{
	    push(@ret, $h->{"text"});
	}

	return \@ret;
    }
    else
    {
	my $aref = $alerttop->{"children"}->{"SurvivorAlertData"}[0]->{"children"}->{$name};

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

=item GenerateFormattedAlert(ADDRESS, SUBJECT, MESSAGE, REPLYOK)

Subroutine to generate XML on stdout in conformance with the
specification, which is available in doc/spec-fmm.html.

  Arguments
  =========
  ADDRESS: A reference to an array of addresses.
  SUBJECT: A string containing the subject (brief message).
  MESSAGE: A string containing the full message.
  REPLYOK: "yes" or "no" to indicate if two way replies are permitted.

  Returns
  =======
  Nothing.
    
=back

=cut

sub GenerateFormattedAlert {
    my $self = shift;
    my ($aref, $subject, $message, $replyok) = (@_);

    my %xmltree = ("name" => "xml",
		   "text" => "",
		   "attrs" => {},
		   "children" => {},
		   "parent" => undef);

    my %sfadata = ("name" => "SurvivorFormattedAlertData",
		   "text" => "",
		   "attrs" => {"Version" => "2.0",
			       "ReplyOK" => $replyok},
		   "children" => {},
		   "childorder" => ["Address", "Subject", "Message"],
		   "parent" => \%xmltree);
    $xmltree{"children"}{"SurvivorFormattedAlertData"} = [\%sfadata];

    foreach my $a (@{$aref})
    {
	my %adata = ("name" => "Address",
		     "text" => $a,
		     "attrs" => {},
		     "children" => {},
		     "parent" => \%sfadata);
	push(@{$sfadata{"children"}{"Address"}}, \%adata);
    }

    my %sdata = ("name" => "Subject",
		 "text" => $subject,
		 "attrs" => {},
		 "children" => {},
		 "parent" => \%sfadata);
    $sfadata{"children"}{"Subject"} = [\%sdata];

    my %mdata = ("name" => "Message",
		 "text" => $message,
		 "attrs" => {},
		 "children" => {},
		 "parent" => \%sfadata);
    $sfadata{"children"}{"Message"} = [\%mdata];

    my $sx = new Survivor::XML();

    $sx->GenerateStdout(\%xmltree);
}
    
=over 4

=item ParseAlert()

Subroutine to parse stdin in conformance with the specification,
which is available in doc/spec-fmm.html.

C<Data()> will have the values of the provided data.

  Arguments
  =========
  None.

  Returns
  =======
  MODEXEC_OK if successful, MODEXEC_PROBLEM on error.

=back

=cut

sub ParseAlert {
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
    return($alerttop->{"children"}->{"SurvivorAlertData"}[0]->{"attrs"}{"ReplyOK"});
}
