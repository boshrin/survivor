# Survivor::WebAuth.pm
#
# Version: $Revision: 0.1 $
# Author: Benjamin Oshrin
# Date: $Date: 2004/03/02 17:40:58 $
#
# Copyright (c) 2003 - 2004
# The Trustees of Columbia University in the City of New York
# Academic Information Systems
# 
# License restrictions apply, see doc/license.html for details.

package Survivor::WebAuth;

use strict;

use vars qw( @ISA $VERSION @EXPORT %opt $requesttop );
use Exporter;
use Survivor;
use Survivor::XML;
@ISA = qw( Exporter Survivor );

@EXPORT = @Survivor::EXPORT;

=head1 NAME

Survivor::WebAuth contains utilities for Survivor Perl Web Authentication
modules.

=head1 SYNOPSIS

See src/modules/webauth for sample implementations.

=cut

=head1 CONSTRUCTOR

=over 4

=item new ()

Creates a C<Survivor::WebAuth>, which is a reference to a newly created
Survivor::WebAuth object. C<new()> takes the following arguments:

  Arguments
  =========
  None.

  Returns
  =======
  A reference to a newly created Survivor::WebAuth object.

=back

=cut

sub new {
    my $class = shift;
    my $self = {};
    bless $self, $class;

    return $self;
}

=over 4

=item GenerateWebAuthResult(AUTHOK, USERNAME, MEMBEROF, ERROR, DEFERRAL,
			    SKIPFLAGS)

Subroutine to generate XML on stdout in conformance with the
specification, which is available in doc/spec-wam.html.

  Arguments
  =========
  AUTHOK: "yes", "deferred", or "no"
  USERNAME: If AUTHOK is "yes", the authenticated user
  MEMBEROF: If AUTHOK is "yes", a reference to an array of USERNAME's groups
            (Use undef if there are no groups)
  ERROR: If AUTHTOK is "no", an error.
  DEFERRAL: If AUTHOK is "deferred", data to be transmitted to the client.
  SKIPFLAGS: A reference to an array of flags to be skipped when sw
             generates the reply document.

=back

=cut

sub GenerateWebAuthResult {
    my $self = shift;
    my ($authok, $username, $mref, $err, $deftext, $sfref) = (@_);

    my %xmltree = ("name" => "xml",
		   "text" => "",
		   "attrs" => {},
		   "children" => {},
		   "parent" => undef);

    my %swardata = ("name" => "SurvivorWebAuthResult",
		    "text" => "",
		    "attrs" => {"Version" => "1.0",
				"AuthOK" => $authok},
		    "children" => {},
		    "childorder" => ["Error", "Username", "MemberOf",
				     "Deferral", "SkipFlag"],
		    "parent" => \%xmltree);
    $xmltree{"children"}{"SurvivorWebAuthResult"} = [\%swardata];

    if($authok eq "yes")
    {
	my %udata = ("name" => "Username",
		     "text" => $username,
		     "attrs" => {},
		     "children" => {},
		     "parent" => \%swardata);
	$swardata{"children"}{"Username"} = [\%udata];

	if(defined $mref)
	{
	    foreach my $m (@{$mref})
	    {
		my %mdata = ("name" => "MemberOf",
			     "text" => $m,
			     "attrs" => {},
			     "children" => {},
			     "parent" => \%swardata);
		push(@{$swardata{"children"}{"MemberOf"}}, \%mdata);
	    }
	}
    }
    elsif($authok eq "deferred")
    {
	my %ddata = ("name" => "Deferral",
		     "text" => $deftext,
		     "attrs" => {},
		     "children" => {},
		     "parent" => \%swardata);
	$swardata{"children"}{"Deferral"} = [\%ddata];
    }
    else
    {
	my %edata = ("name" => "Error",
		     "text" => $err,
		     "attrs" => {},
		     "children" => {},
		     "parent" => \%swardata);
	$swardata{"children"}{"Error"} = [\%edata];
    }

    if(defined $sfref)
    {
	foreach my $sf (@{$sfref})
	{
	    my %sfdata = ("name" => "SkipFlag",
			  "text" => $sf,
			  "attrs" => {},
			  "children" => {},
			  "parent" => \%swardata);
	    push(@{$swardata{"children"}{"SkipFlag"}}, \%sfdata);
	}
    }

    my $sx = new Survivor::XML();

    $sx->GenerateStdout(\%xmltree);
}

