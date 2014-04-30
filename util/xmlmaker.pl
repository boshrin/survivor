#!/usr/bin/perl

# Generate XML suitable for passing to a module.  The XML generated
# is based on argv[0] and additional arguments.  Specifics are
# described below.

# Version: $Revision: 0.6 $
# Author: Benjamin Oshrin
# Date: $Date: 2006/01/23 02:41:54 $
#
# Copyright (c) 2002 - 2005
# The Trustees of Columbia University in the City of New York
# Academic Information Systems
# 
# License restrictions apply, see doc/license.html for details.

# Usage:
# (1) ./alertxml [-f fixreturncode] [-F fixsummary] -h hosts
#                [-H helpfile] -i instances -I instance
#                -r returncode -R module:calllists:recipients
#                -s summary -S service -t unixtime -T token
#
# (2) ./authreq -s sourceip -u uri [-o name=value[,name=value[...]]]
#
# (3) ./reportreq [-C|-H] -h histdir -t tmpdir -u uriprefix
#     where histdir contains the history files to be read
#     (eg: INSTDIR/instance/history/host/foo/bar)
#
# (4) ./checkreq -h host[,host,...] [-t timeout]
#     [-o name=value[,name=value[...]]]
#
# (5) ./fixreq [-t timeout] [-o name=value[,name=value[...]]]
#
# (6) ./transportreq -h host[,host,...] -m module [-t timeout] -T modtype
#     [-o name=value[,name=value[...]]] [-r name=value[,name=value[...]]]
#
# (7) ./formatreq -a addr[,addr] -m message [-r replyto] -s subject

use strict;
use FindBin;
use lib "$FindBin::Bin/../modules/common";
use Survivor::XML;

my $sx = new Survivor::XML();

# Everything starts the same
my %xmltree = ("name" => "xml",
	       "text" => "",
	       "attrs" => {},
	       "children" => {},
	       "parent" => undef);

