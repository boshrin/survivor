# Survivor::XML.pm
#
# Version: $Revision: 0.10 $
# Author: Benjamin Oshrin
# Date: $Date: 2007/02/06 13:16:55 $
#
# Copyright (c) 2003 - 2007
# The Trustees of Columbia University in the City of New York
# Academic Information Systems
# 
# License restrictions apply, see doc/license.html for details.

package Survivor::XML;

use strict;

use vars qw( @ISA $VERSION @EXPORT %opt );
use Exporter;
use Survivor;
@ISA = qw( Exporter Survivor );

@EXPORT = @Survivor::EXPORT;

=head1 NAME

Survivor::XML contains very simple XML handlers designed for use within
the Survivor package.

=head1 SYNOPSIS

These routines should not be called directly.  Instead, other (public)
routines should be called, which will invoke the XML handlers when
necessary.

=cut

=head1 CONSTRUCTOR

=over 4

=item new ()

Creates a C<Survivor::XML>, which is a reference to a newly created
Survivor::XML object. C<new()> takes the following arguments:

  Arguments
  =========
  None.

  Returns
  =======
  A reference to a newly created Survivor::XML object.

=back

=cut

sub new {
    my $class = shift;
    my $self = {};
    bless $self, $class;

    return $self;
}

=over 4

=item ParseFH (FH)

Subroutine to parse an XML document from a file handle.

  Arguments
  =========
  FH: File handle to read from

  Returns
  =======
  A named hash with the following contents:

  name: A string containing the name of the element.
  text: A string containing the text between the element open/close tags.
  attrs: A reference to a named hash consisting of any attributes passed
         in the element open tag.
  children: A reference to a named hash of references to an array.  Each
            reference is indexed by the name of the child element.  Each
            item in the array is a reference to a hash, containing these same
            elements (name, text, attrs, children, parent).
  parent: A reference pointing back to the parent named hash, or undef
          for the top level hash.

=back

=cut

sub ParseFH {
    my $self = shift;
    my ($fh) = (@_);

    my @xmlblock = <$fh>;

    return(ParseBlock($self, @xmlblock));
}

=over 4

=item ParseStdin ()

Subroutine to parse an XML document on stdin.

  Arguments
  =========
  None.

  Returns
  =======
  A named hash with the following contents:

  name: A string containing the name of the element.
  text: A string containing the text between the element open/close tags.
  attrs: A reference to a named hash consisting of any attributes passed
         in the element open tag.
  children: A reference to a named hash of references to an array.  Each
            reference is indexed by the name of the child element.  Each
            item in the array is a reference to a hash, containing these same
            elements (name, text, attrs, children, parent).
  parent: A reference pointing back to the parent named hash, or undef
          for the top level hash.

=back

=cut

sub ParseStdin {
    my $self = shift;

    # We implement a hack similar to the one in sr to avoid blocking
    # in circumstances where we don't get an EOF.  (Solaris 9 sometimes
    # fails to send one on close().)  After we read the first line,
    # we stop reading if more than a second passes between lines.

    my @xmlblock = ();
    my $done = 0;

    # If we block here, that's sort of OK, we'll be terminated
    # eventually by the caller.
    my $r = <>;

    push(@xmlblock, $r);

    while(!$done) {
	eval {
	    undef($r);
	    
	    local $SIG{ALRM} =
		sub { $done++; };
	    
	    alarm 1;
	    $r = <>;
	    alarm 0;

	    if(defined $r)
	    {	    
		# Store what we read
		push(@xmlblock, $r);
	    }
	    else
	    {
		$done++;
	    }
	};
    }
    
    return(ParseBlock($self, @xmlblock));
}

=over 4

=item ParseBlock (XMLBLOCK)

Subroutine to parse an XML document in an array.

  Arguments
  =========
  XMLBLOCK: A list (array) of lines consisting of the XML document.

  Returns
  =======
  A named hash with the following contents:

  name: A string containing the name of the element.
  text: A string containing the text between the element open/close tags.
  attrs: A reference to a named hash consisting of any attributes passed
         in the element open tag.
  children: A reference to a named hash of references to an array.  Each
            reference is indexed by the name of the child element.  Each
            item in the array is a reference to a hash, containing these same
            elements (name, text, attrs, children, parent).
  parent: A reference pointing back to the parent named hash, or undef
          for the top level hash.

=back

=cut

