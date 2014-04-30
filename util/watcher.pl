#!/usr/bin/perl

# Meta-monitor, a watcher to watch the watcher.  The script will talk to
# sw and get a SurvivorStatus XML document.

# Version: $Revision: 0.3 $
# Author: Benjamin Oshrin
# Date: $Date: 2005/12/22 04:06:51 $
#
# Copyright (c) 2003 - 2005
# The Trustees of Columbia University in the City of New York
# Academic Information Systems
# 
# License restrictions apply, see doc/license.html for details.

# Usage: ./watcher -p swuri [-i instance] [-t timeout] [-n notify] [-q]

use strict;
use FindBin;
use lib "$FindBin::Bin/../mod/common";
use Survivor::XML;

use URI;
use HTTP::Request::Common;
use LWP::UserAgent;
use Mail::Mailer;

my $sx = new Survivor::XML();

my %opt;
my %flags = ("i" => "arg",
	     "n" => "arg",
	     "p" => "arg",
	     "q" => "bool",
	     "t" => "arg");

my $err = $sx->ParseFlags(\%flags, \%opt);

my $rc = MODEXEC_PROBLEM;
my $quiet = 0;
my $notify = "";

if(defined $opt{'q'})
{
    $quiet = 1;
}

if(defined $opt{'n'})
{
    $notify = $opt{'n'};
}

if($err eq "")
{
    if(defined $opt{'p'})
    {
	# Generate the uri, including an instance if specified
	
	my $uri = $opt{'p'} . "?status=all";

	if(defined $opt{'i'})
	{
	    $uri .= "&i=" . $opt{'i'};
	}

	# Determine the timeout, or use the default

	my $timeout = 30;

	if(defined $opt{'t'})
	{
	    $timeout = $opt{'t'};
	}

	# Try to establish a connection

	my $ua = LWP::UserAgent->new(timeout => $timeout);
	my $response = $ua->get($uri);

	# And look at the response

	if($$response{_rc} == 200)
	{
	    # Pull out the data we're interested in and look at it.
	    
	    my @xmlblock = split(/\n/, $$response{_content});
	    my $xmltop = $sx->ParseBlock(@xmlblock);

	    my $now = time();
	    my $errmsg = "";
	    my $infomsg = "";
	    my $checksched = 0;

	    # The times of the last check and alert cycles
	    
	    my $acycle = $xmltop->{"children"}->{"SurvivorStatus"}[0]->{"children"}->{"SchedulerStatus"}[0]->{"children"}->{"LastAlertCycle"}[0]->{"text"};
	    my $ccycle = $xmltop->{"children"}->{"SurvivorStatus"}[0]->{"children"}->{"SchedulerStatus"}[0]->{"children"}->{"LastCheckCycle"}[0]->{"text"};
	    
	    if($acycle + 600 < $now)
	    {
		$errmsg .= "Alert scheduler has not run in "
		    . (($now - $acycle) / 60) . " minutes.\n";
	    }
	    else
	    {
		$infomsg .= "Alert scheduler is running.\n";
	    }

	    if($ccycle + 600 < $now)
	    {
		$errmsg .= "Check scheduler has not run in "
		    . (($now - $acycle) / 60) . " minutes.\n";
	    }
	    else
	    {
		$infomsg .= "Check scheduler is running.\n";
		$checksched = 1;
	    }
	    
	    # The configuration parse status
	    
	    if($xmltop->{"children"}->{"SurvivorStatus"}[0]->{"children"}->{"ConfigurationStatus"}[0]->{"attrs"}{"ParseOK"} eq "yes")
	    {
		$infomsg .= "Configuration files parse OK.\n";
	    }
	    else
	    {
		$errmsg .= "Configuration file parse failed:\n" . $xmltop->{"children"}->{"SurvivorStatus"}[0]->{"children"}->{"ConfigurationStatus"}[0]->{"children"}->{"ParseError"}[0]->{"text"};
	    }

	    if($checksched)
	    {
		# Looking for stalled checks, but not if the scheduler is off

		foreach my $sc (@{$xmltop->{"children"}->{"SurvivorStatus"}[0]->{"children"}->{"InstanceStatus"}[0]->{"children"}->{"StalledCheck"}}) {
		    $errmsg .= "Stalled check: " . $sc->{"attrs"}{"Service"}
		    . "@" . $sc->{"attrs"}{"Host"} . "\n";
		}
	    }

	    # Dump any information we have

	    if($errmsg ne "")
	    {
		$errmsg .= $infomsg;
		&do_message($errmsg, "error");
	    }
	    else
	    {
		&do_message($infomsg, "info");
		$rc = MODEXEC_OK;
	    }
	}
	else
	{
	    &do_message("Unexpected return code " . $$response{_rc}
			. " received\n", "error");
	}
    }
    else
    {
	&do_message("Missing required argument '-p'\n", "error");
    }
}
else
{
    &do_message("$err\n", "error");
}

exit($rc);

# Subroutine to generate messages to the appropriate location.
#
# Arguments
# =========
# MSG: Message to generate
# MSGTYPE: "info" or "error"

sub do_message {
    my($msg, $msgtype) = (@_);

    if(!$quiet || $msgtype eq "error")
    {
	if($notify eq "")
	{
	    # Just use stdout/stderr

	    if($msgtype eq "info")
	    {
		if(defined $opt{'i'})
		{
		    print "Instance " . $opt{'i'} . " results...\n";
		}

		print $msg;
	    }
	    else
	    {
		if(defined $opt{'i'})
		{
		    printf STDERR "Instance " . $opt{'i'} . " results...";
		}

		printf(STDERR $msg);
	    }
	}
	else
	{
	    # Create a mailer and generate the message

	    my $mailer = new Mail::Mailer;

	    if($mailer->open( { To => $notify,
				Subject => "survivor watcher notification" }))
	    {
		if(defined $opt{'i'})
		{
		    print $mailer "Instance " . $opt{'i'} . " results...\n";
		}

		print $mailer $msg;
		$mailer->close;
	    }
	    else
	    {
		# Fall back

		printf(STDERR "watcher.pl unable to open Mailer");
	    }
	}
    }
}