=over 4

=item Option(NAME)

Obtain the value of the option NAME if set.

  Arguments
  =========
  NAME: The option to obtain.

  Returns
  =======
  A string containing the value, or undef if not found.

=back

=cut

sub Option() {
    my $self = shift;
    my($name) = (@_);

    my $oref = $requesttop->{"children"}->{"SurvivorWebAuthRequest"}[0]->{"children"}->{"ModuleOption"};

    foreach my $o (@{$oref})
    {
	if(defined $o->{"attrs"}{"OptionName"} &&
	   $o->{"attrs"}{"OptionName"} eq $name)
	{
	    return($o->{"children"}->{"OptionValue"}[0]->{"text"});
	}
    }

    # Return undef if nothing was found, to distinguish from an empty string
    return(undef);
}
    
=over 4

=item ParseWebAuthRequest(FORMAT)

Subroutine to parse stdin in conformance with the specification,
which is available in doc/spec-wam.html.

  Arguments
  =========
  FORMAT: A reference to an argument format hash.

  Returns
  =======
  MODEXEC_OK if successful.  On error, the subroutine will generate
  a SurvivorWebAuthResult document on stdout with an appropriate
  error and will exit.

=back

=cut

sub ParseWebAuthRequest {
    my $self = shift;
    my ($f) = @_;
    my $err = "";

    my $sx = new Survivor::XML();

    $requesttop = $sx->ParseStdin();

    if($requesttop) {
	# Take any ModuleOptions and parse them as named arguments.
	# In order to do this, extract the provided values and store
	# them in an array, then call ParseNamedArgs

	my @named = ();

	my $oref = $requesttop->{"children"}->{"SurvivorWebAuthRequest"}[0]->{"children"}->{"ModuleOption"};
	
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
	
	$err = $self->ParseNamedArgs($f, "return", @named);

	if($err eq "") {
	    return MODEXEC_OK;
	}
    } else {
	$err = "Valid SurvivorWebAuthRequest not provided";
    }

    # Generate an error document and exit
    
    my %xmltree = ("name" => "xml",
		   "text" => "",
		   "attrs" => {},
		   "children" => {},
		   "parent" => undef);

    my %swardata = ("name" => "SurvivorWebAuthResult",
		   "text" => "",
		   "attrs" => {"Version" => "1.0",
			       "AuthOK" => "no"},
		   "children" => {},
		   "childorder" => ["Error"],
		   "parent" => \%xmltree);
    $xmltree{"children"}{"SurvivorWebAuthResult"} = [\%swardata];

    my %edata = ("name" => "Error",
		 "text" => $err,
		 "attrs" => {},
		 "children" => {},
		 "parent" => \%swardata);
    $swardata{"children"}{"Error"} = [\%edata];

    $sx->GenerateStdout(\%xmltree);

    exit(MODEXEC_MISCONFIG);
}

=over 4

=item RequestedURI()

Subroutine to determine the URI that was requested when the authentication
request was tripped.  This URI can be used as a reentry point.

  Arguments
  =========
  None.

  Returns
  =======
  The requested URI, or undef if none was provided.
    
=back

=cut

sub RequestedURI() {
    return $requesttop->{"children"}->{"SurvivorWebAuthRequest"}[0]->{"children"}->{"RequestedURI"}[0]->{"text"};
}

=over 4

=item SourceIP()

Subroutine to determine the numerical IP address (eg: 127.0.0.1) from
which the transaction appears to have originated.

  Arguments
  =========
  None.

  Returns
  =======
  The source IP, or undef if none was provided.
    
=back

=cut

sub SourceIP() {
    return $requesttop->{"children"}->{"SurvivorWebAuthRequest"}[0]->{"children"}->{"SourceIP"}[0]->{"text"};
}
