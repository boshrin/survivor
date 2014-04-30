#!/usr/bin/perl

# Script to convert from v0.9.3 history format to v0.9.4 format.
# The scheduler must not be running while this script is run.
#
# Usage: convert-history.pl <historydir>

# Version: $Revision: 0.2 $
# Author: Benjamin Oshrin
# Date: $Date: 2004/04/02 23:07:07 $

use Time::Local;

%months = ("Jan" => 0, "Feb" => 1, "Mar" => 2, "Apr" => 3, "May" => 4,
	   "Jun" => 5, "Jul" => 6, "Aug" => 7, "Sep" => 8, "Oct" => 9,
	   "Nov" => 10, "Dec" => 11);

$historydir = $ARGV[0] . "/host";

if(opendir(HISTDIR, $historydir)) {
    @hosts = grep(!/^\.\.?/, readdir(HISTDIR));
    closedir(HISTDIR);

    $done = 0;
    
    foreach $h (@hosts) {
	$hostdir = $historydir . "/" . $h;

	printf("Processing $h (%d%% done)\n",
	       (($done * 100) / scalar(@hosts)));

	if(opendir(HOSTDIR, $hostdir)) {
	    @services = grep(!/^\.\.?/, readdir(HOSTDIR));
	    closedir(HOSTDIR);

	    foreach $s (@services) {
		# We look for alerthistory, checkhistory, and fixhistory
		# and convert whatever we find.

		@todo = ("alerthistory", "checkhistory", "fixhistory");

		foreach $t (@todo) {
		    $hfile = $hostdir . "/" . $s . "/" . $t;
		    $newfile = $hfile . ".new";

		    $ok = 0;
		    
		    if(open(INFILE, $hfile)) {
			@statinfo = stat(INFILE);

			if(open(OUTFILE, "> $newfile")) {
			    # Copy and convert the contents from the old file
			    # to the new

			    printf(" %s/%s\n", $s, $t);
			    
			    while(<INFILE>) {
				($x1, $min, $x3, $data) = split(/:/, $_, 4);
				
				($wday, $mon, $day, $hour) =
				    split(/  ?/, $x1, 4);
				($sec, $year) = split(/ /, $x3, 2);

				eval {
				    $etime = timelocal($sec,$min,$hour,
						       $day,$months{$mon},
						       $year);
				};

				if ($@) {
				    printf("WARNING: Skipping poorly formatted line unsuitable for timelocal: $_");
				} else {
				    if($year =~ /\//) {
					# This is already a v0.9.4 record
					printf(OUTFILE "$_");
				    } else {
					printf(OUTFILE
					       "$x1:$min:$x3/$etime:$data");
				    }
				}
			    }

			    close(OUTFILE);

			    # Reset ownership and permissions on new file

			    chown $statinfo[4], $statinfo[5], $newfile;
			    chmod $statinfo[2], $newfile;

			    $ok++;
			}

			close(INFILE);
		    }

		    # Unlink the old file and move the new one into place

		    if($ok) {
			unlink($hfile);
			rename($newfile, $hfile);
		    } else {
			# Don't complain if the file doesn't exist
			if(-e $hfile) {
			    printf("Error converting $hfile\n");
			}
		    }
		}
	    }
	}

	$done++;
    }
}
