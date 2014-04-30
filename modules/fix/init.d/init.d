#!/usr/bin/perl -w

# init.d fix module
#
# Version: $Revision: 0.6 $
# Author: Benjamin Oshrin and Matt Selsky
# Date: $Date: 2004/05/03 22:18:24 $
#
# Copyright (c) 2002 - 2003
# The Trustees of Columbia University in the City of New York
# Academic Information Systems
# 
# License restrictions apply, see doc/license.html for details.

use strict;
use FindBin;
use lib "$FindBin::Bin/../common";
use Survivor::Fix;

use IO::Select;

# argformat describes the arguments we expect.  GetOpt will then use
# this to handle all the argument parsing and validation for us.
# The key is the name of the argument, value is a string describing the
# argument, using the bold faced types and modifiers described in
# doc/cm-args.html.

my %argformat = ("service" => "string list",
		 "order"   => "optional string one(fifo,filo) default(fifo)",
		 "stop"    => "optional boolean default(1)");

# Create a new Fix object.

my $sf = new Survivor::Fix();

# GetOpt will exit on error, so we need not check for return.

$sf->GetOpt(\%argformat);

# Cycle through the list of services, stopping them if requested,
# then starting them in fifo or filo order.  We can't rely on the
# exit code of the startup scripts to indicate success or failure,
# so we just assume success if every requested script is executable.

my $r = MODEXEC_OK;
my $err = "";
my $ok = "";

my @sarray = $sf->Arg("service");
my @filoarray = ();

foreach my $svc (@sarray) {
    if($sf->Arg("stop") == 1) {
	run_script($svc, "stop");
    }
    
    push(@filoarray, $svc);
}

# Any failure prevents us from starting things.  We might try service
# correspondence (if foo stops, then foo can be started), but safer
# not to.

if($r == MODEXEC_OK) {
    if($sf->Arg("order") eq "filo") {
	while(my $svc = pop(@filoarray)) {
	    run_script($svc, "start");
	}
    } else {
	foreach my $svc (@sarray) {
	    run_script($svc, "start");
	}
    }
}

$err .= $ok;
chop($err);

# Result will generate the required output and exit with the required
# value.

$sf->Result($r, $err);

# run_script handles all the yuckiness.

sub run_script {
    my($svc, @cmd) = @_;

    my $exec = "/etc/init.d/$svc";

    if(-x $exec) {
	my $pid = open(CHILD, "-|");

	if(defined $pid) {
	    if($pid) {
		# Parent

		# Wait for the child to exit, then read the output

		wait();
		
		# We don't know how much output will be produced, so
		# we select over the child input until no more data
		# is ready.

		my $readset = new IO::Select();
		$readset->add(fileno(CHILD));

		my $done = 0;

		while(!$done) {		
		    # Wait up to 5 seconds to read more data.  It might make
		    # sense to make this customizable.

		    my @ready = $readset->can_read(5);
		    
		    if(scalar(@ready) > 0) {
			# Use sysread, not buffered I/O. It's OK that we
			# read a fixed amount, since we'll loop and read
			# some more when select returns ready.
			my $inbuf;
			my $rlen = sysread(CHILD, $inbuf, 80);

			if($rlen == 0) {
			    # If we didn't read anything, assume there is
			    # nothing left to read and break the loop.
			    $done ++;
			} else {
			    # Convert new lines and append
			    $inbuf =~ s/\n/\/\//g;
			    $ok .= $inbuf;
			}
		    } else {
			$done++;
		    }
		}
		
		close(CHILD);

		# Chop off last linefeed indicator and append separator
		$ok =~ s/\/\/$//;
		$ok .= ",";
	    } else {
		# Child

		# Redirect stdin and stderr so as not to hold open
		# sr filehandles, causing it to hang.  stdout is piped
		# to the parent, so don't close it.

		open(STDIN, "/dev/null");
		open(STDERR, "> /dev/null");
		
		exec($exec, @cmd);
	    }
	} else {
	    $err .= "fork failed,";
	    $r = MODEXEC_PROBLEM;
	}
    } else {
	$err .= "$exec is not executable,";
	$r = MODEXEC_PROBLEM;
    }
}
