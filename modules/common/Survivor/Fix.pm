# Survivor::Fix.pm
#
# Version: $Revision: 0.7 $
# Author: Benjamin Oshrin and Matt Selsky
# Date: $Date: 2005/04/16 21:47:16 $
#
# Copyright (c) 2002 - 2005
# The Trustees of Columbia University in the City of New York
# Academic Information Systems
# 
# License restrictions apply, see doc/license.html for details.

package Survivor::Fix;

use strict;

use vars qw( @ISA $VERSION @EXPORT %opt $timeout );
use Exporter;
use Survivor;
use Survivor::XML;
@ISA = qw( Exporter Survivor );

@EXPORT = @Survivor::EXPORT;

=head1 NAME

Survivor::Fix contains utilities for Survivor Perl Fix modules.

=head1 SYNOPSIS

See src/modules/fix/test/test for sample implementations.

=cut

=head1 CONSTRUCTOR

=over 4

=item new ()

Creates a C<Survivor::Fix>, which is a reference to a newly created
Survivor::Fix object. C<new()> takes the following arguments:

  Arguments
  =========
  None.

  Returns
  =======
  A reference to a newly created Survivor::Fix object.

=back

=cut

sub new {
    my $class = shift;
    my $self = {};
    bless $self, $class;

    return $self;
}

=over 4

=item ExitError (CODE, VALUE, STRING)

Subroutine to produce specification compliant error messages and then
exit.

  Arguments
  =========
  CODE: The exit code.
  VALUE: This value is ignored, and is provided for compatibility with
         Check::ExitError.
  STRING: The descriptive string.

  Returns
  =======
  Nothing.

=back

=cut

sub ExitError {
    my $self = shift;

    my($ex, $sv, $err) = @_;
    
    Result($self, $ex, $err);
}

=over 4

=item GetOpt(FORMAT)

Subroutine to parse arguments in conformance with specification.  This
routine uses the XML Argument Format specification as the format of
the expected arguments provided in the check.cf configuration file.
The definition of this format can be found in doc/spec-xmlarg.html.

C<Arg()> will have the values of the provided arguments.

If a usage error is encountered, an error will be generated and
C<exit(MODEXEC_MISCONFIG)> will be called.

  Arguments
  =========
  FORMAT: A reference to an argument format hash.

  Returns
  =======
  If successful.

=back

=cut

sub GetOpt {
    my $self = shift;
    my ($f) = @_;

    my @named;

    # Read the named arguments via XML on stdin
    my $sx = new Survivor::XML();

    my $requesttop = $sx->ParseStdin();

    if($requesttop) {
	# Extract timeout
	
	$timeout = $requesttop->{"children"}->{"SurvivorFixData"}[0]->{"children"}->{"Timeout"}[0]->{"text"};
	
        # Take any ModuleOptions and parse them as named arguments.
        # In order to do this, extract the provided values and store
        # them in an array, then call ParseNamedArgs

        my @named = ();

        my $oref = $requesttop->{"children"}->{"SurvivorFixData"}[0]->{"children"}->{"ModuleOption"};
        
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
	$self->Result(MODEXEC_MISCONFIG, "Valid SurvivorFixData not provided");
    }
}

=over 4

=item Result (CODE, STRING)

Subroutine to display result document and exit with that result.

  Arguments
  =========
  CODE: The exit code.
  STRING: The descriptive string.

  Returns
  =======
  Nothing.

=back

=cut

sub Result {
    my $self = shift;

    my($ex, $s) = @_;
    
    my %xmltree = ("name" => "xml",
                   "text" => "",
                   "attrs" => {},
                   "children" => {},
                   "parent" => undef);

    my %sfrdata = ("name" => "SurvivorFixResult",
		   "text" => "",
		   "attrs" => {"Version" => "1.0"},
		   "children" => {},
		   "childorder" => ["ReturnCode", "Comment"],
		   "parent" => \%xmltree);
    $xmltree{"children"}{"SurvivorFixResult"} = [\%sfrdata];

    my %frrdata = ("name" => "ReturnCode",
		   "text" => ($ex eq '' ? 0 : $ex),
		   "attrs" => {},
		   "children" => {},
		   "parent" => \%sfrdata);
    $sfrdata{"children"}{"ReturnCode"} = [\%frrdata];
    
    my %frcdata = ("name" => "Comment",
		   "text" => (defined $s ? $s : ''),
		   "attrs" => {},
		   "children" => {},
		   "parent" => \%sfrdata);
    $sfrdata{"children"}{"Comment"} = [\%frcdata];

    my $sx = new Survivor::XML();

    $sx->GenerateStdout(\%xmltree);
    
    # check for non-integer exit values
    if( $ex =~ /^\d+$/ ) { exit($ex) }
    else { exit }
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
