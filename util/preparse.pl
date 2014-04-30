#!/usr/bin/perl

# Configuration file preparser (to verify before installing).
# This version only checks that hosts are not defined more than once
# in the host classes.  It might be useful to replace this with a
# compiled version that provides complete checks of all files.
#
# Version: $Revision: 0.6 $
# Author: Benjamin Oshrin
# Date: $Date: 2003/01/29 02:05:46 $

# Usage: preparse.pl <configdir>
# Exit: 0 if successful, non-zero on error

# Copyright (c) 2002 - 2003
# The Trustees of Columbia University in the City of New York
# Academic Information Systems
# 
# License restrictions apply, see doc/license.html for details.

if(scalar(@ARGV) != 1) {
    printf("Usage: $0 <host.cf>\n");
    exit(1);
}

$r = 0;

if(open(HOSTCF, $ARGV[0])) {
    # Tokenize the file.  Compare this to libsrv/config.l for amusement.

    undef(@tokens);
    while(<HOSTCF>) {
	$_ =~ s/#.*$//;       # Toss comments
	$_ =~ s/^[\t ]*//;    # Toss initial spaces
	$_ =~ s/[\t ]*$//;    # Toss final spaces
	push(@tokens, split(/[ \n]+/, $_));
    }
    
    close(HOSTCF);

    undef(@hs);
    $class = "";
    $hosts = 0;

    while($t = shift(@tokens)) {
	if($t eq "class") {
	    $class = shift(@tokens);
	    shift(@tokens);  # Open brace
	} elsif($t eq "hosts") {
	    shift(@tokens);  # Open brace

	    while(($t = shift(@tokens)) && ($t ne "}")) {
		if($t eq "all") {
		    printf("ERROR: $t is a reserved keyword\n");
		    $r++;
		} elsif($hs{$t}) {
		    printf("ERROR: $t already a member of class %s (redefined in class $class)\n",
			   $hs{$t});
		    $r++;
		} else {
		    $hs{$t} = $class;
		}
	    }
	}
    }
} else {
    printf("ERROR: Unable to open $ARGV[0]\n");
    $r = 1;
}

exit($r);
