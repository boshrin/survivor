#!/usr/bin/perl

# exitd sleeps for a random amount of time between the number of
# minutes specified on the command line and then exits.  It is useful
# for testing, but not much else.  This script requires perl 5.004 or
# later for rand() to work correctly enough.
#
# Version: $Revision: 0.2 $
# Author: Benjamin Oshrin
# Date: $Date: 2003/01/29 02:06:06 $

# Usage: exitd.pl <minsleep> <maxsleep>
# Exit: 0 if successful, non-zero on error

# Copyright (c) 2002 - 2003
# The Trustees of Columbia University in the City of New York
# Academic Information Systems
# 
# License restrictions apply, see doc/license.html for details.

if(scalar(@ARGV) != 2) {
    printf("Usage: $0 <minsleep> <maxsleep>\n");
    exit(1);
}

$min = $ARGV[0];
$max = $ARGV[1];

$r = rand();
$smin = (($max - $min) * $r) + $min;
$ssec = $smin * 60;

# Fork into background
$pid = fork();

if($pid == 0) {
    sleep($ssec);
}

exit(0);