if($0 =~ /alertxml$/)
{
    my %opt;
    my %flags = ("f" => "arg",
		 "F" => "arg",
		 "h" => "arg",
		 "H" => "arg",
		 "i" => "arg",
		 "I" => "arg",
		 "r" => "arg",
		 "R" => "arg",
		 "s" => "arg",
		 "S" => "arg",
		 "t" => "arg",
		 "T" => "arg");

    my $err = $sx->ParseFlags(\%flags, \%opt);

    if($err eq "")
    {
	# Build a structure containing the provided data, starting with
	# the SurvivorAlertData element.  Specify the order of children
	# as per the DTD.

	my %sadata = ("name" => "SurvivorAlertData",
		      "text" => "",
		      "attrs" => {"Version" => "1.1",
			          "ReplyOK" => "no"},
		      "children" => {},
		      "childorder" => ["RecipientSet",
				       "Summary",
				       "HelpFile",
				       "Host",
				       "Instance",
				       "Instances",
				       "FixSummary",
				       "FixReturnCode",
				       "ReturnCode",
				       "Service",
				       "Time",
				       "Token"],
		      "parent" => \%xmltree);
	$xmltree{"children"}{"SurvivorAlertData"} = [\%sadata];

	# Now generate the remaining data in DTD order.

	if(defined $opt{'R'})
	{
	    my($mod,$cls,$addrs) = split(/:/, $opt{'R'});

	    # RecipientSet
	    my %Rdata = ("name" => "RecipientSet",
			 "text" => "",
			 "attrs" => {},
			 "children" => {},
			 "parent" => \%sadata);
	    $sadata{"children"}{"RecipientSet"} = [\%Rdata];

	    # Module subelement
	    my %Rmdata = ("name" => "Module",
			  "text" => "",
			  "attrs" => {"Name" => $mod},
			  "children" => {},
			  "childorder" => ["Address", "CallList"],
			  "parent" => \%Rdata);
	    $Rdata{"children"}{"Module"} = [\%Rmdata];

	    # Address subsubelements
	    foreach my $a (split/,/, $addrs) {
		my %Rmadata = ("name" => "Address",
			       "text" => $a,
			       "attrs" => {},
			       "children" => {},
			       "parent" => \%Rmdata);
		push(@{$Rmdata{"children"}{"Address"}}, \%Rmadata);
	    }
	    
	    # Calllist subsubelement
	    foreach my $c (split/,/, $cls) {
		my %Rmcdata = ("name" => "CallList",
			       "text" => $c,
			       "attrs" => {},
			       "children" => {},
			       "parent" => \%Rmdata);
		push(@{$Rmdata{"children"}{"CallList"}}, \%Rmcdata);
	    }
	}
	else
	{
	    printf(STDERR "Missing required argument '-R'\n");
	}

	if(defined $opt{'s'})
	{
	    # Summary
	    my %sdata = ("name" => "Summary",
			 "text" => $opt{'s'},
			 "attrs" => {},
			 "children" => {},
			 "parent" => \%sadata);
	    $sadata{"children"}{"Summary"} = [\%sdata];
	}
	else
	{
	    printf(STDERR "Missing required argument '-s'\n");
	}
	
	if(defined $opt{'H'})
	{
	    # Helpfile
	    my %Hdata = ("name" => "HelpFile",
			 "text" => $opt{'H'},
			 "attrs" => {},
			 "children" => {},
			 "parent" => \%sadata);
	    $sadata{"children"}{"HelpFile"} = [\%Hdata];
	}
	
	if(defined $opt{'h'})
	{
	    # Hosts

	    foreach my $h (split(/,/, $opt{'h'})) {
		my %hdata = ("name" => "Host",
			     "text" => $h,
			     "attrs" => {},
			     "children" => {},
			     "parent" => \%sadata);
		push(@{$sadata{"children"}{"Host"}}, \%hdata);
	    }
	}
	
	if(defined $opt{'I'})
	{
	    # Instance
	    my %Idata = ("name" => "Instance",
			 "text" => $opt{'I'},
			 "attrs" => {},
			 "children" => {},
			 "parent" => \%sadata);
	    $sadata{"children"}{"Instance"} = [\%Idata];
	}
	else
	{
	    printf(STDERR "Missing required argument '-I'\n");
	}
	
	if(defined $opt{'i'})
	{
	    # Instances
	    my %idata = ("name" => "Instances",
			 "text" => $opt{'i'},
			 "attrs" => {},
			 "children" => {},
			 "parent" => \%sadata);
	    $sadata{"children"}{"Instances"} = [\%idata];
	}
	else
	{
	    printf(STDERR "Missing required argument '-i'\n");
	}
	
	if(defined $opt{'F'})
	{
	    # Fix Summary
	    my %Fdata = ("name" => "FixSummary",
			 "text" => $opt{'F'},
			 "attrs" => {},
			 "children" => {},
			 "parent" => \%sadata);
	    $sadata{"children"}{"FixSummary"} = [\%Fdata];
	}
	
	if(defined $opt{'f'})
	{
	    # Fix Return Code
	    my %fdata = ("name" => "FixReturnCode",
			 "text" => $opt{'f'},
			 "attrs" => {},
			 "children" => {},
			 "parent" => \%sadata);
	    $sadata{"children"}{"FixReturnCode"} = [\%fdata];
	}
	
	if(defined $opt{'r'})
	{
	    # Return Code
	    my %rdata = ("name" => "ReturnCode",
			 "text" => $opt{'r'},
			 "attrs" => {},
			 "children" => {},
			 "parent" => \%sadata);
	    $sadata{"children"}{"ReturnCode"} = [\%rdata];
	}
	else
	{
	    printf(STDERR "Missing required argument '-r'\n");
	}
	
	if(defined $opt{'S'})
	{
	    # Service
	    my %Sdata = ("name" => "Service",
			 "text" => $opt{'S'},
			 "attrs" => {},
			 "children" => {},
			 "parent" => \%sadata);
	    $sadata{"children"}{"Service"} = [\%Sdata];
	}
	else
	{
	    printf(STDERR "Missing required argument '-S'\n");
	}
	
	if(defined $opt{'t'})
	{
	    # Time
	    my %tdata = ("name" => "Time",
			 "text" => $opt{'t'},
			 "attrs" => {},
			 "children" => {},
			 "parent" => \%sadata);
	    $sadata{"children"}{"Time"} = [\%tdata];
	}
	else
	{
	    printf(STDERR "Missing required argument '-t'\n");
	}
	
	if(defined $opt{'T'})
	{
	    # Token
	    my %Tdata = ("name" => "Token",
			 "text" => $opt{'T'},
			 "attrs" => {},
			 "children" => {},
			 "parent" => \%sadata);
	    $sadata{"children"}{"Token"} = [\%Tdata];
	}
	else
	{
	    printf(STDERR "Missing required argument '-T'\n");
	}
    }
    else
    {
	printf(STDERR "$err\n");
    }
}
elsif($0 =~ /authreq$/)
{
    my %opt;
    my %flags = ("o" => "arg",
		 "s" => "arg",
		 "u" => "arg");

    my $err = $sx->ParseFlags(\%flags, \%opt);

    if($err eq "")
    {
	# Build a structure containing the provided data, starting with
	# the SurvivorWebAuthResult element.  Specify the order of
	# children as per the DTD.

	my %srdata = ("name" => "SurvivorWebAuthRequest",
		      "text" => "",
		      "attrs" => {"Version" => "1.0"},
		      "children" => {},
		      "childorder" => ["RequestedURI",
				       "SourceIP",
				       "ModuleOption"],
		      "parent" => \%xmltree);

	$xmltree{"children"}{"SurvivorWebAuthRequest"} = [\%srdata];

	# Now generate the remaining data in DTD order.

	if(defined $opt{'u'})
	{
	    # Requested URI
	    my %udata = ("name" => "RequestedURI",
			 "text" => $opt{'u'},
			 "attrs" => {},
			 "children" => {},
			 "parent" => \%srdata);
	    $srdata{"children"}{"RequestedURI"} = [\%udata];
	}
	else
	{
	    printf(STDERR "Missing required argument '-u'\n");
	}

	if(defined $opt{'s'})
	{
	    # Source IP
	    my %sdata = ("name" => "SourceIP",
			 "text" => $opt{'s'},
			 "attrs" => {},
			 "children" => {},
			 "parent" => \%srdata);
	    $srdata{"children"}{"SourceIP"} = [\%sdata];
	}
	else
	{
	    printf(STDERR "Missing required argument '-s'\n");
	}

	if(defined $opt{'o'})
	{
	    # Options
	    foreach my $op (split(/,/, $opt{'o'}))
	    {
		my($on,$ov) = split(/=/, $op);

		my %odata = ("name" => "ModuleOption",
			     "text" => "",
			     "attrs" => {"OptionName" => $on},
			     "children" => {},
			     "parent" => \%srdata);

		my %ovdata = ("name" => "OptionValue",
			      "text" => $ov,
			      "attrs" => {},
			      "children" => {},
			      "parent" => \%odata);
		
		push(@{$odata{"children"}{"OptionValue"}}, \%ovdata);
		push(@{$srdata{"children"}{"ModuleOption"}}, \%odata);
	    }
	}
    }
    else
    {
	printf(STDERR "$err\n");
    }
}
elsif($0 =~ /checkreq$/)
{
    my %opt;
    my %flags = ("h" => "arg",
		 "o" => "arg",
		 "t" => "arg");

    my $err = $sx->ParseFlags(\%flags, \%opt);

    if($err eq "")
    {
	# Build a structure containing the provided data, starting
	# with the SurvivorCheckData element.  Specify the order of
	# children as per the DTD.

	my %scdata = ("name" => "SurvivorCheckData",
		      "text" => "",
		      "attrs" => {"Version" => "1.0"},
		      "children" => {},
		      "childorder" => ["Host",
				       "Timeout",
				       "ModuleOption"],
		      "parent" => \%xmltree);

	$xmltree{"children"}{"SurvivorCheckData"} = [\%scdata];

	# Now generate the remaining data in DTD order.

	if(defined $opt{'h'})
	{
	    foreach my $h (split(/,/, $opt{'h'})) {
		my %hdata = ("name" => "Host",
			     "text" => $h,
			     "attrs" => {},
			     "children" => {},
			     "parent" => \%scdata);
		push(@{$scdata{"children"}{"Host"}}, \%hdata);
	    }
	}
	else
	{
	    printf(STDERR "Missing required argument '-h'\n");
	}

	if(defined $opt{'t'})
	{
	    # Timeout
	    my %tdata = ("name" => "Timeout",
			 "text" => $opt{'t'},
			 "attrs" => {},
			 "children" => {},
			 "parent" => \%scdata);
	    $scdata{"children"}{"Timeout"} = [\%tdata];
	}

	if(defined $opt{'o'})
	{
	    foreach my $arg (split(/,/, $opt{'o'}))
	    {
		my ($n, $v) = split(/=/, $arg, 2);

		# ModuleOption
		my %modata = ("name" => "ModuleOption",
			      "text" => "",
			      "attrs" => {"OptionName" => $n},
			      "children" => {},
			      "childorder" => ["OptionValue"],
			      "parent" => \%scdata);
		push(@{$scdata{"children"}{"ModuleOption"}}, \%modata);

		# and OptionValue
		my %ovdata = ("name" => "OptionValue",
			      "text" => $v,
			      "attrs" => {},
			      "children" => {},
			      "parent" => \%modata);
		$modata{"children"}{"OptionValue"} = [\%ovdata];
	    }
	}
    }
    else
    {
	printf(STDERR "$err\n");
    }
}
elsif($0 =~ /fixreq$/)
{
    my %opt;
    my %flags = ("o" => "arg",
		 "t" => "arg");

    my $err = $sx->ParseFlags(\%flags, \%opt);

    if($err eq "")
    {
	# Build a structure containing the provided data, starting
	# with the SurvivorFixData element.  Specify the order of
	# children as per the DTD.

	my %sfdata = ("name" => "SurvivorFixData",
		      "text" => "",
		      "attrs" => {"Version" => "1.0"},
		      "children" => {},
		      "childorder" => ["Timeout",
				       "ModuleOption"],
		      "parent" => \%xmltree);

	$xmltree{"children"}{"SurvivorFixData"} = [\%sfdata];

	# Now generate the remaining data in DTD order.

	if(defined $opt{'t'})
	{
	    # Timeout
	    my %tdata = ("name" => "Timeout",
			 "text" => $opt{'t'},
			 "attrs" => {},
			 "children" => {},
			 "parent" => \%sfdata);
	    $sfdata{"children"}{"Timeout"} = [\%tdata];
	}

	if(defined $opt{'o'})
	{
	    foreach my $arg (split(/,/, $opt{'o'}))
	    {
		my ($n, $v) = split(/=/, $arg, 2);

		# ModuleOption
		my %modata = ("name" => "ModuleOption",
			      "text" => "",
			      "attrs" => {"OptionName" => $n},
			      "children" => {},
			      "childorder" => ["OptionValue"],
			      "parent" => \%sfdata);
		push(@{$sfdata{"children"}{"ModuleOption"}}, \%modata);

		# and OptionValue
		my %ovdata = ("name" => "OptionValue",
			      "text" => $v,
			      "attrs" => {},
			      "children" => {},
			      "parent" => \%modata);
		$modata{"children"}{"OptionValue"} = [\%ovdata];
	    }
	}
    }
    else
    {
	printf(STDERR "$err\n");
    }
}
elsif($0 =~ /formatreq$/)
{
    my %opt;
    my %flags = ("a" => "arg",
		 "m" => "arg",
		 "r" => "arg",
		 "s" => "arg");

    my $err = $sx->ParseFlags(\%flags, \%opt);

    if($err eq "")
    {
	# Build a structure containing the provided data, starting
	# with the SurvivorFormattedAlertData element.  Specify the order of
	# children as per the DTD.

	my %sfdata = ("name" => "SurvivorFormattedAlertData",
		      "text" => "",
		      "attrs" => {"Version" => "1.1"},
		      "children" => {},
		      "childorder" => ["Address",
				       "ReplyTo",
				       "Subject",
				       "Message"],
		      "parent" => \%xmltree);

	$xmltree{"children"}{"SurvivorFormattedData"} = [\%sfdata];

	# Now generate the remaining data in DTD order.

	if(defined $opt{'a'})
	{
	    # Addresses
	    foreach my $a (split(/,/, $opt{'a'})) {
		my %adata = ("name" => "Address",
			     "text" => $a,
			     "attrs" => {},
			     "children" => {},
			     "parent" => \%sfdata);
		push(@{$sfdata{"children"}{"Address"}}, \%adata);
	    }
	}
	else
	{
	    printf(STDERR "Missing required argument '-a'\n");
	}

	if(defined $opt{'r'})
	{
	    # ReplyTo
	    my %rdata = ("name" => "ReplyTo",
			 "text" => $opt{'r'},
			 "attrs" => {},
			 "children" => {},
			 "parent" => \%sfdata);
	    $sfdata{"children"}{"ReplyTo"} = [\%rdata];
	}

	if(defined $opt{'s'})
	{
	    # Subject
	    my %sdata = ("name" => "Subject",
			 "text" => $opt{'s'},
			 "attrs" => {},
			 "children" => {},
			 "parent" => \%sfdata);
	    $sfdata{"children"}{"Subject"} = [\%sdata];
	}
	else
	{
	    printf(STDERR "Missing required argument '-s'\n");
	}

	if(defined $opt{'m'})
	{
	    # Message
	    my %mdata = ("name" => "Message",
			 "text" => $opt{'m'},
			 "attrs" => {},
			 "children" => {},
			 "parent" => \%sfdata);
	    $sfdata{"children"}{"Message"} = [\%mdata];
	}
	else
	{
	    printf(STDERR "Missing required argument '-m'\n");
	}
    }
    else
    {
	printf(STDERR "$err\n");
    }
}
elsif($0 =~ /reportreq$/)
{
    my %opt;
    my %flags = ("C" => "bool",
		 "h" => "arg",
		 "H" => "bool",
		 "t" => "arg",
		 "u" => "arg");

    my $err = $sx->ParseFlags(\%flags, \%opt);

    if($err eq "")
    {
	# Build a structure containing the provided data, starting with
	# the SurvivorReportRequest element.  Specify the order of
	# children as per the DTD.

	my %srdata = ("name" => "SurvivorReportRequest",
		      "text" => "",
		      "attrs" => {"Version" => "1.0"},
		      "children" => {},
		      "childorder" => ["Formatting",
				       "DataSet"],
		      "parent" => \%xmltree);

	$xmltree{"children"}{"SurvivorReportRequest"} = [\%srdata];

	# Now for the data

	my %fdata = ("name" => "Formatting",
		     "text" => "",
		     "attrs" => {"Style" => (defined $opt{'H'} ? "html" :
					     (defined $opt{'C'} ? "check" :
					      "text"))},
		     "children" => {},
		     "childorder" => ["TmpDir",
				      "TmpURIPrefix"],
		     "parent" => \%srdata);
	$srdata{"children"}{"Formatting"} = [\%fdata];

	if(defined $opt{'t'})
	{
	    my %tdata = ("name" => "TmpDir",
			 "text" => $opt{'t'},
			 "attrs" => {},
			 "children" => {},
			 "parent" => \%fdata);
	    $fdata{"children"}{"TmpDir"} = [\%tdata];
	}

	if(defined $opt{'u'})
	{
	    my %udata = ("name" => "TmpURIPrefix",
			 "text" => $opt{'u'},
			 "attrs" => {},
			 "children" => {},
			 "parent" => \%fdata);
	    $fdata{"children"}{"TmpURIPrefix"} = [\%udata];
	}

	if(defined $opt{'h'})
	{
	    # Set up metadata

	    my (@pathtokens) = split(/\//, $opt{'h'});
	    my $service = pop(@pathtokens);
	    my $host = pop(@pathtokens);

	    my %ddata = ("name" => "DataSet",
			 "text" => "",
			 "attrs" => {"Host" => $host,
				     "Service" => $service},
			 "children" => {},
			 "childorder" => ["AlertData",
					  "CheckData",
					  "CommandData",
					  "FixData"],
			 "parent" => \%srdata);
	    push(@{$srdata{"children"}{"DataSet"}}, \%ddata);
	    
	    # Look for alert history

	    if(open(ALERTIN, $opt{'h'} . "/alerthistory"))
	    {
		while(<ALERTIN>)
		{
		    chomp();
		    
		    # Pull out and discard the human readable timestamp

		    my ($h, $x) = split(/\//, $_, 2);

		    my($t, $arc, $crc, $r) = split(/:/, $x, 4);
		    my($rx, $rv) = split(/ via /, $r, 2);
		    my(@rs) = split(',', $rx);

		    my %adata = ("name" => "AlertData",
				 "text" => "",
				 "attrs" => {},
				 "children" => {},
				 "childorder" => ["Time",
						 "AlertRC",
						 "CheckRC",
						 "Recipient",
						 "Via"],
				 "parent" => \%ddata);
		    push(@{$ddata{"children"}{"AlertData"}}, \%adata);

		    my %atdata = ("name" => "Time",
				  "text" => $t,
				  "attrs" => {},
				  "children" => {},
				  "parent" => \%adata);
		    $adata{"children"}{"Time"} = [\%atdata];

		    my %aadata = ("name" => "AlertRC",
				  "text" => $arc,
				  "attrs" => {},
				  "children" => {},
				  "parent" => \%adata);
		    $adata{"children"}{"AlertRC"} = [\%aadata];

		    my %acdata = ("name" => "CheckRC",
				  "text" => $crc,
				  "attrs" => {},
				  "children" => {},
				  "parent" => \%adata);
		    $adata{"children"}{"CheckRC"} = [\%acdata];

		    foreach my $r (@rs)
		    {
			my %ardata = ("name" => "Recipient",
				      "text" => $r,
				      "attrs" => {},
				      "children" => {},
				      "parent" => \%adata);
			push(@{$adata{"children"}{"Recipient"}}, \%ardata);
		    }
		    
		    my %avdata = ("name" => "Via",
				  "text" => $rv,
				  "attrs" => {},
				  "children" => {},
				  "parent" => \%adata);
		    $adata{"children"}{"Via"} = [\%avdata];
		}
		
		close(ALERTIN);
	    }
	    
	    # Look for check history

	    if(open(CHECKIN, $opt{'h'} . "/checkhistory"))
	    {
		while(<CHECKIN>)
		{
		    chomp();
		    
		    # Pull out and discard the human readable timestamp

		    my ($h, $x) = split(/\//, $_, 2);

		    my($t, $crc, $sc, $cmt) = split(/:/, $x, 4);

		    my %cdata = ("name" => "CheckData",
				 "text" => "",
				 "attrs" => {},
				 "children" => {},
				 "childorder" => ["Time",
						  "CheckRC",
						  "Scalar",
						  "Comment"],
				 "parent" => \%ddata);
		    push(@{$ddata{"children"}{"CheckData"}}, \%cdata);

		    my %ctdata = ("name" => "Time",
				  "text" => $t,
				  "attrs" => {},
				  "children" => {},
				  "parent" => \%cdata);
		    $cdata{"children"}{"Time"} = [\%ctdata];

		    my %crdata = ("name" => "CheckRC",
				  "text" => $crc,
				  "attrs" => {},
				  "children" => {},
				  "parent" => \%cdata);
		    $cdata{"children"}{"CheckRC"} = [\%crdata];

		    my %csdata = ("name" => "Scalar",
				  "text" => $sc,
				  "attrs" => {},
				  "children" => {},
				  "parent" => \%cdata);
		    $cdata{"children"}{"Scalar"} = [\%csdata];
		    
		    my %ccdata = ("name" => "Comment",
				  "text" => $cmt,
				  "attrs" => {},
				  "children" => {},
				  "parent" => \%cdata);
		    $cdata{"children"}{"Comment"} = [\%ccdata];
		}
		
		close(CHECKIN);
	    }
	    
	    # Look for command history

	    if(open(COMMANDIN, $opt{'h'} . "/commandhistory"))
	    {
		while(<COMMANDIN>)
		{
		    chomp();
		    
		    # Pull out and discard the human readable timestamp

		    my ($h, $x) = split(/\//, $_, 2);

		    my($t, $cmd, $w, $cmt) = split(/:/, $x, 4);

		    my %cdata = ("name" => "CommandData",
				 "text" => "",
				 "attrs" => {},
				 "children" => {},
				 "childorder" => ["Time",
						  "Command",
						  "Who",
						  "Comment"],
				 "parent" => \%ddata);
		    push(@{$ddata{"children"}{"CommandData"}}, \%cdata);

		    my %ctdata = ("name" => "Time",
				  "text" => $t,
				  "attrs" => {},
				  "children" => {},
				  "parent" => \%cdata);
		    $cdata{"children"}{"Time"} = [\%ctdata];

		    my %cmdata = ("name" => "Command",
				  "text" => $cmd,
				  "attrs" => {},
				  "children" => {},
				  "parent" => \%cdata);
		    $cdata{"children"}{"Command"} = [\%cmdata];

		    my %cwdata = ("name" => "Who",
				  "text" => $w,
				  "attrs" => {},
				  "children" => {},
				  "parent" => \%cdata);
		    $cdata{"children"}{"Who"} = [\%cwdata];
		    
		    my %ccdata = ("name" => "Comment",
				  "text" => $cmt,
				  "attrs" => {},
				  "children" => {},
				  "parent" => \%cdata);
		    $cdata{"children"}{"Comment"} = [\%ccdata];
		}
		
		close(COMMANDIN);
	    }
	    
	    # Look for fix history

	    if(open(FIXIN, $opt{'h'} . "/fixhistory"))
	    {
		while(<FIXIN>)
		{
		    chomp();
		    
		    # Pull out and discard the human readable timestamp

		    my ($h, $x) = split(/\//, $_, 2);

		    my($t, $frc, $z, $w, $cmt) = split(/:/, $x, 5);

		    my %fdata = ("name" => "FixData",
				 "text" => "",
				 "attrs" => {},
				 "children" => {},
				 "childorder" => ["Time",
						  "FixRC",
						  "Who",
						  "Comment"],
				 "parent" => \%ddata);
		    push(@{$ddata{"children"}{"FixData"}}, \%fdata);

		    my %ftdata = ("name" => "Time",
				  "text" => $t,
				  "attrs" => {},
				  "children" => {},
				  "parent" => \%fdata);
		    $fdata{"children"}{"Time"} = [\%ftdata];

		    my %frdata = ("name" => "FixRC",
				  "text" => $frc,
				  "attrs" => {},
				  "children" => {},
				  "parent" => \%fdata);
		    $fdata{"children"}{"FixRC"} = [\%frdata];

		    my %fwdata = ("name" => "Who",
				  "text" => $w,
				  "attrs" => {},
				  "children" => {},
				  "parent" => \%fdata);
		    $fdata{"children"}{"Who"} = [\%fwdata];
		    
		    my %fcdata = ("name" => "Comment",
				  "text" => $cmt,
				  "attrs" => {},
				  "children" => {},
				  "parent" => \%fdata);
		    $fdata{"children"}{"Comment"} = [\%fcdata];
		}
		
		close(FIXIN);
	    }
	}
    }
    else
    {
	printf(STDERR "$err\n");
    }
}
elsif($0 =~ /transportreq$/)
{
    my %opt;
    my %flags = ("h" => "arg",
		 "m" => "arg",
		 "o" => "arg",
		 "r" => "arg",
		 "t" => "arg",
		 "T" => "arg");

    my $err = $sx->ParseFlags(\%flags, \%opt);

    if($err eq "")
    {
	# Build a structure containing the provided data, starting
	# with the SurvivorTransportData element.  Specify the order of
	# children as per the DTD.

	my %stdata = ("name" => "SurvivorTransportData",
		      "text" => "",
		      "attrs" => {"Version" => "1.0"},
		      "children" => {},
		      "childorder" => ["Host",
				       "Timeout",
				       "ModuleOption",
				       "RemoteModule"],
		      "parent" => \%xmltree);

	$xmltree{"children"}{"SurvivorTransportData"} = [\%stdata];

	# Now generate the remaining data in DTD order.

	if(defined $opt{'h'})
	{
	    foreach my $h (split(/,/, $opt{'h'})) {
		my %hdata = ("name" => "Host",
			     "text" => $h,
			     "attrs" => {},
			     "children" => {},
			     "parent" => \%stdata);
		push(@{$stdata{"children"}{"Host"}}, \%hdata);
	    }
	}
	else
	{
	    printf(STDERR "Missing required argument '-h'\n");
	}

	if(defined $opt{'t'})
	{
	    # Timeout
	    my %tdata = ("name" => "Timeout",
			 "text" => $opt{'t'},
			 "attrs" => {},
			 "children" => {},
			 "parent" => \%stdata);
	    $stdata{"children"}{"Timeout"} = [\%tdata];
	}

	if(defined $opt{'o'})
	{
	    foreach my $arg (split(/,/, $opt{'o'}))
	    {
		my ($n, $v) = split(/=/, $arg, 2);

		# ModuleOption
		my %modata = ("name" => "ModuleOption",
			      "text" => "",
			      "attrs" => {"OptionName" => $n},
			      "children" => {},
			      "childorder" => ["OptionValue"],
			      "parent" => \%stdata);
		push(@{$stdata{"children"}{"ModuleOption"}}, \%modata);

		# and OptionValue
		my %ovdata = ("name" => "OptionValue",
			      "text" => $v,
			      "attrs" => {},
			      "children" => {},
			      "parent" => \%modata);
		$modata{"children"}{"OptionValue"} = [\%ovdata];
	    }
	}

	if(defined $opt{'m'} && defined $opt{'T'})
	{
	    # RemoteModule
	    my %rmdata = ("name" => "RemoteModule",
			  "text" => "",
			  "attrs" => {},
			  "children" => {},
			  "childorder" => ["Module", "ModType",
					   "ModuleOption"],
			  "parent" => \%stdata);
	    $stdata{"children"}{"RemoteModule"} = [\%rmdata];

	    # Module
	    my %rmmdata = ("name" => "Module",
			   "text" => $opt{'m'},
			   "attrs" => {},
			   "children" => {},
			   "parent" => \%rmdata);
	    $rmdata{"children"}{"Module"} = [\%rmmdata];

	    # Module Type
	    my %rmtdata = ("name" => "ModType",
			   "text" => $opt{'T'},
			   "attrs" => {},
			   "children" => {},
			   "parent" => \%rmdata);
	    $rmdata{"children"}{"ModType"} = [\%rmtdata];

	    if(defined $opt{'r'})
	    {
		foreach my $arg (split(/,/, $opt{'r'}))
		{
		    my ($n, $v) = split(/=/, $arg, 2);
		    
		    # ModuleOption
		    my %modata = ("name" => "ModuleOption",
				  "text" => "",
				  "attrs" => {"OptionName" => $n},
				  "children" => {},
				  "childorder" => ["OptionValue"],
				  "parent" => \%stdata);
		    push(@{$rmdata{"children"}{"ModuleOption"}}, \%modata);
		    
		    # and OptionValue
		    my %ovdata = ("name" => "OptionValue",
				  "text" => $v,
				  "attrs" => {},
				  "children" => {},
				  "parent" => \%modata);
		    $modata{"children"}{"OptionValue"} = [\%ovdata];
		}
	    }
	}
	else
	{
	    printf(STDERR "Missing required argument '-m' or '-T'\n");
	}
    }
    else
    {
	printf(STDERR "$err\n");
    }
}
else
{
    printf(STDERR "Cannot generate XML for $0\n");
}
	
# Generate the tree
	
$sx->GenerateStdout(\%xmltree);