sub ParseBlock {
    my ($self, @xmlblock) = (@_);

    # To track the parse tree
    my %topel;
    my $curel;

    # What is currently being parsed.  These are more informative than
    # necessary.
    my $curtag = "";
    my $curtext = "";
    my $depth = 0;

    # Initialize the top level element, and set the current element
    # to the top element.
    $topel{"name"} = "xml";
    $topel{"text"} = "";
    $topel{"attrs"} = {};
    $topel{"children"} = {};
    $topel{"parent"} = undef;
    $curel = \%topel;
    
    foreach $_ (@xmlblock)
    {
	# For each line, iterate through looking for tags and contents

	my $linedone = 0;
    
	while(!$linedone)
	{
	    if(/([^\<\>]*)(\<[^\>]*\>)(.*)/)
	    {
		my $tag = substr($2, 1, length($2)-2);
		
		# We currently just skip <?blah> and <!blah> style tags,
		# since no DTD requires them (except <?xml>).

		if(!($tag =~ /^[\?\!]/))
		{	    
		    if($curtag ne "")
		    {
			# More text for the current tag
			$curtext .= $1;
		    }

		    if($tag =~ /^[\/]/)
		    {
			# Close tag, store the text, reset, and move one
			# level up.

			# Convert any &lt; or &gt; to < or >, undoing
			# the escaping done in dump_element.

			$curtext =~ s/&lt;/</g;
			$curtext =~ s/&gt;/>/g;
			$curtext =~ s/&amp;/&/g;
		    
			$tag = substr($tag, 1, length($tag)-1);
			$curel->{"text"} = $curtext;
			$curel = $curel->{"parent"};
			$curtag = "";
			$curtext = "";
			$depth--;
		    }
		    else
		    {
			# Open tag, nest one level

			my %newtag;

			# Get the name and any attributes
			my($tagname, @attrs) = split(/[ \t]/, $tag);

			# Initialize a new hash to hold the data
			$newtag{"name"} = $tagname;
			$newtag{"text"} = "";
			$newtag{"attrs"} = {};
			$newtag{"children"} = {};
			$newtag{"parent"} = $curel;

			foreach $a (@attrs)
			{
			    # Split the attributes into name/value pairs
			    my($n,$v) = split(/=/, $a);
			    # Get rid of quotes around v
			    my $v2 = substr($v, 1, length($v)-2);
			    
			    $newtag{"attrs"}{$n} = $v2;
			}

			# Push the new element onto the current elements
			# array of children, then make the new element
			# the current element.
			if(!defined $curel->{"children"}->{$tagname}) {
			    $curel->{"children"}->{$tagname} = [];
			}
			push(@{$curel->{"children"}->{$tagname}}, \%newtag);
			$curel = \%newtag;
			$curtag = $tagname;
			$curtext = "";  # Clobber any text parsed so far
			$depth++;
		    }
		}

		# If the third match is empty, we've finished the line
		if(!defined($3) || $3 eq "") { $linedone++; }
		else { $_ = $3 . "\n"; }
	    }
	    else
	    {
		# More text for the current tag, take the whole line
		$curtext .= $_;
		$linedone++;
	    }
	}
    }
    
    return(\%topel);
}

=over 4

=item Generate(FH, REF)

Subroutine to generate XML.

  Arguments
  =========
  FH: Filehandle to write to.
  REF: A reference to a named hash of the format returned by
       C<ParseStdin()>, except an additional entry "childorder"
       is permitted that contains a reference to an array describing
       the order the child elements are to be printed in.

  Returns
  =======
  Nothing.

=back

=cut

sub Generate {
    my $self = shift;
    my ($fh, $topel) = @_;

    # Make sure we're unbuffered

    my $ofh = select($fh);
    $| = 1;
    select($ofh);
    
    # Start with an xml tag.
    
    printf($fh "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>");

    # If childorder was given, use it, otherwise dump everything
    
    if(defined $topel->{"childorder"})
    {
	foreach my $co (@{$topel->{"childorder"}})
	{
	    if(defined $topel->{"children"}->{$co})
	    {	
		foreach my $tt (@{$topel->{"children"}->{$co}})
		{
		    &dump_element($fh, $tt, 0);
		}
	    }
	}
    }
    else
    {
	foreach my $t (keys %{$topel->{"children"}})
	{
	    foreach my $tt (@{$topel->{"children"}->{$t}})
	    {
		&dump_element($fh, $tt, 0);
	    }
	}
    }

    printf($fh "\n");
}

=over 4

=item GenerateStdout(REF)

Subroutine to generate XML on standard out.

  Arguments
  =========
  REF: A reference to a named hash of the format returned by
       C<ParseStdin()>, except an additional entry "childorder"
       is permitted that contains a reference to an array describing
       the order the child elements are to be printed in.

  Returns
  =======
  Nothing.

=back

=cut

sub GenerateStdout {
    my $self = shift;
    my ($topel) = @_;

    Generate($self, \*STDOUT, $topel);
}

=over 4

=item dump_element(FH, REF, DEPTH)

Subroutine to generate XML for one element.

  Arguments
  =========
  FH: Filehandle to write to.
  REF: A reference to a named hash of the format returned by
       C<ParseStdin()>.
  DEPTH: The current level of nesting.

  Returns
  =======
  Nothing.

=back

=cut

sub dump_element {
    my($fh, $ref, $depth) = (@_);

    # Before we begin, convert any < to &lt; and > to &gt; in $ref->text.
    # We wouldn't have to do this if we parsed the DTD on the other end.
    # See also corresponding decoding in parse_block and equivalent in
    # libparsexml.
    
    # As long as we're here, convert & to &amp;, but before other
    # conversions.

    my $text = $ref->{"text"};
    $text =~ s/&/&amp;/g;
    $text =~ s/\</&lt;/g;
    $text =~ s/\>/&gt;/g;
    
    printf($fh "\n%${depth}s<%s", "", $ref->{"name"});

    foreach my $k (keys %{$ref->{"attrs"}})
    {
	printf($fh " %s=\"%s\"", $k, $ref->{"attrs"}{$k});
    }
    
    printf($fh ">%s", $text);

    # If childorder was given, use it, otherwise dump everything
    
    if(defined $ref->{"childorder"})
    {
	foreach my $co (@{$ref->{"childorder"}})
	{
	    if(defined $ref->{"children"}->{$co})
	    {	
		foreach my $tt (@{$ref->{"children"}->{$co}})
		{
		    &dump_element($fh, $tt, $depth+1);
		}
	    }
	}
    }
    else
    {
	foreach my $t (keys %{$ref->{"children"}})
	{
	    foreach my $tt (@{$ref->{"children"}->{$t}})
	    {
		&dump_element($fh, $tt, $depth+1);
	    }
	}
    }

    printf($fh "</%s>", $ref->{"name"});
}
