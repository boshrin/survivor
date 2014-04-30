#!/usr/bin/perl

# Gather some statistics from the history records.
# This script ignores the locking conventions, it isn't production quality.

use Time::Local;

%months = ( "Jan" => 0,
	    "Feb" => 1,
	    "Mar" => 2,
	    "Apr" => 3,
	    "May" => 4,
	    "Jun" => 5,
	    "Jul" => 6,
	    "Aug" => 7,
	    "Sep" => 8,
	    "Oct" => 9,
	    "Nov" => 10,
	    "Dec" => 11 );

if(scalar(@ARGV) < 1) {
    printf("Usage: $0 <historydir> [<MM> <DD> <YYYY> [<MM> <DD> <YYYY>]]\n");
    exit(1);
}

$alerts = 0;
$ftime = 0;
$ttime = 0;
undef(%alertwho);

if(scalar(@ARGV) > 1) {
    $ftime = timelocal(0,0,0,$ARGV[2],$ARGV[1]-1,$ARGV[3]);

    if(scalar(@ARGV) > 4) {
	$ttime = timelocal(59,59,23,$ARGV[5],$ARGV[4]-1,$ARGV[6]);
    }
}

$historydir = $ARGV[0] . "/host";

if(opendir(HISTDIR, $historydir)) {
    @hosts = grep(!/^\.\.?/, readdir(HISTDIR));
    closedir(HISTDIR);

    foreach $h (@hosts) {
	$hostdir = $historydir . "/" . $h;

	if(opendir(HOSTDIR, $hostdir)) {
	    @services = grep(!/^\.\.?/, readdir(HOSTDIR));
	    closedir(HOSTDIR);

	    foreach $s (@services) {
		$alerthistory = $hostdir . "/" . $s . "/alerthistory";

		if(open(ALERTS, $alerthistory)) {
		    while(<ALERTS>) {
			chop($_);

			($when,$min,$when2,$exit,$scalar,$who) =
			    split(/:/, $_, 6);

			($wday,$mon,$mday,$hour) = split(/ +/, $when, 4);
			($sec, $year) = split(/ /, $when2, 2);

			$time = timelocal($sec,$min,$hour,$mday,
					  $months{$mon},$year);

			if($time > $ftime
			   && ($ttime == 0 || $time <= $ttime)) {
			    $who =~ s/\ via\ .*$//;
			    @whos = split(/,/, $who);
			    
			    foreach $w (@whos) {
				$alertwho{$w}++;
			    }
			    
			    $alerts++;
			}
		    }
		    
		    close(ALERTS);
		}
	    }
	}
    }
}

printf("Alert history recorded $alerts alerts.\n\n");

foreach $k (keys %alertwho) {
    printf("$k => %d\n", $alertwho{$k});
}

exit(0);
